/*
===============================================================================

  FILE:  wktparser.h

  CONTENTS:
    parser for crs wkt strings

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
#ifndef WKTPARSER_H
#define WKTPARSER_H

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

const int EPSG_METER = 9001;    // Linear_Meter
const int EPSG_FEET = 9002;     // Linear_Foot
const int EPSG_SURFEET = 9003;  // Linear_Foot_US_Survey

enum PROJECTION_METHOD {
  pro_Albers,
  pro_Polyconic,
  pro_Cassini,
  pro_HotineObliqueMercatorA,
  pro_HotineObliqueMercatorB,
  pro_LambertEqualArea,
  pro_LambertConicConformal1,
  pro_LambertConicConformal2,
  pro_MercatorA,
  pro_MercatorB,
  pro_ObliqueStereographic,
  pro_TransverseMercator,
  pro_GaussConform
};

const std::unordered_map<std::string, PROJECTION_METHOD> wkt1projection = {
    {"Albers_Conic_Equal_Area", pro_Albers},
    {"Hotine_Oblique_Mercator", pro_HotineObliqueMercatorA},
    {"Lambert_Conformal_Conic_1SP", pro_LambertConicConformal1},
    {"Lambert_Conformal_Conic", pro_LambertConicConformal2},
    {"Oblique_Stereographic", pro_ObliqueStereographic},
    {"Transverse_Mercator", pro_TransverseMercator}};

// tokens to identify wkt1/wkt2
const std::vector<std::string> Wkt1Tkn = {"COMPD_CS", "FITTED_CS", "GEOCCS", "GEOGCS", "LOCAL_CS", "PROJCS", "VERT_CS"};
const std::vector<std::string> Wkt2Tkn = {"BOUNDCRS",    "COMPOUNDCRS",        "ENGCRS",  "ENGINEERINGCRS", "GEODCRS", "GEODETICCRS",
                                          "IMAGECRS",    "PARAMETRICCRS",      "PROJCRS", "PROJECTEDCRS",   "TIMECRS", "VERTCRS",
                                          "VERTICALCRS", "COORDINATEOPERATION"};

/// helper function to get unit-epsg out of unit value
int UnitValueToEpsg(double val);

/// <summary>
/// wkt parser for wkt1 and wkt2
/// read the wkt and create a key-value map of all entries for fast and simple access to each element.
/// provide some access functions to fetch values.
/// </summary>
/// ### sample input wkt of
///   PROJCS[\"NAD83(2011) / UTM zone 17N\"
///       GEOGCS[\"NAD83(2011)\"
///           AUTHORITY[\"EPSG\",\"6318\"]]
///       PROJECTION[\"Transverse_Mercator\"]
///       PARAMETER[\"false_northing\",0]
///       AUTHORITY[\"EPSG\",\"6346\"]]
/// ### will result in a map like:
///   projcs = NAD83(2011) / UTM zone 17N
///   projcs.geogcs = NAD83(2011)
///   projcs.geogcs.authority = EPSG|6318
///   projcs.projection = Transverse_Mercator
///   projcs.parameter = false_northing|0
///   projcs.authority = EPSG|6346
/// ###
/// this map is quite easy to check for certain values, like
///    obj.ValueInt("PROJCS.AUTHORITY")   // case insensitive!
/// which will result "6346"
///
class WktParser {
 private:
  bool isWktUnknown = true;

 protected:
  std::string compoundPfx = "";

 public:
  std::unordered_multimap<std::string, std::string> map;
  bool isWkt1 = false;
  bool isCompound = false; // wkt is a compound wkt
  WktParser(const std::string wktin);
  /// output parsed wkt to console for debugging
  void Debug();
  /// <summary>
  /// check if a value is available and assign this if possible
  /// </summary>
  /// <param name="key">key to look for like 'projcs.authority'</param>
  /// <param name="val">value of key</param>
  /// <returns>value is available and assigned to val</returns>
  bool HasValueStr(const std::string& key, std::string& val);
  /// <summary>
  /// get a value of the parsed wkt if available, "" otherwise
  /// </summary>
  /// <param name="key">key to look for like 'projcs.authority'</param>
  /// <returns></returns>
  std::string ValueStr(const std::string& key);
  /// <summary>
  /// check if a value is available as int and assign this if possible
  /// </summary>
  /// <param name="key">key to look for like 'projcs.authority'</param>
  /// <param name="val">integer value of key, if key value is "abc|123" just 123 will be assigned</param>
  /// <returns>value is available and assigned to val</returns>
  bool HasValueInt(const std::string& key, int& val);
  bool HasValueDouble(const std::string& key, double& val);
  /// <summary>
  /// get a integer value of the parsed wkt if available, def otherwise
  /// </summary>
  /// <param name="key">key to look for like 'projcs.authority'</param>
  /// <returns>integer value of key, if key value is "abc|123" just 123 will be returned</returns>
  int ValueInt(const std::string& key, const double def = 0);
  /// <summary>
  /// get a double value of the parsed wkt if available, def otherwise
  /// </summary>
  /// <param name="key">key to look for like 'projcs.authority'</param>
  /// <returns>integer value of key, if key value is "abc|123" just 123 will be returned</returns>
  double ValueDouble(const std::string& key, const double def = 0);
  /// <summary>
  /// returns the value of a certain key/sub combination or the default value
  /// </summary>
  /// <param name="key">key to look for like 'projcs.parameter':'scale_factor'</param>
  /// <param name="sub">1st value to look for in this key like 'scale_factor'</param>
  /// <param name="def">default value</param>
  std::string ValueSubStr(const std::string& key, const std::string sub, const std::string def = "");
  /// <summary>
  /// returns the value of a certain key/sub combination or the default value
  /// </summary>
  /// <param name="key">key to look for like 'projcs.parameter':'scale_factor'</param>
  /// <param name="sub">1st value to look for in this key like 'scale_factor'</param>
  /// <param name="def">default value</param>
  int ValueSubInt(const std::string& key, const std::string sub, const int def = 0);
  /// <summary>
  /// returns the value of a certain key/sub combination or the default value
  /// </summary>
  /// <param name="key">key to look for like 'projcs.parameter':'scale_factor'</param>
  /// <param name="sub">1st value to look for in this key like 'scale_factor'</param>
  /// <param name="def">default value</param>
  double ValueSubDouble(const std::string& key, const std::string sub, const double def = 0);
};

/// <summary>
/// semantic extension for WktParser. Delivers values out of a wkt1/wkt2 string by function instead of keys
/// </summary>
class WktParserSem : public WktParser {
 public:
  WktParserSem(const std::string wktin);
  int Pcs_Epsg();
  int Vert_Epsg();
  int Gcs_Epsg();
  double Pcs_Unit();
  double Vert_Unit();
  double Gcs_Unit();
  int Vert_Unit_Epsg();
  bool HasProjection(PROJECTION_METHOD& pm);
  double ProjectionFalseEasting();
  double ProjectionFalseNorthing();
  double ProjectionLatitudeOfOrigin();
  double ProjectionLatitudeOfCenter();
  double ProjectionLongitudeOfCenter();
  double ProjectionCentralMeridian();
  double ProjectionScaleFactor();
  double ProjectionCentralStandardParallel1();
  double ProjectionCentralStandardParallel2();
};

#endif WKTPARSER_H