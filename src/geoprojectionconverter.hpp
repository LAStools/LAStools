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

    Converting between Oblique Stereographic and latitude / longitude
    formulas from "Oblique Stereographic Alternative" by Gerald Evenden and Rueben Schulz

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
    chuck.gantz@globalstar.com
    gpotts@imagelinks.com
    craig.larrimore@noaa.gov

  COPYRIGHT:

    (c) 2007-2017, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

     1 November 2018 -- changes requested by Kirk Waters including GEO_GCS_NAD83_CORS96
     7 September 2018 -- introduced the LASCopyString macro to replace _strdup
    30 October 2017 -- '-vertical_evrf2007' for European Vertical Reference Frame 2007
     1 February 2017 -- set_projection_from_ogc_wkt() from EPSG code of OGC WKT string
     9 November 2016 -- support "user defined" AlbersEqualArea projection in GeoTIFF
    30 July 2016 -- no more special handling for stateplanes. just parse for EPSG code
     9 January 2016 -- use GeographicTypeGeoKey not GeogGeodeticDatumGeoKey for custom
     2 January 2016 -- parse 'pcs.csv' file when unknown EPSG code is encountered
    28 June 2015 -- tried to add the Oblique Mercator projection (very incomplete)
    16 May 2015 -- added the Albers Equal Area Conic projection
     3 March 2015 -- LCC/TM custom projections write GeogGeodeticDatumGeoKey
    13 August 2014 -- added long overdue ECEF (geocentric) conversion
     8 February 2007 -- created after interviews with purdue and google

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

#define GEO_ELLIPSOID_AIRY            1
#define GEO_ELLIPSOID_BESSEL_1841     3
#define GEO_ELLIPSOID_BESSEL_NAMIBIA  4
#define GEO_ELLIPSOID_CLARKE1866      5
#define GEO_ELLIPSOID_CLARKE1880      6
#define GEO_ELLIPSOID_GRS1967        10
#define GEO_ELLIPSOID_GRS1980        11
#define GEO_ELLIPSOID_INTERNATIONAL  14
#define GEO_ELLIPSOID_KRASSOWSKY     15
#define GEO_ELLIPSOID_WGS72          22
#define GEO_ELLIPSOID_WGS84          23
#define GEO_ELLIPSOID_ID74           24

#define GEO_GCS_CH1903             4149
#define GEO_GCS_NAD83_HARN         4152
#define GEO_GCS_NZGD2000           4167
#define GEO_GCS_HD72               4237
#define GEO_GCS_ETRS89             4258
#define GEO_GCS_NAD27              4267
#define GEO_GCS_NAD83              4269
#define GEO_GCS_OSGB1936           4277
#define GEO_GCS_GDA94              4283
#define GEO_GCS_SAD69              4291
#define GEO_GCS_WGS72              4322
#define GEO_GCS_WGS72BE            4324
#define GEO_GCS_WGS84              4326
#define GEO_GCS_NAD83_CSRS         4617
#define GEO_GCS_SWEREF99           4619
#define GEO_GCS_NAD83_NSRS2007     4759
#define GEO_GCS_NAD83_2011         6318
#define GEO_GCS_NAD83_PA11         6322
#define GEO_GCS_NAD83_CORS96       6783

#define GEO_SPHEROID_AIRY          7001
#define GEO_SPHEROID_BESSEL1841    7004
#define GEO_SPHEROID_CLARKE1866    7008
#define GEO_SPHEROID_GRS80         7019
#define GEO_SPHEROID_INTERNATIONAL 7022
#define GEO_SPHEROID_WGS84         7030
#define GEO_SPHEROID_GRS67         7036
#define GEO_SPHEROID_WGS72         7043

#define GEO_VERTICAL_WGS84         5030
#define GEO_VERTICAL_NGVD29        5102
#define GEO_VERTICAL_NAVD88        5103
#define GEO_VERTICAL_CGVD28        5114
#define GEO_VERTICAL_DVR90         5206
#define GEO_VERTICAL_EVRF2007      5215
#define GEO_VERTICAL_NN54          5776
#define GEO_VERTICAL_DHHN92        5783
#define GEO_VERTICAL_NN2000        5941
#define GEO_VERTICAL_CGVD2013      6647 
#define GEO_VERTICAL_DHHN2016      7837
#define GEO_VERTICAL_NZVD2016      7839 

#define GEO_VERTICAL_NAVD88_GEOID96   965103
#define GEO_VERTICAL_NAVD88_GEOID99   995103
#define GEO_VERTICAL_NAVD88_GEOID03  1035103
#define GEO_VERTICAL_NAVD88_GEOID06  1065103
#define GEO_VERTICAL_NAVD88_GEOID09  1095103
#define GEO_VERTICAL_NAVD88_GEOID12  1125103
#define GEO_VERTICAL_NAVD88_GEOID12A 1135103
#define GEO_VERTICAL_NAVD88_GEOID12B 1145103

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
  double aeac_latitude_of_center_degree;
  double aeac_longitude_of_center_degree;
  double aeac_first_std_parallel_degree;
  double aeac_second_std_parallel_degree;
  double aeac_latitude_of_center_radian;
  double aeac_longitude_of_center_radian;
  double aeac_first_std_parallel_radian;
  double aeac_second_std_parallel_radian;
  double aeac_n;
  double aeac_C;
  double aeac_two_es;
  double aeac_rho0;
  double aeac_one_MINUS_es2;
  double aeac_Albers_a_OVER_n;
};

class GeoProjectionParametersHOM : public GeoProjectionParameters
{
public:
  double hom_false_easting_meter;
  double hom_false_northing_meter;
  double hom_latitude_of_center_degree;
  double hom_longitude_of_center_degree;
  double hom_azimuth_degree;
  double hom_rectified_grid_angle_degree;
  double hom_scale_factor;
  double hom_latitude_of_center_radian;
  double hom_longitude_of_center_radian;
  double hom_azimuth_radian;
  double hom_rectified_grid_angle_radian;

  double hom_A;
  double hom_B;
  double hom_H;
  double hom_g0;
  double hom_l0;
};

class GeoProjectionParametersOS : public GeoProjectionParameters
{
public:
  double os_false_easting_meter;
  double os_false_northing_meter;
  double os_lat_origin_degree;
  double os_long_meridian_degree;
  double os_scale_factor;
  double os_lat_origin_radian;
  double os_long_meridian_radian;
  double os_R2;
  double os_C;
  double os_phic0;
  double os_sinc0;
  double os_cosc0;
  double os_ratexp;
  double os_K;
  double os_gf;
};

class GeoProjectionConverter
{
public:

  // parse command line arguments

  bool parse(int argc, char* argv[]);
  int unparse(char* string) const;

  // set & get current projection

  bool set_projection_from_geo_keys(int num_geo_keys, const GeoProjectionGeoKeys* geo_keys, char* geo_ascii_params, double* geo_double_params, char* description=0);
  bool get_geo_keys_from_projection(int& num_geo_keys, GeoProjectionGeoKeys** geo_keys, int& num_geo_double_params, double** geo_double_params, bool source=true);
  bool set_projection_from_ogc_wkt(const char* ogc_wkt, char* description=0);
  bool get_ogc_wkt_from_projection(int& len, char** ogc_wkt, bool source=true);
  bool get_prj_from_projection(int& len, char** prj, bool source=true);
  bool get_proj4_string_from_projection(int& len, char** proj4, bool source=true);

  short get_GTRasterTypeGeoKey() const;
  short get_GeographicTypeGeoKey() const;
  short get_GeogGeodeticDatumGeoKey() const;
  short get_GeogPrimeMeridianGeoKey() const;
  short get_GeogLinearUnitsGeoKey() const;
  double get_GeogLinearUnitSizeGeoKey() const;
  short get_GeogAngularUnitsGeoKey() const;
  double get_GeogAngularUnitSizeGeoKey() const;
  double get_GeogSemiMajorAxisGeoKey() const;
  double get_GeogSemiMinorAxisGeoKey() const;
  double get_GeogInvFlatteningGeoKey() const;
  short get_GeogAzimuthUnitsGeoKey() const;
  double get_GeogPrimeMeridianLongGeoKey() const;

  bool set_GTModelTypeGeoKey(short value, char* description=0);
  short get_GTModelTypeGeoKey() const;

  bool set_ProjectedCSTypeGeoKey(short value, char* description=0);
  short get_ProjectedCSTypeGeoKey(bool source=true) const;

  int set_GeogEllipsoidGeoKey(short value);
  short get_GeogEllipsoidGeoKey() const;

  bool set_ProjLinearUnitsGeoKey(short value, bool source=true);
  short get_ProjLinearUnitsGeoKey(bool source=true) const;

  bool set_VerticalUnitsGeoKey(short value);
  short get_VerticalUnitsGeoKey(bool source=true) const;

  bool set_VerticalCSTypeGeoKey(short value, char* description=0);
  short get_VerticalCSTypeGeoKey();

  bool set_reference_ellipsoid(int id, char* description=0);
  int get_ellipsoid_id() const;
  const char* get_ellipsoid_name() const;

  bool set_gcs(short code, char* description=0);
  const char* get_gcs_name() const;

  bool set_no_projection(char* description=0, bool source=true);
  bool set_latlong_projection(char* description=0, bool source=true);
  bool set_longlat_projection(char* description=0, bool source=true);

  bool set_ecef_projection(char* description, bool source=true, const char* name=0);

  bool set_target_utm_projection(char* description, const char* name=0);
  bool set_utm_projection(char* zone, char* description=0, bool source=true, const char* name=0, bool is_mga=false);
  bool set_utm_projection(int zone, bool northern, char* description=0, bool source=true, const char* name=0, bool is_mga=false);
  void set_lambert_conformal_conic_projection(double falseEastingMeter, double falseNorthingMeter, double latOriginDegree, double longMeridianDegree, double firstStdParallelDegree, double secondStdParallelDegree, char* description=0, bool source=true, const char* name=0);
  void set_transverse_mercator_projection(double falseEastingMeter, double falseNorthingMeter, double latOriginDegree, double longMeridianDegree, double scaleFactor, char* description=0, bool source=true, const char* name=0);
  void set_albers_equal_area_conic_projection(double falseEastingMeter, double falseNorthingMeter, double latCenterDegree, double longCenterDegree, double firstStdParallelDegree, double secondStdParallelDegree, char* description=0, bool source=true, const char* name=0);
  void set_hotine_oblique_mercator_projection(double falseEastingMeter, double falseNorthingMeter, double latCenterDegree, double longCenterDegree, double azimuthDegree, double rectifiedGridAngleDegree, double scaleFactor, char* description=0, bool source=true, const char* name=0);
  void set_oblique_stereographic_projection(double falseEastingMeter, double falseNorthingMeter, double latOriginDegree, double longMeridianDegree, double scaleFactor, char* description=0, bool source=true, const char* name=0);

  const char* get_state_plane_nad27_lcc_zone(int i) const;
  bool set_state_plane_nad27_lcc(const char* zone, char* description=0, bool source=true, const char* name=0);
  void print_all_state_plane_nad27_lcc() const;
  const char* get_state_plane_nad83_lcc_zone(int i) const;
  bool set_state_plane_nad83_lcc(const char* zone, char* description=0, bool source=true, const char* name=0);
  void print_all_state_plane_nad83_lcc() const;

  const char* get_state_plane_nad27_tm_zone(int i) const;
  bool set_state_plane_nad27_tm(const char* zone, char* description=0, bool source=true, const char* name=0);
  void print_all_state_plane_nad27_tm() const;
  const char* get_state_plane_nad83_tm_zone(int i) const;
  bool set_state_plane_nad83_tm(const char* zone, char* description=0, bool source=true, const char* name=0);
  void print_all_state_plane_nad83_tm() const;

  bool set_epsg_code(short code, char* description=0, bool source=true);

  void reset_projection(bool source=true);
  bool has_projection(bool source=true) const;
  const char* get_projection_name(bool source=true) const;

  void set_coordinates_in_survey_feet(bool source=true);
  void set_coordinates_in_feet(bool source=true);
  void set_coordinates_in_meter(bool source=true);

  void reset_coordinate_units(bool source=true);
  bool has_coordinate_units(bool source=true) const;
  const char* get_coordinate_unit_description_string(bool abrev=true, bool source=true) const;

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

  bool HOMtoLL(const double HOMEastingMeter, const double HOMNorthingMeter, double& LatDegree, double& LongDegree, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersHOM* hom) const;
  bool LLtoHOM(const double LatDegree, const double LongDegree, double &HOMEastingMeter, double &HOMNorthingMeter, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersHOM* hom) const;

  bool OStoLL(const double OSEastingMeter, const double OSNorthingMeter, double& LatDegree,  double& LongDegree, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersOS* os) const;
  bool LLtoOS(const double LatDegree, const double LongDegree, double& OSEastingMeter,  double& OSNorthingMeter, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersOS* os) const;

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

  // helps us to find the 'pcs.csv' file
  char* argv_zero;

private:

  // parameters for gtiff
  int num_geo_keys;
  GeoProjectionGeoKeys* geo_keys;
  char* geo_ascii_params;
  double* geo_double_params;

  // codes and names according to EPSG
  int gcs_code;
  char gcs_name[28];
  int datum_code;
  char datum_name[60];
  int spheroid_code;

  // parameters for the reference ellipsoid
  GeoProjectionEllipsoid* ellipsoid;

  // parameters for the projection
  GeoProjectionParameters* source_projection;
  GeoProjectionParameters* target_projection;

  // vertical coordinate system
  short vertical_geokey;
  int vertical_geoid;

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
  void compute_hom_parameters(bool source);
  void compute_os_parameters(bool source);
};

#endif
