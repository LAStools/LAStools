/*
===============================================================================

  FILE:  geoprojectionconverter.hpp
  
  CONTENTS:

    Easy conversion between horizontal datums: UTM coodinates, Transverse
    Mercator, Lambert Conformal Conic, ECEF, Albers Equal Area Conic,
    Oblique Mercator, and Longitude/Latitude. Parameters for all standard
    US stateplanes (TM and LLC, NAD27 and NAD83) are included.

    Converting between UTM coodinates and latitude / longitude coodinates
    adapted from code written by Chuck Gantz (chuck.gantz@globalstar.com)
  
    Converting between Lambert Conformal Conic and latitude / longitude
    adapted from code written by Garrett Potts (gpotts@imagelinks.com)

    Converting between Transverse Mercator and latitude / longitude
    adapted from code written by Garrett Potts (gpotts@imagelinks.com)

    Converting between ECEF (geocentric) and latitude / longitude
    adapted from code written by Craig Larrimore (craig.larrimore@noaa.gov), B. Archinal, and C. Goad

    Converting between Albers Equal Area Conic and latitude / longitude
    adapted from code written by U.S. Army Topographic Engineering Center

    Converting between Oblique Mercator and latitude / longitude
    adapted from code written by U.S. Army Topographic Engineering Center

  PROGRAMMERS:
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
    chuck.gantz@globalstar.com
    gpotts@imagelinks.com
    craig.larrimore@noaa.gov
  
  COPYRIGHT:
  
    (c) 2007-2015, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    28 June 2015 -- tried to add the Oblique Mercator projection (incomplete)
    16 May 2015 -- added the Albers Equal Area Conic projection
    03 March 2015 -- LCC/TM custom projections write GeogGeodeticDatumGeoKey
    13 August 2014 -- added long overdue ECEF (geocentric) conversion
    08 February 2007 -- created after interviews with purdue and google
  
===============================================================================
*/
#ifndef GEO_PROJECTION_CONVERTER_HPP
#define GEO_PROJECTION_CONVERTER_HPP

struct GeoProjectionGeoKeys
{
  unsigned short key_id;
  unsigned short tiff_tag_location;
  unsigned short count;
  unsigned short value_offset;
};

#define GEO_ELLIPSOID_NAD27 5
#define GEO_ELLIPSOID_NAD83 11
#define GEO_ELLIPSOID_Inter 14
#define GEO_ELLIPSOID_SAD69 19
#define GEO_ELLIPSOID_WGS72 22
#define GEO_ELLIPSOID_WGS84 23
#define GEO_ELLIPSOID_ID74  24
#define GEO_ELLIPSOID_GDA94 GEO_ELLIPSOID_NAD83

#define GEO_VERTICAL_WGS84   5030
#define GEO_VERTICAL_NAVD29  5102
#define GEO_VERTICAL_NAVD88  5103

class GeoProjectionEllipsoid
{
public:
  int id;
  char* name;
  double equatorial_radius;
  double polar_radius;
  double eccentricity_squared;
  double inverse_flattening;
  double eccentricity_prime_squared;
  double eccentricity;
  double eccentricity_e1;
};

class GeoProjectionParameters
{
public:
  int type;
  char name[256];
  short geokey;
  GeoProjectionParameters() { type = -1; geokey = 0; };
};

class GeoProjectionParametersUTM : public GeoProjectionParameters
{
public:
  int utm_zone_number;
  char utm_zone_letter;
  bool utm_northern_hemisphere;
  int utm_long_origin;
};

class GeoProjectionParametersLCC : public GeoProjectionParameters
{
public:
  double lcc_false_easting_meter;
  double lcc_false_northing_meter;
  double lcc_lat_origin_degree;
  double lcc_long_meridian_degree;
  double lcc_first_std_parallel_degree;
  double lcc_second_std_parallel_degree;
  double lcc_lat_origin_radian;
  double lcc_long_meridian_radian;
  double lcc_first_std_parallel_radian;
  double lcc_second_std_parallel_radian;
  double lcc_n;
  double lcc_aF;
  double lcc_rho0;
};

class GeoProjectionParametersTM : public GeoProjectionParameters
{
public:
  double tm_false_easting_meter;
  double tm_false_northing_meter;
  double tm_lat_origin_degree;
  double tm_long_meridian_degree;
  double tm_scale_factor;
  double tm_lat_origin_radian;
  double tm_long_meridian_radian;
  double tm_ap;
  double tm_bp;
  double tm_cp;
  double tm_dp;
  double tm_ep;
};

class GeoProjectionParametersAEAC : public GeoProjectionParameters
{
public:
  double aeac_false_easting_meter;
  double aeac_false_northing_meter;
  double aeac_lat_origin_degree;
  double aeac_long_meridian_degree;
  double aeac_first_std_parallel_degree;
  double aeac_second_std_parallel_degree;
  double aeac_lat_origin_radian;
  double aeac_long_meridian_radian;
  double aeac_first_std_parallel_radian;
  double aeac_second_std_parallel_radian;
  double aeac_n;
  double aeac_C;
  double aeac_two_es;
  double aeac_rho0;
  double aeac_one_MINUS_es2;
  double aeac_Albers_a_OVER_n;
};

class GeoProjectionParametersOM : public GeoProjectionParameters
{
public:
  double om_false_easting_meter;
  double om_false_northing_meter;
  double om_lat_origin_degree;
  double om_long_meridian_degree;
  double om_first_std_parallel_degree;
  double om_second_std_parallel_degree;
  double om_lat_origin_radian;
  double om_long_meridian_radian;
  double om_first_std_parallel_radian;
  double om_second_std_parallel_radian;
  double om_n;
  double om_C;
  double om_two_es;
  double om_rho0;
  double om_one_MINUS_es2;
  double om_Albers_a_OVER_n;
};

class GeoProjectionConverter
{
public:

  // parse command line arguments

  bool parse(int argc, char* argv[]);
  int unparse(char* string) const;

  // set & get current projection

  bool set_projection_from_geo_keys(int num_geo_keys, GeoProjectionGeoKeys* geo_keys, char* geo_ascii_params, double* geo_double_params, char* description=0);
  bool get_geo_keys_from_projection(int& num_geo_keys, GeoProjectionGeoKeys** geo_keys, int& num_geo_double_params, double** geo_double_params, bool source=true);
  short get_GTModelTypeGeoKey();
  short get_GTRasterTypeGeoKey();
  short get_GeographicTypeGeoKey(bool source=true);
  short get_GeogGeodeticDatumGeoKey(bool source=true);
  short get_GeogPrimeMeridianGeoKey();
  short get_GeogLinearUnitsGeoKey();
  double get_GeogLinearUnitSizeGeoKey();
  short get_GeogAngularUnitsGeoKey();
  double get_GeogAngularUnitSizeGeoKey();
  short get_GeogEllipsoidGeoKey(bool source=true);
  double get_GeogSemiMajorAxisGeoKey();
  double get_GeogSemiMinorAxisGeoKey();
  double get_GeogInvFlatteningGeoKey();
  short get_GeogAzimuthUnitsGeoKey();
  double get_GeogPrimeMeridianLongGeoKey();

  bool set_ProjectedCSTypeGeoKey(short value, char* description=0);
  short get_ProjectedCSTypeGeoKey(bool source=true);

  void set_ProjLinearUnitsGeoKey(short value);
  short get_ProjLinearUnitsGeoKey(bool source=true);

  void set_VerticalUnitsGeoKey(short value);
  short get_VerticalUnitsGeoKey(bool source=true);

  void set_VerticalCSTypeGeoKey(short value);
  short get_VerticalCSTypeGeoKey();

  bool set_reference_ellipsoid(int id, char* description=0);
  int get_ellipsoid_id() const;
  const char* get_ellipsoid_name() const;

  bool set_no_projection(char* description=0, bool source=true);
  bool set_latlong_projection(char* description=0, bool source=true);
  bool set_longlat_projection(char* description=0, bool source=true);

  bool set_ecef_projection(char* description, bool source=true);

  bool set_target_utm_projection(char* description);
  bool set_utm_projection(char* zone, char* description=0, bool source=true);
  bool set_utm_projection(int zone, bool northern, char* description=0, bool source=true);
  void set_lambert_conformal_conic_projection(double falseEastingMeter, double falseNorthingMeter, double latOriginDegree, double longMeridianDegree, double firstStdParallelDegree, double secondStdParallelDegree, char* description=0, bool source=true);
  void set_transverse_mercator_projection(double falseEastingMeter, double falseNorthingMeter, double latOriginDegree, double longMeridianDegree, double scaleFactor, char* description=0, bool source=true);
  void set_albers_equal_area_conic_projection(double falseEastingMeter, double falseNorthingMeter, double latOriginDegree, double longMeridianDegree, double firstStdParallelDegree, double secondStdParallelDegree, char* description=0, bool source=true);
  void set_oblique_mercator_projection(double falseEastingMeter, double falseNorthingMeter, double latOriginDegree, double longMeridianDegree, double firstStdParallelDegree, double secondStdParallelDegree, char* description=0, bool source=true);

  const char* get_state_plane_nad27_lcc_zone(int i) const;
  bool set_state_plane_nad27_lcc(const char* zone, char* description=0, bool source=true);
  void print_all_state_plane_nad27_lcc() const;
  const char* get_state_plane_nad83_lcc_zone(int i) const;
  bool set_state_plane_nad83_lcc(const char* zone, char* description=0, bool source=true);
  void print_all_state_plane_nad83_lcc() const;

  const char* get_state_plane_nad27_tm_zone(int i) const;
  bool set_state_plane_nad27_tm(const char* zone, char* description=0, bool source=true);
  void print_all_state_plane_nad27_tm() const;
  const char* get_state_plane_nad83_tm_zone(int i) const;
  bool set_state_plane_nad83_tm(const char* zone, char* description=0, bool source=true);
  void print_all_state_plane_nad83_tm() const;

  const char* get_epsg_code_description(int i) const;
  short get_epsg_code_value(int i) const;
  bool set_epsg_code(short code, char* description=0, bool source=true);
  void print_all_epsg_codes() const;

  void reset_projection(bool source=true);
  bool has_projection(bool source=true) const;
  const char* get_projection_name(bool source=true) const;

  void set_coordinates_in_survey_feet(bool source=true);
  void set_coordinates_in_feet(bool source=true);
  void set_coordinates_in_meter(bool source=true);

  void reset_coordinate_units(bool source=true);
  bool has_coordinate_units(bool source=true) const;
  const char* get_coordinate_unit_description_string(bool abrev=true, bool source=true);

  void set_elevation_in_survey_feet(bool source=true);
  void set_elevation_in_feet(bool source=true);
  void set_elevation_in_meter(bool source=true);

  void reset_elevation_units(bool source=true);
  bool has_elevation_units(bool source=true) const;
  const char* get_elevation_unit_description_string(bool abrev=true, bool source=true);

  void set_elevation_offset_in_meter(float elevation_offset);

  // specific conversion routines

  bool compute_utm_zone(const double LatDegree, const double LongDegree, GeoProjectionParametersUTM* utm) const;
  bool UTMtoLL(const double UTMEastingMeter, const double UTMNorthingMeter, double& LatDegree,  double& LongDegree, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersUTM* utm) const;
  bool LLtoUTM(const double LatDegree, const double LongDegree, double &UTMEastingMeter, double &UTMNorthingMeter, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersUTM* utm) const;

  bool LCCtoLL(const double LCCEastingMeter, const double LCCNorthingMeter, double& LatDegree,  double& LongDegree, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersLCC* lcc) const;
  bool LLtoLCC(const double LatDegree, const double LongDegree, double &LCCEastingMeter, double &LCCNorthingMeter, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersLCC* lcc) const;

  bool TMtoLL(const double TMEastingMeter, const double TMNorthingMeter, double& LatDegree,  double& LongDegree, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersTM* tm) const;
  bool LLtoTM(const double LatDegree, const double LongDegree, double &TMEastingMeter, double &TMNorthingMeter, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersTM* tm) const;

  bool ECEFtoLL(const double ECEFMeterX, const double ECEFMeterY, const double ECEFMeterZ, double& LatDegree,  double& LongDegree, double& ElevationMeter, const GeoProjectionEllipsoid* ellipsoid) const;
  bool LLtoECEF(const double LatDegree, const double LongDegree, const double ElevationMeter, double &ECEFMeterX, double &ECEFMeterY, double &ECEFMeterZ, const GeoProjectionEllipsoid* ellipsoid) const;

  bool AEACtoLL(const double AEACEastingMeter, const double AEACNorthingMeter, double& LatDegree, double& LongDegree, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersAEAC* aeac) const;
  bool LLtoAEAC(const double LatDegree, const double LongDegree, double &AEACEastingMeter, double &AEACNorthingMeter, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersAEAC* aeac) const;

  bool OMtoLL(const double OMEastingMeter, const double OMNorthingMeter, double& LatDegree, double& LongDegree, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersOM* om) const;
  bool LLtoOM(const double LatDegree, const double LongDegree, double &OMEastingMeter, double &OMNorthingMeter, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersOM* om) const;

  GeoProjectionConverter();
  ~GeoProjectionConverter();

  // from current projection to longitude/latitude/elevation_in_meter

  bool to_lon_lat_ele(double* point) const;
  bool to_lon_lat_ele(const double* point, double& longitude, double& latitude, double& elevation_in_meter) const;

  // from current projection to target projection 

  bool to_target(double* point) const;
  bool to_target(const double* point, double& x, double& y, double& elevation) const;

  double get_target_precision() const;
  void set_target_precision(double target_precision);

  double get_target_elevation_precision() const;
  void set_target_elevation_precision(double target_elevation_precision);

  // for interfacing with common geo-spatial formats

//  int get_img_projection_number(bool source=true) const;
  bool get_dtm_projection_parameters(short* horizontal_units, short* vertical_units, short* coordinate_system, short* coordinate_zone, short* horizontal_datum, short* vertical_datum, bool source=true);
  bool set_dtm_projection_parameters(short horizontal_units, short vertical_units, short coordinate_system, short coordinate_zone, short horizontal_datum, short vertical_datum, bool source=true);
  
private:
  // parameters for gtiff
  int num_geo_keys;
  GeoProjectionGeoKeys* geo_keys;
  char* geo_ascii_params;
  double* geo_double_params;

  // parameters for the reference ellipsoid
  GeoProjectionEllipsoid* ellipsoid;

  // parameters for the projection
  GeoProjectionParameters* source_projection;
  GeoProjectionParameters* target_projection;

  // vertical coordinate system
  short vertical_geokey;

  // parameters for coordinate scaling
  bool coordinate_units_set[2];
  double coordinates2meter, meter2coordinates;
  bool elevation_units_set[2];
  double elevation2meter, meter2elevation;
  float elevation_offset_in_meter;

  double target_precision;
  double target_elevation_precision;

  // helper functions
  void set_projection(GeoProjectionParameters* projection, bool source);
  void set_geokey(short geokey, bool source);
  void check_geokey(short geokey, bool source);
  GeoProjectionParameters* get_projection(bool source) const;
  void compute_lcc_parameters(bool source);
  void compute_tm_parameters(bool source);
  void compute_aeac_parameters(bool source);
  void compute_om_parameters(bool source);
};

#endif
