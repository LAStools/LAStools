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

#include <vector>
#include <filesystem>
#include <regex>
#include <sstream>
#include <algorithm>
#include <cstring>

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
proj_cs_get_type_t proj_cs_get_type_ptr = nullptr;
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

/// Function to get the home directory of the current user
const char* getHomeDirectory() {
#ifdef _WIN32
  return getenv("USERPROFILE");
#else
  return getenv("HOME"); 
#endif
}

/// Function for determining the standard programme paths
const char** getDefaultProgramPaths(size_t& numPaths) {
#ifdef _WIN32
  // Windows: Use environment variables or API for Program Files directories
  static const char* defaultPaths[] = {
      getenv("ProgramFiles"),
      getenv("ProgramFiles(x86)"),
      nullptr};
  // Count valid paths
  numPaths = 0;
  while (defaultPaths[numPaths] != nullptr) {
    ++numPaths;
  }
  return defaultPaths;
#elif __APPLE__
  // macOS: Standard directories
  static const char* defaultPaths[] = {"/Applications/", "/usr/local/", nullptr};
  numPaths = sizeof(defaultPaths) / sizeof(defaultPaths[0]) - 1;
  return defaultPaths;
#else
  // Linux/Unix: Standard directories
  static const char* defaultPaths[] = {"/usr/local/", "/opt/", "/usr/share/", nullptr};
  numPaths = sizeof(defaultPaths) / sizeof(defaultPaths[0]) - 1;
  return defaultPaths;
#endif
}

/// Function for parsing the version number from the directory name
static std::vector<int> parseVersion(const char* versionStr) {
  std::vector<int> versionNumbers;
  std::regex versionRegex("(\\d+)");
  std::string versionString(versionStr);
  std::sregex_iterator it(versionString.begin(), versionString.end(), versionRegex);
  std::sregex_iterator end;

  while (it != end) {
    versionNumbers.push_back(std::stoi(it->str()));
    ++it;
  }

  return versionNumbers;
}

#ifdef __unix__
static char* findUnixLibProjPath() {
  // Check system-wide directories
  const std::string systemLibPaths[] = {
      "/usr/lib/", "/usr/bin/",
      "/usr/local/bin/"
      "/usr/lib/x86_64-linux-gnu/",
      "/lib/x86_64-linux-gnu/"};

  for (const auto& systemLibPath : systemLibPaths) {
    std::filesystem::path binPath = systemLibPath;

    if (std::filesystem::is_directory(binPath) && std::filesystem::exists(binPath / "libproj.so")) {
      char* resultPath = new char[binPath.string().size() + 1];
      strcpy_las(resultPath, binPath.string().size() + 1, binPath.string().c_str());
      return resultPath;
    }
  }

  // Optional: Use the user's home directory if necessary
  const char* homeDir = getHomeDirectory();
  if (homeDir) {
    std::filesystem::path userLibPath(homeDir);
    userLibPath /= ".local";  // the .local/lib directory for user installations
    userLibPath /= "lib";

    if (std::filesystem::is_directory(userLibPath) && std::filesystem::exists(userLibPath / "libproj.so")) {
      char* resultPath = new char[userLibPath.string().size() + 1];
      strcpy_las(resultPath, userLibPath.string().size() + 1, userLibPath.string().c_str());
      return resultPath;
    }
  }

 return nullptr;
}
#endif

/// Comparison function for version numbers
static bool compareVersions(const char* v1, const char* v2) {
  std::vector<int> version1 = parseVersion(v1);
  std::vector<int> version2 = parseVersion(v2);

  size_t len = std::min(version1.size(), version2.size());
  for (size_t i = 0; i < len; ++i) {
    if (version1[i] > version2[i]) return true;
    if (version1[i] < version2[i]) return false;
  }

  return version1.size() > version2.size();
}

/// Finds the latest QGIS installation path by first checking the `QGIS_PREFIX_PATH` environment variable.
/// If the environment variable is not set, it searches through default installation directories and selects the latest version based on directory names.
/// appends "bin" to the path, where the proj lib is located
static char* findLatestQGISInstallationPath() {
  // 1. Check the environment variable QGIS_PREFIX_PATH
  const char* qgisPathEnv = getenv("QGIS_PREFIX_PATH");
  std::filesystem::path binPath;

#ifdef _WIN32
  if (qgisPathEnv) {
    if (std::filesystem::exists(qgisPathEnv))
    {
      std::filesystem::path path(qgisPathEnv);
      path /= "bin";  // Append "bin" to the path
      char* resultPath = new char[path.string().size() + 1];
      strcpy_las(resultPath, path.string().size() + 1, path.string().c_str());
      return resultPath;
    }
    else
    {
      LASMessage(LAS_WARNING, "QGIS_PREFIX_PATH set [%s] but not found", qgisPathEnv);
    }
  }

// 2. If the environment variable is not set, check default paths
  size_t numPaths = 0;
  const char** defaultPaths = getDefaultProgramPaths(numPaths);
  char* latestVersionPath = nullptr;
  char* latestVersionName = nullptr;

  // Checking the standard paths (e.g. "C:\Program Files\QGIS 3.36.3\bin\qgis-bin.exe")
  for (size_t i = 0; i < numPaths; ++i) {
    for (const auto& programEntry : std::filesystem::directory_iterator(defaultPaths[i])) {
      if (programEntry.is_directory()) {
        std::string directoryName = programEntry.path().filename().string();
        // Check whether the name begins with �QGIS �
        if (directoryName.find("QGIS ") == 0) {
          const char* dirName = directoryName.c_str();
          if (!latestVersionName || compareVersions(dirName, latestVersionName)) {
            free(latestVersionName);
            delete[] latestVersionPath;
            latestVersionName = strdup_las(dirName);
            std::filesystem::path path(programEntry.path());
            path /= "bin";  // Append "bin" to the path
            latestVersionPath = new char[path.string().size() + 1];
            strcpy_las(latestVersionPath, path.string().size() + 1, path.string().c_str());
          }
        }
      }
    }
  }
  free(latestVersionName);
  return latestVersionPath;
#else
  // First check the QGIS_PREFIX_PATH, then the system-wide library directories
  if (qgisPathEnv) {
    std::filesystem::path path(qgisPathEnv);
    binPath = path / "lib";

    if (std::filesystem::is_directory(binPath) && std::filesystem::exists(binPath / "libproj.so")) {
      char* resultPath = new char[binPath.string().size() + 1];
      strcpy_las(resultPath, binPath.string().size() + 1, binPath.string().c_str());
      LASMessage(LAS_VERBOSE, "QGIS path found [%s]", resultPath);
      return resultPath;
    }
  }

  // Check system-wide standard directories
  return findUnixLibProjPath();
#endif

  return nullptr;
}

/// Finds the latest QGIS installation path in Conda environments
static char* findLatestCondaInstallationPath() {
  // 1. Check the environment variable CONDA_PREFIX
  const char* condaPathEnv = getenv("CONDA_PREFIX");
  std::filesystem::path binPath;

#ifdef _WIN32
  if (condaPathEnv) {
    if (std::filesystem::exists(condaPathEnv)) {
      std::filesystem::path path(condaPathEnv);
      path /= "Library";  // Append "Library" to the path
      path /= "bin";  // Append "bin" to the path
      char* resultPath = new char[path.string().size() + 1];
      strcpy_las(resultPath, path.string().size() + 1, path.string().c_str());
      return resultPath;
    }
  }

  // 2. If the environment variable is not set, check standart path
  const char* homeDir = getHomeDirectory();
  if (!homeDir) return nullptr;

  std::filesystem::path homePath(homeDir);
  char* latestVersionPath = nullptr;

  // Iterate through directories in the home directory
  for (const auto& entry : std::filesystem::directory_iterator(homePath)) {
    std::string directoryName = entry.path().filename().string();
    if (directoryName.find("miniconda") == 0 || directoryName.find("anaconda") == 0) {
      binPath = entry.path() / "Library" / "bin";

      if (std::filesystem::exists(binPath) && std::filesystem::is_directory(binPath)) {
        delete[] latestVersionPath;
        latestVersionPath = new char[binPath.string().size() + 1];
        strcpy_las(latestVersionPath, binPath.string().size() + 1, binPath.string().c_str());
        return latestVersionPath;
      }
    }
  }
#else
  // First check the CONDA_PREFIX, then the system-wide library directories
  if (condaPathEnv) {
    std::filesystem::path path(condaPathEnv);
    binPath = path / "lib";

    if (std::filesystem::is_directory(binPath)  && std::filesystem::exists(binPath / "libproj.so")) {
      char* resultPath = new char[binPath.string().size() + 1];
      strcpy_las(resultPath, binPath.string().size() + 1, binPath.string().c_str());
      return resultPath;
    }
  }

  // Check system-wide standard directories
  return findUnixLibProjPath();
#endif

  return nullptr;
}

/// Finds the latest proj library path within the QGIS installation path
static char* findLatestProjLibraryPath(const char* binPath) {
  if (!binPath) return nullptr;

  // Check if the binPath exists and is a directory
  if (!std::filesystem::exists(binPath) || !std::filesystem::is_directory(binPath)) {
    return nullptr;
  }

#ifdef _WIN32
  const char* projLibraryPattern = "proj_";
  const char* fileExtension = ".dll";

  char* latestVersionPath = nullptr;
  char* latestVersionName = nullptr;

  for (const auto& entry : std::filesystem::directory_iterator(binPath)) {
    std::string fileName = entry.path().filename().string();
    if (fileName.find(projLibraryPattern) != std::string::npos && fileName.find(fileExtension) != std::string::npos) {
      if (!latestVersionName || compareVersions(fileName.c_str(), latestVersionName)) {
        free(latestVersionName);
        delete[] latestVersionPath;
        latestVersionName = strdup_las(fileName.c_str());
        latestVersionPath = new char[entry.path().string().size() + 1];
        strcpy_las(latestVersionPath, entry.path().string().size() + 1, entry.path().string().c_str());
      }
    }
  }
  free(latestVersionName);
  return latestVersionPath;
#else
  // libproj.so always refers to the corresponding version
  const char* projLibraryName = "libproj.so";

  for (const auto& entry : std::filesystem::directory_iterator(binPath)) {
    std::string fileName = entry.path().filename().string();
    if (fileName == projLibraryName) {
      char* resultPath = new char[entry.path().string().size() + 1];
      strcpy_las(resultPath, entry.path().string().size() + 1, entry.path().string().c_str());
      return resultPath;
    }
  }
#endif

  return nullptr;
}

/// Dynamically loads the PROJ library
/// 1. load library via environment variabl 'LASTOOLSPROJ'
/// 2. fallback: load library from standard paths (executable directory)
/// 3. set the PROJ data directory using an environment variable 'PROJ_LIB'
bool load_proj_library(const char* path, bool isNecessary/*=true*/) {
  proj_lib_handle = nullptr;

  // 1. Load library via environment variable
  const char* proj_path = getenv("LASTOOLS_PROJ");
  if (proj_path) {
    char* projLibPath = findLatestProjLibraryPath(proj_path);
    if (projLibPath) {
      proj_lib_handle = LOAD_LIBRARY(projLibPath);
      delete[] projLibPath;  // Free memory after usage
    }
  }

  //2. fallback : load library from QGIS environments (default directorys)
  if (!proj_lib_handle) {
    char* qgisPath = findLatestQGISInstallationPath();
    if (qgisPath) {
      char* proj_lib_path = findLatestProjLibraryPath(qgisPath);
      delete[] qgisPath;

      if (proj_lib_path) {  
        // Try to load the library
        proj_lib_handle = LOAD_LIBRARY(proj_lib_path);
        delete[](proj_lib_path);
      }
    }
  }

  // 3. fallback : load library from Conda environments (default directorys)
  if (!proj_lib_handle) {
    char* condaPath = findLatestCondaInstallationPath();
    if (condaPath) {
      char* proj_lib_path = findLatestProjLibraryPath(condaPath);
      delete[] condaPath;

      if (proj_lib_path) {
        // Try to load the library
        proj_lib_handle = LOAD_LIBRARY(proj_lib_path);
        delete[] proj_lib_path;
      }
    }
  }

  // Check if the PROJ lib could be loaded
  if (!proj_lib_handle) {
    if (isNecessary) {
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

  if (!proj_as_wkt_ptr || !proj_as_proj_string_ptr || !proj_as_projjson_ptr || !proj_get_source_crs_ptr || !proj_get_target_crs_ptr ||
      !proj_destroy_ptr || !proj_context_create_ptr || !proj_context_destroy_ptr || !proj_get_id_code_ptr || !proj_get_ellipsoid_ptr ||
      !proj_ellipsoid_get_parameters_ptr || !proj_crs_get_datum_ensemble_ptr || !proj_get_name_ptr || !proj_datum_ensemble_get_member_count_ptr ||
      !proj_datum_ensemble_get_accuracy_ptr || !proj_datum_ensemble_get_member_ptr || !proj_crs_get_datum_ptr ||
      !proj_crs_get_coordinate_system_ptr || !proj_cs_get_type_ptr || !proj_cs_get_axis_count_ptr || !proj_cs_get_axis_info_ptr || !proj_create_ptr ||
      !proj_create_argv_ptr || !proj_create_crs_to_crs_ptr || !proj_create_crs_to_crs_from_pj_ptr || !proj_create_from_wkt_ptr ||
      !proj_context_errno_ptr || !proj_context_errno_string_ptr || !proj_coord_ptr || !proj_trans_ptr || !proj_get_type_ptr || !proj_is_crs_ptr)
  {
    unload_proj_library();
    laserror("Failed to load necessary PROJ functions.");
  }
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
}