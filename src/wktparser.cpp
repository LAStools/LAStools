/*
===============================================================================

  FILE:  wktparser.cpp

  CONTENTS:

  see corresponding header file

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2025, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the Apache Public License 2.0 published by the Apache Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
===============================================================================
*/
#include "wktparser.h"

#include "mydefs.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <math.h>

int UnitValueToEpsg(double val) {
  if (val == 1.0) {
    return EPSG_METER;
  } else if (fabs(val - 0.3048006096012192) < 0.000000001) {
    return EPSG_SURFEET;
  } else {
    return EPSG_FEET;
  }
}

WktParser::WktParser() {
}

void WktParser::SetWkt(const std::string wktin) {
  wktraw = wktin;
  std::string work = wktin;
  std::string token = "";
  std::vector<std::string> vkey;
  std::vector<std::string> vval;
  bool inStr = false;
  bool inEscape = false;

  // add key to key vector
  auto vkeyAdd = [&]() {
    token = BOOST_PRE trim(token);
    if (!token.empty()) {
      vkey.push_back(token);
      token = "";
    }
  };

  // add value to value vector
  auto vvalAdd = [&]() {
    if (!token.empty()) {
      vval.push_back(token);
      token = "";
    }
  };

  // parse wkt
  for (char& c : work) {
    if (inStr) {
      // get string till trailing "
      if (c == '\\') {
        inEscape = true;
      } else if (inEscape || c != '"') {
        inEscape = false;
        token += c;
      } else {
        // string close
        inEscape = false;
        inStr = false;
      }
    } else if (c == '[') {
      // finalize last values and start a new key-value pair
      if (!vkey.empty() && !vval.empty()) {
        map.insert(std::pair<std::string, std::string>(VectorDelimited(vkey, "."), VectorDelimited(vval, "|")));
        vval.clear();
      }
      // extend or start new key
      BOOST_PRE to_lower(token);
      // - detect wkt version
      if (isWktUnknown) {
        isWktUnknown = false;
        if (StringInVector(token, Wkt1Tkn, false)) {
          isWkt1 = true;
        }
        if (StringInVector(token, Wkt2Tkn, false)) {
          if (isWkt1) {
            // warn: invalid
            LASMessage(LAS_WARNING, "WKT2/WKT1 mismatch");
          }
          isWkt1 = false;
          if (!silent) LASMessage(LAS_VERBOSE, "WKT2 detected");  // optional: warning
        }
        // check coupound
        if (isWkt1) {
          isCompound = token.compare("compd_cs") == 0;
          if (isCompound) {
            compoundPfx = "compd_cs.";
          }
        } else {
          isCompound = token.compare("compoundcrs") == 0;
          if (isCompound) {
            compoundPfx = "compoundcrs.";
          }
        }
        if (isCompound) {
          if (!silent) LASMessage(LAS_VERBOSE, "WKT: compound CS detected");
        }
      }
      vkeyAdd();
    } else if (c == ']') {
      // add value to current key and go back one key-level
      /*
      // optional: handle first value as key
      bool push = false;
      if (vval.size() > 0) {
        vkey.push_back(BOOST_PRE to_lower_copy(vval[0]));
        vval.erase(vval.begin());
        push = true;
      }
      */
      //
      vvalAdd();
      if (!vkey.empty() && !vval.empty()) {
        map.insert(std::pair<std::string, std::string>(VectorDelimited(vkey, "."), VectorDelimited(vval, "|")));
        vval.clear();
      }
      vkey.pop_back();
      /* optional: handle first value as key
      if (push) {
        vkey.pop_back();
      }
      */
    } else if (c == ',') {
      // add value to value vector
      vvalAdd();
    } else if (c == '"') {
      // start string parser
      inStr = true;
      inEscape = false;
    } else {
      token += c;
    }
  }
}

std::string WktParser::WktFormat(bool flat, const short indent, const short indent_offset) {
  if (flat) {
    // optional format as single line
    return ReplaceString(ReplaceString(ReplaceString(wktraw, "\n", ""), "\r", ""), ", ", ",");
  }
  // default: format with linebreaks and indent
  std::string work = wktraw;
  std::string token = "";
  std::string result = "";
  bool inStr = false;
  bool inEscape = false;
  int level = 0;
  // parse wkt
  for (char& c : work) {
    if (inStr) {
      // get string till trailing "
      if (c == '\\') {
        inEscape = true;
      } else if (inEscape || c != '"') {
        inEscape = false;
        token += c;
      } else {
        // string close
        inEscape = false;
        inStr = false;
      }
    } else if (c == '[') {
      // output last key in new line with indent
      if (!result.empty()) result += '\n';
      result += std::string(indent_offset, ' ') + std::string(level * indent, ' ') + BOOST_PRE trim(token) + c;
      level++;
      token = "";
    } else if (c == ']') {
      // close values
      result += BOOST_PRE trim(token) + c;
      token = "";
      level--;
    } else if (c == ',') {
      // add this value
      result += BOOST_PRE trim(token) + c;
      token = "";
    } else if (c == '"') {
      // start string parser
      inStr = true;
      inEscape = false;
    } else {
      token += c;
    }
  }
  return result;
}

void WktParser::Debug() {
  for (auto const& [key, val] : map) {
    std::cout << key << " = " << val << std::endl;
  }
}

bool WktParser::HasValueStr(const std::string& key, std::string& val) {
  if (auto search = map.find(to_lower_copy(key)); search != map.end()) {
    val = search->second;  // e.g. "EPSG|123"
    return true;
  } else {
    return false;
  }
}

std::string WktParser::ValueStr(const std::string& key) {
  std::string result;
  if (HasValueStr(key, result)) {
    return result;
  } else {
    return "";
  }
}

bool WktParser::HasValueInt(const std::string& key, int& val) {
  std::string res;
  if (HasValueStr(key, res)) {
    size_t pos = res.find('|');
    try {
      if (pos != std::string::npos) {
        val = std::stoi(res.substr(pos + 1, 999));
      } else {
        val = std::stoi(res);
      }
      return true;
    } catch (const std::exception&) {
      return false;
    }
  } else {
    return false;
  }
}

bool WktParser::HasValueDouble(const std::string& key, double& val) {
  std::string res;
  if (HasValueStr(key, res)) {
    size_t pos = res.find('|');
    try {
      if (pos != std::string::npos) {
        // by default try 2nd value in values (1st is usually text)
        val = std::stod(res.substr(pos + 1, 999));
      } else {
        // if only one value: try this
        val = std::stod(res);
      }
      return true;
    } catch (const std::exception&) {
      return false;
    }
  } else {
    return false;
  }
}

int WktParser::ValueInt(const std::string& key, const double def) {
  int ret;
  if (HasValueInt(key, ret)) {
    return ret;
  } else {
    return def;
  }
}

double WktParser::ValueDouble(const std::string& key, const double def) {
  double ret;
  if (HasValueDouble(key, ret)) {
    return ret;
  } else {
    return def;
  }
}

std::string WktParser::ValueSubStr(const std::string& key, const std::string sub, const std::string def) {
  std::string keylc = BOOST_PRE to_lower_copy(key);
  std::string subcc = BOOST_PRE to_lower_copy(sub);
  auto range = map.equal_range(keylc);
  for (auto item = range.first; item != range.second; ++item) {
    std::string val = item->second;
    std::string out;
    if (GetTokenNext(val, "|", out) && (BOOST_PRE to_lower_copy(out).compare(sub) == 0)) {
      if (GetTokenNext(val, "|", out)) {
        return out;
      }
    }
  }
  return def;
}

int WktParser::ValueSubInt(const std::string& key, const std::string sub, const int def) {
  std::string res = ValueSubStr(key, sub);
  if (res.empty()) {
    return def;
  } else {
    return stoidefault(res, def);
  }
}

double WktParser::ValueSubDouble(const std::string& key, const std::string sub, const double def) {
  std::string res = ValueSubStr(key, sub);
  if (res.empty()) {
    return def;
  } else {
    return stoddefault(res, def);
  }
}

/* *************************
    WktParserSem
************************* */

// WktParserSem::WktParserSem(const std::string wktin) : WktParser(wktin) {}

int WktParserSem::Pcs_Epsg() {
  int res;
  if (isWkt1) {
    res = ValueInt(compoundPfx + "PROJCS.AUTHORITY");
  } else {
    res = ValueInt(compoundPfx + "PROJCRS.ID");
  }
  return res;
}

int WktParserSem::Vert_Epsg() {
  int res;
  if (isWkt1) {
    res = ValueInt(compoundPfx + "VERT_CS.AUTHORITY");
  } else {
    res = ValueInt(compoundPfx + "VERTCRS.ID");
  }
  return res;
}
int WktParserSem::Vert_Unit_Epsg() {
  int res = 0;
  if (isWkt1) {
    res = ValueInt(compoundPfx + "VERT_CS.UNIT.AUTHORITY");
  } else {
    // no epsg unit in wkt2
  }
  if (res == 0) {
    // reverse calculation from unit
    res = UnitValueToEpsg(Vert_Unit());
  }
  return res;
}

int WktParserSem::Gcs_Epsg() {
  int res;
  if (isWkt1) {
    res = ValueInt(compoundPfx + "GEOGCS.AUTHORITY");
  } else {
    res = ValueInt(compoundPfx + "GEOGCRS.ID");
  }
  return res;
}

bool WktParserSem::HasProjection(PROJECTION_METHOD& pm) {
  std::string key;
  std::string projection;
  if (isWkt1) {
    key = "PROJCS.PROJECTION";
  } else {
    key = "PROJCRS.CONVERSION.METHOD";
  }
  projection = ValueStr(compoundPfx + key);
  if (auto search = wkt1projection.find(to_lower_copy(projection)); search != wkt1projection.end()) {
    pm = search->second;
    return true;
  }
  return false;
}

double WktParserSem::ProjectionFalseEasting() {
  double res;
  if (isWkt1) {
    res = ValueSubDouble(compoundPfx + "PROJCS.PARAMETER", "false_easting");
  } else {
    res = ValueSubDouble(compoundPfx + "PROJCRS.CONVERSION.PARAMETER", "False easting");
  }
  return res;
}
double WktParserSem::ProjectionFalseNorthing() {
  double res;
  if (isWkt1) {
    res = ValueSubDouble(compoundPfx + "PROJCS.PARAMETER", "false_northing");
  } else {
    res = ValueSubDouble(compoundPfx + "PROJCRS.CONVERSION.PARAMETER", "False northing");
  }
  return res;
}

double WktParserSem::ProjectionLatitudeOfOrigin() {
  double res;
  if (isWkt1) {
    res = ValueSubDouble(compoundPfx + "PROJCS.PARAMETER", "latitude_of_origin");
  } else {
    res = ValueSubDouble(compoundPfx + "PROJCRS.CONVERSION.PARAMETER", "Latitude of natural origin");
  }
  return res;
}
double WktParserSem::ProjectionLatitudeOfCenter() {
  double res;
  if (isWkt1) {
    res = ValueSubDouble(compoundPfx + "PROJCS.PARAMETER", "latitude_of_center");
  } else {
    res = ValueSubDouble(compoundPfx + "PROJCRS.CONVERSION.PARAMETER", "latitude of projection centre");
  }
  return res;
}
double WktParserSem::ProjectionLongitudeOfCenter() {
  double res;
  if (isWkt1) {
    res = ValueSubDouble(compoundPfx + "PROJCS.PARAMETER", "longitude_of_center");
  } else {
    res = ValueSubDouble(compoundPfx + "PROJCRS.CONVERSION.PARAMETER", "longitude of projection centre");
  }
  return res;
}

double WktParserSem::ProjectionCentralMeridian() {
  double res;
  if (isWkt1) {
    res = ValueSubDouble(compoundPfx + "PROJCS.PARAMETER", "central_meridian");
  } else {
    res = ValueSubDouble(compoundPfx + "PROJCRS.CONVERSION.PARAMETER", "Longitude of natural origin");
  }
  return res;
}
double WktParserSem::ProjectionCentralStandardParallel1() {
  double res;
  if (isWkt1) {
    res = ValueSubDouble(compoundPfx + "PROJCS.PARAMETER", "standard_parallel_1");
  } else {
    res = ValueSubDouble(compoundPfx + "PROJCRS.CONVERSION.PARAMETER", "Latitude of 1st standard parallel");
  }
  return res;
}
double WktParserSem::ProjectionCentralStandardParallel2() {
  double res;
  if (isWkt1) {
    res = ValueSubDouble(compoundPfx + "PROJCS.PARAMETER", "standard_parallel_2");
  } else {
    res = ValueSubDouble(compoundPfx + "PROJCRS.CONVERSION.PARAMETER", "Latitude of 2nd standard parallel");
  }
  return res;
}

double WktParserSem::ProjectionScaleFactor() {
  double res;
  if (isWkt1) {
    res = ValueSubDouble(compoundPfx + "PROJCS.PARAMETER", "scale_factor", 1);
  } else {
    res = ValueSubDouble(compoundPfx + "PROJCRS.CONVERSION.PARAMETER", "Scale factor at natural origin", 1);
  }
  return res;
}

double WktParserSem::Pcs_Unit() {
  double res;
  if (isWkt1) {
    res = ValueDouble(compoundPfx + "PROJCS.UNIT", 1);
  } else {
    res = ValueDouble(compoundPfx + "PROJCRS.CS.AXIS.LENGTHUNIT", 1);
  }
  return res;
}

double WktParserSem::Vert_Unit() {
  double res;
  if (isWkt1) {
    res = ValueDouble(compoundPfx + "VERT_CS.UNIT", 1);
  } else {
    res = ValueDouble(compoundPfx + "VERTCRS.CS.AXIS.LENGTHUNIT", 1);
  }
  return res;
}

double WktParserSem::Gcs_Unit() {
  double res;
  if (isWkt1) {
    res = ValueDouble(compoundPfx + "GEOCCS.UNIT", 1);
  } else {
    res = ValueDouble(compoundPfx + "GEODCRS.CS.AXIS.LENGTHUNIT", 1);
  }
  return res;
}
