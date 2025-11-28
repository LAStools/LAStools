/*
===============================================================================

  FILE:  proj_loader.cpp

  CONTENTS:

    see corresponding header file

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2024, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the Apache Public License 2.0 published by the Apache Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    see corresponding header file

===============================================================================
*/
#include "lasmessage.hpp"
#include "proj_loader.h"
#include "proj_wrapper.h"

#include <vector>
#include <filesystem>
#include <regex>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <array>
#include <set>

#ifdef min
#undef min
#endif

// Global handle for the dynamically loaded library
PROJ_LIB_HANDLE proj_lib_handle = nullptr;

// Initialise function pointer variables
proj_as_wkt_t proj_as_wkt_ptr = nullptr;
proj_as_proj_string_t proj_as_proj_string_ptr = nullptr;
proj_as_projjson_t proj_as_projjson_ptr = nullptr;
proj_get_source_crs_t proj_get_source_crs_ptr = nullptr;
proj_get_target_crs_t proj_get_target_crs_ptr = nullptr;
proj_destroy_t proj_destroy_ptr = nullptr;
proj_context_create_t proj_context_create_ptr = nullptr;
proj_context_destroy_t proj_context_destroy_ptr = nullptr;
proj_get_id_code_t proj_get_id_code_ptr = nullptr;
proj_get_ellipsoid_t proj_get_ellipsoid_ptr = nullptr;
proj_ellipsoid_get_parameters_t proj_ellipsoid_get_parameters_ptr = nullptr;
proj_crs_get_datum_ensemble_t proj_crs_get_datum_ensemble_ptr = nullptr;
proj_get_name_t proj_get_name_ptr = nullptr;
proj_datum_ensemble_get_member_count_t proj_datum_ensemble_get_member_count_ptr = nullptr;
proj_datum_ensemble_get_accuracy_t proj_datum_ensemble_get_accuracy_ptr = nullptr;
proj_datum_ensemble_get_member_t proj_datum_ensemble_get_member_ptr = nullptr;
proj_crs_get_datum_t proj_crs_get_datum_ptr = nullptr;
proj_crs_get_coordinate_system_t proj_crs_get_coordinate_system_ptr = nullptr;
proj_cs_get_axis_count_t proj_cs_get_axis_count_ptr = nullptr;
proj_cs_get_axis_info_t proj_cs_get_axis_info_ptr = nullptr;
proj_create_t proj_create_ptr = nullptr;
proj_create_argv_t proj_create_argv_ptr = nullptr;
proj_create_crs_to_crs_t proj_create_crs_to_crs_ptr = nullptr;
proj_create_crs_to_crs_from_pj_t proj_create_crs_to_crs_from_pj_ptr = nullptr;
proj_create_from_wkt_t proj_create_from_wkt_ptr = nullptr;
proj_context_errno_t proj_context_errno_ptr = nullptr;
proj_context_errno_string_t proj_context_errno_string_ptr = nullptr;
proj_coord_t proj_coord_ptr = nullptr;
proj_trans_t proj_trans_ptr = nullptr;
proj_get_type_t proj_get_type_ptr = nullptr;
proj_is_crs_t proj_is_crs_ptr = nullptr;
proj_cs_get_type_t proj_cs_get_type_ptr = nullptr;
proj_info_t proj_info_ptr = nullptr;
proj_log_func_t proj_log_func_ptr = nullptr;

/// Function for parsing the version number from the directory name
static std::vector<int> parseVersion(std::string versionStr) {
  std::vector<int> versionNumbers;
  std::regex versionRegex("(\\d+)");
  std::sregex_iterator it(versionStr.begin(), versionStr.end(), versionRegex);
  std::sregex_iterator end;

  while (it != end) {
    versionNumbers.push_back(std::stoi(it->str()));
    ++it;
  }

  return versionNumbers;
}

#if defined(__unix__) || defined(__APPLE__)
/// Returns the path to the proj lib if lib exists, or optionally the path with the latest lib version in the directory
std::string getProjLibraryPath(const std::string &dir, bool withLatestVersion = false) {
  std::regex projRegex(R"(libproj\.so(\.\d+)+$)"); 
  std::vector<std::string> candidates;

  if (!std::filesystem::exists(dir)) return "";

  for (const auto& entry : std::filesystem::directory_iterator(dir)) {
    try {
      std::string file = entry.path().filename().string();
      if (std::regex_match(file, projRegex)) {
        if (withLatestVersion == false) {
          return dir;
        }
        candidates.push_back(entry.path().string());
      }
    } catch (const std::filesystem::filesystem_error&) {
      continue;
    }
  }

  if (candidates.empty()) return "";

  // Sort by latest version (.so.22 / .so.25.2.1 etc.)
  std::sort(candidates.begin(), candidates.end());
  return candidates.back();  // latest version
}
#endif

/// Compare lib version numbers. Return ture if v1 latest version
static bool compareVersions(std::string v1, std::string v2) {
  std::vector<int> version1 = parseVersion(v1);
  std::vector<int> version2 = parseVersion(v2);

  size_t len = std::min(version1.size(), version2.size());
  for (size_t i = 0; i < len; ++i) {
    if (version1[i] > version2[i]) return true;
    if (version1[i] < version2[i]) return false;
  }

  return version1.size() > version2.size();
}

/// Finds the latest PROJ library within the installation path
static std::string findLatestProjLibraryPath(const std::string& path) {
  if (path.empty()) return "";

  // Check if the binPath exists and is a directory
  try {
    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
      return "";
    }
  } catch (const std::filesystem::filesystem_error&) {
  }

  std::filesystem::path binPath(path);
  std::string latestVersionPath;
  std::string latestVersionName;

#ifdef _WIN32
  const char* projLibraryPattern = "proj_";
  const char* fileExtension = ".dll";

  for (const auto& entry : std::filesystem::directory_iterator(binPath)) {
    try {
      std::string fileName = entry.path().filename().string();
      if (fileName.find(projLibraryPattern) != std::string::npos && fileName.find(fileExtension) != std::string::npos) {
        if (latestVersionName.empty() || compareVersions(fileName, latestVersionName)) {
          latestVersionName = fileName;
          latestVersionPath = entry.path().string();
        }
      }
    } catch (const std::filesystem::filesystem_error&) {
      continue;
    }
  }
#else
  // libproj.so always refers to the corresponding version
  //if (std::filesystem::exists(binPath / "libproj.so")) {
  //  latestVersion = (binPath / "libproj.so").string();
  //} else {
  latestVersionPath = getProjLibraryPath(binPath, true);
  //}
#endif
  if (!latestVersionPath.empty()) {
    return latestVersionPath;
  }
  return "";
}

/// Search for the PROJ library within the standard installation directories by recursively traversing them.
static std::string findProjLibInStandartPathsRecursive(std::set<std::filesystem::path>& searchedPaths) {
  const char* homeDir = nullptr;
  std::vector<std::filesystem::path> pathsToCheck;

#ifdef _WIN32
  // Windows-Standardpfade
  pathsToCheck.push_back("C:\\Program Files\\");
  pathsToCheck.push_back("C:\\Program Files (x86)\\");

  const char* userProfile = getenv("USERPROFILE");
  if (userProfile) {
    pathsToCheck.push_back(std::filesystem::path(userProfile) / "AppData/Local/Programs");
  }
#else
  // Linux/macOS-Standardpfade
  const std::string systemLibPaths[] = {
      "/usr/lib/x86_64-linux-gnu/", "/usr/lib/", "/usr/bin/", "/usr/local/lib/", "/usr/local/bin/", "/opt/", "/usr/share/", "/lib/x86_64-linux-gnu/"};
  homeDir = getHomeDirectory();

  for (const auto& p : systemLibPaths) pathsToCheck.push_back(p);
  if (homeDir) pathsToCheck.push_back(std::filesystem::path(homeDir) / ".local/lib");
#endif
  // 1. Recursive fallback
  for (const auto& basePath : pathsToCheck) {
    std::string logEntry = basePath.string() + " (searching this path recursively)";
    searchedPaths.insert(logEntry);
    try {
      for (const auto& entry : std::filesystem::recursive_directory_iterator(basePath)) {
        if (std::filesystem::exists(entry) && entry.is_directory()) {
          // searchedPaths.insert(entry.path());
          const std::string resultPath = findLatestProjLibraryPath(entry.path().string());
          if (!resultPath.empty()) return resultPath;
        }
      }
    } catch (const std::filesystem::filesystem_error&) {
      continue;
    }
  }
  return "";
}

/// Search for the PROJ library within the standard installation directories.
static std::string findProjLibInStandartPaths(std::set<std::filesystem::path>& searchedPaths) {
  const char* homeDir = nullptr;
  std::vector<std::filesystem::path> pathsToCheck;
  std::string latestVersionPath;
  std::string latestVersionName;

#ifdef _WIN32
  // Windows-Standardpfade
  pathsToCheck.push_back("C:\\Program Files\\");
  pathsToCheck.push_back("C:\\Program Files (x86)\\");

  const char* userProfile = getenv("USERPROFILE");
  if (userProfile) {
    pathsToCheck.push_back(std::filesystem::path(userProfile) / "AppData/Local/Programs");
  }
#else
  // Linux/macOS-Standardpfade
  const std::string systemLibPaths[] = {
      "/usr/lib/x86_64-linux-gnu/", 
      "/usr/lib/", 
      "/usr/bin/", 
      "/usr/local/lib/", 
      "/usr/local/bin/", 
      "/opt/", 
      "/usr/share/", 
      "/lib/x86_64-linux-gnu/"};
  homeDir = getHomeDirectory();

  for (const auto& p : systemLibPaths) pathsToCheck.push_back(p);
  if (homeDir) pathsToCheck.push_back(std::filesystem::path(homeDir) / ".local/lib");
#endif

  // 1. Check only top-level dirs
  for (const auto& basePath : pathsToCheck) {
    try {
      if (std::filesystem::exists(basePath) && std::filesystem::is_directory(basePath)) {
        searchedPaths.insert(basePath);
        const std::string resultPath = findLatestProjLibraryPath(basePath.string());

        if (!resultPath.empty()) {
          std::filesystem::path projPath(resultPath);
          std::string fileName = projPath.filename().string();

          if (latestVersionName.empty() || compareVersions(fileName, latestVersionName)) {
            latestVersionName = fileName;
            latestVersionPath = projPath.string();
          }
        }
      }
    } catch (const std::filesystem::filesystem_error&) {
      continue;
    }
  }
  if (!latestVersionPath.empty()) {
    LASMessage(LAS_VERY_VERBOSE, "PROJ library found in path: %s", latestVersionPath.c_str());
    return latestVersionPath;
  }
  return "";
}

/// Checks whether the loaded PROJ version reaches at least minMajor.minMinor.
/// Returns a warning if this version is too old.
static void checkProjVersion() {
  MyPJ_INFO info = my_proj_info();
  
  if (info.version) {
    LASMessage(LAS_VERBOSE, "Loaded PROJ version: %s", info.version);

    if (info.major < 9) {
      LASMessage(LAS_WARNING, "The loaded PROJ version '%s' is older than version 9.0.0. Full functionality cannot be guaranteed with this version.", info.version);
    }
  } else {
    LASMessage(LAS_WARNING, "The loaded PROJ version could not be determined");
  }
}

/// Finds the latest QGIS installation path by first checking the `QGIS_PREFIX_PATH` environment variable.
/// If the environment variable is not set, it searches through default installation directories and selects the latest version based on directory names.
/// appends "bin" to the path, where the proj lib is located
static std::string findLatestQGISInstallationPath(std::set<std::filesystem::path>& searchedPaths) {
  // 1. Check the environment variable QGIS_PREFIX_PATH
  const char* qgisPathEnv = getenv("QGIS_PREFIX_PATH");
  std::string latestVersionPath;
  std::string latestVersionName;

#ifdef _WIN32
  if (qgisPathEnv) {
    try {
      if (std::filesystem::exists(qgisPathEnv)) {
        std::filesystem::path path(qgisPathEnv);
        path /= "bin";  // Append "bin" to the path
        if (std::filesystem::exists(path))  {
          searchedPaths.insert(path);
          LASMessage(LAS_VERY_VERBOSE, "PROJ library used via QGIS installation path: %s", path.string().c_str());
          return path.string();
        } else {
          LASMessage(LAS_WARNING, "QGIS_PREFIX_PATH set but bin folder [%s] not found", path.string().c_str());
        }
      } else {
        LASMessage(LAS_WARNING, "QGIS_PREFIX_PATH set [%s] but not found", qgisPathEnv);
      }
    } catch (const std::filesystem::filesystem_error&) {
    }
  }

// 2. If the environment variable is not set, check default paths
  size_t numPaths = 0;
  const char** defaultPaths = getDefaultProgramPaths(numPaths);

  // Checking the standard paths (e.g. "C:\Program Files\QGIS 3.36.3\bin\qgis-bin.exe")
  for (size_t i = 0; i < numPaths; ++i) {
    const char* pathCStr = defaultPaths[i];
    if (!pathCStr) continue;

    searchedPaths.insert(std::filesystem::path(pathCStr));
    for (const auto& programEntry : std::filesystem::directory_iterator(pathCStr)) {
      try {
        if (std::filesystem::is_directory(programEntry.path())) {
          std::string folderName = programEntry.path().filename().string();
          // Check whether the name begins with QGIS 
          if (folderName.rfind("QGIS ") == 0) {
            searchedPaths.insert(std::filesystem::path(programEntry.path()));

            if (latestVersionName.empty() || compareVersions(folderName, latestVersionName)) {
              latestVersionName = folderName;
              std::filesystem::path path(programEntry.path());
              path /= "bin";  // Append "bin" to the path
              if (std::filesystem::exists(path)) {
                searchedPaths.insert(path);
                latestVersionPath = path.string();
              }
            }
          }
        }
      }
      catch (const std::filesystem::filesystem_error&) {
        continue;
      }
    }
  }
#else
  // First check the QGIS_PREFIX_PATH, then the system-wide library directories
  std::filesystem::path libPath;

  if (qgisPathEnv) {
    std::filesystem::path path(qgisPathEnv);
    libPath = path / "lib";
    try {
      if (std::filesystem::is_directory(libPath)) {
        searchedPaths.insert(libPath);
        LASMessage(LAS_VERY_VERBOSE, "PROJ library used via QGIS installation path: %s", libPath.string().c_str());
        return libPath.string();     
      }
    } catch (const std::filesystem::filesystem_error&) {
    }
  }

  const std::string qgisLibPaths[] = {
  "/usr/lib/qgis/", 
  "/usr/share/qgis/", 
  "/opt/qgis/"};

  const char* homeDir = getHomeDirectory();
  std::vector<std::filesystem::path> pathsToCheck;

  for (const auto& p : qgisLibPaths) pathsToCheck.push_back(p);

  if (homeDir) {
    pathsToCheck.push_back(std::filesystem::path(homeDir) / ".local/qgis");
    pathsToCheck.push_back(std::filesystem::path(homeDir) / ".local/share/flatpak/app/org.qgis.qgis/current/active/files");
  }

  for (const auto& basePath : pathsToCheck) {
    searchedPaths.insert(basePath);
    try {
      if (!std::filesystem::exists(basePath) || !std::filesystem::is_directory(basePath)) continue;

      libPath = basePath / "lib";
      if (std::filesystem::exists(libPath) && std::filesystem::is_directory(libPath)) {
        searchedPaths.insert(libPath);
        std::string proj = getProjLibraryPath(libPath.string(), true);

        if (!proj.empty()) {
          std::filesystem::path projPath(proj);
          std::string fileName = projPath.filename().string();

          if (latestVersionName.empty() || compareVersions(fileName, latestVersionName)) {
            latestVersionName = fileName;
            latestVersionPath = projPath.parent_path().string();
          }
        }
      }
    } catch (const std::filesystem::filesystem_error&) {
      continue;
    }
  }
#endif

  if (!latestVersionPath.empty()) {
    LASMessage(LAS_VERY_VERBOSE, "PROJ library used via QGIS installation path: %s", latestVersionPath.c_str());
    return latestVersionPath;
  }
  return "";
}

/// Finds the OSGeo4W installation path.
/// It searches through default installation directories and appends "bin" to the path, where the proj lib is located
#ifdef _WIN32
static std::string findLatestOSGeo4WInstallationPath(std::set<std::filesystem::path>& searchedPaths) {
  // 1. If the environment variable is not set, check default OSGeo4W paths
  size_t numPaths = 0;
  const char** defaultPaths = getDefaultProgramPaths(numPaths);

  // Checking the standard paths (e.g. "C:\OSGeo4W\bin" or "C:\Program Files\OSGeo4W\bin\")
  for (size_t i = 0; i < numPaths; ++i) {
    const char* pathCStr = defaultPaths[i];
    if (!pathCStr) continue;

    searchedPaths.insert(std::filesystem::path(pathCStr));

    for (const auto& programEntry : std::filesystem::directory_iterator(pathCStr)) {
      try {
        if (std::filesystem::is_directory(programEntry.path())) {
          std::string directoryName = programEntry.path().filename().string();
          // Check whether the name is OSGeo4W64 or OSGeo4W
          if (directoryName.find("OSGeo4W64") == 0 || directoryName.find("OSGeo4W") == 0) {
            searchedPaths.insert(std::filesystem::path(programEntry.path()));
            std::filesystem::path path(programEntry.path());
            path /= "bin";  // Append "bin" to the path
            if (std::filesystem::exists(path)) {
              searchedPaths.insert(path);
              LASMessage(LAS_VERY_VERBOSE, "PROJ library used via OSGeo4W installation path: %s", path.string().c_str());
              return path.string();
            }
          }
        }
      } catch (const std::filesystem::filesystem_error&) {
        continue;
      }
    }
  }
  return "";
}
#endif

/// Finds the latest PROJ installation path in Conda environments
static std::string findLatestCondaInstallationPath(std::set<std::filesystem::path>& searchedPaths) {
  // 1. Check the environment variable CONDA_PREFIX
  const char* condaPathEnv = getenv("CONDA_PREFIX");
  std::filesystem::path binPath;
  std::string latestVersionPath;
  std::string latestVersionName;

#ifdef _WIN32
  if (condaPathEnv) {
    try {
      if (std::filesystem::exists(condaPathEnv)) {
        std::filesystem::path path(condaPathEnv);
        path /= "Library";  // Append "Library" to the path
        path /= "bin";  // Append "bin" to the path
        if (std::filesystem::exists(path)) {
          searchedPaths.insert(path);
          LASMessage(LAS_VERY_VERBOSE, "PROJ library used via Conda installation path: %s", path.string().c_str());
          return path.string();
        } else {
          LASMessage(LAS_WARNING, "CONDA_PREFIX set but bin Library/bin folder [%s] not found", path.string().c_str());
        }
      } else {
        LASMessage(LAS_WARNING, "CONDA_PREFIX set [%s] but not found", condaPathEnv);
      }
    }
    catch (const std::filesystem::filesystem_error&) {
    }
  }

  // 2. If the environment variable is not set, check standart path
  const char* homeDir = getHomeDirectory();
  if (!homeDir) return "";

  std::filesystem::path homePath(homeDir);

  // Iterate through directories in the home directory
  for (const auto& entry : std::filesystem::directory_iterator(homePath)) {
    searchedPaths.insert(entry.path());
    try {
      std::string directoryName = entry.path().filename().string();
      if (directoryName.find("miniconda") == 0 || directoryName.find("anaconda") == 0) {
        binPath = entry.path() / "Library" / "bin";
      
        if (std::filesystem::exists(binPath) && std::filesystem::is_directory(binPath)) {
          searchedPaths.insert(binPath);
          std::string proj = findLatestProjLibraryPath(binPath.string().c_str());

          if (!proj.empty()) {
            std::filesystem::path projPath(proj);
            std::string fileName = projPath.filename().string();
 
            if (latestVersionName.empty() || compareVersions(fileName, latestVersionName)) {
              latestVersionName = fileName;
              latestVersionPath = projPath.parent_path().string();
            }
          }
        }
      }
    } catch (const std::filesystem::filesystem_error&) {
      continue;
    }
  }
#else
  // First check the CONDA_PREFIX, then the system-wide library directories
  if (condaPathEnv) {
    std::filesystem::path path(condaPathEnv);
    binPath = path / "lib";

    if (std::filesystem::is_directory(binPath)) {
      searchedPaths.insert(binPath);
      latestVersionPath = binPath.string();
    }
  }

  // 2. If the environment variable is not set, check standart path
  const char* homeDir = getenv("HOME");
  if (!homeDir) return "";
  
  std::filesystem::path homePath(homeDir);
  std::vector<std::string> condaPrefixes = {"miniconda3", "anaconda3"};
  
  for (const std::string& prefix : condaPrefixes) {
    std::filesystem::path basePath = homePath / prefix;
    try {
      if (!std::filesystem::exists(basePath)) continue;
      searchedPaths.insert(basePath);
      // Check Base Environment
      std::filesystem::path baseLibPath = basePath / "lib";
      searchedPaths.insert(baseLibPath);
      if (std::filesystem::exists(baseLibPath)) {
        std::string proj = getProjLibraryPath(baseLibPath.string(), true);

        if (!proj.empty()) {
          std::filesystem::path projPath(proj);
          std::string fileName = projPath.filename().string();

          if (latestVersionName.empty() || compareVersions(fileName, latestVersionName)) {
            latestVersionName = fileName;
            latestVersionPath = projPath.parent_path().string();
          }
        }
      }

      // Check all environments under envs/
      std::vector<std::filesystem::path> envsPaths = {
          basePath / "envs", 
          basePath / "envs/qgis", 
          basePath / "envs/proj"};

      for (const auto& envsPath : envsPaths) {
        try {
          if (!std::filesystem::exists(envsPath) || !std::filesystem::is_directory(envsPath)) continue;
          
          for (auto& entry : std::filesystem::directory_iterator(envsPath)) {
            try {
              searchedPaths.insert(entry.path());
              if (!std::filesystem::is_directory(entry.path())) continue;
              std::filesystem::path envLibPath = entry.path() / "lib";
              
              if (std::filesystem::is_directory(envLibPath)) {
                searchedPaths.insert(envLibPath);
                std::string proj = getProjLibraryPath(envLibPath.string(), true);
              
                if (!proj.empty()) {
                  std::filesystem::path projPath(proj);
                  std::string fileName = projPath.filename().string();

                  if (latestVersionName.empty() || compareVersions(fileName, latestVersionName)) {
                    latestVersionName = fileName;
                    latestVersionPath = projPath.parent_path().string();
                  }
                }
              }
            } catch (const std::filesystem::filesystem_error&) {
              continue;
            }
          }
        } catch (const std::filesystem::filesystem_error&) {
          continue;
        }
      }
    } catch (const std::filesystem::filesystem_error&) {
      continue;
    }
  }
#endif
  if (!latestVersionPath.empty()) {
    LASMessage(LAS_VERY_VERBOSE, "PROJ library found via Conda installation path: %s", latestVersionPath.c_str());
    return latestVersionPath;
  }
  return "";
}

/// Dynamically loads the PROJ library
/// 1. load library via environment variabl 'LASTOOLS_PROJ'
/// 1.2. set the PROJ data directory using an environment variable 'PROJ_LIB'
/// 2. fallback: try to load library from QGIS installation
/// 3. fallback: try to load library from OSGeo4W installation (just for windows)
/// 4. fallback: try to load library from Conda installation
/// 5. fallback: try to load library from standard paths
/// 6. fallback: try to load library from standard paths recursive
bool load_proj_library(const char* path, bool isNecessary/*=true*/) {
  proj_lib_handle = nullptr;
  std::set<std::filesystem::path> searchedPaths;
  std::set<std::string> latestVerionSet;

  // 1. Load library via environment variable
  const char* env = getenv("LASTOOLS_PROJ");
  std::string proj_path = env ? env : "";
  if (!proj_path.empty()) {
    const std::string projLibPath = findLatestProjLibraryPath(proj_path);
    if (!projLibPath.empty()) {
      proj_lib_handle = LOAD_LIBRARY(projLibPath.c_str());
      if (proj_lib_handle) {
        LASMessage(LAS_VERY_VERBOSE, "PROJ library used via environment variable 'LASTOOLS_PROJ': %s", proj_path.c_str());
        LASMessage(LAS_VERBOSE, "Latest PROJ library used via environment variable: %s", projLibPath.c_str());
      } else {
        LASMessage(LAS_WARNING, "PROJ Library '%s' was found but could not be loaded. "
            "Please check that all dependencies are present and the library is compatible with your system.",  projLibPath.c_str());
      }
    } else {
      LASMessage(LAS_WARNING, "The environment variable LASTOOLS_PROJ: '%s' is set, but no PROJ library can be found at this location. Continued...", proj_path.c_str());
    }
  }
  // Get proj data path via environment variable
  const char* projDataPath = getenv("PROJ_LIB");
  if (projDataPath) {
    LASMessage(LAS_VERY_VERBOSE, "PROJ data path used via environment variable 'PROJ_LIB': %s", projDataPath);
  }

  //2. fallback : load library from QGIS environments (default directorys)
  if (!proj_lib_handle) {
    const std::string qgisPath = findLatestQGISInstallationPath(searchedPaths);
    if (!qgisPath.empty()) {
      const std::string proj_lib_path = findLatestProjLibraryPath(qgisPath);

      if (!proj_lib_path.empty()) {
        latestVerionSet.insert(proj_lib_path);
        LASMessage(LAS_VERBOSE, "Latest PROJ library used via QGIS installation: %s", proj_lib_path.c_str());
      }
    }
  }

#ifdef _WIN32
  // 3. fallback : load library from OSGeo4W environments (default directorys)
  if (!proj_lib_handle) {
    const std::string osgeoPath = findLatestOSGeo4WInstallationPath(searchedPaths);
    if (!osgeoPath.empty()) {
      const std::string proj_lib_path = findLatestProjLibraryPath(osgeoPath);

      if (!proj_lib_path.empty()) {
        latestVerionSet.insert(proj_lib_path);
        LASMessage(LAS_VERBOSE, "Latest PROJ library used via OCGeo4W installation: %s", proj_lib_path.c_str());
      }
    }
  }
#endif

  // 4. fallback : load library from Conda environments (default directorys)
  if (!proj_lib_handle) {
    const std::string condaPath = findLatestCondaInstallationPath(searchedPaths);
    if (!condaPath.empty()) {
      const std::string proj_lib_path = findLatestProjLibraryPath(condaPath);

      if (!proj_lib_path.empty()) {
        latestVerionSet.insert(proj_lib_path);
        LASMessage(LAS_VERBOSE, "Latest PROJ library used via conda installation: %s", proj_lib_path.c_str());
      }
    }
  }

  // 5. fallback : Check standart system directories
  if (!proj_lib_handle) {
    const std::string proj_lib_path = findProjLibInStandartPaths(searchedPaths);
    
    if (!proj_lib_path.empty()) {
      latestVerionSet.insert(proj_lib_path);
      LASMessage(LAS_VERBOSE, "Latest PROJ library used via standard installation: %s", proj_lib_path.c_str());
    }
  }

    // 6. fallback : Check standart system directories recuryive
  if (!proj_lib_handle && latestVerionSet.empty()) {
    const std::string proj_lib_path = findProjLibInStandartPathsRecursive(searchedPaths);

    if (!proj_lib_path.empty()) {
      latestVerionSet.insert(proj_lib_path);
      LASMessage(LAS_VERBOSE, "Latest PROJ library used via installation: %s", proj_lib_path.c_str());
    }
  }

  //load final handle with latest PROJ version
  if (!latestVerionSet.empty() && !proj_lib_handle) {
    std::vector<std::string> sortedVersions;

    // Collect all valid paths
    for (const std::string& projPath : latestVerionSet) {
      if (!projPath.empty()) {
        sortedVersions.push_back(projPath);
      }
    }
    
    // Sort by version (highest first)
    std::sort(sortedVersions.begin(), sortedVersions.end(), [](const std::string& a, const std::string& b) {
      std::filesystem::path pa(a), pb(b);
      return compareVersions(pa.filename().string(), pb.filename().string());
    });
    
    // Now try to load iteratively
    for (const std::string& candidate : sortedVersions) {
      proj_lib_handle = LOAD_LIBRARY(candidate.c_str());
      if (proj_lib_handle) {
        LASMessage(LAS_VERBOSE, "The latest valid PROJ library found will be used: %s", candidate.c_str());
        break;
      } else {
        LASMessage(LAS_WARNING, "PROJ Library '%s' was found but could not be loaded. "
            "Please check that all dependencies are present and the library is compatible with your system.", candidate.c_str());
      }
    }
  }

  // Check if the PROJ lib could be loaded
  if (!proj_lib_handle) {
    if (isNecessary) {
      LASMessage(LAS_VERY_VERBOSE, "All paths searched to find PROJ library:");
      for (const std::filesystem::path& path : searchedPaths) {
        LASMessage(LAS_VERY_VERBOSE, "%s", path.string().c_str());
      }
      laserror("PROJ Installation not found. For more information see proj_README.md.");
    } else {
      // LASMessage(LAS_WARNING, "PROJ Installation not found, the functionalities of LASTools are used instead. For more information see proj_README.md.");
      return false;
    }
  }

#pragma warning(push)
#pragma warning(disable : 6387)
  // 4. resolve function pointer
  proj_as_wkt_ptr = (proj_as_wkt_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_as_wkt");
  proj_as_proj_string_ptr = (proj_as_proj_string_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_as_proj_string");
  proj_as_projjson_ptr = (proj_as_projjson_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_as_projjson");
  proj_get_source_crs_ptr = (proj_get_source_crs_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_get_source_crs");
  proj_get_target_crs_ptr = (proj_get_target_crs_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_get_target_crs");
  proj_destroy_ptr = (proj_destroy_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_destroy");
  proj_context_create_ptr = (proj_context_create_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_context_create");
  proj_context_destroy_ptr = (proj_context_destroy_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_context_destroy");
  proj_get_id_code_ptr = (proj_get_id_code_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_get_id_code");
  proj_get_ellipsoid_ptr = (proj_get_ellipsoid_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_get_ellipsoid");
  proj_ellipsoid_get_parameters_ptr = (proj_ellipsoid_get_parameters_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_ellipsoid_get_parameters");
  proj_crs_get_datum_ensemble_ptr = (proj_crs_get_datum_ensemble_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_crs_get_datum_ensemble");
  proj_get_name_ptr = (proj_get_name_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_get_name");
  proj_datum_ensemble_get_member_count_ptr = (proj_datum_ensemble_get_member_count_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_datum_ensemble_get_member_count");
  proj_datum_ensemble_get_accuracy_ptr = (proj_datum_ensemble_get_accuracy_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_datum_ensemble_get_accuracy");
  proj_datum_ensemble_get_member_ptr = (proj_datum_ensemble_get_member_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_datum_ensemble_get_member");
  proj_crs_get_datum_ptr = (proj_crs_get_datum_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_crs_get_datum");
  proj_crs_get_coordinate_system_ptr = (proj_crs_get_coordinate_system_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_crs_get_coordinate_system");
  proj_cs_get_type_ptr = (proj_cs_get_type_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_cs_get_type");
  proj_cs_get_axis_count_ptr = (proj_cs_get_axis_count_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_cs_get_axis_count");
  proj_cs_get_axis_info_ptr = (proj_cs_get_axis_info_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_cs_get_axis_info");
  proj_create_ptr = (proj_create_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_create");
  proj_create_argv_ptr = (proj_create_argv_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_create_argv");
  proj_create_crs_to_crs_ptr = (proj_create_crs_to_crs_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_create_crs_to_crs");
  proj_create_crs_to_crs_from_pj_ptr = (proj_create_crs_to_crs_from_pj_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_create_crs_to_crs_from_pj");
  proj_create_from_wkt_ptr = (proj_create_from_wkt_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_create_from_wkt");
  proj_context_errno_ptr = (proj_context_errno_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_context_errno");
  proj_context_errno_string_ptr = (proj_context_errno_string_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_context_errno_string");
  proj_coord_ptr = (proj_coord_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_coord");
  proj_trans_ptr = (proj_trans_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_trans");
  proj_get_type_ptr = (proj_get_type_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_get_type");
  proj_is_crs_ptr = (proj_is_crs_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_is_crs");
  proj_info_ptr = (proj_info_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_info");
  proj_log_func_ptr = (proj_log_func_t)GET_PROC_ADDRESS(proj_lib_handle, "proj_log_func");

  if (!proj_as_wkt_ptr || !proj_as_proj_string_ptr || !proj_as_projjson_ptr || !proj_get_source_crs_ptr || !proj_get_target_crs_ptr ||
      !proj_destroy_ptr || !proj_context_create_ptr || !proj_context_destroy_ptr || !proj_get_id_code_ptr || !proj_get_ellipsoid_ptr ||
      !proj_ellipsoid_get_parameters_ptr || !proj_crs_get_datum_ensemble_ptr || !proj_get_name_ptr || !proj_datum_ensemble_get_member_count_ptr ||
      !proj_datum_ensemble_get_accuracy_ptr || !proj_datum_ensemble_get_member_ptr || !proj_crs_get_datum_ptr ||
      !proj_crs_get_coordinate_system_ptr || !proj_cs_get_type_ptr || !proj_cs_get_axis_count_ptr || !proj_cs_get_axis_info_ptr || !proj_create_ptr ||
      !proj_create_argv_ptr || !proj_create_crs_to_crs_ptr || !proj_create_crs_to_crs_from_pj_ptr || !proj_create_from_wkt_ptr ||
      !proj_context_errno_ptr || !proj_context_errno_string_ptr || !proj_coord_ptr || !proj_trans_ptr || !proj_get_type_ptr || !proj_is_crs_ptr ||
      !proj_info_ptr || !proj_log_func_ptr)
  {
    std::string version = "Unknown";
    if (proj_info_ptr) {
      MyPJ_INFO info = my_proj_info();
      if (info.version) version = info.version;
    }
    unload_proj_library();
    laserror("Failed to load necessary PROJ functions. This application requires PROJ version 9.0.0 or later for full functionality. Loaded PROJ version: %s", version.c_str());
  }

#ifdef _WIN32
  // gpf at linux
  checkProjVersion();
#endif

  return true;
#pragma warning(pop)
}

/// unloads the PROJ library and sets all associated function pointers to nullptr.
void unload_proj_library() {
  if (proj_lib_handle) {  // Check whether lib_handle is valid
    FREE_LIBRARY(proj_lib_handle);  // Unload library
    proj_lib_handle = nullptr;      // Set handle to nullptr
  }

  // Set function pointer to nullptr
  proj_as_wkt_ptr = nullptr;
  proj_as_proj_string_ptr = nullptr;
  proj_as_projjson_ptr = nullptr;
  proj_get_source_crs_ptr = nullptr;
  proj_get_target_crs_ptr = nullptr;
  proj_destroy_ptr = nullptr;
  proj_context_create_ptr = nullptr;
  proj_context_destroy_ptr = nullptr;
  proj_get_id_code_ptr = nullptr;
  proj_get_ellipsoid_ptr = nullptr;
  proj_ellipsoid_get_parameters_ptr = nullptr;
  proj_crs_get_datum_ensemble_ptr = nullptr;
  proj_get_name_ptr = nullptr;
  proj_datum_ensemble_get_member_count_ptr = nullptr;
  proj_datum_ensemble_get_accuracy_ptr = nullptr;
  proj_datum_ensemble_get_member_ptr = nullptr;
  proj_crs_get_datum_ptr = nullptr;
  proj_crs_get_coordinate_system_ptr = nullptr;
  proj_cs_get_type_ptr = nullptr;
  proj_cs_get_axis_count_ptr = nullptr;
  proj_cs_get_axis_info_ptr = nullptr;
  proj_create_ptr = nullptr;
  proj_create_argv_ptr = nullptr;
  proj_create_crs_to_crs_ptr = nullptr;
  proj_create_crs_to_crs_from_pj_ptr = nullptr;
  proj_create_from_wkt_ptr = nullptr;
  proj_context_errno_ptr = nullptr;
  proj_context_errno_string_ptr = nullptr;
  proj_coord_ptr = nullptr;
  proj_trans_ptr = nullptr;
  proj_get_type_ptr = nullptr;
  proj_is_crs_ptr = nullptr;
  proj_info_ptr = nullptr;
  proj_log_func_ptr = nullptr;
}

/// User-specific error handler for PROJ errors and warnings
extern "C" void myCustomProjErrorHandler(void* app_data, int level, const char* msg) {
  if (!msg) return;

  if (level != PJ_LOG_ERROR) return;

  LASMessage(LAS_INFO, "PROJ error: %s", msg);
}
