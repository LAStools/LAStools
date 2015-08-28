/*
===============================================================================

  FILE:  geoprojectionconverter.cpp
  
  CONTENTS:
  
    see corresponding header file
  
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
  
    see corresponding header file
  
===============================================================================
*/
#include "geoprojectionconverter.hpp"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const double PI = 3.141592653589793238462643383279502884197169;
static const double TWO_PI = PI * 2;
static const double PI_OVER_2 = PI / 2;
static const double PI_OVER_4 = PI / 4;
static const double deg2rad = PI / 180.0;
static const double rad2deg = 180.0 / PI;

static const double feet2meter = 0.3048;
static const double surveyfeet2meter = 0.3048006096012;

static const int GEO_PROJECTION_UTM      = 0;
static const int GEO_PROJECTION_LCC      = 1;
static const int GEO_PROJECTION_TM       = 2;
static const int GEO_PROJECTION_LONG_LAT = 3;
static const int GEO_PROJECTION_LAT_LONG = 4;
static const int GEO_PROJECTION_ECEF     = 5;
static const int GEO_PROJECTION_AEAC     = 6;
static const int GEO_PROJECTION_OM       = 7;
static const int GEO_PROJECTION_NONE     = 8;

class ReferenceEllipsoid
{
public:
  ReferenceEllipsoid(int id, char* name, double equatorialRadius, double eccentricitySquared, double inverseFlattening)
  {
    this->id = id;
    this->name = name; 
    this->equatorialRadius = equatorialRadius;
    this->eccentricitySquared = eccentricitySquared;
    this->inverseFlattening = inverseFlattening;
  }
  int id;
  char* name;
  double equatorialRadius; 
  double eccentricitySquared;  
  double inverseFlattening;  
};

static const ReferenceEllipsoid ellipsoid_list[] = 
{
  //  d, Ellipsoid name, Equatorial Radius, square of eccentricity, inverse flattening  
  ReferenceEllipsoid( -1, "Placeholder", 0, 0, 0),  //placeholder to allow array indices to match id numbers
  ReferenceEllipsoid( 1, "Airy", 6377563.396, 0.00667054, 299.3249646),
  ReferenceEllipsoid( 2, "Australian National", 6378160.0, 0.006694542, 298.25),
  ReferenceEllipsoid( 3, "Bessel 1841", 6377397.155, 0.006674372, 299.1528128),
  ReferenceEllipsoid( 4, "Bessel 1841 (Nambia) ", 6377483.865, 0.006674372, 299.1528128),
  ReferenceEllipsoid( 5, "Clarke 1866 (NAD-27)", 6378206.4, 0.006768658, 294.9786982),
  ReferenceEllipsoid( 6, "Clarke 1880", 6378249.145, 0.006803511, 293.465),
  ReferenceEllipsoid( 7, "Everest 1830", 6377276.345, 0.006637847, 300.8017),
  ReferenceEllipsoid( 8, "Fischer 1960 (Mercury) ", 6378166, 0.006693422, 298.3),
  ReferenceEllipsoid( 9, "Fischer 1968", 6378150, 0.006693422, 298.3),
  ReferenceEllipsoid( 10, "GRS 1967", 6378160, 0.006694605, 298.247167427),
  ReferenceEllipsoid( 11, "GRS 1980 (NAD-83,GDA-94)", 6378137, 0.00669438002290, 298.257222101),
  ReferenceEllipsoid( 12, "Helmert 1906", 6378200, 0.006693422, 298.3),
  ReferenceEllipsoid( 13, "Hough", 6378270, 0.00672267, 297.0),
  ReferenceEllipsoid( 14, "International", 6378388, 0.00672267, 297.0),
  ReferenceEllipsoid( 15, "Krassovsky", 6378245, 0.006693422, 298.3),
  ReferenceEllipsoid( 16, "Modified Airy", 6377340.189, 0.00667054, 299.3249646),
  ReferenceEllipsoid( 17, "Modified Everest", 6377304.063, 0.006637847, 300.8017),
  ReferenceEllipsoid( 18, "Modified Fischer 1960", 6378155, 0.006693422, 298.3),
  ReferenceEllipsoid( 19, "South American 1969", 6378160, 0.006694542, 298.25),
  ReferenceEllipsoid( 20, "WGS 60", 6378165, 0.006693422, 298.3),
  ReferenceEllipsoid( 21, "WGS 66", 6378145, 0.006694542, 298.25),
  ReferenceEllipsoid( 22, "WGS-72", 6378135, 0.006694318, 298.26),
  ReferenceEllipsoid( 23, "WGS-84", 6378137, 0.00669437999013, 298.257223563),
  ReferenceEllipsoid( 24, "Indonesian National 1974", 6378160, 0.0066946091071419115, 298.2469988070381)
};

static const short PCS_NAD27_Alabama_East = 26729;
static const short PCS_NAD27_Alabama_West = 26730;
static const short PCS_NAD27_Alaska_zone_1 = 26731; /* Hotine Oblique Mercator Projection not supported*/
static const short PCS_NAD27_Alaska_zone_2 = 26732;
static const short PCS_NAD27_Alaska_zone_3 = 26733;
static const short PCS_NAD27_Alaska_zone_4 = 26734;
static const short PCS_NAD27_Alaska_zone_5 = 26735;
static const short PCS_NAD27_Alaska_zone_6 = 26736;
static const short PCS_NAD27_Alaska_zone_7 = 26737;
static const short PCS_NAD27_Alaska_zone_8 = 26738;
static const short PCS_NAD27_Alaska_zone_9 = 26739;
static const short PCS_NAD27_Alaska_zone_10 = 26740;
static const short PCS_NAD27_California_I = 26741;
static const short PCS_NAD27_California_II = 26742;
static const short PCS_NAD27_California_III = 26743;
static const short PCS_NAD27_California_IV = 26744;
static const short PCS_NAD27_California_V = 26745;
static const short PCS_NAD27_California_VI = 26746;
static const short PCS_NAD27_California_VII = 26747;
static const short PCS_NAD27_Arizona_East = 26748;
static const short PCS_NAD27_Arizona_Central = 26749;
static const short PCS_NAD27_Arizona_West = 26750;
static const short PCS_NAD27_Arkansas_North = 26751;
static const short PCS_NAD27_Arkansas_South = 26752;
static const short PCS_NAD27_Colorado_North = 26753;
static const short PCS_NAD27_Colorado_Central = 26754;
static const short PCS_NAD27_Colorado_South = 26755;
static const short PCS_NAD27_Connecticut = 26756;
static const short PCS_NAD27_Delaware = 26757;
static const short PCS_NAD27_Florida_East = 26758;
static const short PCS_NAD27_Florida_West = 26759;
static const short PCS_NAD27_Florida_North = 26760;
static const short PCS_NAD27_Hawaii_zone_1 = 26761;
static const short PCS_NAD27_Hawaii_zone_2 = 26762;
static const short PCS_NAD27_Hawaii_zone_3 = 26763;
static const short PCS_NAD27_Hawaii_zone_4 = 26764;
static const short PCS_NAD27_Hawaii_zone_5 = 26765;
static const short PCS_NAD27_Georgia_East = 26766;
static const short PCS_NAD27_Georgia_West = 26767;
static const short PCS_NAD27_Idaho_East = 26768;
static const short PCS_NAD27_Idaho_Central = 26769;
static const short PCS_NAD27_Idaho_West = 26770;
static const short PCS_NAD27_Illinois_East = 26771;
static const short PCS_NAD27_Illinois_West = 26772;
static const short PCS_NAD27_Indiana_East = 26773;
static const short PCS_NAD27_Indiana_West = 26774;
static const short PCS_NAD27_Iowa_North = 26775;
static const short PCS_NAD27_Iowa_South = 26776;
static const short PCS_NAD27_Kansas_North = 26777;
static const short PCS_NAD27_Kansas_South = 26778;
static const short PCS_NAD27_Kentucky_North = 26779;
static const short PCS_NAD27_Kentucky_South = 26780;
static const short PCS_NAD27_Louisiana_North = 26781;
static const short PCS_NAD27_Louisiana_South = 26782;
static const short PCS_NAD27_Maine_East = 26783;
static const short PCS_NAD27_Maine_West = 26784;
static const short PCS_NAD27_Maryland = 26785;
static const short PCS_NAD27_Massachusetts = 26786;
static const short PCS_NAD27_Massachusetts_Is = 26787;
static const short PCS_NAD27_Michigan_North = 26788;
static const short PCS_NAD27_Michigan_Central = 26789;
static const short PCS_NAD27_Michigan_South = 26790;
static const short PCS_NAD27_Minnesota_North = 26791;
static const short PCS_NAD27_Minnesota_Central = 26792;
static const short PCS_NAD27_Minnesota_South = 26793;
static const short PCS_NAD27_Mississippi_East = 26794;
static const short PCS_NAD27_Mississippi_West = 26795;
static const short PCS_NAD27_Missouri_East = 26796;
static const short PCS_NAD27_Missouri_Central = 26797;
static const short PCS_NAD27_Missouri_West = 26798;
static const short PCS_NAD27_Montana_North = 32001;
static const short PCS_NAD27_Montana_Central = 32002;
static const short PCS_NAD27_Montana_South = 32003;
static const short PCS_NAD27_Nebraska_North = 32005;
static const short PCS_NAD27_Nebraska_South = 32006;
static const short PCS_NAD27_Nevada_East = 32007;
static const short PCS_NAD27_Nevada_Central = 32008;
static const short PCS_NAD27_Nevada_West = 32009;
static const short PCS_NAD27_New_Hampshire = 32010;
static const short PCS_NAD27_New_Jersey = 32011;
static const short PCS_NAD27_New_Mexico_East = 32012;
static const short PCS_NAD27_New_Mexico_Central = 32013;
static const short PCS_NAD27_New_Mexico_West = 32014;
static const short PCS_NAD27_New_York_East = 32015;
static const short PCS_NAD27_New_York_Central = 32016;
static const short PCS_NAD27_New_York_West = 32017;
static const short PCS_NAD27_New_York_Long_Is = 32018;
static const short PCS_NAD27_North_Carolina = 32019;
static const short PCS_NAD27_North_Dakota_N = 32020;
static const short PCS_NAD27_North_Dakota_S = 32021;
static const short PCS_NAD27_Ohio_North = 32022;
static const short PCS_NAD27_Ohio_South = 32023;
static const short PCS_NAD27_Oklahoma_North = 32024;
static const short PCS_NAD27_Oklahoma_South = 32025;
static const short PCS_NAD27_Oregon_North = 32026;
static const short PCS_NAD27_Oregon_South = 32027;
static const short PCS_NAD27_Pennsylvania_N = 32028;
static const short PCS_NAD27_Pennsylvania_S = 32029;
static const short PCS_NAD27_Rhode_Island = 32030;
static const short PCS_NAD27_South_Carolina_N = 32031;
static const short PCS_NAD27_South_Carolina_S = 32033;
static const short PCS_NAD27_South_Dakota_N = 32034;
static const short PCS_NAD27_South_Dakota_S = 32035;
static const short PCS_NAD27_Tennessee = 2204;
static const short PCS_NAD27_Texas_North = 32037;
static const short PCS_NAD27_Texas_North_Central = 32038;
static const short PCS_NAD27_Texas_Central = 32039;
static const short PCS_NAD27_Texas_South_Central = 32040;
static const short PCS_NAD27_Texas_South = 32041;
static const short PCS_NAD27_Utah_North = 32042;
static const short PCS_NAD27_Utah_Central = 32043;
static const short PCS_NAD27_Utah_South = 32044;
static const short PCS_NAD27_Vermont = 32045;
static const short PCS_NAD27_Virginia_North = 32046;
static const short PCS_NAD27_Virginia_South = 32047;
static const short PCS_NAD27_Washington_North = 32048;
static const short PCS_NAD27_Washington_South = 32049;
static const short PCS_NAD27_West_Virginia_N = 32050;
static const short PCS_NAD27_West_Virginia_S = 32051;
static const short PCS_NAD27_Wisconsin_North = 32052;
static const short PCS_NAD27_Wisconsin_Central = 32053;
static const short PCS_NAD27_Wisconsin_South = 32054;
static const short PCS_NAD27_Wyoming_East = 32055;
static const short PCS_NAD27_Wyoming_East_Central = 32056;
static const short PCS_NAD27_Wyoming_West_Central = 32057;
static const short PCS_NAD27_Wyoming_West = 32058;
static const short PCS_NAD27_Puerto_Rico = 32059;
static const short PCS_NAD27_St_Croix = 32060;

static const short PCS_NAD83_Alabama_East = 26929;
static const short PCS_NAD83_Alabama_West = 26930;
static const short PCS_NAD83_Alaska_zone_1 = 26931; /* Hotine Oblique Mercator Projection not supported*/
static const short PCS_NAD83_Alaska_zone_2 = 26932;
static const short PCS_NAD83_Alaska_zone_3 = 26933;
static const short PCS_NAD83_Alaska_zone_4 = 26934;
static const short PCS_NAD83_Alaska_zone_5 = 26935;
static const short PCS_NAD83_Alaska_zone_6 = 26936;
static const short PCS_NAD83_Alaska_zone_7 = 26937;
static const short PCS_NAD83_Alaska_zone_8 = 26938;
static const short PCS_NAD83_Alaska_zone_9 = 26939;
static const short PCS_NAD83_Alaska_zone_10 = 26940;
static const short PCS_NAD83_California_1 = 26941;
static const short PCS_NAD83_California_2 = 26942;
static const short PCS_NAD83_California_3 = 26943;
static const short PCS_NAD83_California_4 = 26944;
static const short PCS_NAD83_California_5 = 26945;
static const short PCS_NAD83_California_6 = 26946;
static const short PCS_NAD83_Arizona_East = 26948;
static const short PCS_NAD83_Arizona_Central = 26949;
static const short PCS_NAD83_Arizona_West = 26950;
static const short PCS_NAD83_Arkansas_North = 26951;
static const short PCS_NAD83_Arkansas_South = 26952;
static const short PCS_NAD83_Colorado_North = 26953;
static const short PCS_NAD83_Colorado_Central = 26954;
static const short PCS_NAD83_Colorado_South = 26955;
static const short PCS_NAD83_Connecticut = 26956;
static const short PCS_NAD83_Delaware = 26957;
static const short PCS_NAD83_Florida_East = 26958;
static const short PCS_NAD83_Florida_West = 26959;
static const short PCS_NAD83_Florida_North = 26960;
static const short PCS_NAD83_Hawaii_zone_1 = 26961;
static const short PCS_NAD83_Hawaii_zone_2 = 26962;
static const short PCS_NAD83_Hawaii_zone_3 = 26963;
static const short PCS_NAD83_Hawaii_zone_4 = 26964;
static const short PCS_NAD83_Hawaii_zone_5 = 26965;
static const short PCS_NAD83_Georgia_East = 26966;
static const short PCS_NAD83_Georgia_West = 26967;
static const short PCS_NAD83_Idaho_East = 26968;
static const short PCS_NAD83_Idaho_Central = 26969;
static const short PCS_NAD83_Idaho_West = 26970;
static const short PCS_NAD83_Illinois_East = 26971;
static const short PCS_NAD83_Illinois_West = 26972;
static const short PCS_NAD83_Indiana_East = 26973;
static const short PCS_NAD83_Indiana_West = 26974;
static const short PCS_NAD83_Iowa_North = 26975;
static const short PCS_NAD83_Iowa_South = 26976;
static const short PCS_NAD83_Kansas_North = 26977;
static const short PCS_NAD83_Kansas_South = 26978;
static const short PCS_NAD83_Kentucky_North = 2205;
static const short PCS_NAD83_Kentucky_South = 26980;
static const short PCS_NAD83_Louisiana_North = 26981;
static const short PCS_NAD83_Louisiana_South = 26982;
static const short PCS_NAD83_Maine_East = 26983;
static const short PCS_NAD83_Maine_West = 26984;
static const short PCS_NAD83_Maryland = 26985;
static const short PCS_NAD83_Massachusetts = 26986;
static const short PCS_NAD83_Massachusetts_Is = 26987;
static const short PCS_NAD83_Michigan_North = 26988;
static const short PCS_NAD83_Michigan_Central = 26989;
static const short PCS_NAD83_Michigan_South = 26990;
static const short PCS_NAD83_Minnesota_North = 26991;
static const short PCS_NAD83_Minnesota_Central = 26992;
static const short PCS_NAD83_Minnesota_South = 26993;
static const short PCS_NAD83_Mississippi_East = 26994;
static const short PCS_NAD83_Mississippi_West = 26995;
static const short PCS_NAD83_Missouri_East = 26996;
static const short PCS_NAD83_Missouri_Central = 26997;
static const short PCS_NAD83_Missouri_West = 26998;
static const short PCS_NAD83_Montana = 32100;
static const short PCS_NAD83_Nebraska = 32104;
static const short PCS_NAD83_Nevada_East = 32107;
static const short PCS_NAD83_Nevada_Central = 32108;
static const short PCS_NAD83_Nevada_West = 32109;
static const short PCS_NAD83_New_Hampshire = 32110;
static const short PCS_NAD83_New_Jersey = 32111;
static const short PCS_NAD83_New_Mexico_East = 32112;
static const short PCS_NAD83_New_Mexico_Central = 32113;
static const short PCS_NAD83_New_Mexico_West = 32114;
static const short PCS_NAD83_New_York_East = 32115;
static const short PCS_NAD83_New_York_Central = 32116;
static const short PCS_NAD83_New_York_West = 32117;
static const short PCS_NAD83_New_York_Long_Is = 32118;
static const short PCS_NAD83_North_Carolina = 32119;
static const short PCS_NAD83_North_Dakota_N = 32120;
static const short PCS_NAD83_North_Dakota_S = 32121;
static const short PCS_NAD83_Ohio_North = 32122;
static const short PCS_NAD83_Ohio_South = 32123;
static const short PCS_NAD83_Oklahoma_North = 32124;
static const short PCS_NAD83_Oklahoma_South = 32125;
static const short PCS_NAD83_Oregon_North = 32126;
static const short PCS_NAD83_Oregon_South = 32127;
static const short PCS_NAD83_Pennsylvania_N = 32128;
static const short PCS_NAD83_Pennsylvania_S = 32129;
static const short PCS_NAD83_Rhode_Island = 32130;
static const short PCS_NAD83_South_Carolina = 32133;
static const short PCS_NAD83_South_Dakota_N = 32134;
static const short PCS_NAD83_South_Dakota_S = 32135;
static const short PCS_NAD83_Tennessee = 32136;
static const short PCS_NAD83_Texas_North = 32137;
static const short PCS_NAD83_Texas_North_Central = 32138;
static const short PCS_NAD83_Texas_Central = 32139;
static const short PCS_NAD83_Texas_South_Central = 32140;
static const short PCS_NAD83_Texas_South = 32141;
static const short PCS_NAD83_Utah_North = 32142;
static const short PCS_NAD83_Utah_Central = 32143;
static const short PCS_NAD83_Utah_South = 32144;
static const short PCS_NAD83_Vermont = 32145;
static const short PCS_NAD83_Virginia_North = 32146;
static const short PCS_NAD83_Virginia_South = 32147;
static const short PCS_NAD83_Washington_North = 32148;
static const short PCS_NAD83_Washington_South = 32149;
static const short PCS_NAD83_West_Virginia_N = 32150;
static const short PCS_NAD83_West_Virginia_S = 32151;
static const short PCS_NAD83_Wisconsin_North = 32152;
static const short PCS_NAD83_Wisconsin_Central = 32153;
static const short PCS_NAD83_Wisconsin_South = 32154;
static const short PCS_NAD83_Wyoming_East = 32155;
static const short PCS_NAD83_Wyoming_East_Central = 32156;
static const short PCS_NAD83_Wyoming_West_Central = 32157;
static const short PCS_NAD83_Wyoming_West = 32158;
static const short PCS_NAD83_Puerto_Rico = 32161;

static const short GCTP_NAD83_Alabama_East = 101;
static const short GCTP_NAD83_Alabama_West = 102;
static const short GCTP_NAD83_Alaska_zone_1 = 5001; /* Hotine Oblique Mercator Projection not supported*/
static const short GCTP_NAD83_Alaska_zone_2 = 5002;
static const short GCTP_NAD83_Alaska_zone_3 = 5003;
static const short GCTP_NAD83_Alaska_zone_4 = 5004;
static const short GCTP_NAD83_Alaska_zone_5 = 5005;
static const short GCTP_NAD83_Alaska_zone_6 = 5006;
static const short GCTP_NAD83_Alaska_zone_7 = 5007;
static const short GCTP_NAD83_Alaska_zone_8 = 5008;
static const short GCTP_NAD83_Alaska_zone_9 = 5009;
static const short GCTP_NAD83_Alaska_zone_10 = 5010;
static const short GCTP_NAD83_California_1 = 401;
static const short GCTP_NAD83_California_2 = 402;
static const short GCTP_NAD83_California_3 = 403;
static const short GCTP_NAD83_California_4 = 404;
static const short GCTP_NAD83_California_5 = 405;
static const short GCTP_NAD83_California_6 = 406;
static const short GCTP_NAD83_Arizona_East = 201;
static const short GCTP_NAD83_Arizona_Central = 202;
static const short GCTP_NAD83_Arizona_West = 203;
static const short GCTP_NAD83_Arkansas_North = 301;
static const short GCTP_NAD83_Arkansas_South = 302;
static const short GCTP_NAD83_Colorado_North = 501;
static const short GCTP_NAD83_Colorado_Central = 502;
static const short GCTP_NAD83_Colorado_South = 503;
static const short GCTP_NAD83_Connecticut = 600;
static const short GCTP_NAD83_Delaware = 700;
static const short GCTP_NAD83_Florida_East = 901;
static const short GCTP_NAD83_Florida_West = 902;
static const short GCTP_NAD83_Florida_North = 903;
static const short GCTP_NAD83_Hawaii_zone_1 = 5101;
static const short GCTP_NAD83_Hawaii_zone_2 = 5102;
static const short GCTP_NAD83_Hawaii_zone_3 = 5103;
static const short GCTP_NAD83_Hawaii_zone_4 = 5104;
static const short GCTP_NAD83_Hawaii_zone_5 = 5105;
static const short GCTP_NAD83_Georgia_East = 1001;
static const short GCTP_NAD83_Georgia_West = 1002;
static const short GCTP_NAD83_Idaho_East = 1101;
static const short GCTP_NAD83_Idaho_Central = 1102;
static const short GCTP_NAD83_Idaho_West = 1103;
static const short GCTP_NAD83_Illinois_East = 1201;
static const short GCTP_NAD83_Illinois_West = 1202;
static const short GCTP_NAD83_Indiana_East = 1301;
static const short GCTP_NAD83_Indiana_West = 1302;
static const short GCTP_NAD83_Iowa_North = 1401;
static const short GCTP_NAD83_Iowa_South = 1402;
static const short GCTP_NAD83_Kansas_North = 1501;
static const short GCTP_NAD83_Kansas_South = 1502;
static const short GCTP_NAD83_Kentucky_North = 1601;
static const short GCTP_NAD83_Kentucky_South = 1602;
static const short GCTP_NAD83_Louisiana_North = 1701;
static const short GCTP_NAD83_Louisiana_South = 1702;
static const short GCTP_NAD83_Maine_East = 1801;
static const short GCTP_NAD83_Maine_West = 1802;
static const short GCTP_NAD83_Maryland = 1900;
static const short GCTP_NAD83_Massachusetts = 2001;
static const short GCTP_NAD83_Massachusetts_Is = 2002;
static const short GCTP_NAD83_Michigan_North = 2111;
static const short GCTP_NAD83_Michigan_Central = 2112;
static const short GCTP_NAD83_Michigan_South = 2113;
static const short GCTP_NAD83_Minnesota_North = 2201;
static const short GCTP_NAD83_Minnesota_Central = 2202;
static const short GCTP_NAD83_Minnesota_South = 2203;
static const short GCTP_NAD83_Mississippi_East = 2301;
static const short GCTP_NAD83_Mississippi_West = 2302;
static const short GCTP_NAD83_Missouri_East = 2401;
static const short GCTP_NAD83_Missouri_Central = 2402;
static const short GCTP_NAD83_Missouri_West = 2403;
static const short GCTP_NAD83_Montana = 2500;
static const short GCTP_NAD83_Nebraska = 2600;
static const short GCTP_NAD83_Nevada_East = 2701;
static const short GCTP_NAD83_Nevada_Central = 2702;
static const short GCTP_NAD83_Nevada_West = 2703;
static const short GCTP_NAD83_New_Hampshire = 2800;
static const short GCTP_NAD83_New_Jersey = 2900;
static const short GCTP_NAD83_New_Mexico_East = 3001;
static const short GCTP_NAD83_New_Mexico_Central = 3002;
static const short GCTP_NAD83_New_Mexico_West = 3003;
static const short GCTP_NAD83_New_York_East = 3101;
static const short GCTP_NAD83_New_York_Central = 3102;
static const short GCTP_NAD83_New_York_West = 3103;
static const short GCTP_NAD83_New_York_Long_Is = 3104;
static const short GCTP_NAD83_North_Carolina = 3200;
static const short GCTP_NAD83_North_Dakota_N = 3301;
static const short GCTP_NAD83_North_Dakota_S = 3302;
static const short GCTP_NAD83_Ohio_North = 3401;
static const short GCTP_NAD83_Ohio_South = 3402;
static const short GCTP_NAD83_Oklahoma_North = 3501;
static const short GCTP_NAD83_Oklahoma_South = 3502;
static const short GCTP_NAD83_Oregon_North = 3601;
static const short GCTP_NAD83_Oregon_South = 3602;
static const short GCTP_NAD83_Pennsylvania_N = 3701;
static const short GCTP_NAD83_Pennsylvania_S = 3702;
static const short GCTP_NAD83_Rhode_Island = 3800;
static const short GCTP_NAD83_South_Carolina = 3900;
static const short GCTP_NAD83_South_Dakota_N = 4001;
static const short GCTP_NAD83_South_Dakota_S = 4002;
static const short GCTP_NAD83_Tennessee = 4100;
static const short GCTP_NAD83_Texas_North = 4201;
static const short GCTP_NAD83_Texas_North_Central = 4202;
static const short GCTP_NAD83_Texas_Central = 4203;
static const short GCTP_NAD83_Texas_South_Central = 4204;
static const short GCTP_NAD83_Texas_South = 4205;
static const short GCTP_NAD83_Utah_North = 4301;
static const short GCTP_NAD83_Utah_Central = 4302;
static const short GCTP_NAD83_Utah_South = 4303;
static const short GCTP_NAD83_Vermont = 4400;
static const short GCTP_NAD83_Virginia_North = 4501;
static const short GCTP_NAD83_Virginia_South = 4502;
static const short GCTP_NAD83_Washington_North = 4601;
static const short GCTP_NAD83_Washington_South = 4602;
static const short GCTP_NAD83_West_Virginia_N = 4701;
static const short GCTP_NAD83_West_Virginia_S = 4702;
static const short GCTP_NAD83_Wisconsin_North = 4801;
static const short GCTP_NAD83_Wisconsin_Central = 4802;
static const short GCTP_NAD83_Wisconsin_South = 4803;
static const short GCTP_NAD83_Wyoming_East = 4901;
static const short GCTP_NAD83_Wyoming_East_Central = 4902;
static const short GCTP_NAD83_Wyoming_West_Central = 4903;
static const short GCTP_NAD83_Wyoming_West = 4904;
static const short GCTP_NAD83_Puerto_Rico = 5200;

class StatePlaneLCC
{
public:
  StatePlaneLCC(short geokey, char* zone, double falseEastingMeter, double falseNorthingMeter, double latOriginDegree, double longMeridianDegree, double firstStdParallelDegree, double secondStdParallelDegree)
  {
    this->geokey = geokey;
    this->zone = zone;
    this->falseEastingMeter = falseEastingMeter;
    this->falseNorthingMeter = falseNorthingMeter;
    this->latOriginDegree = latOriginDegree;
    this->longMeridianDegree = longMeridianDegree;
    this->firstStdParallelDegree = firstStdParallelDegree;
    this->secondStdParallelDegree = secondStdParallelDegree;
  }
  short geokey;
  char* zone;
  double falseEastingMeter;
  double falseNorthingMeter;
  double latOriginDegree;
  double longMeridianDegree;
  double firstStdParallelDegree;
  double secondStdParallelDegree;
};

static const StatePlaneLCC state_plane_lcc_nad27_list[] =
{
  // zone, false east [m], false north [m], ProjOrig(Lat), CentMerid(Long), 1st std para, 2nd std para 
  StatePlaneLCC(PCS_NAD27_Alaska_zone_10, "AK_10",914401.8288,0,51,-176,51.83333333,53.83333333),
  StatePlaneLCC(PCS_NAD27_Arkansas_North, "AR_N",609601.2192,0,34.33333333,-92,34.93333333,36.23333333),
  StatePlaneLCC(PCS_NAD27_Arkansas_South, "AR_S",609601.2192,0,32.66666667,-92,33.3,34.76666667),
  StatePlaneLCC(PCS_NAD27_California_I, "CA_I",609601.2192,0,39.33333333,-122,40,41.66666667),
  StatePlaneLCC(PCS_NAD27_California_II, "CA_II",609601.2192,0,37.66666667,-122,38.33333333,39.83333333),
  StatePlaneLCC(PCS_NAD27_California_III, "CA_III",609601.2192,0,36.5,-120.5,37.06666667,38.43333333),
  StatePlaneLCC(PCS_NAD27_California_IV, "CA_IV",609601.2192,0,35.33333333,-119,36,37.25),
  StatePlaneLCC(PCS_NAD27_California_V, "CA_V",609601.2192,0,33.5,-118,34.03333333,35.46666667),
  StatePlaneLCC(PCS_NAD27_California_VI, "CA_VI",609601.2192,0,32.16666667,-116.25,32.78333333,33.88333333),
  StatePlaneLCC(PCS_NAD27_California_VII, "CA_VII",1276106.451,1268253.007,34.13333333,-118.3333333,33.86666667,34.41666667),
  StatePlaneLCC(PCS_NAD27_Colorado_North, "CO_N",609601.2192,0,39.33333333,-105.5,39.71666667,40.78333333),
  StatePlaneLCC(PCS_NAD27_Colorado_Central, "CO_C",609601.2192,0,37.83333333,-105.5,38.45,39.75),
  StatePlaneLCC(PCS_NAD27_Colorado_South, "CO_S",609601.2192,0,36.66666667,-105.5,37.23333333,38.43333333),
  StatePlaneLCC(PCS_NAD27_Connecticut, "CT",182880.3658,0,40.83333333,-72.75,41.2,41.86666667),
  StatePlaneLCC(PCS_NAD27_Florida_North, "FL_N",609601.2192,0,29,-84.5,29.58333333,30.75),
  StatePlaneLCC(PCS_NAD27_Iowa_North, "IA_N",609601.2192,0,41.5,-93.5,42.06666667,43.26666667),
  StatePlaneLCC(PCS_NAD27_Iowa_South, "IA_S",609601.2192,0,40,-93.5,40.61666667,41.78333333),
  StatePlaneLCC(PCS_NAD27_Kansas_North, "KS_N",609601.2192,0,38.33333333,-98,38.71666667,39.78333333),
  StatePlaneLCC(PCS_NAD27_Kansas_South, "KS_S",609601.2192,0,36.66666667,-98.5,37.26666667,38.56666667),
  StatePlaneLCC(PCS_NAD27_Kentucky_North, "KY_N",609601.2192,0,37.5,-84.25,37.96666667,38.96666667),
  StatePlaneLCC(PCS_NAD27_Kentucky_South, "KY_S",609601.2192,0,36.33333333,-85.75,36.73333333,37.93333333),
  StatePlaneLCC(PCS_NAD27_Louisiana_North, "LA_N",609601.2192,0,30.66666667,-92.5,31.16666667,32.66666667),
  StatePlaneLCC(PCS_NAD27_Louisiana_South, "LA_S",609601.2192,0,28.66666667,-91.33333333,29.3,30.7),
  StatePlaneLCC(PCS_NAD27_Maryland, "MD",243840.4877,0,37.83333333,-77,38.3,39.45),
  StatePlaneLCC(PCS_NAD27_Massachusetts, "MA_M",182880.3658,0,41,-71.5,41.71666667,42.68333333),
  StatePlaneLCC(PCS_NAD27_Massachusetts_Is, "MA_I",60960.12192,0,41,-70.5,41.28333333,41.48333333),
  StatePlaneLCC(PCS_NAD27_Michigan_North, "MI_N",609601.2192,0,44.78333333,-87,45.48333333,47.08333333),
  StatePlaneLCC(PCS_NAD27_Michigan_Central, "MI_C",609601.2192,0,43.31666667,-84.33333333,44.18333333,45.7),
  StatePlaneLCC(PCS_NAD27_Michigan_South, "MI_S",609601.2192,0,41.5,-84.33333333,42.1,43.66666667),
  StatePlaneLCC(PCS_NAD27_Minnesota_North, "MN_N",609601.2192,0,46.5,-93.1,47.03333333,48.63333333),
  StatePlaneLCC(PCS_NAD27_Minnesota_Central, "MN_C",609601.2192,0,45,-94.25,45.61666667,47.05),
  StatePlaneLCC(PCS_NAD27_Minnesota_South, "MN_S",609601.2192,0,43,-94,43.78333333,45.21666667),
  StatePlaneLCC(PCS_NAD27_Montana_North, "MT_N",609601.2192,0,47,-109.5,47.85,48.71666667),
  StatePlaneLCC(PCS_NAD27_Montana_Central, "MT_C",609601.2192,0,45.83333333,-109.5,46.45,47.88333333),
  StatePlaneLCC(PCS_NAD27_Montana_South, "MT_S",609601.2192,0,44,-109.5,44.86666667,46.4),
  StatePlaneLCC(PCS_NAD27_Nebraska_North, "NE_N",609601.2192,0,41.33333333,-100,41.85,42.81666667),
  StatePlaneLCC(PCS_NAD27_Nebraska_South, "NE_S",609601.2192,0,39.66666667,-99.5,40.28333333,41.71666667),
  StatePlaneLCC(PCS_NAD27_New_York_Long_Is, "NY_LI",609601.2192,30480.06096,40.5,-74,40.66666667,41.03333333),
  StatePlaneLCC(PCS_NAD27_North_Carolina, "NC",609601.2192,0,33.75,-79,34.33333333,36.16666667),
  StatePlaneLCC(PCS_NAD27_North_Dakota_N, "ND_N",609601.2192,0,47,-100.5,47.43333333,48.73333333),
  StatePlaneLCC(PCS_NAD27_North_Dakota_S, "ND_S",609601.2192,0,45.66666667,-100.5,46.18333333,47.48333333),
  StatePlaneLCC(PCS_NAD27_Ohio_North, "OH_N",609601.2192,0,39.66666667,-82.5,40.43333333,41.7),
  StatePlaneLCC(PCS_NAD27_Ohio_South, "OH_S",609601.2192,0,38,-82.5,38.73333333,40.03333333),
  StatePlaneLCC(PCS_NAD27_Oklahoma_North, "OK_N",609601.2192,0,35,-98,35.56666667,36.76666667),
  StatePlaneLCC(PCS_NAD27_Oklahoma_South, "OK_S",609601.2192,0,33.33333333,-98,33.93333333,35.23333333),
  StatePlaneLCC(PCS_NAD27_Oregon_North, "OR_N",609601.2192,0,43.66666667,-120.5,44.33333333,46),
  StatePlaneLCC(PCS_NAD27_Oregon_South, "OR_S",609601.2192,0,41.66666667,-120.5,42.33333333,44),
  StatePlaneLCC(PCS_NAD27_Pennsylvania_N, "PA_N",609601.2192,0,40.16666667,-77.75,40.88333333,41.95),
  StatePlaneLCC(PCS_NAD27_Pennsylvania_S, "PA_S",609601.2192,0,39.33333333,-77.75,39.93333333,40.96666667),
  StatePlaneLCC(PCS_NAD27_Puerto_Rico, "PR",152400.3048,0,17.83333333,-66.43333333,18.03333333,18.43333333),
  StatePlaneLCC(PCS_NAD27_St_Croix, "St.Croix",152400.3048,30480.06096,17.83333333,-66.43333333,18.03333333,18.43333333),
  StatePlaneLCC(PCS_NAD27_South_Carolina_N, "SC_N",609601.2192,0,33,-81,33.76666667,34.96666667),
  StatePlaneLCC(PCS_NAD27_South_Carolina_S, "SC_S",609601.2192,0,31.83333333,-81,32.33333333,33.66666667),
  StatePlaneLCC(PCS_NAD27_South_Dakota_N, "SD_N",609601.2192,0,43.83333333,-100,44.41666667,45.68333333),
  StatePlaneLCC(PCS_NAD27_South_Dakota_S, "SD_S",609601.2192,0,42.33333333,-100.3333333,42.83333333,44.4),
  StatePlaneLCC(PCS_NAD27_Tennessee, "TN",609601.2192,30480.06096,34.66666667,-86,35.25,36.41666667),
  StatePlaneLCC(PCS_NAD27_Texas_North, "TX_N",609601.2192,0,34,-101.5,34.65,36.18333333),
  StatePlaneLCC(PCS_NAD27_Texas_North_Central, "TX_NC",609601.2192,0,31.66666667,-97.5,32.13333333,33.96666667),
  StatePlaneLCC(PCS_NAD27_Texas_Central, "TX_C",609601.2192,0,29.66666667,-100.3333333,30.11666667,31.88333333),
  StatePlaneLCC(PCS_NAD27_Texas_South_Central, "TX_SC",609601.2192,0,27.83333333,-99,28.38333333,30.28333333),
  StatePlaneLCC(PCS_NAD27_Texas_South, "TX_S",609601.2192,0,25.66666667,-98.5,26.16666667,27.83333333),
  StatePlaneLCC(PCS_NAD27_Utah_North, "UT_N",609601.2192,0,40.33333333,-111.5,40.71666667,41.78333333),
  StatePlaneLCC(PCS_NAD27_Utah_Central, "UT_C",609601.2192,0,38.33333333,-111.5,39.01666667,40.65),
  StatePlaneLCC(PCS_NAD27_Utah_South, "UT_S",609601.2192,0,36.66666667,-111.5,37.21666667,38.35),
  StatePlaneLCC(PCS_NAD27_Virginia_North, "VA_N",609601.2192,0,37.66666667,-78.5,38.03333333,39.2),
  StatePlaneLCC(PCS_NAD27_Virginia_South, "VA_S",609601.2192,0,36.33333333,-78.5,36.76666667,37.96666667),
  StatePlaneLCC(PCS_NAD27_Washington_North, "WA_N",609601.2192,0,47,-120.8333333,47.5,48.73333333),
  StatePlaneLCC(PCS_NAD27_Washington_South, "WA_S",609601.2192,0,45.33333333,-120.5,45.83333333,47.33333333),
  StatePlaneLCC(PCS_NAD27_West_Virginia_N, "WV_N",609601.2192,0,38.5,-79.5,39,40.25),
  StatePlaneLCC(PCS_NAD27_West_Virginia_S, "WV_S",609601.2192,0,37,-81,37.48333333,38.88333333),
  StatePlaneLCC(PCS_NAD27_Wisconsin_North, "WI_N",609601.2192,0,45.16666667,-90,45.56666667,46.76666667),
  StatePlaneLCC(PCS_NAD27_Wisconsin_Central, "WI_C",609601.2192,0,43.83333333,-90,44.25,45.5),
  StatePlaneLCC(PCS_NAD27_Wisconsin_South, "WI_S",609601.2192,0,42,-90,42.73333333,44.06666667),
  StatePlaneLCC(0,0,-1,-1,-1,-1,-1,-1)
};

static const StatePlaneLCC state_plane_lcc_nad83_list[] =
{
  // geotiff key, zone, false east [m], false north [m], ProjOrig(Lat), CentMerid(Long), 1st std para, 2nd std para 
  StatePlaneLCC(PCS_NAD83_Alaska_zone_10, "AK_10",1000000,0,51.000000,-176.000000,51.833333,53.833333),
  StatePlaneLCC(PCS_NAD83_Arkansas_North, "AR_N",400000,0,34.333333,-92.000000,34.933333,36.233333),
  StatePlaneLCC(PCS_NAD83_Arkansas_South, "AR_S",400000,400000,32.666667,-92.000000,33.300000,34.766667),
  StatePlaneLCC(PCS_NAD83_California_1, "CA_I",2000000,500000,39.333333,-122.000000,40.000000,41.666667),
  StatePlaneLCC(PCS_NAD83_California_2, "CA_II",2000000,500000,37.666667,-122.000000,38.333333,39.833333),
  StatePlaneLCC(PCS_NAD83_California_3, "CA_III",2000000,500000,36.500000,-120.500000,37.066667,38.433333),
  StatePlaneLCC(PCS_NAD83_California_4, "CA_IV",2000000,500000,35.333333,-119.000000,36.000000,37.250000),
  StatePlaneLCC(PCS_NAD83_California_5, "CA_V",2000000,500000,33.500000,-118.000000,34.033333,35.466667),
  StatePlaneLCC(PCS_NAD83_California_6, "CA_VI",2000000,500000,32.166667,-116.250000,32.783333,33.883333),
  StatePlaneLCC(PCS_NAD83_Colorado_North, "CO_N",914401.8289,304800.6096,39.333333,-105.500000,39.716667,40.783333),
  StatePlaneLCC(PCS_NAD83_Colorado_Central, "CO_C",914401.8289,304800.6096,37.833333,-105.500000,38.450000,39.750000),
  StatePlaneLCC(PCS_NAD83_Colorado_South, "CO_S",914401.8289,304800.6096,36.666667,-105.500000,37.233333,38.433333),
  StatePlaneLCC(PCS_NAD83_Connecticut, "CT",304800.6096,152400.3048,40.833333,-72.750000,41.200000,41.866667),
  StatePlaneLCC(PCS_NAD83_Florida_North, "FL_N",600000,0,29.000000,-84.500000,29.583333,30.750000),
  StatePlaneLCC(PCS_NAD83_Iowa_North, "IA_N",1500000,1000000,41.500000,-93.500000,42.066667,43.266667),
  StatePlaneLCC(PCS_NAD83_Iowa_South, "IA_S",500000,0,40.000000,-93.500000,40.616667,41.783333),
  StatePlaneLCC(PCS_NAD83_Kansas_North, "KS_N",400000,0,38.333333,-98.000000,38.716667,39.783333),
  StatePlaneLCC(PCS_NAD83_Kansas_South, "KS_S",400000,400000,36.666667,-98.500000,37.266667,38.566667),
  StatePlaneLCC(PCS_NAD83_Kentucky_North, "KY_N",500000,0,37.500000,-84.250000,37.966667,38.966667),
  StatePlaneLCC(PCS_NAD83_Kentucky_South, "KY_S",500000,500000,36.333333,-85.750000,36.733333,37.933333),
  StatePlaneLCC(PCS_NAD83_Louisiana_North, "LA_N",1000000,0,30.500000,-92.500000,31.166667,32.666667),
  StatePlaneLCC(PCS_NAD83_Louisiana_South, "LA_S",1000000,0,28.500000,-91.333333,29.300000,30.700000),
  StatePlaneLCC(PCS_NAD83_Maryland, "MD",400000,0,37.666667,-77.000000,38.300000,39.450000),
  StatePlaneLCC(PCS_NAD83_Massachusetts, "MA_M",200000,750000,41.000000,-71.500000,41.716667,42.683333),
  StatePlaneLCC(PCS_NAD83_Massachusetts_Is, "MA_I",500000,0,41.000000,-70.500000,41.283333,41.483333),
  StatePlaneLCC(PCS_NAD83_Michigan_North, "MI_N",8000000,0,44.783333,-87.000000,45.483333,47.083333),
  StatePlaneLCC(PCS_NAD83_Michigan_Central, "MI_C",6000000,0,43.316667,-84.366667,44.183333,45.700000),
  StatePlaneLCC(PCS_NAD83_Michigan_South, "MI_S",4000000,0,41.500000,-84.366667,42.100000,43.666667),
  StatePlaneLCC(PCS_NAD83_Minnesota_North, "MN_N",800000,100000,46.500000,-93.100000,47.033333,48.633333),
  StatePlaneLCC(PCS_NAD83_Minnesota_Central, "MN_C",800000,100000,45.000000,-94.250000,45.616667,47.050000),
  StatePlaneLCC(PCS_NAD83_Minnesota_South, "MN_S",800000,100000,43.000000,-94.000000,43.783333,45.216667),
  StatePlaneLCC(PCS_NAD83_Montana, "MT",600000,0,44.250000,-109.500000,45.000000,49.000000),
  StatePlaneLCC(PCS_NAD83_Nebraska, "NE",500000,0,39.833333,-100.000000,40.000000,43.000000),
  StatePlaneLCC(PCS_NAD83_New_York_Long_Is, "NY_LI",300000,0,40.166667,-74.000000,40.666667,41.033333),
  StatePlaneLCC(PCS_NAD83_North_Carolina, "NC",609601.22,0,33.750000,-79.000000,34.333333,36.166667),
  StatePlaneLCC(PCS_NAD83_North_Dakota_N, "ND_N",600000,0,47.000000,-100.500000,47.433333,48.733333),
  StatePlaneLCC(PCS_NAD83_North_Dakota_S, "ND_S",600000,0,45.666667,-100.500000,46.183333,47.483333),
  StatePlaneLCC(PCS_NAD83_Ohio_North, "OH_N",600000,0,39.666667,-82.500000,40.433333,41.700000),
  StatePlaneLCC(PCS_NAD83_Ohio_South, "OH_S",600000,0,38.000000,-82.500000,38.733333,40.033333),
  StatePlaneLCC(PCS_NAD83_Oklahoma_North, "OK_N",600000,0,35.000000,-98.000000,35.566667,36.766667),
  StatePlaneLCC(PCS_NAD83_Oklahoma_South, "OK_S",600000,0,33.333333,-98.000000,33.933333,35.233333),
  StatePlaneLCC(PCS_NAD83_Oregon_North, "OR_N",2500000,0,43.666667,-120.500000,44.333333,46.000000),
  StatePlaneLCC(PCS_NAD83_Oregon_South, "OR_S",1500000,0,41.666667,-120.500000,42.333333,44.000000),
  StatePlaneLCC(PCS_NAD83_Pennsylvania_N, "PA_N",600000,0,40.166667,-77.750000,40.883333,41.950000),
  StatePlaneLCC(PCS_NAD83_Pennsylvania_S, "PA_S",600000,0,39.333333,-77.750000,39.933333,40.966667),
  StatePlaneLCC(PCS_NAD83_Puerto_Rico, "PR",200000,200000,17.833333,-66.433333,18.033333,18.433333),
  StatePlaneLCC(PCS_NAD83_South_Carolina, "SC",609600,0,31.833333,-81.000000,32.500000,34.833333),
  StatePlaneLCC(PCS_NAD83_South_Dakota_N, "SD_N",600000,0,43.833333,-100.000000,44.416667,45.683333),
  StatePlaneLCC(PCS_NAD83_South_Dakota_S, "SD_S",600000,0,42.333333,-100.333333,42.833333,44.400000),
  StatePlaneLCC(PCS_NAD83_Tennessee, "TN",600000,0,34.333333,-86.000000,35.250000,36.416667),
  StatePlaneLCC(PCS_NAD83_Texas_North, "TX_N",200000,1000000,34.000000,-101.500000,34.650000,36.183333),
  StatePlaneLCC(PCS_NAD83_Texas_North_Central, "TX_NC",600000,2000000,31.666667,-98.500000,32.133333,33.966667),
  StatePlaneLCC(PCS_NAD83_Texas_Central, "TX_C",700000,3000000,29.666667,-100.333333,30.116667,31.883333),
  StatePlaneLCC(PCS_NAD83_Texas_South_Central, "TX_SC",600000,4000000,27.833333,-99.000000,28.383333,30.283333),
  StatePlaneLCC(PCS_NAD83_Texas_South, "TX_S",300000,5000000,25.666667,-98.500000,26.166667,27.833333),
  StatePlaneLCC(PCS_NAD83_Utah_North, "UT_N",500000,1000000,40.333333,-111.500000,40.716667,41.783333),
  StatePlaneLCC(PCS_NAD83_Utah_Central, "UT_C",500000,2000000,38.333333,-111.500000,39.016667,40.650000),
  StatePlaneLCC(PCS_NAD83_Utah_South, "UT_S",500000,3000000,36.666667,-111.500000,37.216667,38.350000),
  StatePlaneLCC(PCS_NAD83_Virginia_North, "VA_N",3500000,2000000,37.666667,-78.500000,38.033333,39.200000),
  StatePlaneLCC(PCS_NAD83_Virginia_South, "VA_S",3500000,1000000,36.333333,-78.500000,36.766667,37.966667),
  StatePlaneLCC(PCS_NAD83_Washington_North, "WA_N",500000,0,47.000000,-120.833333,47.500000,48.733333),
  StatePlaneLCC(PCS_NAD83_Washington_South, "WA_S",500000,0,45.333333,-120.500000,45.833333,47.333333),
  StatePlaneLCC(PCS_NAD83_West_Virginia_N, "WV_N",600000,0,38.500000,-79.500000,39.000000,40.250000),
  StatePlaneLCC(PCS_NAD83_West_Virginia_S, "WV_S",600000,0,37.000000,-81.000000,37.483333,38.883333),
  StatePlaneLCC(PCS_NAD83_Wisconsin_North, "WI_N",600000,0,45.166667,-90.000000,45.566667,46.766667),
  StatePlaneLCC(PCS_NAD83_Wisconsin_Central, "WI_C",600000,0,43.833333,-90.000000,44.250000,45.500000),
  StatePlaneLCC(PCS_NAD83_Wisconsin_South, "WI_S",600000,0,42.000000,-90.000000,42.733333,44.066667),
  StatePlaneLCC(0,0,-1,-1,-1,-1,-1,-1)
};

class StatePlaneTM
{
public:
  StatePlaneTM(short geokey, char* zone, double falseEastingMeter, double falseNorthingMeter, double latOriginDegree, double longMeridianDegree, double scaleFactor)
  {
    this->geokey = geokey;
    this->zone = zone;
    this->falseEastingMeter = falseEastingMeter;
    this->falseNorthingMeter = falseNorthingMeter;
    this->latOriginDegree = latOriginDegree;
    this->longMeridianDegree = longMeridianDegree;
    this->scaleFactor = scaleFactor;
  }
  short geokey;
  char* zone;
  double falseEastingMeter;
  double falseNorthingMeter;
  double latOriginDegree;
  double longMeridianDegree;
  double scaleFactor;
};

static const StatePlaneTM state_plane_tm_nad27_list[] =
{
  // geotiff key, zone, false east [m], false north [m], ProjOrig(Lat), CentMerid(Long), scale factor
  StatePlaneTM(PCS_NAD27_Alabama_East, "AL_E",152400.3048,0,30.5,-85.83333333,0.99996),
  StatePlaneTM(PCS_NAD27_Alabama_West, "AL_W",152400.3048,0,30,-87.5,0.999933333),
  StatePlaneTM(PCS_NAD27_Alaska_zone_2, "AK_2",152400.3048,0,54,-142,0.9999),
  StatePlaneTM(PCS_NAD27_Alaska_zone_3, "AK_3",152400.3048,0,54,-146,0.9999),
  StatePlaneTM(PCS_NAD27_Alaska_zone_4, "AK_4",152400.3048,0,54,-150,0.9999),
  StatePlaneTM(PCS_NAD27_Alaska_zone_5, "AK_5",152400.3048,0,54,-154,0.9999),
  StatePlaneTM(PCS_NAD27_Alaska_zone_6, "AK_6",152400.3048,0,54,-158,0.9999),
  StatePlaneTM(PCS_NAD27_Alaska_zone_7, "AK_7",213360.4267,0,54,-162,0.9999),
  StatePlaneTM(PCS_NAD27_Alaska_zone_8, "AK_8",152400.3048,0,54,-166,0.9999),
  StatePlaneTM(PCS_NAD27_Alaska_zone_9, "AK_9",182880.3658,0,54,-170,0.9999),
  StatePlaneTM(PCS_NAD27_Arizona_East, "AZ_E",152400.3048,0,31,-110.1666667,0.9999),
  StatePlaneTM(PCS_NAD27_Arizona_Central, "AZ_C",152400.3048,0,31,-111.9166667,0.9999),
  StatePlaneTM(PCS_NAD27_Arizona_West, "AZ_W",152400.3048,0,31,-113.75,0.999933333),
  StatePlaneTM(PCS_NAD27_Delaware, "DE",152400.3048,0,38,-75.41666667,0.999995),
  StatePlaneTM(PCS_NAD27_Florida_East, "FL_E",152400.3048,0,24.33333333,-81,0.999941177),
  StatePlaneTM(PCS_NAD27_Florida_West, "FL_W",152400.3048,0,24.33333333,-82,0.999941177),
  StatePlaneTM(PCS_NAD27_Georgia_East, "GA_E",152400.3048,0,30,-82.16666667,0.9999),
  StatePlaneTM(PCS_NAD27_Georgia_West, "GA_W",152400.3048,0,30,-84.16666667,0.9999),
  StatePlaneTM(PCS_NAD27_Hawaii_zone_1, "HI_1",152400.3048,0,18.83333333,-155.5,0.999966667),
  StatePlaneTM(PCS_NAD27_Hawaii_zone_2, "HI_2",152400.3048,0,20.33333333,-156.6666667,0.999966667),
  StatePlaneTM(PCS_NAD27_Hawaii_zone_3, "HI_3",152400.3048,0,21.16666667,-158,0.99999),
  StatePlaneTM(PCS_NAD27_Hawaii_zone_4, "HI_4",152400.3048,0,21.83333333,-159.5,0.99999),
  StatePlaneTM(PCS_NAD27_Hawaii_zone_5, "HI_5",152400.3048,0,21.66666667,-160.1666667,1),
  StatePlaneTM(PCS_NAD27_Idaho_East, "ID_E",152400.3048,0,41.66666667,-112.1666667,0.999947368),
  StatePlaneTM(PCS_NAD27_Idaho_Central, "ID_C",152400.3048,0,41.66666667,-114,0.999947368),
  StatePlaneTM(PCS_NAD27_Idaho_West, "ID_W",152400.3048,0,41.66666667,-115.75,0.999933333),
  StatePlaneTM(PCS_NAD27_Illinois_East, "IL_E",152400.3048,0,36.66666667,-88.33333333,0.999975),
  StatePlaneTM(PCS_NAD27_Illinois_West, "IL_W",152400.3048,0,36.66666667,-90.16666667,0.999941177),
  StatePlaneTM(PCS_NAD27_Indiana_East, "IN_E",152400.3048,0,37.5,-85.66666667,0.999966667),
  StatePlaneTM(PCS_NAD27_Indiana_West, "IN_W",152400.3048,0,37.5,-87.08333333,0.999966667),
  StatePlaneTM(PCS_NAD27_Maine_East, "ME_E",152400.3048,0,43.83333333,-68.5,0.9999),
  StatePlaneTM(PCS_NAD27_Maine_West, "ME_W",152400.3048,0,42.83333333,-70.16666667,0.999966667),
  StatePlaneTM(PCS_NAD27_Mississippi_East, "MS_E",152400.3048,0,29.66666667,-88.83333333,0.99996),
  StatePlaneTM(PCS_NAD27_Mississippi_West, "MS_W",152400.3048,0,30.5,-90.33333333,0.999941177),
  StatePlaneTM(PCS_NAD27_Missouri_East, "MO_E",152400.3048,0,35.83333333,-90.5,0.999933333),
  StatePlaneTM(PCS_NAD27_Missouri_Central, "MO_C",152400.3048,0,35.83333333,-92.5,0.999933333),
  StatePlaneTM(PCS_NAD27_Missouri_West, "MO_W",152400.3048,0,36.16666667,-94.5,0.999941177),
  StatePlaneTM(PCS_NAD27_Nevada_East, "NV_E",152400.3048,0,34.75,-115.5833333,0.9999),
  StatePlaneTM(PCS_NAD27_Nevada_Central, "NV_C",152400.3048,0,34.75,-116.6666667,0.9999),
  StatePlaneTM(PCS_NAD27_Nevada_West, "NV_W",152400.3048,0,34.75,-118.5833333,0.9999),
  StatePlaneTM(PCS_NAD27_New_Hampshire, "NH",152400.3048,0,42.5,-71.66666667,0.999966667),
  StatePlaneTM(PCS_NAD27_New_Jersey, "NJ",609601.2192,0,38.83333333,-74.66666667,0.999975),
  StatePlaneTM(PCS_NAD27_New_Mexico_East, "NM_E",152400.3048,0,31,-104.3333333,0.999909091),
  StatePlaneTM(PCS_NAD27_New_Mexico_Central, "NM_C",152400.3048,0,31,-106.25,0.9999),
  StatePlaneTM(PCS_NAD27_New_Mexico_West, "NM_W",152400.3048,0,31,-107.8333333,0.999916667),
  StatePlaneTM(PCS_NAD27_New_York_East, "NY_E",152400.3048,0,40,-74.33333333,0.999966667),
  StatePlaneTM(PCS_NAD27_New_York_Central, "NY_C",152400.3048,0,40,-76.58333333,0.9999375),
  StatePlaneTM(PCS_NAD27_New_York_West, "NY_W",152400.3048,0,40,-78.58333333,0.9999375),
  StatePlaneTM(PCS_NAD27_Rhode_Island, "RI",152400.3048,0,41.08333333,-71.5,0.99999375),
  StatePlaneTM(PCS_NAD27_Vermont, "VT",152400.3048,0,42.5,-72.5,0.999964286),
  StatePlaneTM(PCS_NAD27_Wyoming_East, "WY_E",152400.3048,0,40.66666667,-105.1666667,0.999941177),
  StatePlaneTM(PCS_NAD27_Wyoming_East_Central, "WY_EC",152400.3048,0,40.66666667,-107.3333333,0.999941177),
  StatePlaneTM(PCS_NAD27_Wyoming_West_Central, "WY_WC",152400.3048,0,40.66666667,-108.75,0.999941177),
  StatePlaneTM(PCS_NAD27_Wyoming_West, "WY_W",152400.3048,0,40.66666667,-110.0833333,0.999941177),
  StatePlaneTM(0,0,-1,-1,-1,-1,-1)
};

static const StatePlaneTM state_plane_tm_nad83_list[] =
{
  // geotiff key, zone, false east [m], false north [m], ProjOrig(Lat), CentMerid(Long), scale factor
  StatePlaneTM(PCS_NAD83_Alabama_East, "AL_E",200000,0,30.5,-85.83333333,0.99996),
  StatePlaneTM(PCS_NAD83_Alabama_West, "AL_W",600000,0,30,-87.5,0.999933333),
  StatePlaneTM(PCS_NAD83_Alaska_zone_2, "AK_2",500000,0,54,-142,0.9999),
  StatePlaneTM(PCS_NAD83_Alaska_zone_3, "AK_3",500000,0,54,-146,0.9999),
  StatePlaneTM(PCS_NAD83_Alaska_zone_4, "AK_4",500000,0,54,-150,0.9999),
  StatePlaneTM(PCS_NAD83_Alaska_zone_5, "AK_5",500000,0,54,-154,0.9999),
  StatePlaneTM(PCS_NAD83_Alaska_zone_6, "AK_6",500000,0,54,-158,0.9999),
  StatePlaneTM(PCS_NAD83_Alaska_zone_7, "AK_7",500000,0,54,-162,0.9999),
  StatePlaneTM(PCS_NAD83_Alaska_zone_8, "AK_8",500000,0,54,-166,0.9999),
  StatePlaneTM(PCS_NAD83_Alaska_zone_9, "AK_9",500000,0,54,-170,0.9999),
  StatePlaneTM(PCS_NAD83_Arizona_East, "AZ_E",213360,0,31,-110.1666667,0.9999),
  StatePlaneTM(PCS_NAD83_Arizona_Central, "AZ_C",213360,0,31,-111.9166667,0.9999),
  StatePlaneTM(PCS_NAD83_Arizona_West, "AZ_W",213360,0,31,-113.75,0.999933333),
  StatePlaneTM(PCS_NAD83_Delaware, "DE",200000,0,38,-75.41666667,0.999995),
  StatePlaneTM(PCS_NAD83_Florida_East, "FL_E",200000,0,24.33333333,-81,0.999941177),
  StatePlaneTM(PCS_NAD83_Florida_West, "FL_W",200000,0,24.33333333,-82,0.999941177),
  StatePlaneTM(PCS_NAD83_Georgia_East, "GA_E",200000,0,30,-82.16666667,0.9999),
  StatePlaneTM(PCS_NAD83_Georgia_West, "GA_W",700000,0,30,-84.16666667,0.9999),
  StatePlaneTM(PCS_NAD83_Hawaii_zone_1, "HI_1",500000,0,18.83333333,-155.5,0.999966667),
  StatePlaneTM(PCS_NAD83_Hawaii_zone_2, "HI_2",500000,0,20.33333333,-156.6666667,0.999966667),
  StatePlaneTM(PCS_NAD83_Hawaii_zone_3, "HI_3",500000,0,21.16666667,-158,0.99999),
  StatePlaneTM(PCS_NAD83_Hawaii_zone_4, "HI_4",500000,0,21.83333333,-159.5,0.99999),
  StatePlaneTM(PCS_NAD83_Hawaii_zone_5, "HI_5",500000,0,21.66666667,-160.1666667,1),
  StatePlaneTM(PCS_NAD83_Idaho_East, "ID_E",200000,0,41.66666667,-112.1666667,0.999947368),
  StatePlaneTM(PCS_NAD83_Idaho_Central, "ID_C",500000,0,41.66666667,-114,0.999947368),
  StatePlaneTM(PCS_NAD83_Idaho_West, "ID_W",800000,0,41.66666667,-115.75,0.999933333),
  StatePlaneTM(PCS_NAD83_Illinois_East, "IL_E",300000,0,36.66666667,-88.33333333,0.999975),
  StatePlaneTM(PCS_NAD83_Illinois_West, "IL_W",700000,0,36.66666667,-90.16666667,0.999941177),
  StatePlaneTM(PCS_NAD83_Indiana_East, "IN_E",100000,250000,37.5,-85.66666667,0.999966667),
  StatePlaneTM(PCS_NAD83_Indiana_West, "IN_W",900000,250000,37.5,-87.08333333,0.999966667),
  StatePlaneTM(PCS_NAD83_Maine_East, "ME_E",300000,0,43.66666667,-68.5,0.9999),
  StatePlaneTM(PCS_NAD83_Maine_West, "ME_W",900000,0,42.83333333,-70.16666667,0.999966667),
  StatePlaneTM(PCS_NAD83_Mississippi_East, "MS_E",300000,0,29.5,-88.83333333,0.99995),
  StatePlaneTM(PCS_NAD83_Mississippi_West, "MS_W",700000,0,29.5,-90.33333333,0.99995),
  StatePlaneTM(PCS_NAD83_Missouri_East, "MO_E",250000,0,35.83333333,-90.5,0.999933333),
  StatePlaneTM(PCS_NAD83_Missouri_Central, "MO_C",500000,0,35.83333333,-92.5,0.999933333),
  StatePlaneTM(PCS_NAD83_Missouri_West, "MO_W",850000,0,36.16666667,-94.5,0.999941177),
  StatePlaneTM(PCS_NAD83_Nevada_East, "NV_E",200000,8000000,34.75,-115.5833333,0.9999),
  StatePlaneTM(PCS_NAD83_Nevada_Central, "NV_C",500000,6000000,34.75,-116.6666667,0.9999),
  StatePlaneTM(PCS_NAD83_Nevada_West, "NV_W",800000,4000000,34.75,-118.5833333,0.9999),
  StatePlaneTM(PCS_NAD83_New_Hampshire, "NH",300000,0,42.5,-71.66666667,0.999966667),
  StatePlaneTM(PCS_NAD83_New_Jersey, "NJ",150000,0,38.83333333,-74.5,0.9999),
  StatePlaneTM(PCS_NAD83_New_Mexico_East, "NM_E",165000,0,31,-104.3333333,0.999909091),
  StatePlaneTM(PCS_NAD83_New_Mexico_Central, "NM_C",500000,0,31,-106.25,0.9999),
  StatePlaneTM(PCS_NAD83_New_Mexico_West, "NM_W",830000,0,31,-107.8333333,0.999916667),
  StatePlaneTM(PCS_NAD83_New_York_East, "NY_E",150000,0,38.83333333,-74.5,0.9999),
  StatePlaneTM(PCS_NAD83_New_York_Central, "NY_C",250000,0,40,-76.58333333,0.9999375),
  StatePlaneTM(PCS_NAD83_New_York_West, "NY_W",350000,0,40,-78.58333333,0.9999375),
  StatePlaneTM(PCS_NAD83_Rhode_Island, "RI",100000,0,41.08333333,-71.5,0.99999375),
  StatePlaneTM(PCS_NAD83_Vermont, "VT",500000,0,42.5,-72.5,0.999964286),
  StatePlaneTM(PCS_NAD83_Wyoming_East, "WY_E",200000,0,40.5,-105.1666667,0.9999375),
  StatePlaneTM(PCS_NAD83_Wyoming_East_Central, "WY_EC",400000,100000,40.5,-107.3333333,0.9999375),
  StatePlaneTM(PCS_NAD83_Wyoming_West_Central, "WY_WC",600000,0,40.5,-108.75,0.9999375),
  StatePlaneTM(PCS_NAD83_Wyoming_West, "WY_W",800000,100000,40.5,-110.0833333,0.9999375),
  StatePlaneTM(0,0,-1,-1,-1,-1,-1)
};

static const short EPSG_IRENET95_Irish_Transverse_Mercator = 2157;
static const short EPSG_ETRS89_Poland_CS92 = 2180;
static const short EPSG_NZGD2000 = 2193;
static const short EPSG_NAD83_HARN_UTM2_South_American_Samoa = 2195;
static const short EPSG_NAD83_Maryland_ftUS = 2248;
static const short EPSG_NAD83_Texas_Central_ftUS = 2277;
static const short EPSG_NAD83_HARN_Washington_North = 2855;
static const short EPSG_NAD83_HARN_Washington_South = 2856;
static const short EPSG_NAD83_HARN_Virginia_North_ftUS = 2924;
static const short EPSG_NAD83_HARN_Virginia_South_ftUS = 2925;
static const short EPSG_NAD83_HARN_Washington_North_ftUS = 2926;
static const short EPSG_NAD83_HARN_Washington_South_ftUS = 2927;
static const short EPSG_Reseau_Geodesique_Francais_Guyane_1995 = 2972;
static const short EPSG_NAD83_Oregon_Lambert = 2991;
static const short EPSG_NAD83_Oregon_Lambert_ft = 2992;
static const short EPSG_NAD83_BC_Albers = 3005;
static const short EPSG_SWEREF99_TM = 3006;
static const short EPSG_ETRS89_ETRS_LCC = 3034;
static const short EPSG_ETRS89_ETRS_TM34 = 3046;
static const short EPSG_ETRS89_ETRS_TM35 = 3047;
static const short EPSG_ETRS89_ETRS_TM36 = 3048;
static const short EPSG_ETRS89_ETRS_TM35FIN = 3067;
static const short EPSG_Fiji_1956_UTM60_South = 3141;
static const short EPSG_Fiji_1956_UTM1_South = 3142;
static const short EPSG_GDA94_NSW_Lambert = 3308;
static const short EPSG_Fiji_Map_Grid_1986 = 3460;
static const short EPSG_NAD83_NSRS2007_California_zone_3_ftUS = 3494;
static const short EPSG_NAD83_NSRS2007_California_zone_4_ftUS = 3496;
static const short EPSG_NAD83_NSRS2007_Maryland_ftUS = 3582;
static const short EPSG_ETRS89_Portugal_TM06 = 3763;
static const short EPSG_Slovene_National_Grid_1996 = 3794;
static const short EPSG_ETRS89_GK24FIN = 3878;
static const short EPSG_MGI_1901_Slovene_National_Grid = 3912;
static const short EPSG_RGF93_CC42 = 3942;
static const short EPSG_RGF93_CC43 = 3943;
static const short EPSG_RGF93_CC44 = 3944;
static const short EPSG_RGF93_CC45 = 3945;
static const short EPSG_RGF93_CC46 = 3946;
static const short EPSG_RGF93_CC47 = 3947;
static const short EPSG_RGF93_CC48 = 3948;
static const short EPSG_RGF93_CC49 = 3949;
static const short EPSG_RGF93_CC50 = 3950;
static const short EPSG_ETRS89_DKTM1 = 4093;
static const short EPSG_ETRS89_DKTM2 = 4094;
static const short EPSG_ETRS89_DKTM3 = 4095;
static const short EPSG_ETRS89_DKTM4 = 4096;
static const short EPSG_ETRS89_UTM32_north_zE_N = 4647;
static const short EPSG_NAD83_HARN_Conus_Albers = 5071;
static const short EPSG_NAD83_NSRS2007_Conus_Albers = 5072;
static const short EPSG_ETRS89_NTM_zone_5 = 5105;
static const short EPSG_ETRS89_NTM_zone_6 = 5106;
static const short EPSG_ETRS89_NTM_zone_7 = 5107;
static const short EPSG_ETRS89_NTM_zone_8 = 5108;
static const short EPSG_ETRS89_NTM_zone_9 = 5109;
static const short EPSG_ETRS89_NTM_zone_10 = 5110;
static const short EPSG_ETRS89_NTM_zone_11 = 5111;
static const short EPSG_ETRS89_NTM_zone_12 = 5112;
static const short EPSG_ETRS89_NTM_zone_13 = 5113;
static const short EPSG_ETRS89_NTM_zone_14 = 5114;
static const short EPSG_ETRS89_NTM_zone_15 = 5115;
static const short EPSG_ETRS89_NTM_zone_16 = 5116;
static const short EPSG_ETRS89_NTM_zone_17 = 5117;
static const short EPSG_ETRS89_NTM_zone_18 = 5118;
static const short EPSG_ETRS89_NTM_zone_19 = 5119;
static const short EPSG_ETRS89_NTM_zone_20 = 5120;
static const short EPSG_ETRS89_NTM_zone_21 = 5121;
static const short EPSG_ETRS89_NTM_zone_22 = 5122;
static const short EPSG_ETRS89_NTM_zone_23 = 5123;
static const short EPSG_ETRS89_NTM_zone_24 = 5124;
static const short EPSG_ETRS89_NTM_zone_25 = 5125;
static const short EPSG_ETRS89_NTM_zone_26 = 5126;
static const short EPSG_ETRS89_NTM_zone_27 = 5127;
static const short EPSG_ETRS89_NTM_zone_28 = 5128;
static const short EPSG_ETRS89_NTM_zone_29 = 5129;
static const short EPSG_ETRS89_NTM_zone_30 = 5130;
static const short EPSG_ETRS89_UTM33_north_zE_N = 5650;
//static const short NAD83_2011_Conus_Albers = 6350;
static const short EPSG_NAD83_2011_North_Carolina_ftUS = 6543;
static const short EPSG_CH1903_LV03 = 21781;
static const short EPSG_EOV_HD72 = 23700;
static const short EPSG_OSGB_1936 = 27700;
static const short EPSG_Belgian_Lambert_1972 = 31370;

class EPSGcode
{
public:
  EPSGcode(short geokey, char* description)
  {
    this->geokey = geokey;
    this->description = description;
  }
  short geokey;
  char* description;
};

static const EPSGcode epsg_code_list[] =
{
  // geotiff key, description
  EPSGcode(EPSG_IRENET95_Irish_Transverse_Mercator, "IRENET95 Irish Tran."),
  EPSGcode(EPSG_ETRS89_Poland_CS92, "ETRS89 Poland CS92"),
  EPSGcode(EPSG_NZGD2000, "NZGD2000"),
  EPSGcode(EPSG_NAD83_HARN_UTM2_South_American_Samoa, "Amer. Samoa UTM2"),
  EPSGcode(EPSG_NAD83_Maryland_ftUS, "NAD83 Maryland ft"),
  EPSGcode(EPSG_NAD83_Texas_Central_ftUS, "NAD83 Texas Central ft"),
  EPSGcode(EPSG_NAD83_HARN_Washington_North, "NAD83-H Washington N"),
  EPSGcode(EPSG_NAD83_HARN_Washington_South, "NAD83-H Washington S"),
  EPSGcode(EPSG_NAD83_HARN_Virginia_North_ftUS, "NAD83-H Virginia N ft"),
  EPSGcode(EPSG_NAD83_HARN_Virginia_South_ftUS, "NAD83-H Virginia S ft"),
  EPSGcode(EPSG_NAD83_HARN_Washington_North_ftUS, "NAD83-H Washington N ft"),
  EPSGcode(EPSG_NAD83_HARN_Washington_South_ftUS, "NAD83-H Washington S ft"),
  EPSGcode(EPSG_Reseau_Geodesique_Francais_Guyane_1995, "Francais Guyane 95"),
  EPSGcode(EPSG_NAD83_Oregon_Lambert, "NAD83 Oregon Lambert"),
  EPSGcode(EPSG_NAD83_Oregon_Lambert_ft, "NAD83 Oregon Lamb.(ft)"),
  EPSGcode(EPSG_NAD83_BC_Albers, "NAD83 / BC Albers"),
  EPSGcode(EPSG_SWEREF99_TM, "SWEREF99 TM"),
  EPSGcode(EPSG_ETRS89_ETRS_LCC, "ETRS89 LCC"),
  EPSGcode(EPSG_ETRS89_ETRS_TM34, "ETRS89 TM34"),
  EPSGcode(EPSG_ETRS89_ETRS_TM35, "ETRS89 TM35"),
  EPSGcode(EPSG_ETRS89_ETRS_TM36, "ETRS89 TM36"),
  EPSGcode(EPSG_ETRS89_ETRS_TM35FIN, "ETRS89 TM35FIN"),
  EPSGcode(EPSG_Fiji_1956_UTM60_South, "Fiji 1956 UTM60"),
  EPSGcode(EPSG_Fiji_1956_UTM1_South, "Fiji 1956 UTM1"),
  EPSGcode(EPSG_GDA94_NSW_Lambert, "GDA94 NSW Lambert"),
  EPSGcode(EPSG_Fiji_Map_Grid_1986, "Fiji Map Grid 1986"),
  EPSGcode(EPSG_NAD83_NSRS2007_California_zone_3_ftUS, "NSRS2007 Calif 3 ft"),
  EPSGcode(EPSG_NAD83_NSRS2007_California_zone_4_ftUS, "NSRS2007 Calif 4 ft"),
  EPSGcode(EPSG_NAD83_NSRS2007_Maryland_ftUS, "NSRS2007 Maryland ft"),
  EPSGcode(EPSG_ETRS89_Portugal_TM06, "ETRS89 Portugal TM06"),
  EPSGcode(EPSG_Slovene_National_Grid_1996, "Slov. Nat. Grid 1996"),
  EPSGcode(EPSG_ETRS89_GK24FIN, "ETRS89 / GK24FIN"),
  EPSGcode(EPSG_MGI_1901_Slovene_National_Grid, "Slov. Nat. Grid 1901"),
  EPSGcode(EPSG_RGF93_CC42, "RGF93 CC42"),
  EPSGcode(EPSG_RGF93_CC43, "RGF93 CC43"),
  EPSGcode(EPSG_RGF93_CC44, "RGF93 CC44"),
  EPSGcode(EPSG_RGF93_CC45, "RGF93 CC45"),
  EPSGcode(EPSG_RGF93_CC46, "RGF93 CC46"),
  EPSGcode(EPSG_RGF93_CC47, "RGF93 CC47"),
  EPSGcode(EPSG_RGF93_CC48, "RGF93 CC48"),
  EPSGcode(EPSG_RGF93_CC49, "RGF93 CC49"),
  EPSGcode(EPSG_RGF93_CC50, "RGF93 CC50"),
  EPSGcode(EPSG_ETRS89_DKTM1, "ETRS89 DKTM1"),
  EPSGcode(EPSG_ETRS89_DKTM2, "ETRS89 DKTM2"),
  EPSGcode(EPSG_ETRS89_DKTM3, "ETRS89 DKTM3"),
  EPSGcode(EPSG_ETRS89_DKTM4, "ETRS89 DKTM4"),
  EPSGcode(EPSG_ETRS89_UTM32_north_zE_N, "ETRS89 UTM32 zE-N"),
  EPSGcode(EPSG_NAD83_HARN_Conus_Albers, "NAD83-H Conus Albers"),
  EPSGcode(EPSG_NAD83_NSRS2007_Conus_Albers, "NSRS2007 Conus Albers"),
  EPSGcode(EPSG_ETRS89_NTM_zone_5, "ETRS89 NTM5"),
  EPSGcode(EPSG_ETRS89_NTM_zone_6, "ETRS89 NTM6"),
  EPSGcode(EPSG_ETRS89_NTM_zone_7, "ETRS89 NTM7"),
  EPSGcode(EPSG_ETRS89_NTM_zone_8, "ETRS89 NTM8"),
  EPSGcode(EPSG_ETRS89_NTM_zone_9, "ETRS89 NTM9"),
  EPSGcode(EPSG_ETRS89_NTM_zone_10, "ETRS89 NTM10"),
  EPSGcode(EPSG_ETRS89_NTM_zone_11, "ETRS89 NTM11"),
  EPSGcode(EPSG_ETRS89_NTM_zone_12, "ETRS89 NTM12"),
  EPSGcode(EPSG_ETRS89_NTM_zone_13, "ETRS89 NTM13"),
  EPSGcode(EPSG_ETRS89_NTM_zone_14, "ETRS89 NTM14"),
  EPSGcode(EPSG_ETRS89_NTM_zone_15, "ETRS89 NTM15"),
  EPSGcode(EPSG_ETRS89_NTM_zone_16, "ETRS89 NTM16"),
  EPSGcode(EPSG_ETRS89_NTM_zone_17, "ETRS89 NTM17"),
  EPSGcode(EPSG_ETRS89_NTM_zone_18, "ETRS89 NTM18"),
  EPSGcode(EPSG_ETRS89_NTM_zone_19, "ETRS89 NTM19"),
  EPSGcode(EPSG_ETRS89_NTM_zone_20, "ETRS89 NTM20"),
  EPSGcode(EPSG_ETRS89_NTM_zone_21, "ETRS89 NTM21"),
  EPSGcode(EPSG_ETRS89_NTM_zone_22, "ETRS89 NTM22"),
  EPSGcode(EPSG_ETRS89_NTM_zone_23, "ETRS89 NTM23"),
  EPSGcode(EPSG_ETRS89_NTM_zone_24, "ETRS89 NTM24"),
  EPSGcode(EPSG_ETRS89_NTM_zone_25, "ETRS89 NTM25"),
  EPSGcode(EPSG_ETRS89_NTM_zone_26, "ETRS89 NTM26"),
  EPSGcode(EPSG_ETRS89_NTM_zone_27, "ETRS89 NTM27"),
  EPSGcode(EPSG_ETRS89_NTM_zone_28, "ETRS89 NTM28"),
  EPSGcode(EPSG_ETRS89_NTM_zone_29, "ETRS89 NTM29"),
  EPSGcode(EPSG_ETRS89_NTM_zone_30, "ETRS89 NTM30"),
  EPSGcode(EPSG_ETRS89_UTM33_north_zE_N, "ETRS89 UTM33 zE-N"),
//  EPSGcode(NAD83_2011_Conus_Albers, "NAD83 2011 Conus Albers"),
  EPSGcode(EPSG_NAD83_2011_North_Carolina_ftUS, "NAD83 2011 N Carolina ft"),
  EPSGcode(EPSG_CH1903_LV03, "CH1903 / LV03"),
  EPSGcode(EPSG_EOV_HD72, "Hungarian EOV / HD72"),
  EPSGcode(EPSG_OSGB_1936, "OSGB 1936"),
  EPSGcode(EPSG_Belgian_Lambert_1972, "Belgian Lambert 1972"),
  EPSGcode(0,0)
};

bool GeoProjectionConverter::get_geo_keys_from_projection(int& num_geo_keys, GeoProjectionGeoKeys** geo_keys, int& num_geo_double_params, double** geo_double_params, bool source)
{
  num_geo_keys = 0;
  num_geo_double_params = 0;
  GeoProjectionParameters* projection = (source ? source_projection : target_projection);
  if (projection)
  {
    unsigned short vertical_geokey = get_VerticalCSTypeGeoKey();
    if (projection->type == GEO_PROJECTION_UTM || projection->type == GEO_PROJECTION_LCC || projection->type == GEO_PROJECTION_TM || projection->type == GEO_PROJECTION_AEAC || projection->type == GEO_PROJECTION_OM)
    {
      unsigned short geokey = get_ProjectedCSTypeGeoKey(source);
      if (geokey && geokey != 32767)
      {
        num_geo_keys = 4 + (vertical_geokey ? 1 : 0);
        (*geo_keys) = (GeoProjectionGeoKeys*)malloc(sizeof(GeoProjectionGeoKeys)*num_geo_keys);
        (*geo_double_params) = 0;

        // projected coordinates
        (*geo_keys)[0].key_id = 1024; // GTModelTypeGeoKey
        (*geo_keys)[0].tiff_tag_location = 0;
        (*geo_keys)[0].count = 1;
        (*geo_keys)[0].value_offset = 1; // ModelTypeProjected

        // projection
        (*geo_keys)[1].key_id = 3072; // ProjectedCSTypeGeoKey
        (*geo_keys)[1].tiff_tag_location = 0;
        (*geo_keys)[1].count = 1;
        (*geo_keys)[1].value_offset = geokey;

        // horizontal units
        (*geo_keys)[2].key_id = 3076; // ProjLinearUnitsGeoKey
        (*geo_keys)[2].tiff_tag_location = 0;
        (*geo_keys)[2].count = 1;
        (*geo_keys)[2].value_offset = get_ProjLinearUnitsGeoKey(source);

        // vertical units
        (*geo_keys)[3].key_id = 4099; // VerticalUnitsGeoKey
        (*geo_keys)[3].tiff_tag_location = 0;
        (*geo_keys)[3].count = 1;
        (*geo_keys)[3].value_offset = get_VerticalUnitsGeoKey(source);

        if (vertical_geokey)
        {
          // vertical datum
          (*geo_keys)[4].key_id = 4096; // VerticalCSTypeGeoKey
          (*geo_keys)[4].tiff_tag_location = 0;
          (*geo_keys)[4].count = 1;
          (*geo_keys)[4].value_offset = vertical_geokey;
        }
        return true;
      }
      else if (projection->type == GEO_PROJECTION_LCC)
      {
        GeoProjectionParametersLCC* lcc = (GeoProjectionParametersLCC*)projection;

        num_geo_keys = 13 + (vertical_geokey ? 1 : 0);
        (*geo_keys) = (GeoProjectionGeoKeys*)malloc(sizeof(GeoProjectionGeoKeys)*num_geo_keys);
        num_geo_double_params = 6;
        (*geo_double_params) = (double*)malloc(sizeof(double)*num_geo_double_params);

        // projected coordinates
        (*geo_keys)[0].key_id = 1024; // GTModelTypeGeoKey
        (*geo_keys)[0].tiff_tag_location = 0;
        (*geo_keys)[0].count = 1;
        (*geo_keys)[0].value_offset = 1; // ModelTypeProjected

        // user-defined custom LCC projection 
        (*geo_keys)[1].key_id = 3072; // ProjectedCSTypeGeoKey
        (*geo_keys)[1].tiff_tag_location = 0;
        (*geo_keys)[1].count = 1;
        (*geo_keys)[1].value_offset = 32767; // user-defined

        // which projection do we use
        (*geo_keys)[2].key_id = 3075; // ProjCoordTransGeoKey
        (*geo_keys)[2].tiff_tag_location = 0;
        (*geo_keys)[2].count = 1;
        (*geo_keys)[2].value_offset = 8; // CT_LambertConfConic_2SP 

        // which units do we use
        (*geo_keys)[3].key_id = 3076; // ProjCoordTransGeoKey
        (*geo_keys)[3].tiff_tag_location = 0;
        (*geo_keys)[3].count = 1;
        (*geo_keys)[3].value_offset = get_ProjLinearUnitsGeoKey(source); 

        // here come the 6 double parameters

        (*geo_keys)[4].key_id = 3078; // ProjStdParallel1GeoKey
        (*geo_keys)[4].tiff_tag_location = 34736;
        (*geo_keys)[4].count = 1;
        (*geo_keys)[4].value_offset = 0;
        (*geo_double_params)[0] = lcc->lcc_first_std_parallel_degree;

        (*geo_keys)[5].key_id = 3079; // ProjStdParallel2GeoKey
        (*geo_keys)[5].tiff_tag_location = 34736;
        (*geo_keys)[5].count = 1;
        (*geo_keys)[5].value_offset = 1;
        (*geo_double_params)[1] = lcc->lcc_second_std_parallel_degree;

        (*geo_keys)[6].key_id = 3088; // ProjCenterLongGeoKey
        (*geo_keys)[6].tiff_tag_location = 34736;
        (*geo_keys)[6].count = 1;
        (*geo_keys)[6].value_offset = 2;
        (*geo_double_params)[2] = lcc->lcc_long_meridian_degree;

        (*geo_keys)[7].key_id = 3081; // ProjNatOriginLatGeoKey
        (*geo_keys)[7].tiff_tag_location = 34736;
        (*geo_keys)[7].count = 1;
        (*geo_keys)[7].value_offset = 3;
        (*geo_double_params)[3] = lcc->lcc_lat_origin_degree;

        (*geo_keys)[8].key_id = 3082; // ProjFalseEastingGeoKey
        (*geo_keys)[8].tiff_tag_location = 34736;
        (*geo_keys)[8].count = 1;
        (*geo_keys)[8].value_offset = 4;
        if (source)
          (*geo_double_params)[4] = lcc->lcc_false_easting_meter / coordinates2meter;
        else
          (*geo_double_params)[4] = lcc->lcc_false_easting_meter * meter2coordinates;

        (*geo_keys)[9].key_id = 3083; // ProjFalseNorthingGeoKey
        (*geo_keys)[9].tiff_tag_location = 34736;
        (*geo_keys)[9].count = 1;
        (*geo_keys)[9].value_offset = 5;
        if (source)
          (*geo_double_params)[5] = lcc->lcc_false_northing_meter / coordinates2meter;
        else
          (*geo_double_params)[5] = lcc->lcc_false_northing_meter * meter2coordinates;

        // datum
        (*geo_keys)[10].key_id = 2050; // GeogGeodeticDatumGeoKey
        (*geo_keys)[10].tiff_tag_location = 0;
        (*geo_keys)[10].count = 1;
        (*geo_keys)[10].value_offset = get_GeogGeodeticDatumGeoKey(source);

        // ellipsoid
        (*geo_keys)[11].key_id = 2056; // GeogEllipsoidGeoKey
        (*geo_keys)[11].tiff_tag_location = 0;
        (*geo_keys)[11].count = 1;
        (*geo_keys)[11].value_offset = get_GeogEllipsoidGeoKey(source);

        // vertical units
        (*geo_keys)[12].key_id = 4099; // VerticalUnitsGeoKey
        (*geo_keys)[12].tiff_tag_location = 0;
        (*geo_keys)[12].count = 1;
        (*geo_keys)[12].value_offset = get_VerticalUnitsGeoKey(source);

        if (vertical_geokey)
        {
          // vertical datum
          (*geo_keys)[13].key_id = 4096; // VerticalCSTypeGeoKey
          (*geo_keys)[13].tiff_tag_location = 0;
          (*geo_keys)[13].count = 1;
          (*geo_keys)[13].value_offset = vertical_geokey;
        }
        return true;
      }
      else if (projection->type == GEO_PROJECTION_TM)
      {
        GeoProjectionParametersTM* tm = (GeoProjectionParametersTM*)projection;

        num_geo_keys = 12 + (vertical_geokey ? 1 : 0);
        (*geo_keys) = (GeoProjectionGeoKeys*)malloc(sizeof(GeoProjectionGeoKeys)*num_geo_keys);
        num_geo_double_params = 5;
        (*geo_double_params) = (double*)malloc(sizeof(double)*num_geo_double_params);

        // projected coordinates
        (*geo_keys)[0].key_id = 1024; // GTModelTypeGeoKey
        (*geo_keys)[0].tiff_tag_location = 0;
        (*geo_keys)[0].count = 1;
        (*geo_keys)[0].value_offset = 1; // ModelTypeProjected

        // user-defined custom TM projection 
        (*geo_keys)[1].key_id = 3072; // ProjectedCSTypeGeoKey
        (*geo_keys)[1].tiff_tag_location = 0;
        (*geo_keys)[1].count = 1;
        (*geo_keys)[1].value_offset = 32767; // user-defined

        // which projection do we use
        (*geo_keys)[2].key_id = 3075; // ProjCoordTransGeoKey
        (*geo_keys)[2].tiff_tag_location = 0;
        (*geo_keys)[2].count = 1;
        (*geo_keys)[2].value_offset = 1; // CT_TransverseMercator

        // which units do we use
        (*geo_keys)[3].key_id = 3076; // ProjCoordTransGeoKey
        (*geo_keys)[3].tiff_tag_location = 0;
        (*geo_keys)[3].count = 1;
        (*geo_keys)[3].value_offset = get_ProjLinearUnitsGeoKey(source); 

        // here come the 5 double parameters

        (*geo_keys)[4].key_id = 3088; // ProjCenterLongGeoKey
        (*geo_keys)[4].tiff_tag_location = 34736;
        (*geo_keys)[4].count = 1;
        (*geo_keys)[4].value_offset = 0;
        (*geo_double_params)[0] = tm->tm_long_meridian_degree;

        (*geo_keys)[5].key_id = 3081; // ProjNatOriginLatGeoKey
        (*geo_keys)[5].tiff_tag_location = 34736;
        (*geo_keys)[5].count = 1;
        (*geo_keys)[5].value_offset = 1;
        (*geo_double_params)[1] = tm->tm_lat_origin_degree;

        (*geo_keys)[6].key_id = 3092; // ProjScaleAtNatOriginGeoKey
        (*geo_keys)[6].tiff_tag_location = 34736;
        (*geo_keys)[6].count = 1;
        (*geo_keys)[6].value_offset = 2;
        (*geo_double_params)[2] = tm->tm_scale_factor;

        (*geo_keys)[7].key_id = 3082; // ProjFalseEastingGeoKey
        (*geo_keys)[7].tiff_tag_location = 34736;
        (*geo_keys)[7].count = 1;
        (*geo_keys)[7].value_offset = 3;
        if (source)
          (*geo_double_params)[3] = tm->tm_false_easting_meter / coordinates2meter;
        else
          (*geo_double_params)[3] = tm->tm_false_easting_meter * meter2coordinates;

        (*geo_keys)[8].key_id = 3083; // ProjFalseNorthingGeoKey
        (*geo_keys)[8].tiff_tag_location = 34736;
        (*geo_keys)[8].count = 1;
        (*geo_keys)[8].value_offset = 4;
        if (source)
          (*geo_double_params)[4] = tm->tm_false_northing_meter / coordinates2meter;
        else
          (*geo_double_params)[4] = tm->tm_false_northing_meter * meter2coordinates;

        // datum
        (*geo_keys)[9].key_id = 2050; // GeogGeodeticDatumGeoKey
        (*geo_keys)[9].tiff_tag_location = 0;
        (*geo_keys)[9].count = 1;
        (*geo_keys)[9].value_offset = get_GeogGeodeticDatumGeoKey(source);

        // ellipsoid
        (*geo_keys)[10].key_id = 2056; // GeogEllipsoidGeoKey
        (*geo_keys)[10].tiff_tag_location = 0;
        (*geo_keys)[10].count = 1;
        (*geo_keys)[10].value_offset = get_GeogEllipsoidGeoKey(source);

        // vertical units
        (*geo_keys)[11].key_id = 4099; // VerticalUnitsGeoKey
        (*geo_keys)[11].tiff_tag_location = 0;
        (*geo_keys)[11].count = 1;
        (*geo_keys)[11].value_offset = get_VerticalUnitsGeoKey(source);

        if (vertical_geokey)
        {
          // vertical datum
          (*geo_keys)[12].key_id = 4096; // VerticalCSTypeGeoKey
          (*geo_keys)[12].tiff_tag_location = 0;
          (*geo_keys)[12].count = 1;
          (*geo_keys)[12].value_offset = vertical_geokey;
        }
        return true;
      }
      else if (projection->type == GEO_PROJECTION_AEAC)
      {
        GeoProjectionParametersAEAC* aeac = (GeoProjectionParametersAEAC*)projection;

        num_geo_keys = 13 + (vertical_geokey ? 1 : 0);
        (*geo_keys) = (GeoProjectionGeoKeys*)malloc(sizeof(GeoProjectionGeoKeys)*num_geo_keys);
        num_geo_double_params = 6;
        (*geo_double_params) = (double*)malloc(sizeof(double)*num_geo_double_params);

        // projected coordinates
        (*geo_keys)[0].key_id = 1024; // GTModelTypeGeoKey
        (*geo_keys)[0].tiff_tag_location = 0;
        (*geo_keys)[0].count = 1;
        (*geo_keys)[0].value_offset = 1; // ModelTypeProjected

        // user-defined custom LCC projection 
        (*geo_keys)[1].key_id = 3072; // ProjectedCSTypeGeoKey
        (*geo_keys)[1].tiff_tag_location = 0;
        (*geo_keys)[1].count = 1;
        (*geo_keys)[1].value_offset = 32767; // user-defined

        // which projection do we use
        (*geo_keys)[2].key_id = 3075; // ProjCoordTransGeoKey
        (*geo_keys)[2].tiff_tag_location = 0;
        (*geo_keys)[2].count = 1;
        (*geo_keys)[2].value_offset = 11; // CT_AlbersEqualArea 

        // which units do we use
        (*geo_keys)[3].key_id = 3076; // ProjCoordTransGeoKey
        (*geo_keys)[3].tiff_tag_location = 0;
        (*geo_keys)[3].count = 1;
        (*geo_keys)[3].value_offset = get_ProjLinearUnitsGeoKey(source); 

        // here come the 6 double parameters

        (*geo_keys)[4].key_id = 3078; // ProjStdParallel1GeoKey
        (*geo_keys)[4].tiff_tag_location = 34736;
        (*geo_keys)[4].count = 1;
        (*geo_keys)[4].value_offset = 0;
        (*geo_double_params)[0] = aeac->aeac_first_std_parallel_degree;

        (*geo_keys)[5].key_id = 3079; // ProjStdParallel2GeoKey
        (*geo_keys)[5].tiff_tag_location = 34736;
        (*geo_keys)[5].count = 1;
        (*geo_keys)[5].value_offset = 1;
        (*geo_double_params)[1] = aeac->aeac_second_std_parallel_degree;

        (*geo_keys)[6].key_id = 3088; // ProjCenterLongGeoKey
        (*geo_keys)[6].tiff_tag_location = 34736;
        (*geo_keys)[6].count = 1;
        (*geo_keys)[6].value_offset = 2;
        (*geo_double_params)[2] = aeac->aeac_long_meridian_degree;

        (*geo_keys)[7].key_id = 3081; // ProjNatOriginLatGeoKey
        (*geo_keys)[7].tiff_tag_location = 34736;
        (*geo_keys)[7].count = 1;
        (*geo_keys)[7].value_offset = 3;
        (*geo_double_params)[3] = aeac->aeac_lat_origin_degree;

        (*geo_keys)[8].key_id = 3082; // ProjFalseEastingGeoKey
        (*geo_keys)[8].tiff_tag_location = 34736;
        (*geo_keys)[8].count = 1;
        (*geo_keys)[8].value_offset = 4;
        if (source)
          (*geo_double_params)[4] = aeac->aeac_false_easting_meter / coordinates2meter;
        else
          (*geo_double_params)[4] = aeac->aeac_false_easting_meter * meter2coordinates;

        (*geo_keys)[9].key_id = 3083; // ProjFalseNorthingGeoKey
        (*geo_keys)[9].tiff_tag_location = 34736;
        (*geo_keys)[9].count = 1;
        (*geo_keys)[9].value_offset = 5;
        if (source)
          (*geo_double_params)[5] = aeac->aeac_false_northing_meter / coordinates2meter;
        else
          (*geo_double_params)[5] = aeac->aeac_false_northing_meter * meter2coordinates;

        // datum
        (*geo_keys)[10].key_id = 2050; // GeogGeodeticDatumGeoKey
        (*geo_keys)[10].tiff_tag_location = 0;
        (*geo_keys)[10].count = 1;
        (*geo_keys)[10].value_offset = get_GeogGeodeticDatumGeoKey(source);

        // ellipsoid
        (*geo_keys)[11].key_id = 2056; // GeogEllipsoidGeoKey
        (*geo_keys)[11].tiff_tag_location = 0;
        (*geo_keys)[11].count = 1;
        (*geo_keys)[11].value_offset = get_GeogEllipsoidGeoKey(source);

        // vertical units
        (*geo_keys)[12].key_id = 4099; // VerticalUnitsGeoKey
        (*geo_keys)[12].tiff_tag_location = 0;
        (*geo_keys)[12].count = 1;
        (*geo_keys)[12].value_offset = get_VerticalUnitsGeoKey(source);

        if (vertical_geokey)
        {
          // vertical datum
          (*geo_keys)[13].key_id = 4096; // VerticalCSTypeGeoKey
          (*geo_keys)[13].tiff_tag_location = 0;
          (*geo_keys)[13].count = 1;
          (*geo_keys)[13].value_offset = vertical_geokey;
        }
        return true;
      }
      else if (projection->type == GEO_PROJECTION_OM)
      {
        GeoProjectionParametersOM* om = (GeoProjectionParametersOM*)projection;

        num_geo_keys = 13 + (vertical_geokey ? 1 : 0);
        (*geo_keys) = (GeoProjectionGeoKeys*)malloc(sizeof(GeoProjectionGeoKeys)*num_geo_keys);
        num_geo_double_params = 6;
        (*geo_double_params) = (double*)malloc(sizeof(double)*num_geo_double_params);

        // projected coordinates
        (*geo_keys)[0].key_id = 1024; // GTModelTypeGeoKey
        (*geo_keys)[0].tiff_tag_location = 0;
        (*geo_keys)[0].count = 1;
        (*geo_keys)[0].value_offset = 1; // ModelTypeProjected

        // user-defined custom LCC projection 
        (*geo_keys)[1].key_id = 3072; // ProjectedCSTypeGeoKey
        (*geo_keys)[1].tiff_tag_location = 0;
        (*geo_keys)[1].count = 1;
        (*geo_keys)[1].value_offset = 32767; // user-defined

        // which projection do we use
        (*geo_keys)[2].key_id = 3075; // ProjCoordTransGeoKey
        (*geo_keys)[2].tiff_tag_location = 0;
        (*geo_keys)[2].count = 1;
        (*geo_keys)[2].value_offset = 11; // CT_AlbersEqualArea 

        // which units do we use
        (*geo_keys)[3].key_id = 3076; // ProjCoordTransGeoKey
        (*geo_keys)[3].tiff_tag_location = 0;
        (*geo_keys)[3].count = 1;
        (*geo_keys)[3].value_offset = get_ProjLinearUnitsGeoKey(source); 

        // here come the 6 double parameters

        (*geo_keys)[4].key_id = 3078; // ProjStdParallel1GeoKey
        (*geo_keys)[4].tiff_tag_location = 34736;
        (*geo_keys)[4].count = 1;
        (*geo_keys)[4].value_offset = 0;
        (*geo_double_params)[0] = om->om_first_std_parallel_degree;

        (*geo_keys)[5].key_id = 3079; // ProjStdParallel2GeoKey
        (*geo_keys)[5].tiff_tag_location = 34736;
        (*geo_keys)[5].count = 1;
        (*geo_keys)[5].value_offset = 1;
        (*geo_double_params)[1] = om->om_second_std_parallel_degree;

        (*geo_keys)[6].key_id = 3088; // ProjCenterLongGeoKey
        (*geo_keys)[6].tiff_tag_location = 34736;
        (*geo_keys)[6].count = 1;
        (*geo_keys)[6].value_offset = 2;
        (*geo_double_params)[2] = om->om_long_meridian_degree;

        (*geo_keys)[7].key_id = 3081; // ProjNatOriginLatGeoKey
        (*geo_keys)[7].tiff_tag_location = 34736;
        (*geo_keys)[7].count = 1;
        (*geo_keys)[7].value_offset = 3;
        (*geo_double_params)[3] = om->om_lat_origin_degree;

        (*geo_keys)[8].key_id = 3082; // ProjFalseEastingGeoKey
        (*geo_keys)[8].tiff_tag_location = 34736;
        (*geo_keys)[8].count = 1;
        (*geo_keys)[8].value_offset = 4;
        if (source)
          (*geo_double_params)[4] = om->om_false_easting_meter / coordinates2meter;
        else
          (*geo_double_params)[4] = om->om_false_easting_meter * meter2coordinates;

        (*geo_keys)[9].key_id = 3083; // ProjFalseNorthingGeoKey
        (*geo_keys)[9].tiff_tag_location = 34736;
        (*geo_keys)[9].count = 1;
        (*geo_keys)[9].value_offset = 5;
        if (source)
          (*geo_double_params)[5] = om->om_false_northing_meter / coordinates2meter;
        else
          (*geo_double_params)[5] = om->om_false_northing_meter * meter2coordinates;

        // datum
        (*geo_keys)[10].key_id = 2050; // GeogGeodeticDatumGeoKey
        (*geo_keys)[10].tiff_tag_location = 0;
        (*geo_keys)[10].count = 1;
        (*geo_keys)[10].value_offset = get_GeogGeodeticDatumGeoKey(source);

        // ellipsoid
        (*geo_keys)[11].key_id = 2056; // GeogEllipsoidGeoKey
        (*geo_keys)[11].tiff_tag_location = 0;
        (*geo_keys)[11].count = 1;
        (*geo_keys)[11].value_offset = get_GeogEllipsoidGeoKey(source);

        // vertical units
        (*geo_keys)[12].key_id = 4099; // VerticalUnitsGeoKey
        (*geo_keys)[12].tiff_tag_location = 0;
        (*geo_keys)[12].count = 1;
        (*geo_keys)[12].value_offset = get_VerticalUnitsGeoKey(source);

        if (vertical_geokey)
        {
          // vertical datum
          (*geo_keys)[13].key_id = 4096; // VerticalCSTypeGeoKey
          (*geo_keys)[13].tiff_tag_location = 0;
          (*geo_keys)[13].count = 1;
          (*geo_keys)[13].value_offset = vertical_geokey;
        }
        return true;
      }
      else
      {
        fprintf(stderr, "get_geo_keys_from_projection for generic UTM not implemented\n");
      }
    }
    else if (projection->type == GEO_PROJECTION_LAT_LONG || projection->type == GEO_PROJECTION_LONG_LAT)
    {
      num_geo_keys = 3 + (vertical_geokey ? 1 : 0);
      (*geo_keys) = (GeoProjectionGeoKeys*)malloc(sizeof(GeoProjectionGeoKeys)*num_geo_keys);
      (*geo_double_params) = 0;

      // projected coordinates
      (*geo_keys)[0].key_id = 1024; // GTModelTypeGeoKey
      (*geo_keys)[0].tiff_tag_location = 0;
      (*geo_keys)[0].count = 1;
      (*geo_keys)[0].value_offset = 2; // ModelTypeGeographic

      // ellipsoid used with latitude/longitude coordinates
      (*geo_keys)[1].key_id = 2048; // GeographicTypeGeoKey
      (*geo_keys)[1].tiff_tag_location = 0;
      (*geo_keys)[1].count = 1;
      (*geo_keys)[1].value_offset = get_GeographicTypeGeoKey(source);

      // vertical units
      (*geo_keys)[2].key_id = 4099; // VerticalUnitsGeoKey
      (*geo_keys)[2].tiff_tag_location = 0;
      (*geo_keys)[2].count = 1;
      (*geo_keys)[2].value_offset = get_VerticalUnitsGeoKey(source);

      if (vertical_geokey)
      {
        // vertical datum
        (*geo_keys)[3].key_id = 4096; // VerticalCSTypeGeoKey
        (*geo_keys)[3].tiff_tag_location = 0;
        (*geo_keys)[3].count = 1;
        (*geo_keys)[3].value_offset = vertical_geokey;
      }
      return true;
    }
    else if (projection->type == GEO_PROJECTION_ECEF)
    {
      num_geo_keys = 2;
      (*geo_keys) = (GeoProjectionGeoKeys*)malloc(sizeof(GeoProjectionGeoKeys)*num_geo_keys);
      (*geo_double_params) = 0;

      // projected coordinates
      (*geo_keys)[0].key_id = 1024; // GTModelTypeGeoKey
      (*geo_keys)[0].tiff_tag_location = 0;
      (*geo_keys)[0].count = 1;
      (*geo_keys)[0].value_offset = 3; // ModelTypeGeocentric

      // ellipsoid used with earth centered coodinates
      (*geo_keys)[1].key_id = 2048; // GeographicTypeGeoKey
      (*geo_keys)[1].tiff_tag_location = 0;
      (*geo_keys)[1].count = 1;
      (*geo_keys)[1].value_offset = get_GeographicTypeGeoKey(source);
      return true;
    }
  }
  return false;
}

bool GeoProjectionConverter::set_projection_from_geo_keys(int num_geo_keys, GeoProjectionGeoKeys* geo_keys, char* geo_ascii_params, double* geo_double_params, char* description)
{
  bool user_defined_ellipsoid = false;
  int user_defined_projection = 0;
  int offsetProjStdParallel1GeoKey = -1;
  int offsetProjStdParallel2GeoKey = -1;
  int offsetProjNatOriginLatGeoKey = -1;
  int offsetProjFalseEastingGeoKey = -1;
  int offsetProjFalseNorthingGeoKey = -1;
  int offsetProjCenterLongGeoKey = -1;
  int offsetProjScaleAtNatOriginGeoKey = -1;
  bool has_projection = false;
  int ellipsoid = -1;

  this->num_geo_keys = num_geo_keys;
  this->geo_keys = geo_keys;
  this->geo_ascii_params = geo_ascii_params;
  this->geo_double_params = geo_double_params;

  for (int i = 0; i < num_geo_keys; i++)
  {
    switch (geo_keys[i].key_id)
    {
    case 1024: // GTModelTypeGeoKey
      if (geo_keys[i].value_offset == 2) // ModelTypeGeographic
      {
        has_projection = set_longlat_projection(description);
      }
      else if (geo_keys[i].value_offset == 3) // ModelTypeGeocentric
      {
        has_projection = set_ecef_projection(description);
      }
      else if (geo_keys[i].value_offset == 0) // ModelTypeUndefined
      {
        has_projection = set_no_projection(description);
      }
      break;
    case 2048: // GeographicTypeGeoKey
      switch (geo_keys[i].value_offset)
      {
      case 32767: // user-defined GCS
        user_defined_ellipsoid = true;
        break;
      case 4001: // GCSE_Airy1830
        ellipsoid = 1;
        break;
      case 4002: // GCSE_AiryModified1849 
        ellipsoid = 16;
        break;
      case 4003: // GCSE_AustralianNationalSpheroid
        ellipsoid = 2;
        break;
      case 4004: // GCSE_Bessel1841
      case 4005: // GCSE_Bessel1841Modified
        ellipsoid = 3;
        break;
      case 4006: // GCSE_BesselNamibia
        ellipsoid = 4;
        break;
      case 4008: // GCSE_Clarke1866
      case 4009: // GCSE_Clarke1866Michigan
        ellipsoid = GEO_ELLIPSOID_NAD27;
        break;
      case 4010: // GCSE_Clarke1880_Benoit
      case 4011: // GCSE_Clarke1880_IGN
      case 4012: // GCSE_Clarke1880_RGS
      case 4013: // GCSE_Clarke1880_Arc
      case 4014: // GCSE_Clarke1880_SGA1922
      case 4034: // GCSE_Clarke1880
        ellipsoid = 6;
        break;
      case 4015: // GCSE_Everest1830_1937Adjustment
      case 4016: // GCSE_Everest1830_1967Definition
      case 4017: // GCSE_Everest1830_1975Definition
        ellipsoid = 7;
        break;
      case 4018: // GCSE_Everest1830Modified
        ellipsoid = 17;
        break;
      case 4019: // GCSE_GRS1980
        ellipsoid = GEO_ELLIPSOID_NAD83;
        break;
      case 4020: // GCSE_Helmert1906
        ellipsoid = 12;
        break;
      case 4022: // GCSE_International1924
      case 4023: // GCSE_International1967
        ellipsoid = 14;
        break;
      case 4024: // GCSE_Krassowsky1940
        ellipsoid = 15;
        break;
      case 4030: // GCSE_WGS84
        ellipsoid = GEO_ELLIPSOID_WGS84;
        break;
      case 4267: // GCS_NAD27
        ellipsoid = GEO_ELLIPSOID_NAD27;
        break;
      case 4269: // GCS_NAD83
        ellipsoid = GEO_ELLIPSOID_NAD83;
        break;
      case 4283: // GCS_GDA94
        ellipsoid = GEO_ELLIPSOID_GDA94;
        break;
      case 4322: // GCS_WGS_72
        ellipsoid = GEO_ELLIPSOID_WGS72;
        break;
      case 4326: // GCS_WGS_84
        ellipsoid = GEO_ELLIPSOID_WGS84;
        break;
      default:
        fprintf(stderr, "GeographicTypeGeoKey: look-up for %d not implemented\n", geo_keys[i].value_offset);
      }
      break;
    case 2050: // GeogGeodeticDatumGeoKey 
      switch (geo_keys[i].value_offset)
      {
      case 32767: // user-defined GCS
        user_defined_ellipsoid = true;
        break;
      case 6202: // Datum_Australian_Geodetic_Datum_1966
      case 6203: // Datum_Australian_Geodetic_Datum_1984
        ellipsoid = 2;
        break;
      case 6267: // Datum_North_American_Datum_1927
        ellipsoid = GEO_ELLIPSOID_NAD27;
        break;
      case 6269: // Datum_North_American_Datum_1983
        ellipsoid = GEO_ELLIPSOID_NAD83;
        break;
      case 6283: // Datum_Geocentric_Datum_of_Australia_1994
        ellipsoid = GEO_ELLIPSOID_GDA94;
        break;
      case 6322: // Datum_WGS72
        ellipsoid = GEO_ELLIPSOID_WGS72;
        break;
      case 6326: // Datum_WGS84
        ellipsoid = GEO_ELLIPSOID_WGS84;
        break;
      case 6001: // DatumE_Airy1830
        ellipsoid = 1;
        break;
      case 6002: // DatumE_AiryModified1849
        ellipsoid = 16;
        break;
      case 6003: // DatumE_AustralianNationalSpheroid
        ellipsoid = 2;
        break;
      case 6004: // DatumE_Bessel1841
      case 6005: // DatumE_BesselModified
        ellipsoid = 3;
        break;
      case 6006: // DatumE_BesselNamibia
        ellipsoid = 4;
        break;
      case 6008: // DatumE_Clarke1866
      case 6009: // DatumE_Clarke1866Michigan
        ellipsoid = GEO_ELLIPSOID_NAD27;
        break;
      case 6010: // DatumE_Clarke1880_Benoit
      case 6011: // DatumE_Clarke1880_IGN
      case 6012: // DatumE_Clarke1880_RGS
      case 6013: // DatumE_Clarke1880_Arc
      case 6014: // DatumE_Clarke1880_SGA1922
      case 6034: // DatumE_Clarke1880
        ellipsoid = 6;
        break;
      case 6015: // DatumE_Everest1830_1937Adjustment
      case 6016: // DatumE_Everest1830_1967Definition
      case 6017: // DatumE_Everest1830_1975Definition
        ellipsoid = 7;
        break;
      case 6018: // DatumE_Everest1830Modified
        ellipsoid = 17;
        break;
      case 6019: // DatumE_GRS1980
        ellipsoid = GEO_ELLIPSOID_NAD83;
        break;
      case 6020: // DatumE_Helmert1906
        ellipsoid = 12;
        break;
      case 6022: // DatumE_International1924
      case 6023: // DatumE_International1967
        ellipsoid = 14;
        break;
      case 6024: // DatumE_Krassowsky1940
        ellipsoid = 15;
        break;
      case 6030: // DatumE_WGS84
        ellipsoid = GEO_ELLIPSOID_WGS84;
        break;
      default:
        fprintf(stderr, "GeogGeodeticDatumGeoKey: look-up for %d not implemented\n", geo_keys[i].value_offset);
      }
      break;
    case 2052: // GeogLinearUnitsGeoKey 
      switch (geo_keys[i].value_offset)
      {
      case 9001: // Linear_Meter
        set_coordinates_in_meter();
        break;
      case 9002: // Linear_Foot
        set_coordinates_in_feet();
        break;
      case 9003: // Linear_Foot_US_Survey
        set_coordinates_in_survey_feet();
        break;
      default:
        fprintf(stderr, "GeogLinearUnitsGeoKey: look-up for %d not implemented\n", geo_keys[i].value_offset);
      }
      break;
    case 2056: // GeogEllipsoidGeoKey
      switch (geo_keys[i].value_offset)
      {
      case 7001: // Ellipse_Airy_1830
        ellipsoid = 1;
        break;
      case 7002: // Ellipse_Airy_Modified_1849
        ellipsoid = 16;
        break;
      case 7003: // Ellipse_Australian_National_Spheroid
        ellipsoid = 2;
        break;
      case 7004: // Ellipse_Bessel_1841
      case 7005: // Ellipse_Bessel_Modified
        ellipsoid = 3;
        break;
      case 7006: // Ellipse_Bessel_Namibia
        ellipsoid = 4;
        break;
      case 7008: // Ellipse_Clarke_1866
      case 7009: // Ellipse_Clarke_1866_Michigan
        ellipsoid = GEO_ELLIPSOID_NAD27;
        break;
      case 7010: // Ellipse_Clarke1880_Benoit
      case 7011: // Ellipse_Clarke1880_IGN
      case 7012: // Ellipse_Clarke1880_RGS
      case 7013: // Ellipse_Clarke1880_Arc
      case 7014: // Ellipse_Clarke1880_SGA1922
      case 7034: // Ellipse_Clarke1880
        ellipsoid = 6;
        break;
      case 7015: // Ellipse_Everest1830_1937Adjustment
      case 7016: // Ellipse_Everest1830_1967Definition
      case 7017: // Ellipse_Everest1830_1975Definition
        ellipsoid = 7;
        break;
      case 7018: // Ellipse_Everest1830Modified
        ellipsoid = 17;
        break;
      case 7019: // Ellipse_GRS_1980
        ellipsoid = GEO_ELLIPSOID_NAD83;
        break;
      case 7020: // Ellipse_Helmert1906
        ellipsoid = 12;
        break;
      case 7022: // Ellipse_International1924
      case 7023: // Ellipse_International1967
        ellipsoid = 14;
        break;
      case 7024: // Ellipse_Krassowsky1940
        ellipsoid = 15;
        break;
      case 7030: // Ellipse_WGS_84
        ellipsoid = GEO_ELLIPSOID_WGS84;
        break;
      default:
        fprintf(stderr, "GeogEllipsoidGeoKey: look-up for %d not implemented\n", geo_keys[i].value_offset);
      }
      break;
    case 3072: // ProjectedCSTypeGeoKey
      has_projection = set_ProjectedCSTypeGeoKey(geo_keys[i].value_offset, description);
      break;
    case 3075: // ProjCoordTransGeoKey
      user_defined_projection = 0;
      switch (geo_keys[i].value_offset)
      {
      case 1: // CT_TransverseMercator
        user_defined_projection = 1;
        break;
      case 8: // CT_LambertConfConic_2SP
        user_defined_projection = 8;
        break;
      case 11: // CT_AlbersEqualArea
        user_defined_projection = 11;
        break;
      case 2: // CT_TransvMercator_Modified_Alaska
        fprintf(stderr, "ProjCoordTransGeoKey: CT_TransvMercator_Modified_Alaska not implemented\n");
        break;
      case 3: // CT_ObliqueMercator
        fprintf(stderr, "ProjCoordTransGeoKey: CT_ObliqueMercator not implemented\n");
        break;
      case 4: // CT_ObliqueMercator_Laborde
        fprintf(stderr, "ProjCoordTransGeoKey: CT_ObliqueMercator_Laborde not implemented\n");
        break;
      case 5: // CT_ObliqueMercator_Rosenmund
        fprintf(stderr, "ProjCoordTransGeoKey: CT_ObliqueMercator_Rosenmund not implemented\n");
        break;
      case 6: // CT_ObliqueMercator_Spherical
        fprintf(stderr, "ProjCoordTransGeoKey: CT_ObliqueMercator_Spherical not implemented\n");
        break;
      case 7: // CT_Mercator
        fprintf(stderr, "ProjCoordTransGeoKey: CT_Mercator not implemented\n");
        break;
      case 9: // CT_LambertConfConic_Helmert
        fprintf(stderr, "ProjCoordTransGeoKey: CT_LambertConfConic_Helmert not implemented\n");
        break;
      case 10: // CT_LambertAzimEqualArea
        fprintf(stderr, "ProjCoordTransGeoKey: CT_LambertAzimEqualArea not implemented\n");
        break;
      case 12: // CT_AzimuthalEquidistant
        fprintf(stderr, "ProjCoordTransGeoKey: CT_AzimuthalEquidistant not implemented\n");
        break;
      case 13: // CT_EquidistantConic
        fprintf(stderr, "ProjCoordTransGeoKey: CT_EquidistantConic not implemented\n");
        break;
      case 14: // CT_Stereographic
        fprintf(stderr, "ProjCoordTransGeoKey: CT_Stereographic not implemented\n");
        break;
      case 15: // CT_PolarStereographic
        fprintf(stderr, "ProjCoordTransGeoKey: CT_PolarStereographic not implemented\n");
        break;
      case 16: // CT_ObliqueStereographic
        fprintf(stderr, "ProjCoordTransGeoKey: CT_ObliqueStereographic not implemented\n");
        break;
      case 17: // CT_Equirectangular
        fprintf(stderr, "ProjCoordTransGeoKey: CT_Equirectangular not implemented\n");
        break;
      case 18: // CT_CassiniSoldner
        fprintf(stderr, "ProjCoordTransGeoKey: CT_CassiniSoldner not implemented\n");
        break;
      case 19: // CT_Gnomonic
        fprintf(stderr, "ProjCoordTransGeoKey: CT_Gnomonic not implemented\n");
        break;
      case 20: // CT_MillerCylindrical
        fprintf(stderr, "ProjCoordTransGeoKey: CT_MillerCylindrical not implemented\n");
        break;
      case 21: // CT_Orthographic
        fprintf(stderr, "ProjCoordTransGeoKey: CT_Orthographic not implemented\n");
        break;
      case 22: // CT_Polyconic
        fprintf(stderr, "ProjCoordTransGeoKey: CT_Polyconic not implemented\n");
        break;
      case 23: // CT_Robinson
        fprintf(stderr, "ProjCoordTransGeoKey: CT_Robinson not implemented\n");
        break;
      case 24: // CT_Sinusoidal
        fprintf(stderr, "ProjCoordTransGeoKey: CT_Sinusoidal not implemented\n");
        break;
      case 25: // CT_VanDerGrinten
        fprintf(stderr, "ProjCoordTransGeoKey: CT_VanDerGrinten not implemented\n");
        break;
      case 26: // CT_NewZealandMapGrid
        fprintf(stderr, "ProjCoordTransGeoKey: CT_NewZealandMapGrid not implemented\n");
        break;
      case 27: // CT_TransvMercator_SouthOriented
        fprintf(stderr, "ProjCoordTransGeoKey: CT_TransvMercator_SouthOriented not implemented\n");
        break;
      default:
        fprintf(stderr, "ProjCoordTransGeoKey: look-up for %d not implemented\n", geo_keys[i].value_offset);
      }
      break;
    case 3076: // ProjLinearUnitsGeoKey
      set_ProjLinearUnitsGeoKey(geo_keys[i].value_offset);
      break;
    case 3078: // ProjStdParallel1GeoKey
      offsetProjStdParallel1GeoKey = geo_keys[i].value_offset;
      break;
    case 3079: // ProjStdParallel2GeoKey
      offsetProjStdParallel2GeoKey = geo_keys[i].value_offset;
      break;        
    case 3081: // ProjNatOriginLatGeoKey
      offsetProjNatOriginLatGeoKey = geo_keys[i].value_offset;
      break;
    case 3082: // ProjFalseEastingGeoKey
      offsetProjFalseEastingGeoKey = geo_keys[i].value_offset;
      break;
    case 3083: // ProjFalseNorthingGeoKey
      offsetProjFalseNorthingGeoKey = geo_keys[i].value_offset;
      break;
    case 3088: // ProjCenterLongGeoKey
      offsetProjCenterLongGeoKey = geo_keys[i].value_offset;
      break;
    case 3092: // ProjScaleAtNatOriginGeoKey
      offsetProjScaleAtNatOriginGeoKey = geo_keys[i].value_offset;
      break;
    case 4096: // VerticalCSTypeGeoKey 
      set_VerticalCSTypeGeoKey(geo_keys[i].value_offset);
      break;
    case 4099: // VerticalUnitsGeoKey
      set_VerticalUnitsGeoKey(geo_keys[i].value_offset);
      break;
    }
  }

  if (ellipsoid != -1)
  {
    set_reference_ellipsoid(ellipsoid);
  }

  if (user_defined_projection)
  {
    if (user_defined_projection == 1)
    {
      if ((offsetProjFalseEastingGeoKey >= 0) &&
          (offsetProjFalseNorthingGeoKey >= 0) &&
          (offsetProjNatOriginLatGeoKey >= 0) &&
          (offsetProjCenterLongGeoKey >= 0) &&
          (offsetProjScaleAtNatOriginGeoKey >= 0))
      {
        double falseEastingMeter = geo_double_params[offsetProjFalseEastingGeoKey] * coordinates2meter;
        double falseNorthingMeter = geo_double_params[offsetProjFalseNorthingGeoKey] * coordinates2meter;
        double latOriginDeg = geo_double_params[offsetProjNatOriginLatGeoKey];
        double longMeridianDeg = geo_double_params[offsetProjCenterLongGeoKey];
        double scaleFactor = geo_double_params[offsetProjScaleAtNatOriginGeoKey];
        set_transverse_mercator_projection(falseEastingMeter, falseNorthingMeter, latOriginDeg, longMeridianDeg, scaleFactor);
        if (description)
        {
          sprintf(description, "generic transverse mercator");
        }
        has_projection = true;
      }
    }
    else if (user_defined_projection == 8)
    {
      if ((offsetProjFalseEastingGeoKey >= 0) &&
          (offsetProjFalseNorthingGeoKey >= 0) &&
          (offsetProjNatOriginLatGeoKey >= 0) &&
          (offsetProjCenterLongGeoKey >= 0) &&
          (offsetProjStdParallel1GeoKey >= 0) &&
          (offsetProjStdParallel2GeoKey >= 0))
      {
        double falseEastingMeter = geo_double_params[offsetProjFalseEastingGeoKey] * coordinates2meter;
        double falseNorthingMeter = geo_double_params[offsetProjFalseNorthingGeoKey] * coordinates2meter;
        double latOriginDeg = geo_double_params[offsetProjNatOriginLatGeoKey];
        double longOriginDeg = geo_double_params[offsetProjCenterLongGeoKey];
        double firstStdParallelDeg = geo_double_params[offsetProjStdParallel1GeoKey];
        double secondStdParallelDeg = geo_double_params[offsetProjStdParallel2GeoKey];
        set_lambert_conformal_conic_projection(falseEastingMeter, falseNorthingMeter, latOriginDeg, longOriginDeg, firstStdParallelDeg, secondStdParallelDeg);
        if (description)
        {
          sprintf(description, "generic lambert conformal conic");
        }
        has_projection = true;
      }
    }
  }

  return has_projection;
}

short GeoProjectionConverter::get_GTModelTypeGeoKey()
{
  if (num_geo_keys)
  {
    for (int i = 0; i < num_geo_keys; i++)
    {
      if (geo_keys[i].key_id == 1024)
      {
        return geo_keys[i].value_offset;
      }
    }
  }
  return 0;
//  return 2; // assume ModelTypeGeographic
}

short GeoProjectionConverter::get_GTRasterTypeGeoKey()
{
  if (num_geo_keys)
  {
    for (int i = 0; i < num_geo_keys; i++)
    {
      if (geo_keys[i].key_id == 1025)
      {
        return geo_keys[i].value_offset;
      }
    }
  }
  return 0;
//  return 1; // assume RasterPixelIsArea
}

short GeoProjectionConverter::get_GeographicTypeGeoKey(bool source)
{
  if (num_geo_keys)
  {
    for (int i = 0; i < num_geo_keys; i++)
    {
      if (geo_keys[i].key_id == 2048)
      {
        return geo_keys[i].value_offset;
      }
    }
  }
  switch (ellipsoid->id)
  {
  case 1: // GCSE_Airy1830
    return 4001;
  case 2: // GCSE_AustralianNationalSpheroid
    return 4003;
  case 3: // GCSE_Bessel1841
    return 4004;
  case 4: // GCSE_BesselNamibia
    return 4006;
  case GEO_ELLIPSOID_NAD27: // GCS_NAD27
    return 4267;
  case 6: // GCSE_Clarke1880
    return 4034;
  case GEO_ELLIPSOID_NAD83: // GCS_NAD83
    return 4269;
  case 12: // GCSE_Helmert1906
    return 4020;
  case 14: // GCSE_International1924
    return 4022;
  case 15: // GCSE_Krassowsky1940
    return 4024;
  case 16: // GCSE_AiryModified1849 
    return 4002;
  case 17: // GCSE_Everest1830Modified
    return 4018;
  case GEO_ELLIPSOID_WGS72: // GCS_WGS_72
    return 4322;
  case GEO_ELLIPSOID_WGS84: // GCS_WGS_84
    return 4326;
  default:
    fprintf(stderr, "GeographicTypeGeoKey: look-up for ellipsoid with id %d not implemented\n", ellipsoid->id);
  }
  return 0;
}

short GeoProjectionConverter::get_GeogGeodeticDatumGeoKey(bool source)
{
  if (num_geo_keys)
  {
    for (int i = 0; i < num_geo_keys; i++)
    {
      if (geo_keys[i].key_id == 2050)
      {
        return geo_keys[i].value_offset;
      }
    }
  }
  switch (ellipsoid->id)
  {
  case 1: // DatumE_Airy1830
    return 6001;
  case 2: // DatumE_AustralianNationalSpheroid
    return 6003;
  case 3: // DatumE_Bessel1841
    return 6004;
  case 4: // DatumE_BesselNamibia
    return 6006;
  case 5: // Datum_North_American_Datum_1927
    return 6267;
  case 6: // DatumE_Clarke1880
    return 6034;
  case 11: // Datum_North_American_Datum_1983
    return 6269;
  case 12: // DatumE_Helmert1906
    return 6020;
  case 14: // DatumE_International1924
    return 6022;
  case 15: // DatumE_Krassowsky1940
    return 6024;
  case 16: // DatumE_AiryModified1849 
    return 6002;
  case 17: // DatumE_Everest1830Modified
    return 6018;
  case 22: // Datum_WGS72
    return 6322;
  case 23: // Datum_WGS84
    return 6326;
  default:
    fprintf(stderr, "GeogGeodeticDatumGeoKey: look-up for ellipsoid with id %d not implemented\n", ellipsoid->id);
  }
  return 0;
}

short GeoProjectionConverter::get_GeogPrimeMeridianGeoKey()
{
  if (num_geo_keys)
  {
    for (int i = 0; i < num_geo_keys; i++)
    {
      if (geo_keys[i].key_id == 2051)
      {
        return geo_keys[i].value_offset;
      }
    }
  }
  return 0;
}

short GeoProjectionConverter::get_GeogLinearUnitsGeoKey()
{
  if (coordinates2meter == 1.0)
  {
    return 9001; // Linear_Meter
  }
  else if (coordinates2meter == 0.3048)
  {
    return 9002; // Linear_Foot
  }
  else
  {
    return 9003; // assume Linear_Foot_US_Survey
  }
}

double GeoProjectionConverter::get_GeogLinearUnitSizeGeoKey()
{
  if (num_geo_keys && geo_double_params)
  {
    for (int i = 0; i < num_geo_keys; i++)
    {
      if (geo_keys[i].key_id == 2053)
      {
        return geo_double_params[geo_keys[i].value_offset];
      }
    }
  }
  return 0;
}

short GeoProjectionConverter::get_GeogAngularUnitsGeoKey()
{
  if (num_geo_keys)
  {
    for (int i = 0; i < num_geo_keys; i++)
    {
      if (geo_keys[i].key_id == 2054)
      {
        return geo_keys[i].value_offset;
      }
    }
  }
  return 0;
//  return 9102; // assume Angular_Degree
}

double GeoProjectionConverter::get_GeogAngularUnitSizeGeoKey()
{
  if (num_geo_keys && geo_double_params)
  {
    for (int i = 0; i < num_geo_keys; i++)
    {
      if (geo_keys[i].key_id == 2055)
      {
        return geo_double_params[geo_keys[i].value_offset];
      }
    }
  }
  return 0;
}

short GeoProjectionConverter::get_GeogEllipsoidGeoKey(bool source)
{
  if (num_geo_keys)
  {
    for (int i = 0; i < num_geo_keys; i++)
    {
      if (geo_keys[i].key_id == 2056)
      {
        return geo_keys[i].value_offset;
      }
    }
  }
  switch (ellipsoid->id)
  {
  case 1: // Ellipse_Airy_1830
    return 7001;
  case 2: // Ellipse_Australian_National_Spheroid
    return 7003;
  case 3: // Ellipse_Bessel_1841
    return 7004;
  case 4: // Ellipse_Bessel_Namibia
    return 7006;
  case 5: // Ellipse_Clarke_1866
    return 7008;
  case 6: // Ellipse_Clarke1880
    return 7034;
  case 11: // Ellipse_GRS_1980
    return 7019;
  case 12: // Ellipse_Helmert1906
    return 7020;
  case 14: // Ellipse_International1924
    return 7022;
  case 15: // Ellipse_Krassowsky1940
    return 7024;
  case 16: // Ellipse_Airy_Modified_1849
    return 7002;
  case 23: // Ellipse_WGS_84
    return 7030;
  default:
    fprintf(stderr, "GeogEllipsoidGeoKey: look-up for ellipsoid with id %d not implemented\n", ellipsoid->id);
  }
  return 0;
}

double GeoProjectionConverter::get_GeogSemiMajorAxisGeoKey()
{
  if (num_geo_keys && geo_double_params)
  {
    for (int i = 0; i < num_geo_keys; i++)
    {
      if (geo_keys[i].key_id == 2057)
      {
        return geo_double_params[geo_keys[i].value_offset];
      }
    }
  }
  return 0;
}

double GeoProjectionConverter::get_GeogSemiMinorAxisGeoKey()
{
  if (num_geo_keys && geo_double_params)
  {
    for (int i = 0; i < num_geo_keys; i++)
    {
      if (geo_keys[i].key_id == 2058)
      {
        return geo_double_params[geo_keys[i].value_offset];
      }
    }
  }
  return 0;
}

double GeoProjectionConverter::get_GeogInvFlatteningGeoKey()
{
  if (num_geo_keys && geo_double_params)
  {
    for (int i = 0; i < num_geo_keys; i++)
    {
      if (geo_keys[i].key_id == 2059)
      {
        return geo_double_params[geo_keys[i].value_offset];
      }
    }
  }
  return 0;
}

short GeoProjectionConverter::get_GeogAzimuthUnitsGeoKey()
{
  if (num_geo_keys)
  {
    for (int i = 0; i < num_geo_keys; i++)
    {
      if (geo_keys[i].key_id == 2060)
      {
        return geo_keys[i].value_offset;
      }
    }
  }
  return 0;
//  return 9102; // assume Angular_Degree
}

double GeoProjectionConverter::get_GeogPrimeMeridianLongGeoKey()
{
  if (num_geo_keys && geo_double_params)
  {
    for (int i = 0; i < num_geo_keys; i++)
    {
      if (geo_keys[i].key_id == 2061)
      {
        return geo_double_params[geo_keys[i].value_offset];
      }
    }
  }
  return 0;
}

bool GeoProjectionConverter::set_ProjectedCSTypeGeoKey(short value, char* description)
{
  if (value == 32767)
  {
    if (description)
    {
      sprintf(description, "user-defined");
    }
    return true;
  }
  else if (set_epsg_code(value, description))
  {
    return true;
  }
  fprintf(stderr, "set_ProjectedCSTypeGeoKey: look-up for %d not implemented\n", value);
  return false;
}

short GeoProjectionConverter::get_ProjectedCSTypeGeoKey(bool source)
{
  if (source && num_geo_keys)
  {
    int i;
    for (i = 0; i < num_geo_keys; i++)
    {
      if (geo_keys[i].key_id == 3072)
      {
        return geo_keys[i].value_offset;
      }
    }
  }
  GeoProjectionParameters* projection = get_projection(source);
  if (projection)
  {
    if (projection->type == GEO_PROJECTION_UTM)
    {
      if (projection->geokey)
      {
        return projection->geokey;
      }
      else
      {
        GeoProjectionParametersUTM* utm = (GeoProjectionParametersUTM*)projection;
        if (ellipsoid->id == GEO_ELLIPSOID_WGS84)
        {
          if (utm->utm_northern_hemisphere)
          {
            return utm->utm_zone_number + 32600;
          }
          else
          {
            return utm->utm_zone_number + 32700;
          }
        }
        else if (ellipsoid->id == GEO_ELLIPSOID_WGS72)
        {
          if (utm->utm_northern_hemisphere)
          {
            return utm->utm_zone_number + 32200;
          }
          else
          {
            return utm->utm_zone_number + 32300;
          }
        }
        else if (ellipsoid->id == GEO_ELLIPSOID_NAD83)
        {
          if (utm->utm_northern_hemisphere)
          {
            if (3 <= utm->utm_zone_number && utm->utm_zone_number <= 23)
            {
              return utm->utm_zone_number + 26900;
            }
            else if (28 <= utm->utm_zone_number && utm->utm_zone_number <= 38)
            {
              return utm->utm_zone_number + 25800;
            }
            else
            {
              fprintf(stderr, "get_ProjectedCSTypeGeoKey: northern UTM zone %d for NAD83 out-of-range\n", utm->utm_zone_number);
            }
          }
          else
          {
            fprintf(stderr, "get_ProjectedCSTypeGeoKey: southern UTM zone %d for NAD83 does not exist\n", utm->utm_zone_number);
          }
        }
        else if (ellipsoid->id == GEO_ELLIPSOID_NAD27)
        {
          if (utm->utm_northern_hemisphere)
          {
            if (3 <= utm->utm_zone_number && utm->utm_zone_number <= 22)
            {
              return utm->utm_zone_number + 26700;
            }
            else
            {
              fprintf(stderr, "get_ProjectedCSTypeGeoKey: northern UTM zone %d for NAD27 out-of-range\n", utm->utm_zone_number);
            }
          }
          else
          {
            fprintf(stderr, "get_ProjectedCSTypeGeoKey: southern UTM zone %d for NAD27 does not exist\n", utm->utm_zone_number);
          }
        }
        else if (ellipsoid->id == GEO_ELLIPSOID_SAD69)
        {
          if (utm->utm_northern_hemisphere)
          {
            if (18 <= utm->utm_zone_number && utm->utm_zone_number <= 22)
            {
              return utm->utm_zone_number + 29100;
            }
            else
            {
              fprintf(stderr, "get_ProjectedCSTypeGeoKey: northern UTM zone %d for SAD69 out-of-range\n", utm->utm_zone_number);
            }
          }
          else
          {
            if (17 <= utm->utm_zone_number && utm->utm_zone_number <= 25)
            {
              return utm->utm_zone_number + 29160;
            }
            else
            {
              fprintf(stderr, "get_ProjectedCSTypeGeoKey: southern UTM zone %d for SAD69 out-of-range\n", utm->utm_zone_number);
            }
          }
        }
        else if (ellipsoid->id == GEO_ELLIPSOID_Inter)
        {
          if (utm->utm_northern_hemisphere)
          {
            if (28 <= utm->utm_zone_number && utm->utm_zone_number <= 38)
            {
              return utm->utm_zone_number + 23000;
            }
            else
            {
              fprintf(stderr, "get_ProjectedCSTypeGeoKey: northern UTM zone %d for ED50 out-of-range\n", utm->utm_zone_number);
            }
          }
          else
          {
            fprintf(stderr, "get_ProjectedCSTypeGeoKey: southern UTM zone %d for ED50 does not exist\n", utm->utm_zone_number);
          }
        }
        else if (ellipsoid->id == GEO_ELLIPSOID_ID74)
        {
          if (utm->utm_northern_hemisphere)
          {
            if (46 <= utm->utm_zone_number && utm->utm_zone_number <= 53)
            {
              return utm->utm_zone_number + 23800;
            }
            else
            {
              fprintf(stderr, "get_ProjectedCSTypeGeoKey: northern UTM zone %d for ID74 out-of-range\n", utm->utm_zone_number);
            }
          }
          else
          {
            if (46 <= utm->utm_zone_number && utm->utm_zone_number <= 54)
            {
              return utm->utm_zone_number + 23840;
            }
            else
            {
              fprintf(stderr, "get_ProjectedCSTypeGeoKey: southern UTM zone %d for ID74 out-of-range\n", utm->utm_zone_number);
            }
          }
        }
        else
        {
          fprintf(stderr, "get_ProjectedCSTypeGeoKey: look-up for UTM zone %d and ellipsoid with id %d not implemented\n", utm->utm_zone_number, ellipsoid->id);
        }
      }
    }
    else if (projection->type == GEO_PROJECTION_LCC)
    {
      if (projection->geokey)
      {
        return projection->geokey;
      }
      else
      {
        return 32767; // user-defined GCS
      }
    }
    else if (projection->type == GEO_PROJECTION_TM)
    {
      if (projection->geokey)
      {
        return projection->geokey;
      }
      else
      {
        return 32767; // user-defined GCS
      }
    }
    else if (projection->type == GEO_PROJECTION_LONG_LAT)
    {
      return 4326;
    }
    else if (projection->type == GEO_PROJECTION_LAT_LONG)
    {
      return 4326;
    }
    else if (projection->type == GEO_PROJECTION_ECEF)
    {
      return 4978;
    }
    else if (projection->type == GEO_PROJECTION_AEAC)
    {
      if (projection->geokey)
      {
        return projection->geokey;
      }
      else
      {
        return 32767; // user-defined GCS
      }
    }
    else if (projection->type == GEO_PROJECTION_OM)
    {
      if (projection->geokey)
      {
        return projection->geokey;
      }
      else
      {
        return 32767; // user-defined GCS
      }
    }
  }
  return 0;
}

void GeoProjectionConverter::set_ProjLinearUnitsGeoKey(short value)
{
  switch (value)
  {
  case 9001: // Linear_Meter
    set_coordinates_in_meter();
    break;
  case 9002: // Linear_Foot
    set_coordinates_in_feet();
    break;
  case 9003: // Linear_Foot_US_Survey
    set_coordinates_in_survey_feet();
    break;
  default:
    fprintf(stderr, "set_ProjLinearUnitsGeoKey: look-up for %d not implemented\n", value);
  }
}

short GeoProjectionConverter::get_ProjLinearUnitsGeoKey(bool source)
{
  if (source)
  {
    if (source_projection && (source_projection->type == GEO_PROJECTION_LONG_LAT || source_projection->type == GEO_PROJECTION_LAT_LONG))
    {
      return 9000;
    }
    else if (coordinates2meter == 1.0)
    {
      return 9001; // Linear_Meter
    }
    else if (coordinates2meter == 0.3048)
    {
      return 9002; // Linear_Foot
    }
    else
    {
      return 9003; // assume Linear_Foot_US_Survey
    }
  }
  else
  {
    if (target_projection && (target_projection->type == GEO_PROJECTION_LONG_LAT || target_projection->type == GEO_PROJECTION_LAT_LONG))
    {
      return 9000;
    }
    else if (meter2coordinates == 1.0)
    {
      return 9001; // Linear_Meter
    }
    else if (meter2coordinates == 1.0/0.3048)
    {
      return 9002; // Linear_Foot
    }
    else
    {
      return 9003; // assume Linear_Foot_US_Survey
    }
  }
}

void GeoProjectionConverter::set_VerticalUnitsGeoKey(short value)
{
  switch (value)
  {
  case 9001: // Linear_Meter
    set_elevation_in_meter();
    break;
  case 9002: // Linear_Foot
    set_elevation_in_feet();
    break;
  case 9003: // Linear_Foot_US_Survey
    set_elevation_in_survey_feet();
    break;
  default:
    fprintf(stderr, "set_VerticalUnitsGeoKey: look-up for %d not implemented\n", value);
  }
}

short GeoProjectionConverter::get_VerticalUnitsGeoKey(bool source)
{
  if (source)
  {
    if (elevation2meter == 1.0)
    {
      return 9001; // Linear_Meter
    }
    else if (elevation2meter == 0.3048)
    {
      return 9002; // Linear_Foot
    }
    else
    {
      return 9003; // assume Linear_Foot_US_Survey
    }
  }
  else
  {
    if (meter2elevation == 1.0)
    {
      return 9001; // Linear_Meter
    }
    else if (meter2elevation == 1.0/0.3048)
    {
      return 9002; // Linear_Foot
    }
    else
    {
      return 9003; // assume Linear_Foot_US_Survey
    }
  }
}

void GeoProjectionConverter::set_VerticalCSTypeGeoKey(short value)
{
  if ((5000 <= value) && (value <= 5099))      // [5000, 5099] = EPSG Ellipsoid Vertical CS Codes
  {
    vertical_geokey = value;
  }
  else if ((5101 <= value) && (value <= 5199)) // [5100, 5199] = EPSG Orthometric Vertical CS Codes
  {
    vertical_geokey = value;
  }
  else if ((5200 <= value) && (value <= 5999)) // [5200, 5999] = Reserved EPSG
  {
    vertical_geokey = value;
  }
  else
  {
    fprintf(stderr, "set_VerticalCSTypeGeoKey: look-up for %d not implemented\012", value);
  }
}

short GeoProjectionConverter::get_VerticalCSTypeGeoKey()
{
  if (num_geo_keys)
  {
    int i;
    for (i = 0; i < num_geo_keys; i++)
    {
      if (geo_keys[i].key_id == 4096)
      {
        return geo_keys[i].value_offset;
      }
    }
  }
  return vertical_geokey;
}

bool GeoProjectionConverter::set_reference_ellipsoid(int id, char* description)
{
  if (id <= 0 || id >= 25)
  {
    return false;
  }

  ellipsoid->id = id;
  ellipsoid->name = ellipsoid_list[id].name;
  ellipsoid->equatorial_radius = ellipsoid_list[id].equatorialRadius;
  ellipsoid->eccentricity_squared = ellipsoid_list[id].eccentricitySquared;
  ellipsoid->inverse_flattening = ellipsoid_list[id].inverseFlattening;
  ellipsoid->eccentricity_prime_squared = (ellipsoid->eccentricity_squared)/(1-ellipsoid->eccentricity_squared);
  ellipsoid->polar_radius = ellipsoid->equatorial_radius*sqrt(1-ellipsoid->eccentricity_squared);    
  ellipsoid->eccentricity = sqrt(ellipsoid->eccentricity_squared);
  ellipsoid->eccentricity_e1 = (1-sqrt(1-ellipsoid->eccentricity_squared))/(1+sqrt(1-ellipsoid->eccentricity_squared));

  compute_lcc_parameters(true);
  compute_tm_parameters(true);
  compute_lcc_parameters(false);
  compute_tm_parameters(false);

  if (description)
  {
    sprintf(description, "%2d - %s (%g %g)", ellipsoid->id, ellipsoid->name, ellipsoid->equatorial_radius, ellipsoid->eccentricity_squared);
  }

  return true;
}

int GeoProjectionConverter::get_ellipsoid_id() const
{
  return ellipsoid->id;
}

const char* GeoProjectionConverter::get_ellipsoid_name() const
{
  return ellipsoid->name;
}

void GeoProjectionConverter::set_projection(GeoProjectionParameters* projection, bool source)
{
  if (source)
  {
    if (source_projection) delete source_projection;
    source_projection = projection;
  }
  else
  {
    if (target_projection) delete target_projection;
    target_projection = projection;
  }
}

void GeoProjectionConverter::set_geokey(short geokey, bool source)
{
  if (source)
  {
    if (source_projection)
    {
      source_projection->geokey = geokey;
    }
    else
    {
      fprintf(stderr, "WARNING: source_projection not set despite geokey %d\n", geokey);
    }
  }
  else
  {
    if (target_projection)
    {
      target_projection->geokey = geokey;
    }
    else
    {
      fprintf(stderr, "WARNING: target_projection not set despite geokey %d\n", geokey);
    }
  }
}

void GeoProjectionConverter::check_geokey(short geokey, bool source)
{
  if (source)
  {
    if (source_projection)
    {
      if (source_projection->geokey != geokey)
      {
        fprintf(stderr, "WARNING: source_projection->geokey %d != geokey %d\n", source_projection->geokey, geokey);
      }
    }
    else
    {
      fprintf(stderr, "WARNING: source_projection not set despite geokey %d\n", geokey);
    }
  }
  else
  {
    if (target_projection)
    {
      if (target_projection->geokey != geokey)
      {
        fprintf(stderr, "WARNING: target_projection->geokey %d != geokey %d\n", target_projection->geokey, geokey);
      }
    }
    else
    {
      fprintf(stderr, "WARNING: target_projection not set despite geokey %d\n", geokey);
    }
  }
}

GeoProjectionParameters* GeoProjectionConverter::get_projection(bool source) const
{
  if (source)
  {
    return source_projection;
  }
  else
  {
    return target_projection;
  }
}

bool GeoProjectionConverter::set_latlong_projection(char* description, bool source)
{
  GeoProjectionParameters* latlong = new GeoProjectionParameters();
  latlong->type = GEO_PROJECTION_LAT_LONG;
  sprintf(latlong->name, "latitude/longitude");
  set_projection(latlong, source);
  if (description)
  {
    sprintf(description, "%s", latlong->name);
  }
  return true;
}

bool GeoProjectionConverter::set_no_projection(char* description, bool source)
{
  GeoProjectionParameters* no = new GeoProjectionParameters();
  no->type = GEO_PROJECTION_NONE;
  sprintf(no->name, "intentionally no projection");
  set_projection(no, source);

  if (description)
  {
    sprintf(description, "%s", no->name);
  }
  return true;
}

bool GeoProjectionConverter::set_longlat_projection(char* description, bool source)
{
  GeoProjectionParameters* longlat = new GeoProjectionParameters();
  longlat->type = GEO_PROJECTION_LONG_LAT;
  sprintf(longlat->name, "longitude/latitude");
  set_projection(longlat, source);

  if (description)
  {
    sprintf(description, "%s", longlat->name);
  }
  return true;
}

bool GeoProjectionConverter::set_ecef_projection(char* description, bool source)
{
  GeoProjectionParameters* ecef = new GeoProjectionParameters();
  ecef->type = GEO_PROJECTION_ECEF;
  sprintf(ecef->name, "earth-centered earth-fixed");
  set_projection(ecef, source);

  if (description)
  {
    sprintf(description, "%s", ecef->name);
  }
  return true;
}

bool GeoProjectionConverter::set_utm_projection(char* zone, char* description, bool source)
{
  int zone_number;
  char* zone_letter;
  zone_number = strtoul(zone, &zone_letter, 10);
  if (*zone_letter < 'C' || *zone_letter > 'X')
  {
    return false;
  }
  GeoProjectionParametersUTM* utm = new GeoProjectionParametersUTM();
  utm->type = GEO_PROJECTION_UTM;
  utm->utm_zone_number = zone_number;
  utm->utm_zone_letter = *zone_letter;
  if((*zone_letter - 'N') >= 0)
  {
    utm->utm_northern_hemisphere = true; // point is in northern hemisphere
  }
  else
  {
    utm->utm_northern_hemisphere = false; //point is in southern hemisphere
  }
  sprintf(utm->name, "UTM zone %s (%s)", zone, (utm->utm_northern_hemisphere ? "northern hemisphere" : "southern hemisphere"));
  utm->utm_long_origin = (zone_number - 1) * 6 - 180 + 3; // + 3 puts origin in middle of zone
  set_projection(utm, source);
  if (description)
  {
    sprintf(description, "UTM %d %s", zone_number, (utm->utm_northern_hemisphere ? "northern hemisphere" : "southern hemisphere"));
  }
  return true;
}

bool GeoProjectionConverter::set_utm_projection(int zone, bool northern, char* description, bool source)
{
  GeoProjectionParametersUTM* utm = new GeoProjectionParametersUTM();
  utm->type = GEO_PROJECTION_UTM;
  utm->utm_zone_number = zone;
  utm->utm_zone_letter = ' ';
  utm->utm_northern_hemisphere = northern;
  sprintf(utm->name, "UTM zone %d (%s)", zone, (utm->utm_northern_hemisphere ? "northern hemisphere" : "southern hemisphere"));
  utm->utm_long_origin = (zone - 1) * 6 - 180 + 3;  // + 3 puts origin in middle of zone
  set_projection(utm, source);
  if (description)
  {
    sprintf(description, "UTM %d %s", zone, (utm->utm_northern_hemisphere ? "northern hemisphere" : "southern hemisphere"));
  }
  return true;
}

bool GeoProjectionConverter::set_target_utm_projection(char* description)
{
  GeoProjectionParametersUTM* utm = new GeoProjectionParametersUTM();
  utm->type = GEO_PROJECTION_UTM;
  utm->utm_zone_number = -1;
  set_projection(utm, false);
  if (description)
  {
    sprintf(description, "auto select");
  }
  return true;
}

// Configure a Lambert Conic Conformal Projection
//
// The function set_lambert_conformal_conic_projection() receives the Lambert
// Conformal Conic  projection parameters as inputs and sets the corresponding
// state variables.
//
// falseEastingMeter & falseNorthingMeter are just an offset in meters added 
// to the final coordinate calculated.
//
// latOriginDegree & longMeridianDegree are the "center" latitiude and
// longitude in decimal degrees of the area being projected. All coordinates
// will be calculated in meters relative to this point on the earth.
//
// firstStdParallelDegree & secondStdParallelDegree are the two lines of
// longitude in decimal degrees (that is they run east-west) that define
// where the "cone" intersects the earth. They bracket the area being projected.
void GeoProjectionConverter::set_lambert_conformal_conic_projection(double falseEastingMeter, double falseNorthingMeter, double latOriginDegree, double longMeridianDegree, double firstStdParallelDegree, double secondStdParallelDegree, char* description, bool source)
{
  GeoProjectionParametersLCC* lcc = new GeoProjectionParametersLCC();
  lcc->type = GEO_PROJECTION_LCC;
  sprintf(lcc->name, "Lambert Conformal Conic");
  lcc->lcc_false_easting_meter = falseEastingMeter;
  lcc->lcc_false_northing_meter = falseNorthingMeter;
  lcc->lcc_lat_origin_degree = latOriginDegree;
  lcc->lcc_long_meridian_degree = longMeridianDegree;
  lcc->lcc_first_std_parallel_degree = firstStdParallelDegree;
  lcc->lcc_second_std_parallel_degree = secondStdParallelDegree;
  lcc->lcc_lat_origin_radian = deg2rad*lcc->lcc_lat_origin_degree;
  lcc->lcc_long_meridian_radian = deg2rad*lcc->lcc_long_meridian_degree;
  lcc->lcc_first_std_parallel_radian = deg2rad*lcc->lcc_first_std_parallel_degree;
  lcc->lcc_second_std_parallel_radian = deg2rad*lcc->lcc_second_std_parallel_degree;
  set_projection(lcc, source);
  compute_lcc_parameters(source);
  if (description)
  {
    sprintf(description, "false east/north: %g/%g [m], origin lat/ meridian long: %g/%g, parallel 1st/2nd: %g/%g", lcc->lcc_false_easting_meter, lcc->lcc_false_northing_meter, lcc->lcc_lat_origin_degree, lcc->lcc_long_meridian_degree, lcc->lcc_first_std_parallel_degree, lcc->lcc_second_std_parallel_degree);
  }
}

/*
  * The function set_transverse_mercator_projection() receives the Tranverse
  * Mercator projection parameters as input and sets the corresponding state
  * variables. 
  * falseEastingMeter   : Easting/X in meters at the center of the projection
  * falseNorthingMeter  : Northing/Y in meters at the center of the projection
  * latOriginDegree     : Latitude in decimal degree at the origin of the projection
  * longMeridianDegree  : Longitude n decimal degree at the center of the projection
  * scaleFactor         : Projection scale factor
*/
void GeoProjectionConverter::set_transverse_mercator_projection(double falseEastingMeter, double falseNorthingMeter, double latOriginDegree, double longMeridianDegree, double scaleFactor, char* description, bool source)
{
  GeoProjectionParametersTM* tm = new GeoProjectionParametersTM();
  tm->type = GEO_PROJECTION_TM;
  sprintf(tm->name, "Transverse Mercator");
  tm->tm_false_easting_meter = falseEastingMeter;
  tm->tm_false_northing_meter = falseNorthingMeter;
  tm->tm_lat_origin_degree = latOriginDegree;
  tm->tm_long_meridian_degree = longMeridianDegree;
  tm->tm_scale_factor = scaleFactor;
  tm->tm_lat_origin_radian = deg2rad*tm->tm_lat_origin_degree;
  tm->tm_long_meridian_radian = deg2rad*tm->tm_long_meridian_degree;
  set_projection(tm, source);
  compute_tm_parameters(source);
  if (description)
  {
    sprintf(description, "false east/north: %g/%g [m], origin lat/meridian long: %g/%g, scale: %g", tm->tm_false_easting_meter, tm->tm_false_northing_meter, tm->tm_lat_origin_degree, tm->tm_long_meridian_degree, tm->tm_scale_factor);
  }
}

// Configure a Albers Equal Area Conic Projection
//
// The function set_albers_equal_area_conic_projection() receives the Albers 
// Equal Area Conic projection parameters as inputs and sets the corresponding
// state variables.
//
// falseEastingMeter & falseNorthingMeter are just an offset in meters added 
// to the final coordinate calculated.
//
// latOriginDegree & longMeridianDegree are the "center" latitiude and
// longitude in decimal degrees of the area being projected. All coordinates
// will be calculated in meters relative to this point on the earth.
//
// firstStdParallelDegree & secondStdParallelDegree are the two lines of
// longitude in decimal degrees (that is they run east-west) that define
// where the "cone" intersects the earth. They bracket the area being projected.
void GeoProjectionConverter::set_albers_equal_area_conic_projection(double falseEastingMeter, double falseNorthingMeter, double latOriginDegree, double longMeridianDegree, double firstStdParallelDegree, double secondStdParallelDegree, char* description, bool source)
{
  GeoProjectionParametersAEAC* aeac = new GeoProjectionParametersAEAC();
  aeac->type = GEO_PROJECTION_AEAC;
  sprintf(aeac->name, "Albers Equal Area Conic");
  aeac->aeac_false_easting_meter = falseEastingMeter;
  aeac->aeac_false_northing_meter = falseNorthingMeter;
  aeac->aeac_lat_origin_degree = latOriginDegree;
  aeac->aeac_long_meridian_degree = longMeridianDegree;
  aeac->aeac_first_std_parallel_degree = firstStdParallelDegree;
  aeac->aeac_second_std_parallel_degree = secondStdParallelDegree;
  aeac->aeac_lat_origin_radian = deg2rad*aeac->aeac_lat_origin_degree;
  aeac->aeac_long_meridian_radian = deg2rad*aeac->aeac_long_meridian_degree;
  aeac->aeac_first_std_parallel_radian = deg2rad*aeac->aeac_first_std_parallel_degree;
  aeac->aeac_second_std_parallel_radian = deg2rad*aeac->aeac_second_std_parallel_degree;
  set_projection(aeac, source);
  compute_aeac_parameters(source);
  if (description)
  {
    sprintf(description, "false east/north: %g/%g [m], origin lat/ meridian long: %g/%g, parallel 1st/2nd: %g/%g", aeac->aeac_false_easting_meter, aeac->aeac_false_northing_meter, aeac->aeac_lat_origin_degree, aeac->aeac_long_meridian_degree, aeac->aeac_first_std_parallel_degree, aeac->aeac_second_std_parallel_degree);
  }
}

// Configure an Oblique Mercator Projection
//
// The function set_albers_equal_area_conic_projection() receives the Oblique  
// Mercator projection parameters as inputs and sets the corresponding state
// variables.
//
// falseEastingMeter & falseNorthingMeter are just an offset in meters added 
// to the final coordinate calculated.
//
// latOriginDegree & longMeridianDegree are the "center" latitiude and
// longitude in decimal degrees of the area being projected. All coordinates
// will be calculated in meters relative to this point on the earth.
//
// firstStdParallelDegree & secondStdParallelDegree are the two lines of
// longitude in decimal degrees (that is they run east-west) that define
// where the "cone" intersects the earth. They bracket the area being projected.
void GeoProjectionConverter::set_oblique_mercator_projection(double falseEastingMeter, double falseNorthingMeter, double latOriginDegree, double longMeridianDegree, double firstStdParallelDegree, double secondStdParallelDegree, char* description, bool source)
{
  GeoProjectionParametersOM* om = new GeoProjectionParametersOM();
  om->type = GEO_PROJECTION_OM;
  sprintf(om->name, "Oblique Mercator");
  om->om_false_easting_meter = falseEastingMeter;
  om->om_false_northing_meter = falseNorthingMeter;
  om->om_lat_origin_degree = latOriginDegree;
  om->om_long_meridian_degree = longMeridianDegree;
  om->om_first_std_parallel_degree = firstStdParallelDegree;
  om->om_second_std_parallel_degree = secondStdParallelDegree;
  om->om_lat_origin_radian = deg2rad*om->om_lat_origin_degree;
  om->om_long_meridian_radian = deg2rad*om->om_long_meridian_degree;
  om->om_first_std_parallel_radian = deg2rad*om->om_first_std_parallel_degree;
  om->om_second_std_parallel_radian = deg2rad*om->om_second_std_parallel_degree;
  set_projection(om, source);
  compute_om_parameters(source);
  if (description)
  {
    sprintf(description, "false east/north: %g/%g [m], origin lat/ meridian long: %g/%g, parallel 1st/2nd: %g/%g", om->om_false_easting_meter, om->om_false_northing_meter, om->om_lat_origin_degree, om->om_long_meridian_degree, om->om_first_std_parallel_degree, om->om_second_std_parallel_degree);
  }
}

const char* GeoProjectionConverter::get_epsg_code_description(int i) const
{
  if (epsg_code_list[i].description)
  {
    return epsg_code_list[i].description;
  }
  return 0;
}

short GeoProjectionConverter::get_epsg_code_value(int i) const
{
  if (epsg_code_list[i].description)
  {
    return epsg_code_list[i].geokey;
  }
  return 0;
}

bool GeoProjectionConverter::set_epsg_code(short value, char* description, bool source)
{
  int ellipsoid = -1;
  bool utm_northern = false;
  int utm_zone = -1;
  bool is_mga = false;
  char* sp = 0;
  bool sp_nad27 = false;
  bool ecef = false;

  switch (value)
  {
  case 3154: // NAD83(CSRS) / UTM zone 7N
  case 3155: // NAD83(CSRS) / UTM zone 8N
  case 3156: // NAD83(CSRS) / UTM zone 9N
  case 3157: // NAD83(CSRS) / UTM zone 10N
    utm_northern = true; utm_zone = value - 3154 + 7;
    ellipsoid = GEO_ELLIPSOID_NAD83;
    break;
  case 3158: // NAD83(CSRS) / UTM zone 14N
  case 3159: // NAD83(CSRS) / UTM zone 15N
  case 3160: // NAD83(CSRS) / UTM zone 16N
    utm_northern = true; utm_zone = value - 3158 + 14;
    ellipsoid = GEO_ELLIPSOID_NAD83;
    break;
  case 4978:
    ecef = true;
    ellipsoid = GEO_ELLIPSOID_WGS84;
    break;
  case 20137: // PCS_Adindan_UTM_zone_37N
  case 20138: // PCS_Adindan_UTM_zone_38N
    utm_northern = true; utm_zone = value-20100;
    break;
  case 20437: // PCS_Ain_el_Abd_UTM_zone_37N
  case 20438: // PCS_Ain_el_Abd_UTM_zone_38N
  case 20439: // PCS_Ain_el_Abd_UTM_zone_39N
    utm_northern = true; utm_zone = value-20400;
    break;                
  case 20538: // PCS_Afgooye_UTM_zone_38N
  case 20539: // PCS_Afgooye_UTM_zone_39N
    utm_northern = true; utm_zone = value-20500;
    break;
  case 20822: // PCS_Aratu_UTM_zone_22S
  case 20823: // PCS_Aratu_UTM_zone_23S
  case 20824: // PCS_Aratu_UTM_zone_24S
    utm_northern = false; utm_zone = value-20800;
    break;
  case 21148: // PCS_Batavia_UTM_zone_48S
  case 21149: // PCS_Batavia_UTM_zone_49S
  case 21150: // PCS_Batavia_UTM_zone_50S
    utm_northern = false; utm_zone = value-21100;
    break;
  case 21817: // PCS_Bogota_UTM_zone_17N
  case 21818: // PCS_Bogota_UTM_zone_18N
    utm_northern = true; utm_zone = value-21800;
    break;
  case 22032: // PCS_Camacupa_UTM_32S
  case 22033: // PCS_Camacupa_UTM_33S
    utm_northern = false; utm_zone = value-22000;
    break; 
  case 22332: // PCS_Carthage_UTM_zone_32N
    utm_northern = true; utm_zone = 32;
    break; 
  case 22523: // PCS_Corrego_Alegre_UTM_23S
  case 22524: // PCS_Corrego_Alegre_UTM_24S
    utm_northern = false; utm_zone = value-22500;
    break;
  case 22832: // PCS_Douala_UTM_zone_32N
    utm_northern = true; utm_zone = 32;
    break;
  case 23028: // PCS_ED50_UTM_zone_28N
  case 23029: // PCS_ED50_UTM_zone_29N
  case 23030: 
  case 23031: 
  case 23032: 
  case 23033: 
  case 23034: 
  case 23035: 
  case 23036: 
  case 23037: 
  case 23038: // PCS_ED50_UTM_zone_38N
    utm_northern = true; utm_zone = value-23000;
    ellipsoid = GEO_ELLIPSOID_Inter;
    break;
  case 23239: // PCS_Fahud_UTM_zone_39N
  case 23240: // PCS_Fahud_UTM_zone_40N
    utm_northern = true; utm_zone = value-23200;
    break;
  case 23433: // PCS_Garoua_UTM_zone_33N
    utm_northern = true; utm_zone = 33;
    break;
  case 23846: // PCS_ID74_UTM_zone_46N
  case 23847: // PCS_ID74_UTM_zone_47N
  case 23848:
  case 23849:
  case 23850:
  case 23851:
  case 23852:
  case 23853: // PCS_ID74_UTM_zone_53N
    utm_northern = true; utm_zone = value-23800;
    ellipsoid = GEO_ELLIPSOID_ID74;
    break;
  case 23886: // PCS_ID74_UTM_zone_46S
  case 23887: // PCS_ID74_UTM_zone_47S
  case 23888:
  case 23889:
  case 23890:
  case 23891:
  case 23892:
  case 23893:
  case 23894: // PCS_ID74_UTM_zone_54S
    utm_northern = false; utm_zone = value-23840;
    ellipsoid = GEO_ELLIPSOID_ID74;
    break;
  case 23947: // PCS_Indian_1954_UTM_47N
  case 23948: // PCS_Indian_1954_UTM_48N
    utm_northern = true; utm_zone = value-23900;
    break;
  case 24047: // PCS_Indian_1975_UTM_47N
  case 24048: // PCS_Indian_1975_UTM_48N
    utm_northern = true; utm_zone = value-24000;
    break;
  case 24547: // PCS_Kertau_UTM_zone_47N
  case 24548: // PCS_Kertau_UTM_zone_48N
    utm_northern = true; utm_zone = value-24500;
    break;
  case 24720: // PCS_La_Canoa_UTM_zone_20N
  case 24721: // PCS_La_Canoa_UTM_zone_21N
    utm_northern = true; utm_zone = value-24700;
    break;
  case 24818: // PCS_PSAD56_UTM_zone_18N
  case 24819: // PCS_PSAD56_UTM_zone_19N
  case 24820: // PCS_PSAD56_UTM_zone_20N
  case 24821: // PCS_PSAD56_UTM_zone_21N
    utm_northern = true; utm_zone = value-24800;
    break;
  case 24877: // PCS_PSAD56_UTM_zone_17S
  case 24878: // PCS_PSAD56_UTM_zone_18S
  case 24879: // PCS_PSAD56_UTM_zone_19S
  case 24880: // PCS_PSAD56_UTM_zone_20S
    utm_northern = false; utm_zone = value-24860;
    break;
  case 25231: // PCS_Lome_UTM_zone_31N
    utm_northern = true; utm_zone = 31;
    break;
  case 25828: // PCS_ETRS89_UTM_zone_28N
  case 25829: // PCS_ETRS89_UTM_zone_29N
  case 25830: // PCS_ETRS89_UTM_zone_30N
  case 25831: // PCS_ETRS89_UTM_zone_31N
  case 25832: // PCS_ETRS89_UTM_zone_32N
  case 25833: // PCS_ETRS89_UTM_zone_33N
  case 25834: // PCS_ETRS89_UTM_zone_34N
  case 25835: // PCS_ETRS89_UTM_zone_35N
  case 25836: // PCS_ETRS89_UTM_zone_36N
  case 25837: // PCS_ETRS89_UTM_zone_37N
  case 25838: // PCS_ETRS89_UTM_zone_38N
    utm_northern = true; utm_zone = value-25800;
    ellipsoid = GEO_ELLIPSOID_NAD83;
    break;
  case 25932: // PCS_Malongo_1987_UTM_32S
    utm_northern = false; utm_zone = 32;
    break;
  case 26237: // PCS_Massawa_UTM_zone_37N
    utm_northern = true; utm_zone = 37;
    break;
  case 26331: // PCS_Minna_UTM_zone_31N
  case 26332: // PCS_Minna_UTM_zone_32N
    utm_northern = true; utm_zone = value-26300;
    break;
  case 26432: // PCS_Mhast_UTM_zone_32S
    utm_northern = false; utm_zone = 32;
    break;
  case 26632: // PCS_M_poraloko_UTM_32N
    utm_northern = true; utm_zone = 32;
    break;
  case 26692: // PCS_Minna_UTM_zone_32S
    utm_northern = false; utm_zone = 32;
    break;
  case 26703: // PCS_NAD27_UTM_zone_3N
  case 26704: // PCS_NAD27_UTM_zone_4N
  case 26705: // PCS_NAD27_UTM_zone_5N
  case 26706: // PCS_NAD27_UTM_zone_6N
  case 26707: // PCS_NAD27_UTM_zone_7N
  case 26708: // PCS_NAD27_UTM_zone_7N
  case 26709: // PCS_NAD27_UTM_zone_9N
  case 26710: // PCS_NAD27_UTM_zone_10N
  case 26711: // PCS_NAD27_UTM_zone_11N
  case 26712: // PCS_NAD27_UTM_zone_12N
  case 26713: // PCS_NAD27_UTM_zone_13N
  case 26714: // PCS_NAD27_UTM_zone_14N
  case 26715: // PCS_NAD27_UTM_zone_15N
  case 26716: // PCS_NAD27_UTM_zone_16N
  case 26717: // PCS_NAD27_UTM_zone_17N
  case 26718: // PCS_NAD27_UTM_zone_18N
  case 26719: // PCS_NAD27_UTM_zone_19N
  case 26720: // PCS_NAD27_UTM_zone_20N
  case 26721: // PCS_NAD27_UTM_zone_21N
  case 26722: // PCS_NAD27_UTM_zone_22N
    utm_northern = true; utm_zone = value-26700;
    ellipsoid = GEO_ELLIPSOID_NAD27;
    break;
  case 26729: // PCS_NAD27_Alabama_East
    sp_nad27 = true; sp = "AL_E";
    break;
  case 26730: // PCS_NAD27_Alabama_West
    sp_nad27 = true; sp = "AL_W";
    break;
  case 26731: // PCS_NAD27_Alaska_zone_1
    sp_nad27 = true; sp = "AK_1";
    break;
  case 26732: // PCS_NAD27_Alaska_zone_2
    sp_nad27 = true; sp = "AK_2";
    break;
  case 26733: // PCS_NAD27_Alaska_zone_3
    sp_nad27 = true; sp = "AK_3";
    break;
  case 26734: // PCS_NAD27_Alaska_zone_4
    sp_nad27 = true; sp = "AK_4";
    break;
  case 26735: // PCS_NAD27_Alaska_zone_5
    sp_nad27 = true; sp = "AK_5";
    break;
  case 26736: // PCS_NAD27_Alaska_zone_6
    sp_nad27 = true; sp = "AK_6";
    break;
  case 26737: // PCS_NAD27_Alaska_zone_7
    sp_nad27 = true; sp = "AK_7";
    break;
  case 26738: // PCS_NAD27_Alaska_zone_8
    sp_nad27 = true; sp = "AK_8";
    break;
  case 26739: // PCS_NAD27_Alaska_zone_9
    sp_nad27 = true; sp = "AK_9";
    break;
  case 26740: // PCS_NAD27_Alaska_zone_10
    sp_nad27 = true; sp = "AK_10";
    break;
  case 26741: // PCS_NAD27_California_I
    sp_nad27 = true; sp = "CA_I";
    break;
  case 26742: // PCS_NAD27_California_II
    sp_nad27 = true; sp = "CA_II";
    break;
  case 26743: // PCS_NAD27_California_III
    sp_nad27 = true; sp = "CA_III";
    break;
  case 26744: // PCS_NAD27_California_IV
    sp_nad27 = true; sp = "CA_IV";
    break;
  case 26745: // PCS_NAD27_California_V
    sp_nad27 = true; sp = "CA_V";
    break;
  case 26746: // PCS_NAD27_California_VI
    sp_nad27 = true; sp = "CA_VI";
    break;
  case 26747: // PCS_NAD27_California_VII
    sp_nad27 = true; sp = "CA_VII";
    break;
  case 26748: // PCS_NAD27_Arizona_East
    sp_nad27 = true; sp = "AZ_E";
    break;
  case 26749: // PCS_NAD27_Arizona_Central
    sp_nad27 = true; sp = "AZ_C";
    break;
  case 26750: // PCS_NAD27_Arizona_West
    sp_nad27 = true; sp = "AZ_W";
    break;
  case 26751: // PCS_NAD27_Arkansas_North
    sp_nad27 = true; sp = "AR_N";
    break;
  case 26752: // PCS_NAD27_Arkansas_South
    sp_nad27 = true; sp = "AR_S";
    break;
  case 26753: // PCS_NAD27_Colorado_North
    sp_nad27 = true; sp = "CO_N";
    break;
  case 26754: // PCS_NAD27_Colorado_Central
    sp_nad27 = true; sp = "CO_C";
    break;
  case 26755: // PCS_NAD27_Colorado_South
    sp_nad27 = true; sp = "CO_S";
    break;
  case 26756: // PCS_NAD27_Connecticut
    sp_nad27 = true; sp = "CT";
    break;
  case 26757: // PCS_NAD27_Delaware
    sp_nad27 = true; sp = "DE";
    break;
  case 26758: // PCS_NAD27_Florida_East
    sp_nad27 = true; sp = "FL_E";
    break;
  case 26759: // PCS_NAD27_Florida_West
    sp_nad27 = true; sp = "FL_W";
    break;
  case 26760: // PCS_NAD27_Florida_North
    sp_nad27 = true; sp = "FL_N";
    break;
  case 26761: // PCS_NAD27_Hawaii_zone_1
    sp_nad27 = true; sp = "HI_1";
    break;
  case 26762: // PCS_NAD27_Hawaii_zone_2
    sp_nad27 = true; sp = "HI_2";
    break;
  case 26763: // PCS_NAD27_Hawaii_zone_3
    sp_nad27 = true; sp = "HI_3";
    break;
  case 26764: // PCS_NAD27_Hawaii_zone_4
    sp_nad27 = true; sp = "HI_4";
    break;
  case 26765: // PCS_NAD27_Hawaii_zone_5
    sp_nad27 = true; sp = "HI_5";
    break;
  case 26766: // PCS_NAD27_Georgia_East
    sp_nad27 = true; sp = "GA_E";
    break;
  case 26767: // PCS_NAD27_Georgia_West
    sp_nad27 = true; sp = "GA_W";
    break;
  case 26768: // PCS_NAD27_Idaho_East
    sp_nad27 = true; sp = "ID_E";
    break;
  case 26769: // PCS_NAD27_Idaho_Central
    sp_nad27 = true; sp = "ID_C";
    break;
  case 26770: // PCS_NAD27_Idaho_West
    sp_nad27 = true; sp = "ID_W";
    break;
  case 26771: // PCS_NAD27_Illinois_East
    sp_nad27 = true; sp = "IL_E";
    break;
  case 26772: // PCS_NAD27_Illinois_West
    sp_nad27 = true; sp = "IL_W";
    break;
  case 26773: // PCS_NAD27_Indiana_East
    sp_nad27 = true; sp = "IN_E";
    break;
  case 26774: // PCS_NAD27_Indiana_West
    sp_nad27 = true; sp = "IN_W";
    break;
  case 26775: // PCS_NAD27_Iowa_North
    sp_nad27 = true; sp = "IA_N";
    break;
  case 26776: // PCS_NAD27_Iowa_South
    sp_nad27 = true; sp = "IA_S";
    break;
  case 26777: // PCS_NAD27_Kansas_North
    sp_nad27 = true; sp = "KS_N";
    break;
  case 26778: // PCS_NAD27_Kansas_South
    sp_nad27 = true; sp = "KS_S";
    break;
  case 26779: // PCS_NAD27_Kentucky_North
    sp_nad27 = true; sp = "KY_N";
    break;
  case 26780: // PCS_NAD27_Kentucky_South
    sp_nad27 = true; sp = "KY_S";
    break;
  case 26781: // PCS_NAD27_Louisiana_North
    sp_nad27 = true; sp = "LA_N";
    break;
  case 26782: // PCS_NAD27_Louisiana_South
    sp_nad27 = true; sp = "LA_S";
    break;
  case 26783: // PCS_NAD27_Maine_East
    sp_nad27 = true; sp = "ME_E";
    break;
  case 26784: // PCS_NAD27_Maine_West
    sp_nad27 = true; sp = "ME_W";
    break;
  case 26785: // PCS_NAD27_Maryland
    sp_nad27 = true; sp = "MD";
    break;
  case 26786: // PCS_NAD27_Massachusetts
    sp_nad27 = true; sp = "M_M";
    break;
  case 26787: // PCS_NAD27_Massachusetts_Is
    sp_nad27 = true; sp = "M_I";
    break;
  case 26788: // PCS_NAD27_Michigan_North
    sp_nad27 = true; sp = "MI_N";
    break;
  case 26789: // PCS_NAD27_Michigan_Central
    sp_nad27 = true; sp = "MI_C";
    break;
  case 26790: // PCS_NAD27_Michigan_South
    sp_nad27 = true; sp = "MI_S";
    break;
  case 26791: // PCS_NAD27_Minnesota_North
    sp_nad27 = true; sp = "MN_N";
    break;
  case 26792: // PCS_NAD27_Minnesota_Cent
    sp_nad27 = true; sp = "MN_C";
    break;
  case 26793: // PCS_NAD27_Minnesota_South
    sp_nad27 = true; sp = "MN_S";
    break;
  case 26794: // PCS_NAD27_Mississippi_East
    sp_nad27 = true; sp = "MS_E";
    break;
  case 26795: // PCS_NAD27_Mississippi_West
    sp_nad27 = true; sp = "MS_W";
    break;
  case 26796: // PCS_NAD27_Missouri_East
    sp_nad27 = true; sp = "MO_E";
    break;
  case 26797: // PCS_NAD27_Missouri_Central
    sp_nad27 = true; sp = "MO_C";
    break;
  case 26798: // PCS_NAD27_Missouri_West
    sp_nad27 = true; sp = "MO_W";
    break;
  case 26903: // PCS_NAD83_UTM_zone_3N
  case 26904: // PCS_NAD83_UTM_zone_4N
  case 26905: // PCS_NAD83_UTM_zone_5N
  case 26906: // PCS_NAD83_UTM_zone_6N
  case 26907: // PCS_NAD83_UTM_zone_7N
  case 26908: // PCS_NAD83_UTM_zone_7N
  case 26909: // PCS_NAD83_UTM_zone_9N
  case 26910: // PCS_NAD83_UTM_zone_10N
  case 26911: // PCS_NAD83_UTM_zone_11N
  case 26912: // PCS_NAD83_UTM_zone_12N
  case 26913: // PCS_NAD83_UTM_zone_13N
  case 26914: // PCS_NAD83_UTM_zone_14N
  case 26915: // PCS_NAD83_UTM_zone_15N
  case 26916: // PCS_NAD83_UTM_zone_16N
  case 26917: // PCS_NAD83_UTM_zone_17N
  case 26918: // PCS_NAD83_UTM_zone_18N
  case 26919: // PCS_NAD83_UTM_zone_19N
  case 26920: // PCS_NAD83_UTM_zone_20N
  case 26921: // PCS_NAD83_UTM_zone_21N
  case 26922: // PCS_NAD83_UTM_zone_22N
  case 26923: // PCS_NAD83_UTM_zone_23N
    utm_northern = true; utm_zone = value-26900;
    ellipsoid = GEO_ELLIPSOID_NAD83;
    break;
  case 26929: // PCS_NAD83_Alabama_East
    sp_nad27 = false; sp = "AL_E";
    break;
  case 26930: // PCS_NAD83_Alabama_West
    sp_nad27 = false; sp = "AL_W";
    break;
  case 26931: // PCS_NAD83_Alaska_zone_1
    sp_nad27 = false; sp = "AK_1";
    break;
  case 26932: // PCS_NAD83_Alaska_zone_2
    sp_nad27 = false; sp = "AK_2";
    break;
  case 26933: // PCS_NAD83_Alaska_zone_3
    sp_nad27 = false; sp = "AK_3";
    break;
  case 26934: // PCS_NAD83_Alaska_zone_4
    sp_nad27 = false; sp = "AK_4";
    break;
  case 26935: // PCS_NAD83_Alaska_zone_5
    sp_nad27 = false; sp = "AK_5";
    break;
  case 26936: // PCS_NAD83_Alaska_zone_6
    sp_nad27 = false; sp = "AK_6";
    break;
  case 26937: // PCS_NAD83_Alaska_zone_7
    sp_nad27 = false; sp = "AK_7";
    break;
  case 26938: // PCS_NAD83_Alaska_zone_8
    sp_nad27 = false; sp = "AK_8";
    break;
  case 26939: // PCS_NAD83_Alaska_zone_9
    sp_nad27 = false; sp = "AK_9";
    break;
  case 26940: // PCS_NAD83_Alaska_zone_10
    sp_nad27 = false; sp = "AK_10";
    break;
  case 26941: // PCS_NAD83_California_I
    sp_nad27 = false; sp = "CA_I";
    break;
  case 26942: // PCS_NAD83_California_II
    sp_nad27 = false; sp = "CA_II";
    break;
  case 26943: // PCS_NAD83_California_III
    sp_nad27 = false; sp = "CA_III";
    break;
  case 26944: // PCS_NAD83_California_IV
    sp_nad27 = false; sp = "CA_IV";
    break;
  case 26945: // PCS_NAD83_California_V
    sp_nad27 = false; sp = "CA_V";
    break;
  case 26946: // PCS_NAD83_California_VI
    sp_nad27 = false; sp = "CA_VI";
    break;
  case 26947: // PCS_NAD83_California_VII
    sp_nad27 = false; sp = "CA_VII";
    break;
  case 26948: // PCS_NAD83_Arizona_East
    sp_nad27 = false; sp = "AZ_E";
    break;
  case 26949: // PCS_NAD83_Arizona_Central
    sp_nad27 = false; sp = "AZ_C";
    break;
  case 26950: // PCS_NAD83_Arizona_West
    sp_nad27 = false; sp = "AZ_W";
    break;
  case 26951: // PCS_NAD83_Arkansas_North
    sp_nad27 = false; sp = "AR_N";
    break;
  case 26952: // PCS_NAD83_Arkansas_South
    sp_nad27 = false; sp = "AR_S";
    break;
  case 26953: // PCS_NAD83_Colorado_North
    sp_nad27 = false; sp = "CO_N";
    break;
  case 26954: // PCS_NAD83_Colorado_Central
    sp_nad27 = false; sp = "CO_C";
    break;
  case 26955: // PCS_NAD83_Colorado_South
    sp_nad27 = false; sp = "CO_S";
    break;
  case 26956: // PCS_NAD83_Connecticut
    sp_nad27 = false; sp = "CT";
    break;
  case 26957: // PCS_NAD83_Delaware
    sp_nad27 = false; sp = "DE";
    break;
  case 26958: // PCS_NAD83_Florida_East
    sp_nad27 = false; sp = "FL_E";
    break;
  case 26959: // PCS_NAD83_Florida_West
    sp_nad27 = false; sp = "FL_W";
    break;
  case 26960: // PCS_NAD83_Florida_North
    sp_nad27 = false; sp = "FL_N";
    break;
  case 26961: // PCS_NAD83_Hawaii_zone_1
    sp_nad27 = false; sp = "HI_1";
    break;
  case 26962: // PCS_NAD83_Hawaii_zone_2
    sp_nad27 = false; sp = "HI_2";
    break;
  case 26963: // PCS_NAD83_Hawaii_zone_3
    sp_nad27 = false; sp = "HI_3";
    break;
  case 26964: // PCS_NAD83_Hawaii_zone_4
    sp_nad27 = false; sp = "HI_4";
    break;
  case 26965: // PCS_NAD83_Hawaii_zone_5
    sp_nad27 = false; sp = "HI_5";
    break;
  case 26966: // PCS_NAD83_Georgia_East
    sp_nad27 = false; sp = "GA_E";
    break;
  case 26967: // PCS_NAD83_Georgia_West
    sp_nad27 = false; sp = "GA_W";
    break;
  case 26968: // PCS_NAD83_Idaho_East
    sp_nad27 = false; sp = "ID_E";
    break;
  case 26969: // PCS_NAD83_Idaho_Central
    sp_nad27 = false; sp = "ID_C";
    break;
  case 26970: // PCS_NAD83_Idaho_West
    sp_nad27 = false; sp = "ID_W";
    break;
  case 26971: // PCS_NAD83_Illinois_East
    sp_nad27 = false; sp = "IL_E";
    break;
  case 26972: // PCS_NAD83_Illinois_West
    sp_nad27 = false; sp = "IL_W";
    break;
  case 26973: // PCS_NAD83_Indiana_East
    sp_nad27 = false; sp = "IN_E";
    break;
  case 26974: // PCS_NAD83_Indiana_West
    sp_nad27 = false; sp = "IN_W";
    break;
  case 26975: // PCS_NAD83_Iowa_North
    sp_nad27 = false; sp = "IA_N";
    break;
  case 26976: // PCS_NAD83_Iowa_South
    sp_nad27 = false; sp = "IA_S";
    break;
  case 26977: // PCS_NAD83_Kansas_North
    sp_nad27 = false; sp = "KS_N";
    break;
  case 26978: // PCS_NAD83_Kansas_South
    sp_nad27 = false; sp = "KS_S";
    break;
  case 26979: // PCS_NAD83_Kentucky_North
    sp_nad27 = false; sp = "KY_N";
    break;
  case 26980: // PCS_NAD83_Kentucky_South
    sp_nad27 = false; sp = "KY_S";
    break;
  case 26981: // PCS_NAD83_Louisiana_North
    sp_nad27 = false; sp = "LA_N";
    break;
  case 26982: // PCS_NAD83_Louisiana_South
    sp_nad27 = false; sp = "LA_S";
    break;
  case 26983: // PCS_NAD83_Maine_East
    sp_nad27 = false; sp = "ME_E";
    break;
  case 26984: // PCS_NAD83_Maine_West
    sp_nad27 = false; sp = "ME_W";
    break;
  case 26985: // PCS_NAD83_Maryland
    sp_nad27 = false; sp = "MD";
    break;
  case 26986: // PCS_NAD83_Massachusetts
    sp_nad27 = false; sp = "M_M";
    break;
  case 26987: // PCS_NAD83_Massachusetts_Is
    sp_nad27 = false; sp = "M_I";
    break;
  case 26988: // PCS_NAD83_Michigan_North
    sp_nad27 = false; sp = "MI_N";
    break;
  case 26989: // PCS_NAD83_Michigan_Central
    sp_nad27 = false; sp = "MI_C";
    break;
  case 26990: // PCS_NAD83_Michigan_South
    sp_nad27 = false; sp = "MI_S";
    break;
  case 26991: // PCS_NAD83_Minnesota_North
    sp_nad27 = false; sp = "MN_N";
    break;
  case 26992: // PCS_NAD83_Minnesota_Cent
    sp_nad27 = false; sp = "MN_C";
    break;
  case 26993: // PCS_NAD83_Minnesota_South
    sp_nad27 = false; sp = "MN_S";
    break;
  case 26994: // PCS_NAD83_Mississippi_East
    sp_nad27 = false; sp = "MS_E";
    break;
  case 26995: // PCS_NAD83_Mississippi_West
    sp_nad27 = false; sp = "MS_W";
    break;
  case 26996: // PCS_NAD83_Missouri_East
    sp_nad27 = false; sp = "MO_E";
    break;
  case 26997: // PCS_NAD83_Missouri_Central
    sp_nad27 = false; sp = "MO_C";
    break;
  case 26998: // PCS_NAD83_Missouri_West
    sp_nad27 = false; sp = "MO_W";
    break;
  case 28348: // PCS_GDA94_MGA_zone_48
  case 28349:
  case 28350:
  case 28351:
  case 28352:
  case 28353:
  case 28354: // PCS_GDA94_MGA_zone_54
  case 28355: // PCS_GDA94_MGA_zone_55
  case 28356: // PCS_GDA94_MGA_zone_56
  case 28357: // PCS_GDA94_MGA_zone_57
  case 28358: // PCS_GDA94_MGA_zone_58
    utm_northern = false; utm_zone = value-28300; is_mga = true;
    ellipsoid = GEO_ELLIPSOID_GDA94;
    break;
  case 29118: // PCS_SAD69_UTM_zone_18N
  case 29119: // PCS_SAD69_UTM_zone_19N
  case 29120: // PCS_SAD69_UTM_zone_20N
  case 29121: // PCS_SAD69_UTM_zone_21N
  case 29122: // PCS_SAD69_UTM_zone_22N
    utm_northern = true; utm_zone = value-29100;
    ellipsoid = GEO_ELLIPSOID_SAD69;
    break;
  case 29177: // PCS_SAD69_UTM_zone_17S
  case 29178: // PCS_SAD69_UTM_zone_18S
  case 29179: // PCS_SAD69_UTM_zone_19S
  case 29180: // PCS_SAD69_UTM_zone_20S
  case 29181: // PCS_SAD69_UTM_zone_21S
  case 29182: // PCS_SAD69_UTM_zone_22S
  case 29183: // PCS_SAD69_UTM_zone_23S
  case 29184: // PCS_SAD69_UTM_zone_24S
  case 29185: // PCS_SAD69_UTM_zone_25S
    utm_northern = false; utm_zone = value-29160;
    ellipsoid = GEO_ELLIPSOID_SAD69;
    break;
  case 29220: // PCS_Sapper_Hill_UTM_20S
  case 29221: // PCS_Sapper_Hill_UTM_21S
    utm_northern = false; utm_zone = value-29200;
    break;
  case 29333: // PCS_Schwarzeck_UTM_33S
    utm_northern = false; utm_zone = 33;
    break;
  case 29635: // PCS_Sudan_UTM_zone_35N
  case 29636: // PCS_Sudan_UTM_zone_35N
    utm_northern = true; utm_zone = value-29600;
    break;
  case 29738: // PCS_Tananarive_UTM_38S
  case 29739: // PCS_Tananarive_UTM_39S
    utm_northern = false; utm_zone = value-29700;
    break;
  case 29849: // PCS_Timbalai_1948_UTM_49N
  case 29850: // PCS_Timbalai_1948_UTM_50N
    utm_northern = true; utm_zone = value-29800;
    break;
  case 30339: // PCS_TC_1948_UTM_zone_39N
  case 30340: // PCS_TC_1948_UTM_zone_40N
    utm_northern = true; utm_zone = value-30300;
    break;
  case 30729: // PCS_Nord_Sahara_UTM_29N
  case 30730: // PCS_Nord_Sahara_UTM_30N
  case 30731: // PCS_Nord_Sahara_UTM_31N
  case 30732: // PCS_Nord_Sahara_UTM_32N
    utm_northern = true; utm_zone = value-30700;
    break;
  case 31028: // PCS_Yoff_UTM_zone_28N
    utm_northern = true; utm_zone = 28;
    break;
  case 31121: // PCS_Zanderij_UTM_zone_21N
    utm_northern = true; utm_zone = 21;
    break;
  case 32001: // PCS_NAD27_Montana_North
    sp_nad27 = true; sp = "MT_N";
    break;
  case 32002: // PCS_NAD27_Montana_Central
    sp_nad27 = true; sp = "MT_C";
    break;
  case 32003: // PCS_NAD27_Montana_South
    sp_nad27 = true; sp = "MT_S";
    break;
  case 32005: // PCS_NAD27_Nebraska_North
    sp_nad27 = true; sp = "NE_N";
    break;
  case 32006: // PCS_NAD27_Nebraska_South
    sp_nad27 = true; sp = "NE_S";
    break;
  case 32007: // PCS_NAD27_Nevada_East
    sp_nad27 = true; sp = "NV_E";
    break;
  case 32008: // PCS_NAD27_Nevada_Central
    sp_nad27 = true; sp = "NV_C";
    break;
  case 32009: // PCS_NAD27_Nevada_West
    sp_nad27 = true; sp = "NV_W";
    break;
  case 32010: // PCS_NAD27_New_Hampshire
    sp_nad27 = true; sp = "NH";
    break;
  case 32011: // PCS_NAD27_New_Jersey
    sp_nad27 = true; sp = "NJ";
    break;
  case 32012: // PCS_NAD27_New_Mexico_East
    sp_nad27 = true; sp = "NM_E";
    break;
  case 32013: // PCS_NAD27_New_Mexico_Cent
    sp_nad27 = true; sp = "NM_C";
    break;
  case 32014: // PCS_NAD27_New_Mexico_West
    sp_nad27 = true; sp = "NM_W";
    break;
  case 32015: // PCS_NAD27_New_York_East
    sp_nad27 = true; sp = "NY_E";
    break;
  case 32016: // PCS_NAD27_New_York_Central
    sp_nad27 = true; sp = "NY_C";
    break;
  case 32017: // PCS_NAD27_New_York_West
    sp_nad27 = true; sp = "NY_W";
    break;
  case 32018: // PCS_NAD27_New_York_Long_Is
    sp_nad27 = true; sp = "NT_LI";
    break;
  case 32019: // PCS_NAD27_North_Carolina
    sp_nad27 = true; sp = "NC";
    break;
  case 32020: // PCS_NAD27_North_Dakota_N
    sp_nad27 = true; sp = "ND_N";
    break;
  case 32021: // PCS_NAD27_North_Dakota_S
    sp_nad27 = true; sp = "ND_S";
    break;
  case 32022: // PCS_NAD27_Ohio_North
    sp_nad27 = true; sp = "OH_N";
    break;
  case 32023: // PCS_NAD27_Ohio_South
    sp_nad27 = true; sp = "OH_S";
    break;
  case 32024: // PCS_NAD27_Oklahoma_North
    sp_nad27 = true; sp = "OK_N";
    break;
  case 32025: // PCS_NAD27_Oklahoma_South
    sp_nad27 = true; sp = "OK_S";
    break;
  case 32026: // PCS_NAD27_Oregon_North
    sp_nad27 = true; sp = "OR_N";
    break;
  case 32027: // PCS_NAD27_Oregon_South
    sp_nad27 = true; sp = "OR_S";
    break;
  case 32028: // PCS_NAD27_Pennsylvania_N
    sp_nad27 = true; sp = "PA_N";
    break;
  case 32029: // PCS_NAD27_Pennsylvania_S
    sp_nad27 = true; sp = "PA_S";
    break;
  case 32030: // PCS_NAD27_Rhode_Island
    sp_nad27 = true; sp = "RI";
    break;
  case 32031: // PCS_NAD27_South_Carolina_N
    sp_nad27 = true; sp = "SC_N";
    break;
  case 32033: // PCS_NAD27_South_Carolina_S
    sp_nad27 = true; sp = "SC_S";
    break;
  case 32034: // PCS_NAD27_South_Dakota_N
    sp_nad27 = true; sp = "SD_N";
    break;
  case 32035: // PCS_NAD27_South_Dakota_S
    sp_nad27 = true; sp = "SD_S";
    break;
  case 32036: // PCS_NAD27_Tennessee
    sp_nad27 = true; sp = "TN";
    break;
  case 32037: // PCS_NAD27_Texas_North
    sp_nad27 = true; sp = "TX_N";
    break;
  case 32038: // PCS_NAD27_Texas_North_Cen
    sp_nad27 = true; sp = "TX_NC";
    break;
  case 32039: // PCS_NAD27_Texas_Central
    sp_nad27 = true; sp = "TX_C";
    break;
  case 32040: // PCS_NAD27_Texas_South_Cen
    sp_nad27 = true; sp = "TX_SC";
    break;
  case 32041: // PCS_NAD27_Texas_South
    sp_nad27 = true; sp = "TX_S";
    break;
  case 32042: // PCS_NAD27_Utah_North
    sp_nad27 = true; sp = "UT_N";
    break;
  case 32043: // PCS_NAD27_Utah_Central
    sp_nad27 = true; sp = "UT_C";
    break;
  case 32044: // PCS_NAD27_Utah_South
    sp_nad27 = true; sp = "UT_S";
    break;
  case 32045: // PCS_NAD27_Vermont
    sp_nad27 = true; sp = "VT";
    break;
  case 32046: // PCS_NAD27_Virginia_North
    sp_nad27 = true; sp = "VA_N";
    break;
  case 32047: // PCS_NAD27_Virginia_South
    sp_nad27 = true; sp = "VA_S";
    break;
  case 32048: // PCS_NAD27_Washington_North
    sp_nad27 = true; sp = "WA_N";
    break;
  case 32049: // PCS_NAD27_Washington_South
    sp_nad27 = true; sp = "WA_S";
    break;
  case 32050: // PCS_NAD27_West_Virginia_N
    sp_nad27 = true; sp = "WV_N";
    break;
  case 32051: // PCS_NAD27_West_Virginia_S
    sp_nad27 = true; sp = "WV_S";
    break;
  case 32052: // PCS_NAD27_Wisconsin_North
    sp_nad27 = true; sp = "WI_N";
    break;
  case 32053: // PCS_NAD27_Wisconsin_Cen
    sp_nad27 = true; sp = "WI_C";
    break;
  case 32054: // PCS_NAD27_Wisconsin_South
    sp_nad27 = true; sp = "WI_S";
    break;
  case 32055: // PCS_NAD27_Wyoming_East
    sp_nad27 = true; sp = "WY_E";
    break;
  case 32056: // PCS_NAD27_Wyoming_E_Cen
    sp_nad27 = true; sp = "WY_EC";
    break;
  case 32057: // PCS_NAD27_Wyoming_W_Cen
    sp_nad27 = true; sp = "WY_WC";
    break;
  case 32058: // PCS_NAD27_Wyoming_West
    sp_nad27 = true; sp = "WY_W";
    break;
  case 32059: // PCS_NAD27_Puerto_Rico
    sp_nad27 = true; sp = "PR";
    break;
  case 32060: // PCS_NAD27_St_Croix
    sp_nad27 = true; sp = "St.Croix";
    break;
  case 32100: // PCS_NAD83_Montana
    sp_nad27 = false; sp = "MT";
    break;
  case 32104: // PCS_NAD83_Nebraska
    sp_nad27 = false; sp = "NE";
    break;
  case 32107: // PCS_NAD83_Nevada_East
    sp_nad27 = false; sp = "NV_E";
    break;
  case 32108: // PCS_NAD83_Nevada_Central
    sp_nad27 = false; sp = "NV_C";
    break;
  case 32109: // PCS_NAD83_Nevada_West
    sp_nad27 = false; sp = "NV_W";
    break;
  case 32110: // PCS_NAD83_New_Hampshire
    sp_nad27 = false; sp = "NH";
    break;
  case 32111: // PCS_NAD83_New_Jersey
    sp_nad27 = false; sp = "NJ";
    break;
  case 32112: // PCS_NAD83_New_Mexico_East
    sp_nad27 = false; sp = "NM_E";
    break;
  case 32113: // PCS_NAD83_New_Mexico_Cent
    sp_nad27 = false; sp = "NM_C";
    break;
  case 32114: // PCS_NAD83_New_Mexico_West
    sp_nad27 = false; sp = "NM_W";
    break;
  case 32115: // PCS_NAD83_New_York_East
    sp_nad27 = false; sp = "NY_E";
    break;
  case 32116: // PCS_NAD83_New_York_Central
    sp_nad27 = false; sp = "NY_C";
    break;
  case 32117: // PCS_NAD83_New_York_West
    sp_nad27 = false; sp = "NY_W";
    break;
  case 32118: // PCS_NAD83_New_York_Long_Is
    sp_nad27 = false; sp = "NT_LI";
    break;
  case 32119: // PCS_NAD83_North_Carolina
    sp_nad27 = false; sp = "NC";
    break;
  case 32120: // PCS_NAD83_North_Dakota_N
    sp_nad27 = false; sp = "ND_N";
    break;
  case 32121: // PCS_NAD83_North_Dakota_S
    sp_nad27 = false; sp = "ND_S";
    break;
  case 32122: // PCS_NAD83_Ohio_North
    sp_nad27 = false; sp = "OH_N";
    break;
  case 32123: // PCS_NAD83_Ohio_South
    sp_nad27 = false; sp = "OH_S";
    break;
  case 32124: // PCS_NAD83_Oklahoma_North
    sp_nad27 = false; sp = "OK_N";
    break;
  case 32125: // PCS_NAD83_Oklahoma_South
    sp_nad27 = false; sp = "OK_S";
    break;
  case 32126: // PCS_NAD83_Oregon_North
    sp_nad27 = false; sp = "OR_N";
    break;
  case 32127: // PCS_NAD83_Oregon_South
    sp_nad27 = false; sp = "OR_S";
    break;
  case 32128: // PCS_NAD83_Pennsylvania_N
    sp_nad27 = false; sp = "PA_N";
    break;
  case 32129: // PCS_NAD83_Pennsylvania_S
    sp_nad27 = false; sp = "PA_S";
    break;
  case 32130: // PCS_NAD83_Rhode_Island
    sp_nad27 = false; sp = "RI";
    break;
  case 32133: // PCS_NAD83_South_Carolina
    sp_nad27 = false; sp = "SC";
    break;
  case 32134: // PCS_NAD83_South_Dakota_N
    sp_nad27 = false; sp = "SD_N";
    break;
  case 32135: // PCS_NAD83_South_Dakota_S
    sp_nad27 = false; sp = "SD_S";
    break;
  case 32136: // PCS_NAD83_Tennessee
    sp_nad27 = false; sp = "TN";
    break;
  case 32137: // PCS_NAD83_Texas_North
    sp_nad27 = false; sp = "TX_N";
    break;
  case 32138: // PCS_NAD83_Texas_North_Cen
    sp_nad27 = false; sp = "TX_NC";
    break;
  case 32139: // PCS_NAD83_Texas_Central
    sp_nad27 = false; sp = "TX_C";
    break;
  case 32140: // PCS_NAD83_Texas_South_Cen
    sp_nad27 = false; sp = "TX_SC";
    break;
  case 32141: // PCS_NAD83_Texas_South
    sp_nad27 = false; sp = "TX_S";
    break;
  case 32142: // PCS_NAD83_Utah_North
    sp_nad27 = false; sp = "UT_N";
    break;
  case 32143: // PCS_NAD83_Utah_Central
    sp_nad27 = false; sp = "UT_C";
    break;
  case 32144: // PCS_NAD83_Utah_South
    sp_nad27 = false; sp = "UT_S";
    break;
  case 32145: // PCS_NAD83_Vermont
    sp_nad27 = false; sp = "VT";
    break;
  case 32146: // PCS_NAD83_Virginia_North
    sp_nad27 = false; sp = "VA_N";
    break;
  case 32147: // PCS_NAD83_Virginia_South
    sp_nad27 = false; sp = "VA_S";
    break;
  case 32148: // PCS_NAD83_Washington_North
    sp_nad27 = false; sp = "WA_N";
    break;
  case 32149: // PCS_NAD83_Washington_South
    sp_nad27 = false; sp = "WA_S";
    break;
  case 32150: // PCS_NAD83_West_Virginia_N
    sp_nad27 = false; sp = "WV_N";
    break;
  case 32151: // PCS_NAD83_West_Virginia_S
    sp_nad27 = false; sp = "WV_S";
    break;
  case 32152: // PCS_NAD83_Wisconsin_North
    sp_nad27 = false; sp = "WI_N";
    break;
  case 32153: // PCS_NAD83_Wisconsin_Cen
    sp_nad27 = false; sp = "WI_C";
    break;
  case 32154: // PCS_NAD83_Wisconsin_South
    sp_nad27 = false; sp = "WI_S";
    break;
  case 32155: // PCS_NAD83_Wyoming_East
    sp_nad27 = false; sp = "WY_E";
    break;
  case 32156: // PCS_NAD83_Wyoming_E_Cen
    sp_nad27 = false; sp = "WY_EC";
    break;
  case 32157: // PCS_NAD83_Wyoming_W_Cen
    sp_nad27 = false; sp = "WY_WC";
    break;
  case 32158: // PCS_NAD83_Wyoming_West
    sp_nad27 = false; sp = "WY_W";
    break;
  case 32161: // PCS_NAD83_Puerto_Rico_Virgin_Is
    sp_nad27 = false; sp = "PR";
    break;
  case 32201: // PCS_WGS72_UTM_zone_1N 
  case 32202: // PCS_WGS72_UTM_zone_2N 
  case 32203: // PCS_WGS72_UTM_zone_3N 
  case 32204: // PCS_WGS72_UTM_zone_4N 
  case 32205: // PCS_WGS72_UTM_zone_5N 
  case 32206: // PCS_WGS72_UTM_zone_6N 
  case 32207: // PCS_WGS72_UTM_zone_7N 
  case 32208:
  case 32209:
  case 32210:
  case 32211:
  case 32212:
  case 32213:
  case 32214:
  case 32215:
  case 32216:
  case 32217:
  case 32218:
  case 32219:
  case 32220:
  case 32221:
  case 32222:
  case 32223:
  case 32224:
  case 32225:
  case 32226:
  case 32227:
  case 32228:
  case 32229:
  case 32230:
  case 32231:
  case 32232:
  case 32233:
  case 32234:
  case 32235:
  case 32236:
  case 32237:
  case 32238:
  case 32239:
  case 32240:
  case 32241:
  case 32242:
  case 32243:
  case 32244:
  case 32245:
  case 32246:
  case 32247:
  case 32248:
  case 32249:
  case 32250:
  case 32251:
  case 32252:
  case 32253:
  case 32254:
  case 32255:
  case 32256:
  case 32257:
  case 32258:
  case 32259: // PCS_WGS72_UTM_zone_59N 
  case 32260: // PCS_WGS72_UTM_zone_60N 
    utm_northern = true; utm_zone = value-32200;
    ellipsoid = GEO_ELLIPSOID_WGS72;
    break;
  case 32301: // PCS_WGS72_UTM_zone_1S
  case 32302: // PCS_WGS72_UTM_zone_2S
  case 32303: // PCS_WGS72_UTM_zone_3S
  case 32304: // PCS_WGS72_UTM_zone_4S
  case 32305: // PCS_WGS72_UTM_zone_5S
  case 32306: // PCS_WGS72_UTM_zone_6S
  case 32307: // PCS_WGS72_UTM_zone_7S
  case 32308:
  case 32309:
  case 32310:
  case 32311:
  case 32312:
  case 32313:
  case 32314:
  case 32315:
  case 32316:
  case 32317:
  case 32318:
  case 32319:
  case 32320:
  case 32321:
  case 32322:
  case 32323:
  case 32324:
  case 32325:
  case 32326:
  case 32327:
  case 32328:
  case 32329:
  case 32330:
  case 32331:
  case 32332:
  case 32333:
  case 32334:
  case 32335:
  case 32336:
  case 32337:
  case 32338:
  case 32339:
  case 32340:
  case 32341:
  case 32342:
  case 32343:
  case 32344:
  case 32345:
  case 32346:
  case 32347:
  case 32348:
  case 32349:
  case 32350:
  case 32351:
  case 32352:
  case 32353:
  case 32354:
  case 32355:
  case 32356:
  case 32357:
  case 32358:
  case 32359: // PCS_WGS72_UTM_zone_59S
  case 32360: // PCS_WGS72_UTM_zone_60S
    utm_northern = false; utm_zone = value-32300;
    ellipsoid = GEO_ELLIPSOID_WGS72;
    break;
  case 32401: // PCS_WGS72BE_UTM_zone_1N
  case 32402: // PCS_WGS72BE_UTM_zone_2N
  case 32403: // PCS_WGS72BE_UTM_zone_3N
  case 32404: // PCS_WGS72BE_UTM_zone_4N
  case 32405: // PCS_WGS72BE_UTM_zone_5N
  case 32406: // PCS_WGS72BE_UTM_zone_6N
  case 32407: // PCS_WGS72BE_UTM_zone_7N
  case 32408:
  case 32409:
  case 32410:
  case 32411:
  case 32412:
  case 32413:
  case 32414:
  case 32415:
  case 32416:
  case 32417:
  case 32418:
  case 32419:
  case 32420:
  case 32421:
  case 32422:
  case 32423:
  case 32424:
  case 32425:
  case 32426:
  case 32427:
  case 32428:
  case 32429:
  case 32430:
  case 32431:
  case 32432:
  case 32433:
  case 32434:
  case 32435:
  case 32436:
  case 32437:
  case 32438:
  case 32439:
  case 32440:
  case 32441:
  case 32442:
  case 32443:
  case 32444:
  case 32445:
  case 32446:
  case 32447:
  case 32448:
  case 32449:
  case 32450:
  case 32451:
  case 32452:
  case 32453:
  case 32454:
  case 32455:
  case 32456:
  case 32457:
  case 32458:
  case 32459: // PCS_WGS72BE_UTM_zone_59N
  case 32460: // PCS_WGS72BE_UTM_zone_60N
    utm_northern = true; utm_zone = value-32400;
    ellipsoid = GEO_ELLIPSOID_WGS72;
    break;
  case 32501: // PCS_WGS72BE_UTM_zone_1S
  case 32502: // PCS_WGS72BE_UTM_zone_2S
  case 32503: // PCS_WGS72BE_UTM_zone_3S
  case 32504: // PCS_WGS72BE_UTM_zone_4S
  case 32505: // PCS_WGS72BE_UTM_zone_5S
  case 32506: // PCS_WGS72BE_UTM_zone_6S
  case 32507: // PCS_WGS72BE_UTM_zone_7S
  case 32508:
  case 32509:
  case 32510:
  case 32511:
  case 32512:
  case 32513:
  case 32514:
  case 32515:
  case 32516:
  case 32517:
  case 32518:
  case 32519:
  case 32520:
  case 32521:
  case 32522:
  case 32523:
  case 32524:
  case 32525:
  case 32526:
  case 32527:
  case 32528:
  case 32529:
  case 32530:
  case 32531:
  case 32532:
  case 32533:
  case 32534:
  case 32535:
  case 32536:
  case 32537:
  case 32538:
  case 32539:
  case 32540:
  case 32541:
  case 32542:
  case 32543:
  case 32544:
  case 32545:
  case 32546:
  case 32547:
  case 32548:
  case 32549:
  case 32550:
  case 32551:
  case 32552:
  case 32553:
  case 32554:
  case 32555:
  case 32556:
  case 32557:
  case 32558:
  case 32559: // PCS_WGS72BE_UTM_zone_59S
  case 32560: // PCS_WGS72BE_UTM_zone_60S
    utm_northern = false; utm_zone = value-32500;
    ellipsoid = GEO_ELLIPSOID_WGS72;
    break;
  case 32601: // PCS_WGS84_UTM_zone_1N
  case 32602: // PCS_WGS84_UTM_zone_2N
  case 32603: // PCS_WGS84_UTM_zone_3N
  case 32604: // PCS_WGS84_UTM_zone_4N
  case 32605: // PCS_WGS84_UTM_zone_5N
  case 32606: // PCS_WGS84_UTM_zone_6N
  case 32607: // PCS_WGS84_UTM_zone_7N
  case 32608:
  case 32609:
  case 32610:
  case 32611:
  case 32612:
  case 32613:
  case 32614:
  case 32615:
  case 32616:
  case 32617:
  case 32618:
  case 32619:
  case 32620:
  case 32621:
  case 32622:
  case 32623:
  case 32624:
  case 32625:
  case 32626:
  case 32627:
  case 32628:
  case 32629:
  case 32630:
  case 32631:
  case 32632:
  case 32633:
  case 32634:
  case 32635:
  case 32636:
  case 32637:
  case 32638:
  case 32639:
  case 32640:
  case 32641:
  case 32642:
  case 32643:
  case 32644:
  case 32645:
  case 32646:
  case 32647:
  case 32648:
  case 32649:
  case 32650:
  case 32651:
  case 32652:
  case 32653:
  case 32654:
  case 32655:
  case 32656:
  case 32657:
  case 32658:
  case 32659: // PCS_WGS84_UTM_zone_59N
  case 32660: // PCS_WGS84_UTM_zone_60N
    utm_northern = true; utm_zone = value-32600;
    ellipsoid = GEO_ELLIPSOID_WGS84;
    break;
  case 32701: // PCS_WGS84_UTM_zone_1S
  case 32702: // PCS_WGS84_UTM_zone_2S
  case 32703: // PCS_WGS84_UTM_zone_3S
  case 32704: // PCS_WGS84_UTM_zone_4S
  case 32705: // PCS_WGS84_UTM_zone_5S
  case 32706: // PCS_WGS84_UTM_zone_6S
  case 32707: // PCS_WGS84_UTM_zone_7S
  case 32708:
  case 32709:
  case 32710:
  case 32711:
  case 32712:
  case 32713:
  case 32714:
  case 32715:
  case 32716:
  case 32717:
  case 32718:
  case 32719:
  case 32720:
  case 32721:
  case 32722:
  case 32723:
  case 32724:
  case 32725:
  case 32726:
  case 32727:
  case 32728:
  case 32729:
  case 32730:
  case 32731:
  case 32732:
  case 32733:
  case 32734:
  case 32735:
  case 32736:
  case 32737:
  case 32738:
  case 32739:
  case 32740:
  case 32741:
  case 32742:
  case 32743:
  case 32744:
  case 32745:
  case 32746:
  case 32747:
  case 32748:
  case 32749:
  case 32750:
  case 32751:
  case 32752:
  case 32753:
  case 32754:
  case 32755:
  case 32756:
  case 32757:
  case 32758:
  case 32759: // PCS_WGS84_UTM_zone_59S
  case 32760: // PCS_WGS84_UTM_zone_60S
    utm_northern = false; utm_zone = value-32700;
    ellipsoid = GEO_ELLIPSOID_WGS84;
    break;
  default:
    if (value == EPSG_IRENET95_Irish_Transverse_Mercator)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_transverse_mercator_projection(600000.0, 750000.0, 53.5, -8.0, 0.99982, 0, source); // "IRENET95 / Irish Transverse Mercator"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "IRENET95 / Irish Transverse Mercator");
      return true;
    }
    else if (value == EPSG_ETRS89_Poland_CS92)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_transverse_mercator_projection(500000.0, -5300000.0, 0.0, 19.0, 0.9993, 0, source); // "ETRS89 / Poland CS92"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "ETRS89 / Poland CS92");
      return true;
    }
    else if (value == EPSG_NZGD2000)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_transverse_mercator_projection(1600000.0, 10000000.0, 0.0, 173.0, 0.9996, 0, source); // "NZGD2000 / New Zealand Transverse Mercator 2000"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "NZGD2000");
      return true;
    }
    else if (value == EPSG_NAD83_HARN_UTM2_South_American_Samoa)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_transverse_mercator_projection(500000.0, 10000000.0, 0.0, -171.0, 0.9996, 0, source); // "NAD83(HARN) / UTM zone 2S (American Samoa)"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "UTM zone 2S (American Samoa)");
      return true;
    }
    else if (value == EPSG_NAD83_Maryland_ftUS)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_lambert_conformal_conic_projection(400000.0, 0.0, 37.66666666666666, -77, 37.66666666666666, 38.3, 0, source); // "NAD83 / Maryland (ftUS)"
      set_geokey(value, source);
      set_coordinates_in_survey_feet(source);
      if (description) sprintf(description, "NAD83 / Maryland (ftUS)");
      return true;
    }
    else if (value == EPSG_NAD83_Texas_Central_ftUS)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_lambert_conformal_conic_projection(700000.0, 3000000.0, 29.66666666666667, -100.3333333333333, 31.88333333333333, 30.11666666666667, 0, source); // "NAD83 / Texas Central (ftUS)"
      set_geokey(value, source);
      set_coordinates_in_survey_feet(source);
      if (description) sprintf(description, "NAD83 / Texas Central (ftUS)");
      return true;
    }
    else if (value == EPSG_NAD83_HARN_Washington_North)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_lambert_conformal_conic_projection(500000.0, 0.0, 47, -120.8333333333333, 48.73333333333333, 47.5, 0, source); // "NAD83(HARN) / Washington North"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "NAD83(HARN) / Washington North");
      return true;
    }
    else if (value == EPSG_NAD83_HARN_Washington_South)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_lambert_conformal_conic_projection(500000.0, 0.0, 45.33333333333334, -120.5, 47.33333333333334, 45.83333333333334, 0, source); // "NAD83(HARN) / Washington South"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "NAD83(HARN) / Washington South");
      return true;
    }
    else if (value == EPSG_NAD83_HARN_Virginia_North_ftUS)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_lambert_conformal_conic_projection(3500000.0, 2000000.0, 37.66666666666666, -78.5, 39.2, 38.03333333333333, 0, source); // "NAD83(HARN) / Virginia North (ftUS)"
      set_geokey(value, source);
      set_coordinates_in_survey_feet(source);
      if (description) sprintf(description, "NAD83(HARN) / Virginia North (ftUS)");
      return true;
    }
    else if (value == EPSG_NAD83_HARN_Virginia_South_ftUS)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_lambert_conformal_conic_projection(3500000.0, 1000000.0, 36.33333333333334, -78.5, 37.96666666666667, 36.76666666666667, 0, source); // "NAD83(HARN) / Virginia South (ftUS)"
      set_geokey(value, source);
      set_coordinates_in_survey_feet(source);
      if (description) sprintf(description, "NAD83(HARN) / Virginia South (ftUS)");
      return true;
    }
    else if (value == EPSG_NAD83_HARN_Washington_North_ftUS)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_lambert_conformal_conic_projection(500000.0, 0.0, 47, -120.8333333333333, 48.73333333333333, 47.5, 0, source); // "NAD83(HARN) / Washington North (ftUS)"
      set_geokey(value, source);
      set_coordinates_in_survey_feet(source);
      if (description) sprintf(description, "NAD83(HARN) / Washington North (ftUS)");
      return true;
    }
    else if (value == EPSG_NAD83_HARN_Washington_South_ftUS)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_lambert_conformal_conic_projection(500000.0, 0.0, 45.33333333333334, -120.5, 47.33333333333334, 45.83333333333334, 0, source); // "NAD83(HARN) / Washington South (ftUS)"
      set_geokey(value, source);
      set_coordinates_in_survey_feet(source);
      if (description) sprintf(description, "NAD83(HARN) / Washington South (ftUS)");
      return true;
    }
    else if (value == EPSG_Reseau_Geodesique_Francais_Guyane_1995)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_transverse_mercator_projection(500000.0, 0.0, 0.0, -51.0, 0.9996, 0, source); // "Reseau Geodesique Francais Guyane 1995"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "Reseau Geodesique Francais Guyane 1995");
      return true;
    }
    else if (value == EPSG_NAD83_Oregon_Lambert)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_lambert_conformal_conic_projection(400000.0, 0.0, 41.75, -120.5, 43.0, 45.5, 0, source); // "NAD83 / Oregon Lambert"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "NAD83 / Oregon Lambert");
      return true;
    }
    else if (value == EPSG_NAD83_Oregon_Lambert_ft)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_lambert_conformal_conic_projection(400000.0, 0.0, 41.75, -120.5, 43.0, 45.5, 0, source); // "NAD83 / Oregon Lambert (ft)"
      set_geokey(value, source);
      set_coordinates_in_feet(source);
      if (description) sprintf(description, "NAD83 / Oregon Lambert (ft)");
      return true;
    }
    else if (value == EPSG_NAD83_BC_Albers)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_albers_equal_area_conic_projection(1000000.0, 0.0, 45, -126, 50, 58.5, 0, source); // "NAD83 / BC Albers"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "NAD83 / BC Albers");
      return true;
    }
    else if (value == EPSG_SWEREF99_TM)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_transverse_mercator_projection(500000.0, 0.0, 0.0, 15.0, 0.9996, 0, source); // "SWEREF99 TM"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "SWEREF99 TM");
      return true;
    }
    else if (value == EPSG_ETRS89_ETRS_LCC)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_lambert_conformal_conic_projection(4000000.0, 2800000.0, 52.0, 10.0, 35.0, 65.0, 0, source); // "ETRS89 / ETRS-LCC"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "ETRS89 / ETRS-LCC");
      return true;
    }
    else if (value == EPSG_ETRS89_ETRS_TM34)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_transverse_mercator_projection(500000.0, 0.0, 0.0, 21.0, 0.9996, 0, source); // "ETRS89 / ETRS-TM34"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "ETRS89 / ETRS-TM34");
      return true;
    }
    else if (value == EPSG_ETRS89_ETRS_TM35)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_transverse_mercator_projection(500000.0, 0.0, 0.0, 27.0, 0.9996, 0, source); // "ETRS89 / ETRS-TM35"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "ETRS89 / ETRS-TM35");
      return true;
    }
    else if (value == EPSG_ETRS89_ETRS_TM36)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_transverse_mercator_projection(500000.0, 0.0, 0.0, 33.0, 0.9996, 0, source); // "ETRS89 / ETRS-TM36"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "ETRS89 / ETRS-TM36");
      return true;
    }
    else if (value == EPSG_ETRS89_ETRS_TM35FIN)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_transverse_mercator_projection(500000.0, 0.0, 0.0, 27.0, 0.9996, 0, source); // "ETRS89 / ETRS-TM35FIN"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "ETRS89 / ETRS-TM35FIN");
      return true;
    }
    else if (value == EPSG_Fiji_1956_UTM60_South)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_Inter);
      set_utm_projection(60, false, 0, source); // "Fiji 1956 / UTM zone 60S"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "Fiji 1956 / UTM zone 60S");
    }
    else if (value == EPSG_Fiji_1956_UTM1_South)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_Inter);
      set_utm_projection(1, false, 0, source); // "Fiji 1956 / UTM zone 1S"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "Fiji 1956 / UTM zone 1S");
    }
    else if (value == EPSG_GDA94_NSW_Lambert)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_GDA94); // GRS 1980
      set_lambert_conformal_conic_projection(9300000.0, 4500000.0, -33.25, 147, -30.75, -35.75, 0, source); // "GDA94 / NSW Lambert"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "GDA94 / NSW Lambert");
      return true;
    }
    else if (value == EPSG_Fiji_Map_Grid_1986)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_WGS72);
      set_transverse_mercator_projection(2000000.0, 4000000.0, -17.0, 178.75, 0.99985, 0, source); // "Fiji 1986 / Fiji Map Grid"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "Fiji 1986 / Fiji Map Grid");
      return true;
    }
    else if (value == EPSG_NAD83_NSRS2007_California_zone_3_ftUS)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_lambert_conformal_conic_projection(2000000.0, 500000.0, 36.5, -120.5, 38.43333333333333, 37.06666666666667, 0, source); // "NAD83(NSRS2007) / California zone 3 (ftUS)"
      set_geokey(value, source);
      set_coordinates_in_survey_feet(source);
      if (description) sprintf(description, "NAD83(NSRS2007) / California zone 3 (ftUS)");
      return true;
    }
    else if (value == EPSG_NAD83_NSRS2007_California_zone_4_ftUS)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_lambert_conformal_conic_projection(2000000.0, 500000.0, 35.33333333333334, -119, 37.25, 36, 0, source); // "NAD83(NSRS2007) / California zone 4 (ftUS)"
      set_geokey(value, source);
      set_coordinates_in_survey_feet(source);
      if (description) sprintf(description, "NAD83(NSRS2007) / California zone 4 (ftUS)");
      return true;
    }
    else if (value == EPSG_NAD83_NSRS2007_Maryland_ftUS)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_lambert_conformal_conic_projection(400000.0, 0.0, 37.66666666666666, -77, 39.45, 38.3, 0, source); // "NAD83(NSRS2007) / Maryland (ftUS)"
      set_geokey(value, source);
      set_coordinates_in_survey_feet(source);
      if (description) sprintf(description, "NAD83(NSRS2007) / Maryland (ftUS)");
      return true;
    }
    else if (value == EPSG_ETRS89_Portugal_TM06)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_transverse_mercator_projection(0.0, 0.0, 39.66825833333333, -8.133108333333334, 1.0, 0, source); // "ETRS89 / Portugal TM06"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "ETRS89 / Portugal TM06");
      return true;
    }
    else if (value == EPSG_Slovene_National_Grid_1996)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_transverse_mercator_projection(500000.0, -5000000.0, 0.0, 15.0, 0.9999, 0, source); // "Slovenia 1996 / Slovene National Grid"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "Slovenia 1996 / Slovene National Grid");
      return true;
    }
    else if (value == EPSG_ETRS89_GK24FIN)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_transverse_mercator_projection(24500000.0, 0.0, 0.0, 24.0, 1.0, 0, source); // "ETRS89 / GK24FIN"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "ETRS89 / GK24FIN");
      return true;    
    }
    else if (value == EPSG_MGI_1901_Slovene_National_Grid)
    {
      set_reference_ellipsoid(3); // Bessel 1841
      set_transverse_mercator_projection(500000.0, -5000000.0, 0.0, 15.0, 0.9999, 0, source); // "MGI 1901 / Slovene National Grid"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "MGI 1901 / Slovene National Grid");
      return true;    
    }
    else if ((EPSG_RGF93_CC42 <= value) && (value <= EPSG_RGF93_CC50))
    {
      int v = value - EPSG_RGF93_CC42;
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_lambert_conformal_conic_projection(1700000.0, 1200000.0+v*1000000, 42.0+v, 3.0, 41.25+v, 42.75+v, 0, source);
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "RGF93 / CC%d Reseau_Geodesique_Francais_1993", value-3900);
      return true;
    }
    else if ((EPSG_ETRS89_DKTM1 <= value) && (value <= EPSG_ETRS89_DKTM4))
    {
      int v = ((value - EPSG_ETRS89_DKTM1)%4) + 1;
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_transverse_mercator_projection(200000.0*v, -5000000.0, 0.0, 9.0 + (v == 1 ? 0.0 : (v == 2 ? 1.0 : (v == 3 ? 2.75 : 6.0))), 0.99998, 0, source);
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "ETRS89 / DKTM%d", v);
      return true;
    }
    else if (value == EPSG_ETRS89_UTM32_north_zE_N)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_transverse_mercator_projection(32500000.0, 0.0, 0.0, 9.0, 0.9996, 0, source); // "ETRS89 / UTM zone 32N (zE-N)"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "ETRS89 / UTM zone 32N (zE-N)");
      return true;
    }
    else if (value == EPSG_NAD83_HARN_Conus_Albers)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_albers_equal_area_conic_projection(0.0, 0.0, 23.0, -96, 29.5, 45.5, 0, source); // "NAD83(HARN) / Conus Albers"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "NAD83(HARN) / Conus Albers");
      return true;
    }
    else if (value == EPSG_NAD83_NSRS2007_Conus_Albers)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_albers_equal_area_conic_projection(0.0, 0.0, 23.0, -96, 29.5, 45.5, 0, source); // "NAD83(NSRS2007) / Conus Albers"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "NAD83(NSRS2007) / Conus Albers");
      return true;
    }
    else if ((EPSG_ETRS89_NTM_zone_5 <= value) && (value <= EPSG_ETRS89_NTM_zone_30))
    {
      int v = value - 5100;
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_transverse_mercator_projection(100000.0, 1000000.0, 58.0, 0.3+v, 1.0, 0, source);
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "ETRS89 / NTM zone %d", v);
      return true;
    }
    else if (value == EPSG_ETRS89_UTM33_north_zE_N)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_transverse_mercator_projection(33500000.0, 0.0, 0.0, 15.0, 0.9996, 0, source); // "ETRS89 / UTM zone 33N (zE-N)"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "ETRS89 / UTM zone 33N (zE-N)");
      return true;
    }
    else if (value == EPSG_NAD83_2011_North_Carolina_ftUS)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83); // GRS 1980
      set_lambert_conformal_conic_projection(609601.2192024384, 0.0, 33.75, -79, 36.16666666666666, 34.33333333333334, 0, source); // "NAD83(2011) / North Carolina (ftUS)"
      set_geokey(value, source);
      set_coordinates_in_survey_feet(source);
      if (description) sprintf(description, "NAD83(2011) / North Carolina (ftUS)");
    }
    else if (value == EPSG_CH1903_LV03) // should really be Hotine_Oblique_Mercator (but is special case with 90 degree angle that reduced to TM)
    {
      set_reference_ellipsoid(3); // Bessel 1841
      set_transverse_mercator_projection(600000.0, 200000.0, 46.95240555555556, 7.439583333333333, 1.0, 0, source); // "CH1903 / LV03"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "CH1903 / LV03");
      return true;
    }
    else if (value == EPSG_EOV_HD72) // should really be Hotine_Oblique_Mercator (but is special case with 90 degree angle that reduced to TM)
    {
      set_reference_ellipsoid(10); // Hungarian EOV / HD72
      set_transverse_mercator_projection(650000.0, 200000.0, 47.14439372222222, 19.04857177777778, 0.99993, 0, source); // "EOV / HD72 / Hungarian National Grid"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "EOV / HD72 / Hungarian National Grid");
      return true;
    }
    else if (value == EPSG_OSGB_1936)
    {
      set_reference_ellipsoid(1); // Airy 1830
      set_transverse_mercator_projection(400000.0, -100000.0, 49.0, -2.0, 0.9996012717, 0, source); // "OSGB 1936 / British National Grid"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "OSGB 1936 / British National Grid");
      return true;
    } 
    else if (value == EPSG_Belgian_Lambert_1972)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_Inter);
      set_lambert_conformal_conic_projection(150000.013, 5400088.438, 90, 4.367486666666666, 51.16666723333333, 49.8333339, 0, source); // "Belge 1972 / Belgian Lambert 72"
      set_geokey(value, source);
      set_coordinates_in_meter(source);
      if (description) sprintf(description, "Belge 1972 / Belgian Lambert 72");
      return true;
    }
    else
    {
      fprintf(stderr, "set_epsg_code: look-up for %d not implemented\n", value);
    }
  }

  if (ellipsoid == -1)
  {
    if (utm_zone != -1) ellipsoid = GEO_ELLIPSOID_WGS84;
    else if (sp != 0) ellipsoid = (sp_nad27 ? GEO_ELLIPSOID_NAD27 : GEO_ELLIPSOID_NAD83);
  }

  if (ellipsoid != -1)
  {
    set_reference_ellipsoid(ellipsoid);
  }

  if (utm_zone != -1)
  {
    if (set_utm_projection(utm_zone, utm_northern, description, source))
    {
      if (is_mga && description)
      {
        description[0] = 'M';
        description[1] = 'G';
        description[2] = 'A';
      }
      set_geokey(value, source);
      return true;
    }
  }

  if (sp)
  {
    if (sp_nad27)
    {
      if (set_state_plane_nad27_lcc(sp, description, source))
      {
        check_geokey(value, source);
        return true;
      }
      if (set_state_plane_nad27_tm(sp, description, source))
      {
        check_geokey(value, source);
        return true;
      }
    }
    else
    {
      if (set_state_plane_nad83_lcc(sp, description, source))
      {
        check_geokey(value, source);
        return true;
      }
      if (set_state_plane_nad83_tm(sp, description, source))
      {
        check_geokey(value, source);
        return true;
      }
    }
  }

  if (ecef)
  {
    if (set_ecef_projection(description, source))
    {
      set_geokey(value, source);
      return true;
    }
  }

  return false;
}

void GeoProjectionConverter::print_all_epsg_codes() const
{
  int i = 0;
  while (epsg_code_list[i].description)
  {
    fprintf(stderr, "%d - %s\n", (int)epsg_code_list[i].geokey, epsg_code_list[i].description);
    i++;
  }
}

const char* GeoProjectionConverter::get_state_plane_nad27_lcc_zone(int i) const
{
  if (state_plane_lcc_nad27_list[i].zone)
  {
    return state_plane_lcc_nad27_list[i].zone;
  }
  return 0;
}

bool GeoProjectionConverter::set_state_plane_nad27_lcc(const char* zone, char* description, bool source)
{
  int i = 0;
  while (state_plane_lcc_nad27_list[i].zone)
  {
    if (strcmp(zone, state_plane_lcc_nad27_list[i].zone) == 0)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD27);
      set_lambert_conformal_conic_projection(state_plane_lcc_nad27_list[i].falseEastingMeter, state_plane_lcc_nad27_list[i].falseNorthingMeter, state_plane_lcc_nad27_list[i].latOriginDegree, state_plane_lcc_nad27_list[i].longMeridianDegree, state_plane_lcc_nad27_list[i].firstStdParallelDegree, state_plane_lcc_nad27_list[i].secondStdParallelDegree, 0, source);
      set_geokey(state_plane_lcc_nad27_list[i].geokey, source);
      if (description)
      {
        sprintf(description, "stateplane27 %s", state_plane_lcc_nad27_list[i].zone);
      }
      return true;
    }
    i++;
  }
  return false;
}

void GeoProjectionConverter::print_all_state_plane_nad27_lcc() const
{
  int i = 0;
  while (state_plane_lcc_nad27_list[i].zone)
  {
    fprintf(stderr, "%s - false east/north: %g/%g [m], origin lat/meridian long: %g/%g, parallel 1st/2nd: %g/%g\n", state_plane_lcc_nad27_list[i].zone, state_plane_lcc_nad27_list[i].falseEastingMeter, state_plane_lcc_nad27_list[i].falseNorthingMeter, state_plane_lcc_nad27_list[i].latOriginDegree, state_plane_lcc_nad27_list[i].longMeridianDegree, state_plane_lcc_nad27_list[i].firstStdParallelDegree, state_plane_lcc_nad27_list[i].secondStdParallelDegree);
    i++;
  }
}

const char* GeoProjectionConverter::get_state_plane_nad83_lcc_zone(int i) const
{
  if (state_plane_lcc_nad83_list[i].zone)
  {
    return state_plane_lcc_nad83_list[i].zone;
  }
  return 0;
}

bool GeoProjectionConverter::set_state_plane_nad83_lcc(const char* zone, char* description, bool source)
{
  int i = 0;
  while (state_plane_lcc_nad83_list[i].zone)
  {
    if (strcmp(zone, state_plane_lcc_nad83_list[i].zone) == 0)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83);
      set_lambert_conformal_conic_projection(state_plane_lcc_nad83_list[i].falseEastingMeter, state_plane_lcc_nad83_list[i].falseNorthingMeter, state_plane_lcc_nad83_list[i].latOriginDegree, state_plane_lcc_nad83_list[i].longMeridianDegree, state_plane_lcc_nad83_list[i].firstStdParallelDegree, state_plane_lcc_nad83_list[i].secondStdParallelDegree, 0, source);
      set_geokey(state_plane_lcc_nad83_list[i].geokey, source);
      if (description)
      {
        sprintf(description, "stateplane83 %s", state_plane_lcc_nad83_list[i].zone);
      }
      return true;
    }
    i++;
  }
  return false;
}

void GeoProjectionConverter::print_all_state_plane_nad83_lcc() const
{
  int i = 0;
  while (state_plane_lcc_nad83_list[i].zone)
  {
    fprintf(stderr, "%s - false east/north: %g/%g [m], origin lat/meridian long: %g/%g, parallel 1st/2nd: %g/%g\n", state_plane_lcc_nad83_list[i].zone, state_plane_lcc_nad83_list[i].falseEastingMeter, state_plane_lcc_nad83_list[i].falseNorthingMeter,state_plane_lcc_nad83_list[i].latOriginDegree,state_plane_lcc_nad83_list[i].longMeridianDegree,state_plane_lcc_nad83_list[i].firstStdParallelDegree,state_plane_lcc_nad83_list[i].secondStdParallelDegree);
    i++;
  }
}

const char* GeoProjectionConverter::get_state_plane_nad27_tm_zone(int i) const
{
  if (state_plane_tm_nad27_list[i].zone)
  {
    return state_plane_tm_nad27_list[i].zone;
  }
  return 0;
}

bool GeoProjectionConverter::set_state_plane_nad27_tm(const char* zone, char* description, bool source)
{
  int i = 0;
  while (state_plane_tm_nad27_list[i].zone)
  {
    if (strcmp(zone, state_plane_tm_nad27_list[i].zone) == 0)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD27);
      set_transverse_mercator_projection(state_plane_tm_nad27_list[i].falseEastingMeter, state_plane_tm_nad27_list[i].falseNorthingMeter, state_plane_tm_nad27_list[i].latOriginDegree, state_plane_tm_nad27_list[i].longMeridianDegree, state_plane_tm_nad27_list[i].scaleFactor, 0, source);
      set_geokey(state_plane_tm_nad27_list[i].geokey, source);
      if (description)
      {
        sprintf(description, "stateplane27 %s", state_plane_tm_nad27_list[i].zone);
      }
      return true;
    }
    i++;
  }
  return false;
}

void GeoProjectionConverter::print_all_state_plane_nad27_tm() const
{
  int i = 0;
  while (state_plane_tm_nad27_list[i].zone)
  {
    fprintf(stderr, "%s - false east/north: %g/%g [m], origin lat/meridian long: %g/%g, scale factor: %g\n", state_plane_tm_nad27_list[i].zone, state_plane_tm_nad27_list[i].falseEastingMeter, state_plane_tm_nad27_list[i].falseNorthingMeter,state_plane_tm_nad27_list[i].latOriginDegree,state_plane_tm_nad27_list[i].longMeridianDegree,state_plane_tm_nad27_list[i].scaleFactor);
    i++;
  }
}

const char* GeoProjectionConverter::get_state_plane_nad83_tm_zone(int i) const
{
  if (state_plane_tm_nad83_list[i].zone)
  {
    return state_plane_tm_nad83_list[i].zone;
  }
  return 0;
}

bool GeoProjectionConverter::set_state_plane_nad83_tm(const char* zone, char* description, bool source)
{
  int i = 0;
  while (state_plane_tm_nad83_list[i].zone)
  {
    if (strcmp(zone, state_plane_tm_nad83_list[i].zone) == 0)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83);
      set_transverse_mercator_projection(state_plane_tm_nad83_list[i].falseEastingMeter, state_plane_tm_nad83_list[i].falseNorthingMeter, state_plane_tm_nad83_list[i].latOriginDegree, state_plane_tm_nad83_list[i].longMeridianDegree, state_plane_tm_nad83_list[i].scaleFactor, 0, source);
      set_geokey(state_plane_tm_nad83_list[i].geokey, source);
      if (description)
      {
        sprintf(description, "stateplane83 %s", state_plane_tm_nad83_list[i].zone);
      }
      return true;
    }
    i++;
  }
  return false;
}

void GeoProjectionConverter::print_all_state_plane_nad83_tm() const
{
  int i = 0;
  while (state_plane_tm_nad83_list[i].zone)
  {
    fprintf(stderr, "%s - false east/north: %g/%g [m], origin lat/meridian long: %g/%g, scale factor: %g\n", state_plane_tm_nad83_list[i].zone, state_plane_tm_nad83_list[i].falseEastingMeter, state_plane_tm_nad83_list[i].falseNorthingMeter,state_plane_tm_nad83_list[i].latOriginDegree,state_plane_tm_nad83_list[i].longMeridianDegree,state_plane_tm_nad83_list[i].scaleFactor);
    i++;
  }
}

void GeoProjectionConverter::reset_projection(bool source)
{
  if (source)
  {
    if (source_projection) delete source_projection;
    source_projection = 0;
  }
  else
  {
    if (target_projection) delete target_projection;
    target_projection = 0;
  }
}

bool GeoProjectionConverter::has_projection(bool source) const
{
  if (source)
  {
    return (source_projection != 0);
  }
  else
  {
    return (target_projection != 0);
  }
}

const char* GeoProjectionConverter::get_projection_name(bool source) const
{
  if (source)
  {
    return (source_projection ? source_projection->name : 0);
  }
  else
  {
    return (target_projection ? target_projection->name : 0);
  }
}

void GeoProjectionConverter::compute_lcc_parameters(bool source)
{
  GeoProjectionParametersLCC* lcc = (GeoProjectionParametersLCC*)(source ? source_projection : target_projection);

  if (!lcc || lcc->type != GEO_PROJECTION_LCC) return;

  double es_sin = ellipsoid->eccentricity * sin(lcc->lcc_lat_origin_radian);
  double t0 = tan(PI_OVER_4 - lcc->lcc_lat_origin_radian / 2) /  pow((1.0 - es_sin) / (1.0 + es_sin), (ellipsoid->eccentricity / 2.0));
  es_sin = ellipsoid->eccentricity * sin(lcc->lcc_first_std_parallel_radian);
  double t1 = tan(PI_OVER_4 - lcc->lcc_first_std_parallel_radian / 2) /  pow((1.0 - es_sin) / (1.0 + es_sin), (ellipsoid->eccentricity / 2.0));
  double m1 = cos(lcc->lcc_first_std_parallel_radian) / sqrt(1.0 - es_sin * es_sin);

  if (fabs(lcc->lcc_first_std_parallel_radian - lcc->lcc_second_std_parallel_radian) > 1.0e-10)
  {
    es_sin = ellipsoid->eccentricity * sin(lcc->lcc_second_std_parallel_radian);
    double t2 = tan(PI_OVER_4 - lcc->lcc_second_std_parallel_radian / 2) /  pow((1.0 - es_sin) / (1.0 + es_sin), (ellipsoid->eccentricity / 2.0));
    double m2 = cos(lcc->lcc_second_std_parallel_radian) / sqrt(1.0 - es_sin * es_sin);
    lcc->lcc_n = log(m1 / m2) / log(t1 / t2);
  }
  else
  {
    lcc->lcc_n = sin(lcc->lcc_first_std_parallel_radian);
  }

  lcc->lcc_aF = ellipsoid->equatorial_radius * m1 / (lcc->lcc_n * pow(t1, lcc->lcc_n));

  if ((t0 == 0) && (lcc->lcc_n < 0))
    lcc->lcc_rho0 = 0.0;
  else
    lcc->lcc_rho0 = lcc->lcc_aF * pow(t0, lcc->lcc_n);
}

void GeoProjectionConverter::compute_tm_parameters(bool source)
{
  GeoProjectionParametersTM* tm = (GeoProjectionParametersTM*)(source ? source_projection : target_projection);

  if (!tm || tm->type != GEO_PROJECTION_TM) return;

  double tn = (ellipsoid->equatorial_radius - ellipsoid->polar_radius) / (ellipsoid->equatorial_radius + ellipsoid->polar_radius);
  double tn2 = tn * tn;
  double tn3 = tn2 * tn;
  double tn4 = tn3 * tn;
  double tn5 = tn4 * tn;

  tm->tm_ap = ellipsoid->equatorial_radius * (1.e0 - tn + 5.e0 * (tn2 - tn3)/4.e0 + 81.e0 * (tn4 - tn5)/64.e0 );
  tm->tm_bp = 3.e0 * ellipsoid->equatorial_radius * (tn - tn2 + 7.e0 * (tn3 - tn4) /8.e0 + 55.e0 * tn5/64.e0 )/2.e0;
  tm->tm_cp = 15.e0 * ellipsoid->equatorial_radius * (tn2 - tn3 + 3.e0 * (tn4 - tn5 )/4.e0) /16.0;
  tm->tm_dp = 35.e0 * ellipsoid->equatorial_radius * (tn3 - tn4 + 11.e0 * tn5 / 16.e0) / 48.e0;
  tm->tm_ep = 315.e0 * ellipsoid->equatorial_radius * (tn4 - tn5) / 512.e0;
}

void GeoProjectionConverter::compute_aeac_parameters(bool source)
{
  GeoProjectionParametersAEAC* aeac = (GeoProjectionParametersAEAC*)(source ? source_projection : target_projection);

  if (!aeac || aeac->type != GEO_PROJECTION_AEAC) return;

  aeac->aeac_one_MINUS_es2 = 1.0 - ellipsoid->eccentricity_squared;
  aeac->aeac_two_es = 2 * ellipsoid->eccentricity;

  double sin_lat = sin(aeac->aeac_lat_origin_radian);
  double es_sin = sin_lat * ellipsoid->eccentricity;
  double one_MINUS_SQRes_sin = 1.0 - (es_sin * es_sin);
  double q0 = aeac->aeac_one_MINUS_es2 * (sin_lat / (one_MINUS_SQRes_sin) - (1.0/aeac->aeac_two_es)*log((1.0 - es_sin) / (1.0 + es_sin)));

  double sin_lat_1 = sin(aeac->aeac_first_std_parallel_radian);
  double cos_lat = cos(aeac->aeac_first_std_parallel_radian);
  es_sin = sin_lat_1 * ellipsoid->eccentricity;
  one_MINUS_SQRes_sin = 1.0 - (es_sin * es_sin);
  double m1 = cos_lat / sqrt(one_MINUS_SQRes_sin);
  double q1 = aeac->aeac_one_MINUS_es2 * (sin_lat_1 / (one_MINUS_SQRes_sin) - (1.0/aeac->aeac_two_es)*log((1.0 - es_sin) / (1.0 + es_sin)));

  double SQRm1 = m1 * m1;
  if (fabs(aeac->aeac_first_std_parallel_radian - aeac->aeac_second_std_parallel_radian) > 1.0e-10)
  {
    sin_lat = sin(aeac->aeac_second_std_parallel_radian);
    cos_lat = cos(aeac->aeac_second_std_parallel_radian);
    es_sin = sin_lat * ellipsoid->eccentricity;
    one_MINUS_SQRes_sin = 1.0 - (es_sin * es_sin);
    double m2 = cos_lat / sqrt(one_MINUS_SQRes_sin);
    double q2 = aeac->aeac_one_MINUS_es2 * (sin_lat / (one_MINUS_SQRes_sin) - (1.0/aeac->aeac_two_es)*log((1.0 - es_sin) / (1.0 + es_sin)));
    aeac->aeac_n = (SQRm1 - m2 * m2) / (q2 - q1);
  }
  else
    aeac->aeac_n = sin_lat_1;

  aeac->aeac_C = SQRm1 + aeac->aeac_n * q1;
  aeac->aeac_Albers_a_OVER_n = ellipsoid->equatorial_radius / aeac->aeac_n;
  double nq0 = aeac->aeac_n * q0;
  if (aeac->aeac_C < nq0)
    aeac->aeac_rho0 = 0;
  else
    aeac->aeac_rho0 = aeac->aeac_Albers_a_OVER_n * sqrt(aeac->aeac_C - nq0);
}

void GeoProjectionConverter::compute_om_parameters(bool source)
{
  GeoProjectionParametersOM* om = (GeoProjectionParametersOM*)(source ? source_projection : target_projection);

  if (!om || om->type != GEO_PROJECTION_OM) return;

  om->om_one_MINUS_es2 = 1.0 - ellipsoid->eccentricity_squared;
  om->om_two_es = 2 * ellipsoid->eccentricity;

  double sin_lat = sin(om->om_lat_origin_radian);
  double es_sin = sin_lat * ellipsoid->eccentricity;
  double one_MINUS_SQRes_sin = 1.0 - (es_sin * es_sin);
  double q0 = om->om_one_MINUS_es2 * (sin_lat / (one_MINUS_SQRes_sin) - (1.0/om->om_two_es)*log((1.0 - es_sin) / (1.0 + es_sin)));

  double sin_lat_1 = sin(om->om_first_std_parallel_radian);
  double cos_lat = cos(om->om_first_std_parallel_radian);
  es_sin = sin_lat_1 * ellipsoid->eccentricity;
  one_MINUS_SQRes_sin = 1.0 - (es_sin * es_sin);
  double m1 = cos_lat / sqrt(one_MINUS_SQRes_sin);
  double q1 = om->om_one_MINUS_es2 * (sin_lat_1 / (one_MINUS_SQRes_sin) - (1.0/om->om_two_es)*log((1.0 - es_sin) / (1.0 + es_sin)));

  double SQRm1 = m1 * m1;
  if (fabs(om->om_first_std_parallel_radian - om->om_second_std_parallel_radian) > 1.0e-10)
  {
    sin_lat = sin(om->om_second_std_parallel_radian);
    cos_lat = cos(om->om_second_std_parallel_radian);
    es_sin = sin_lat * ellipsoid->eccentricity;
    one_MINUS_SQRes_sin = 1.0 - (es_sin * es_sin);
    double m2 = cos_lat / sqrt(one_MINUS_SQRes_sin);
    double q2 = om->om_one_MINUS_es2 * (sin_lat / (one_MINUS_SQRes_sin) - (1.0/om->om_two_es)*log((1.0 - es_sin) / (1.0 + es_sin)));
    om->om_n = (SQRm1 - m2 * m2) / (q2 - q1);
  }
  else
    om->om_n = sin_lat_1;

  om->om_C = SQRm1 + om->om_n * q1;
  om->om_Albers_a_OVER_n = ellipsoid->equatorial_radius / om->om_n;
  double nq0 = om->om_n * q0;
  if (om->om_C < nq0)
    om->om_rho0 = 0;
  else
    om->om_rho0 = om->om_Albers_a_OVER_n * sqrt(om->om_C - nq0);
}

// converts UTM coords to lat/long.  Equations from USGS Bulletin 1532 
// East Longitudes are positive, West longitudes are negative. 
// North latitudes are positive, South latitudes are negative
// Lat and LongDegree are in decimal degrees. 
// adapted from code written by Chuck Gantz- chuck.gantz@globalstar.com

bool GeoProjectionConverter::UTMtoLL(const double UTMEastingMeter, const double UTMNorthingMeter, double& LatDegree,  double& LongDegree, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersUTM* utm) const
{
  const double k0 = 0.9996;

  double x = UTMEastingMeter - 500000.0; // remove 500,000 meter offset for longitude
  double y = UTMNorthingMeter;

  if (!utm->utm_northern_hemisphere)
  {
    y -= 10000000.0; //remove 10,000,000 meter offset used for southern hemisphere
  }

  double M = y / k0;
  double mu = M/(ellipsoid->equatorial_radius*(1-ellipsoid->eccentricity_squared/4-3*ellipsoid->eccentricity_squared*ellipsoid->eccentricity_squared/64-5*ellipsoid->eccentricity_squared*ellipsoid->eccentricity_squared*ellipsoid->eccentricity_squared/256));

  double phi1Rad = mu  + (3*ellipsoid->eccentricity_e1/2-27*ellipsoid->eccentricity_e1*ellipsoid->eccentricity_e1*ellipsoid->eccentricity_e1/32)*sin(2*mu) 
                       + (21*ellipsoid->eccentricity_e1*ellipsoid->eccentricity_e1/16-55*ellipsoid->eccentricity_e1*ellipsoid->eccentricity_e1*ellipsoid->eccentricity_e1*ellipsoid->eccentricity_e1/32)*sin(4*mu)
                       + (151*ellipsoid->eccentricity_e1*ellipsoid->eccentricity_e1*ellipsoid->eccentricity_e1/96)*sin(6*mu);

  double N1 = ellipsoid->equatorial_radius/sqrt(1-ellipsoid->eccentricity_squared*sin(phi1Rad)*sin(phi1Rad));
  double T1 = tan(phi1Rad)*tan(phi1Rad);
  double C1 = ellipsoid->eccentricity_prime_squared*cos(phi1Rad)*cos(phi1Rad);
  double R1 = ellipsoid->equatorial_radius*(1-ellipsoid->eccentricity_squared)/pow(1-ellipsoid->eccentricity_squared*sin(phi1Rad)*sin(phi1Rad), 1.5);
  double D = x/(N1*k0);

  LatDegree = phi1Rad - (N1*tan(phi1Rad)/R1)*(D*D/2-(5+3*T1+10*C1-4*C1*C1-9*ellipsoid->eccentricity_prime_squared)*D*D*D*D/24
                + (61+90*T1+298*C1+45*T1*T1-252*ellipsoid->eccentricity_prime_squared-3*C1*C1)*D*D*D*D*D*D/720);
  LatDegree = LatDegree * rad2deg;

  LongDegree = (D-(1+2*T1+C1)*D*D*D/6+(5-2*C1+28*T1-3*C1*C1+8*ellipsoid->eccentricity_prime_squared+24*T1*T1)*D*D*D*D*D/120)/cos(phi1Rad);
  LongDegree = LongDegree * rad2deg + utm->utm_long_origin;

  return true;
}

// converts lat/long to UTM coords.  Equations from USGS Bulletin 1532 
// East Longitudes are positive, West longitudes are negative. 
// North latitudes are positive, South latitudes are negative
// LatDegree and LongDegree are in decimal degrees
// adapted from code written by Chuck Gantz- chuck.gantz@globalstar.com

bool GeoProjectionConverter::compute_utm_zone(const double LatDegree, const double LongDegree, GeoProjectionParametersUTM* utm) const
{
  // Make sure the longitude is between -180.00 .. 179.9
  double LongTemp = (LongDegree+180)-int((LongDegree+180)/360)*360-180; // -180.00 .. 179.9;
  utm->utm_northern_hemisphere = (LatDegree >= 0);
  utm->utm_zone_number = (int)((LongTemp + 180)/6) + 1;
  if( LatDegree >= 56.0 && LatDegree < 64.0 && LongTemp >= 3.0 && LongTemp < 12.0 ) utm->utm_zone_number = 32;
  // Special zones for Svalbard
  if( LatDegree >= 72.0 && LatDegree < 84.0 ) 
  {
    if(      LongTemp >= 0.0  && LongTemp <  9.0 ) utm->utm_zone_number = 31;
    else if( LongTemp >= 9.0  && LongTemp < 21.0 ) utm->utm_zone_number = 33;
    else if( LongTemp >= 21.0 && LongTemp < 33.0 ) utm->utm_zone_number = 35;
    else if( LongTemp >= 33.0 && LongTemp < 42.0 ) utm->utm_zone_number = 37;
  }
  utm->utm_long_origin = (utm->utm_zone_number - 1) * 6 - 180 + 3;  // + 3 puts origin in middle of zone
  if((84 >= LatDegree) && (LatDegree >= 72)) utm->utm_zone_letter = 'X';
  else if((72 > LatDegree) && (LatDegree >= 64)) utm->utm_zone_letter = 'W';
  else if((64 > LatDegree) && (LatDegree >= 56)) utm->utm_zone_letter = 'V';
  else if((56 > LatDegree) && (LatDegree >= 48)) utm->utm_zone_letter = 'U';
  else if((48 > LatDegree) && (LatDegree >= 40)) utm->utm_zone_letter = 'T';
  else if((40 > LatDegree) && (LatDegree >= 32)) utm->utm_zone_letter = 'S';
  else if((32 > LatDegree) && (LatDegree >= 24)) utm->utm_zone_letter = 'R';
  else if((24 > LatDegree) && (LatDegree >= 16)) utm->utm_zone_letter = 'Q';
  else if((16 > LatDegree) && (LatDegree >= 8)) utm->utm_zone_letter = 'P';
  else if(( 8 > LatDegree) && (LatDegree >= 0)) utm->utm_zone_letter = 'N';
  else if(( 0 > LatDegree) && (LatDegree >= -8)) utm->utm_zone_letter = 'M';
  else if((-8> LatDegree) && (LatDegree >= -16)) utm->utm_zone_letter = 'L';
  else if((-16 > LatDegree) && (LatDegree >= -24)) utm->utm_zone_letter = 'K';
  else if((-24 > LatDegree) && (LatDegree >= -32)) utm->utm_zone_letter = 'J';
  else if((-32 > LatDegree) && (LatDegree >= -40)) utm->utm_zone_letter = 'H';
  else if((-40 > LatDegree) && (LatDegree >= -48)) utm->utm_zone_letter = 'G';
  else if((-48 > LatDegree) && (LatDegree >= -56)) utm->utm_zone_letter = 'F';
  else if((-56 > LatDegree) && (LatDegree >= -64)) utm->utm_zone_letter = 'E';
  else if((-64 > LatDegree) && (LatDegree >= -72)) utm->utm_zone_letter = 'D';
  else if((-72 > LatDegree) && (LatDegree >= -80)) utm->utm_zone_letter = 'C';
  else return false; // latitude is outside UTM limits
  return true;
}

bool GeoProjectionConverter::LLtoUTM(const double LatDegree, const double LongDegree, double &UTMEastingMeter, double &UTMNorthingMeter, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersUTM* utm) const
{
  const double k0 = 0.9996;
  
  // Make sure the longitude is between -180.00 .. 179.9
  double LongTemp = (LongDegree+180)-int((LongDegree+180)/360)*360-180; // -180.00 .. 179.9;
  double LatRad = LatDegree*deg2rad;
  double LongRad = LongTemp*deg2rad;
  double LongOriginRad = ((utm->utm_zone_number - 1)*6 - 180 + 3) * deg2rad;  // + 3 puts origin in middle of zone

  double N = ellipsoid->equatorial_radius/sqrt(1-ellipsoid->eccentricity_squared*sin(LatRad)*sin(LatRad));
  double T = tan(LatRad)*tan(LatRad);
  double C = ellipsoid->eccentricity_prime_squared*cos(LatRad)*cos(LatRad);
  double A = cos(LatRad)*(LongRad-LongOriginRad);

  double M = ellipsoid->equatorial_radius*((1  - ellipsoid->eccentricity_squared/4 - 3*ellipsoid->eccentricity_squared*ellipsoid->eccentricity_squared/64  - 5*ellipsoid->eccentricity_squared*ellipsoid->eccentricity_squared*ellipsoid->eccentricity_squared/256)*LatRad 
              - (3*ellipsoid->eccentricity_squared/8  + 3*ellipsoid->eccentricity_squared*ellipsoid->eccentricity_squared/32  + 45*ellipsoid->eccentricity_squared*ellipsoid->eccentricity_squared*ellipsoid->eccentricity_squared/1024)*sin(2*LatRad)
             + (15*ellipsoid->eccentricity_squared*ellipsoid->eccentricity_squared/256 + 45*ellipsoid->eccentricity_squared*ellipsoid->eccentricity_squared*ellipsoid->eccentricity_squared/1024)*sin(4*LatRad) 
             - (35*ellipsoid->eccentricity_squared*ellipsoid->eccentricity_squared*ellipsoid->eccentricity_squared/3072)*sin(6*LatRad));
  
  UTMEastingMeter = (double)(k0*N*(A+(1-T+C)*A*A*A/6
          + (5-18*T+T*T+72*C-58*ellipsoid->eccentricity_prime_squared)*A*A*A*A*A/120)
          + 500000.0);

  UTMNorthingMeter = (double)(k0*(M+N*tan(LatRad)*(A*A/2+(5-T+9*C+4*C*C)*A*A*A*A/24
         + (61-58*T+T*T+600*C-330*ellipsoid->eccentricity_prime_squared)*A*A*A*A*A*A/720)));

  if (LatDegree < 0)
  {
    UTMNorthingMeter += 10000000.0; //10000000 meter offset for southern hemisphere
  }

  return true;
}

/*
An alternate way to convert Lambert Conic Conformal Northing/Easting coordinates
into Latitude & Longitude coordinates. The code adapted from Brenor Brophy
(brenor dot brophy at gmail dot com) Homepage:  www.brenorbrophy.com 
*/
void lcc2ll( double e2, // Square of ellipsoid->eccentricity
             double a,  // Equatorial Radius
             double firstStdParallel, 
             double secondStdParallel,
             double latOfOrigin,
             double longOfOrigin,
             double lccEasting,  // Lambert coordinates of the point
             double lccNorthing,
             double falseEasting,
             double falseNorthing,
             double &LatDegree, // Latitude & Longitude of the point
             double &LongDegree)
{
  double e = sqrt(e2);
  double phi1  = deg2rad*firstStdParallel;  // Latitude of 1st std parallel
  double phi2  = deg2rad*secondStdParallel; // Latitude of 2nd std parallel
  double phio  = deg2rad*latOfOrigin;       // Latitude of  Origin
  double lamdao  = deg2rad*longOfOrigin;    // Longitude of  Origin
  double E    = lccEasting;
  double N    = lccNorthing;
  double Ef    = falseEasting;
  double Nf    = falseNorthing;

  double m1 = cos(phi1) / sqrt((1 - e2*sin(phi1)*sin(phi1)));
  double m2 = cos(phi2) / sqrt(( 1 - e2*sin(phi2)*sin(phi2)));
  double t1 = tan(PI_OVER_4-(phi1/2)) / pow(( ( 1 - e*sin(phi1) ) / ( 1 + e*sin(phi1) )),e/2);
  double t2 = tan(PI_OVER_4-(phi2/2)) / pow(( ( 1 - e*sin(phi2) ) / ( 1 + e*sin(phi2) )),e/2);
  double to = tan(PI_OVER_4-(phio/2)) / pow(( ( 1 - e*sin(phio) ) / ( 1 + e*sin(phio) )),e/2);
  double n  = (log(m1)-log(m2)) / (log(t1)-log(t2));
  double F  = m1/(n*pow(t1,n));
  double rf  = a*F*pow(to,n);
  double r_  = sqrt( pow((E-Ef),2) + pow((rf-(N-Nf)),2) );
  double t_  = pow(r_/(a*F),(1/n));
  double theta_ = atan((E-Ef)/(rf-(N-Nf)));

  double lamda  = theta_/n + lamdao;
  double phi0  = PI_OVER_2 - 2*atan(t_);
   phi1  = PI_OVER_2 - 2*atan(t_*pow(((1-e*sin(phi0))/(1+e*sin(phi0))),e/2));
   phi2  = PI_OVER_2 - 2*atan(t_*pow(((1-e*sin(phi1))/(1+e*sin(phi1))),e/2));
  double phi  = PI_OVER_2 - 2*atan(t_*pow(((1-e*sin(phi2))/(1+e*sin(phi2))),e/2));
  
  LatDegree = rad2deg*phi;
  LongDegree = rad2deg*lamda;
}

/*
An alternate way to convert a Latitude/Longitude coordinate to an Northing/
Easting coordinate on a Lambert Conic Projection. The Northing/Easting
parameters are calculated are in meters (because the datum used is in
meters) and are relative to the falseNorthing/falseEasting coordinate.
Which in turn is relative to the Lat/Long of origin. The formula were
obtained from URL: http://www.ihsenergy.com/epsg/guid7_2.html.
The code adapted from Brenor Brophy (brenor dot brophy at gmail dot com)
Homepage:  www.brenorbrophy.com 
*/
void ll2lcc( double e2, // Square of ellipsoid->eccentricity
             double a,  // Equatorial Radius
             double firstStdParallel, 
             double secondStdParallel,
             double latOfOrigin,
             double longOfOrigin,
             double &lccEasting,  // Lambert coordinates of the point
             double &lccNorthing,
             double falseEasting,
             double falseNorthing,
             double LatDegree, // Latitude & Longitude of the point
             double LongDegree)
{
  double e = sqrt(e2);
  double phi = deg2rad*LatDegree;           // Latitude to convert
  double phi1 = deg2rad*firstStdParallel;   // Latitude of 1st std parallel
  double phi2 = deg2rad*secondStdParallel;  // Latitude of 2nd std parallel
  double lamda = deg2rad*LongDegree;        // Lonitude to convert
  double phio = deg2rad*latOfOrigin;        // Latitude of  Origin
  double lamdao = deg2rad*longOfOrigin;     // Longitude of  Origin

  double m1 = cos(phi1) / sqrt((1 - e2*sin(phi1)*sin(phi1)));
  double m2 = cos(phi2) / sqrt((1 - e2*sin(phi2)*sin(phi2)));
  double t1 = tan(PI_OVER_4-(phi1/2)) / pow(( ( 1 - e*sin(phi1) ) / ( 1 + e*sin(phi1) )),e/2);
  double t2 = tan(PI_OVER_4-(phi2/2)) / pow(( ( 1 - e*sin(phi2) ) / ( 1 + e*sin(phi2) )),e/2);
  double to = tan(PI_OVER_4-(phio/2)) / pow(( ( 1 - e*sin(phio) ) / ( 1 + e*sin(phio) )),e/2);
  double t = tan(PI_OVER_4-(phi /2)) / pow(( ( 1 - e*sin(phi) ) / ( 1 + e*sin(phi) )),e/2);
  double n = (log(m1)-log(m2)) / (log(t1)-log(t2));
  double F = m1/(n*pow(t1,n));
  double rf = a*F*pow(to,n);
  double r = a*F*pow(t,n);
  double theta = n*(lamda - lamdao);

  lccEasting = falseEasting + r*sin(theta);
  lccNorthing = falseNorthing + rf - r*cos(theta);
}

/*
  * The function LCCtoLL() converts Lambert Conformal Conic projection
  * (easting and northing) coordinates to Geodetic (latitude and longitude)
  * coordinates, according to the current ellipsoid and Lambert Conformal
  * Conic projection parameters.
  *
  *   LCCEastingMeter   : input Easting/X in meters 
  *   LLCNorthingMeter  : input Northing/Y in meters
  *   LatDegree         : output Latitude in decimal degrees
  *   LongDegree        : output Longitude in decimal degrees
  *
  * adapted from code by Garrett Potts ((C) 2000 ImageLinks Inc.)
*/
bool GeoProjectionConverter::LCCtoLL(const double LCCEastingMeter, const double LCCNorthingMeter, double& LatDegree, double& LongDegree, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersLCC* lcc) const
{
/* >>> alternate way to compute (but seems less precise) <<<
  lcc2ll(ellipsoid->eccentricity_squared,
            ellipsoid->equatorial_radius,
            lcc->lcc_first_std_parallel_degree,
            lcc->lcc_second_std_parallel_degree,
            lcc->lcc_lat_origin_degree,
            lcc->lcc_long_meridian_degree,
            LCCEastingMeter,
            LCCNorthingMeter,
            lcc->lcc_false_easting_meter,
            lcc->lcc_false_northing_meter,
            LatDegree,
            LongDegree);
  return true;
*/

  double dx = LCCEastingMeter - lcc->lcc_false_easting_meter;
  double dy = LCCNorthingMeter - lcc->lcc_false_northing_meter;

  double rho0_MINUS_dy = lcc->lcc_rho0 - dy;
  double rho = sqrt(dx * dx + (rho0_MINUS_dy) * (rho0_MINUS_dy));

  if (lcc->lcc_n < 0.0)
  {
    rho *= -1.0;
    dy *= -1.0;
    dx *= -1.0;
    rho0_MINUS_dy *= -1.0;
  }

  if (rho != 0.0)
  {
    double theta = atan2(dx, rho0_MINUS_dy);
    double t = pow(rho / (lcc->lcc_aF), 1.0 / lcc->lcc_n);
    double PHI = PI_OVER_2 - 2.0 * atan(t);
    double tempPHI = 0.0;
    while (fabs(PHI - tempPHI) > 4.85e-10)
    {
      double es_sin = ellipsoid->eccentricity * sin(PHI);
      tempPHI = PHI;
      PHI = PI_OVER_2 - 2.0 * atan(t * pow((1.0 - es_sin) / (1.0 + es_sin), ellipsoid->eccentricity / 2.0));
    }
    LatDegree = PHI;
    LongDegree = theta / lcc->lcc_n + lcc->lcc_long_meridian_radian;

    if (fabs(LatDegree) < 2.0e-7)  /* force tiny lat to 0 */
      LatDegree = 0.0;
    else if (LatDegree > PI_OVER_2) /* force distorted lat to 90, -90 degrees */
      LatDegree = 90.0;
    else if (LatDegree < -PI_OVER_2)
      LatDegree = -90.0;
    else
      LatDegree = rad2deg*LatDegree;

    if (fabs(LongDegree) < 2.0e-7)  /* force tiny long to 0 */
      LongDegree = 0.0;
    else if (LongDegree > PI) /* force distorted long to 180, -180 degrees */
      LongDegree = 180.0;
    else if (LongDegree < -PI)
      LongDegree = -180.0;
    else
      LongDegree = rad2deg*LongDegree;
  }
  else
  {
    if (lcc->lcc_n > 0.0)
      LatDegree = 90.0;
    else
      LatDegree = -90.0;
    LongDegree = lcc->lcc_long_meridian_degree;
  }
  return true;
}

/*
  * The function LLtoLCC() converts Geodetic (latitude and longitude)
  * coordinates to Lambert Conformal Conic projection (easting and
  * northing) coordinates, according to the current ellipsoid and
  * Lambert Conformal Conic projection parameters. 
  *
  *   LatDegree         : input Latitude in decimal degrees
  *   LongDegree        : input Longitude in decimal degrees
  *   LCCEastingMeter   : output Easting/X in meters 
  *   LCCNorthingMeter  : output Northing/Y in meters
  *
  * adapted from code by Garrett Potts ((C) 2000 ImageLinks Inc.)
*/
bool GeoProjectionConverter::LLtoLCC(const double LatDegree, const double LongDegree, double& LCCEastingMeter,  double& LCCNorthingMeter, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersLCC* lcc) const
{
/* >>> alternate way to compute (but seems less precise) <<< 
  ll2lcc(ellipsoid->eccentricity_squared,
            ellipsoid->equatorial_radius,
            lcc->lcc_first_std_parallel_degree,
            lcc->lcc_second_std_parallel_degree,
            lcc->lcc_lat_origin_degree,
            lcc->lcc_long_meridian_degree,
            LCCEastingMeter,
            LCCNorthingMeter,
            lcc->lcc_false_easting_meter,
            lcc->lcc_false_northing_meter,
            LatDegreeAlt,
            LongDegreeAlt);
  return true;
*/
  double rho = 0.0;
  double Latitude = LatDegree*deg2rad;
  double Longitude = LongDegree*deg2rad;

  if (fabs(fabs(Latitude) - PI_OVER_2) > 1.0e-10)
  {
    double slat = sin(Latitude);
    double es_sin = ellipsoid->eccentricity*slat;
    double t = tan(PI_OVER_4 - Latitude / 2) / pow((1.0 - es_sin) / (1.0 + es_sin), ellipsoid->eccentricity/2);
    rho = lcc->lcc_aF * pow(t, lcc->lcc_n);
  }
  else
  {
    if ((Latitude * lcc->lcc_n) <= 0)
    { // Point can not be projected
      return false;
    }
  }

  double dlam = Longitude - lcc->lcc_long_meridian_radian;

  double theta = lcc->lcc_n * dlam;

  LCCEastingMeter = rho * sin(theta) + lcc->lcc_false_easting_meter;
  LCCNorthingMeter = lcc->lcc_rho0 - rho * cos(theta) + lcc->lcc_false_northing_meter;

  return true;
}

#define SPHSN(Latitude) ((double) (ellipsoid->equatorial_radius / sqrt( 1.e0 - ellipsoid->eccentricity_squared * pow(sin(Latitude), 2))))

#define SPHTMD(Latitude) ((double) (tm->tm_ap * Latitude \
                          - tm->tm_bp * sin(2.e0 * Latitude) + tm->tm_cp * sin(4.e0 * Latitude) \
                          - tm->tm_dp * sin(6.e0 * Latitude) + tm->tm_ep * sin(8.e0 * Latitude) ) )

#define DENOM(Latitude) ((double) (sqrt(1.e0 - ellipsoid->eccentricity_squared * pow(sin(Latitude),2))))
#define SPHSR(Latitude) ((double) (ellipsoid->equatorial_radius * (1.e0 - ellipsoid->eccentricity_squared) / pow(DENOM(Latitude), 3)))

/*
  * The function LLtoTM() converts geodetic (latitude and longitude)
  * coordinates to Transverse Mercator projection (easting and northing)
  * coordinates, according to the current ellipsoid and Transverse Mercator 
  * projection parameters.  
  *
  *   LatDegree        : input Latitude in decimal degrees
  *   LongDegree       : input Longitude in decimal degrees
  *   TMEastingMeter   : output Easting/X in meters 
  *   TMNorthingMeter  : output Northing/Y in meters
  *
  * adapted from code by Garrett Potts ((C) 2000 ImageLinks Inc.)
*/
bool GeoProjectionConverter::LLtoTM(const double LatDegree, const double LongDegree, double& TMEastingMeter,  double& TMNorthingMeter, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersTM* tm) const
{
  double Latitude = LatDegree*deg2rad;
  double Longitude = LongDegree*deg2rad;

  double c;       /* Cosine of latitude                          */
  double c2;
  double c3;
  double c5;
  double c7;
  double dlam;    /* Delta longitude - Difference in Longitude       */
  double eta;     /* constant - ellipsoid->eccentricity_prime_squared *c *c                   */
  double eta2;
  double eta3;
  double eta4;
  double s;       /* Sine of latitude                        */
  double sn;      /* Radius of curvature in the prime vertical       */
  double t;       /* Tangent of latitude                             */
  double tan2;
  double tan3;
  double tan4;
  double tan5;
  double tan6;
  double t1;      /* Term in coordinate conversion formula - GP to Y */
  double t2;      /* Term in coordinate conversion formula - GP to Y */
  double t3;      /* Term in coordinate conversion formula - GP to Y */
  double t4;      /* Term in coordinate conversion formula - GP to Y */
  double t5;      /* Term in coordinate conversion formula - GP to Y */
  double t6;      /* Term in coordinate conversion formula - GP to Y */
  double t7;      /* Term in coordinate conversion formula - GP to Y */
  double t8;      /* Term in coordinate conversion formula - GP to Y */
  double t9;      /* Term in coordinate conversion formula - GP to Y */
  double tmd;     /* True Meridional distance                        */
  double tmdo;    /* True Meridional distance for latitude of origin */

  if (Longitude > PI) Longitude -= TWO_PI;

  dlam = Longitude - tm->tm_long_meridian_radian;

  if (dlam > PI)
    dlam -= TWO_PI;
  if (dlam < -PI)
    dlam += TWO_PI;
  if (fabs(dlam) < 2.e-10)
    dlam = 0.0;

  s = sin(Latitude);
  c = cos(Latitude);
  c2 = c * c;
  c3 = c2 * c;
  c5 = c3 * c2;
  c7 = c5 * c2;
  t = tan (Latitude);
  tan2 = t * t;
  tan3 = tan2 * t;
  tan4 = tan3 * t;
  tan5 = tan4 * t;
  tan6 = tan5 * t;
  eta = ellipsoid->eccentricity_prime_squared * c2;
  eta2 = eta * eta;
  eta3 = eta2 * eta;
  eta4 = eta3 * eta;

  /* radius of curvature in prime vertical */
  sn = SPHSN(Latitude);

  /* True Meridianal Distances */
  tmd = SPHTMD(Latitude);

  /*  Origin  */
  tmdo = SPHTMD (tm->tm_lat_origin_radian);

  /* northing */
  t1 = (tmd - tmdo) * tm->tm_scale_factor;
  t2 = sn * s * c * tm->tm_scale_factor/ 2.e0;
  t3 = sn * s * c3 * tm->tm_scale_factor * (5.e0 - tan2 + 9.e0 * eta 
                                             + 4.e0 * eta2) /24.e0; 

  t4 = sn * s * c5 * tm->tm_scale_factor * (61.e0 - 58.e0 * tan2
                                             + tan4 + 270.e0 * eta - 330.e0 * tan2 * eta + 445.e0 * eta2
                                             + 324.e0 * eta3 -680.e0 * tan2 * eta2 + 88.e0 * eta4 
                                             -600.e0 * tan2 * eta3 - 192.e0 * tan2 * eta4) / 720.e0;

  t5 = sn * s * c7 * tm->tm_scale_factor * (1385.e0 - 3111.e0 * 
                                             tan2 + 543.e0 * tan4 - tan6) / 40320.e0;

  TMNorthingMeter = tm->tm_false_northing_meter + t1 + pow(dlam,2.e0) * t2 + pow(dlam,4.e0) * t3 + pow(dlam,6.e0) * t4 + pow(dlam,8.e0) * t5; 

  /* Easting */
  t6 = sn * c * tm->tm_scale_factor;
  t7 = sn * c3 * tm->tm_scale_factor * (1.e0 - tan2 + eta ) /6.e0;
  t8 = sn * c5 * tm->tm_scale_factor * (5.e0 - 18.e0 * tan2 + tan4
                                         + 14.e0 * eta - 58.e0 * tan2 * eta + 13.e0 * eta2 + 4.e0 * eta3 
                                         - 64.e0 * tan2 * eta2 - 24.e0 * tan2 * eta3 )/ 120.e0;
  t9 = sn * c7 * tm->tm_scale_factor * ( 61.e0 - 479.e0 * tan2
                                          + 179.e0 * tan4 - tan6 ) /5040.e0;

  TMEastingMeter = tm->tm_false_easting_meter + dlam * t6 + pow(dlam,3.e0) * t7 + pow(dlam,5.e0) * t8 + pow(dlam,7.e0) * t9;

  return true;
}
 
/*
  * The function TMtoLL() converts Transverse Mercator projection (easting and
  * northing) coordinates to geodetic (latitude and longitude) coordinates, 
  * according to the current ellipsoid and Transverse Mercator projection
  * parameters.
  *
  *   TMEastingMeter   : input Easting/X in meters 
  *   TMNorthingMeter  : input Northing/Y in meters
  *   LatDegree        : output Latitude in decimal degrees
  *   LongDegree       : output Longitude in decimal degrees
  *
  * adapted from code by Garrett Potts ((C) 2000 ImageLinks Inc.)
*/
bool GeoProjectionConverter::TMtoLL(const double TMEastingMeter, const double TMNorthingMeter, double& LatDegree,  double& LongDegree, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersTM* tm) const
{
  double c;       /* Cosine of latitude                              */
  double de;      /* Delta easting - Difference in Easting           */
  double dlam;    /* Delta longitude - Difference in Longitude       */
  double eta;     /* constant - eccentricity_prime_squared           */
  double eta2;
  double eta3;
  double eta4;
  double ftphi;   /* Footpoint latitude                              */
  int    i;       /* Loop iterator                                   */
  double s;       /* Sine of latitude                                */
  double sn;      /* Radius of curvature in the prime vertical       */
  double sr;      /* Radius of curvature in the meridian             */
  double t;       /* Tangent of latitude                             */
  double tan2;
  double tan4;
  double t10;     /* Term in coordinate conversion formula - GP to Y */
  double t11;     /* Term in coordinate conversion formula - GP to Y */
  double t12;     /* Term in coordinate conversion formula - GP to Y */
  double t13;     /* Term in coordinate conversion formula - GP to Y */
  double t14;     /* Term in coordinate conversion formula - GP to Y */
  double t15;     /* Term in coordinate conversion formula - GP to Y */
  double t16;     /* Term in coordinate conversion formula - GP to Y */
  double t17;     /* Term in coordinate conversion formula - GP to Y */
  double tmd;     /* True Meridional distance                        */
  double tmdo;    /* True Meridional distance for latitude of origin */

  /* True Meridional Distances for latitude of origin */
  tmdo = SPHTMD(tm->tm_lat_origin_radian);

  /*  Origin  */
  tmd = tmdo + (TMNorthingMeter - tm->tm_false_northing_meter) / tm->tm_scale_factor; 

  /* First Estimate */
  sr = SPHSR(0.e0);
  ftphi = tmd/sr;

  for (i = 0; i < 5 ; i++)
  {
   t10 = SPHTMD (ftphi);
   sr = SPHSR(ftphi);
   ftphi = ftphi + (tmd - t10) / sr;
  }

  /* Radius of Curvature in the meridian */
  sr = SPHSR(ftphi);

  /* Radius of Curvature in the meridian */
  sn = SPHSN(ftphi);

  /* Sine Cosine terms */
  s = sin(ftphi);
  c = cos(ftphi);

  /* Tangent Value  */
  t = tan(ftphi);
  tan2 = t * t;
  tan4 = tan2 * tan2;
  eta = ellipsoid->eccentricity_prime_squared * pow(c,2);
  eta2 = eta * eta;
  eta3 = eta2 * eta;
  eta4 = eta3 * eta;
  de = TMEastingMeter - tm->tm_false_easting_meter;
  if (fabs(de) < 0.0001)
   de = 0.0;

  /* Latitude */
  double Latitude;
  t10 = t / (2.e0 * sr * sn * pow(tm->tm_scale_factor, 2));
  t11 = t * (5.e0  + 3.e0 * tan2 + eta - 4.e0 * pow(eta,2)
            - 9.e0 * tan2 * eta) / (24.e0 * sr * pow(sn,3) 
                                    * pow(tm->tm_scale_factor,4));
  t12 = t * (61.e0 + 90.e0 * tan2 + 46.e0 * eta + 45.E0 * tan4
            - 252.e0 * tan2 * eta  - 3.e0 * eta2 + 100.e0 
            * eta3 - 66.e0 * tan2 * eta2 - 90.e0 * tan4
            * eta + 88.e0 * eta4 + 225.e0 * tan4 * eta2
            + 84.e0 * tan2* eta3 - 192.e0 * tan2 * eta4)
       / ( 720.e0 * sr * pow(sn,5) * pow(tm->tm_scale_factor, 6) );
  t13 = t * ( 1385.e0 + 3633.e0 * tan2 + 4095.e0 * tan4 + 1575.e0 
             * pow(t,6))/ (40320.e0 * sr * pow(sn,7) * pow(tm->tm_scale_factor,8));
  Latitude = ftphi - pow(de,2) * t10 + pow(de,4) * t11 - pow(de,6) * t12 + pow(de,8) * t13;

  t14 = 1.e0 / (sn * c * tm->tm_scale_factor);

  t15 = (1.e0 + 2.e0 * tan2 + eta) / (6.e0 * pow(sn,3) * c * 
                                     pow(tm->tm_scale_factor,3));

  t16 = (5.e0 + 6.e0 * eta + 28.e0 * tan2 - 3.e0 * eta2
        + 8.e0 * tan2 * eta + 24.e0 * tan4 - 4.e0 
        * eta3 + 4.e0 * tan2 * eta2 + 24.e0 
        * tan2 * eta3) / (120.e0 * pow(sn,5) * c  
                          * pow(tm->tm_scale_factor,5));

  t17 = (61.e0 +  662.e0 * tan2 + 1320.e0 * tan4 + 720.e0 
        * pow(t,6)) / (5040.e0 * pow(sn,7) * c 
                       * pow(tm->tm_scale_factor,7));

  /* Difference in Longitude */
  dlam = de * t14 - pow(de,3) * t15 + pow(de,5) * t16 - pow(de,7) * t17;

  /* Longitude */
  double Longitude = tm->tm_long_meridian_radian + dlam;
  while (Latitude > PI_OVER_2)
  {
    Latitude = PI - Latitude;
    Longitude += PI;
    if (Longitude > PI)
     Longitude -= TWO_PI;
  }

  while (Latitude < -PI_OVER_2)
  {
    Latitude = - (Latitude + PI);
    Longitude += PI;
    if (Longitude > PI)
      Longitude -= TWO_PI;
  }
  if (Longitude > TWO_PI)
    Longitude -= TWO_PI;
  if (Longitude < -PI)
    Longitude += TWO_PI;

  LatDegree = rad2deg*Latitude;
  LongDegree = rad2deg*Longitude;

  return true;
}

/*
  * The function ECEFtoLL() converts geocentric Earth-Centered Earth-Fixed
  * (ECEF)coordinates to geodetic (latitude and longitude) coordinates on
  * the provided ellipsoid
  *
  *   ECEFMeterX       : input X coordinate in meters 
  *   ECEFMeterY       : input Y coordinate in meters 
  *   ECEFMeterZ       : input Z coordinate in meters 
  *   LatDegree        : output Latitude in decimal degrees
  *   LongDegree       : output Longitude in decimal degrees
  *   ElevationMeter   : output Elevation in meters
  *
  * adapted from code by Craig Larrimore (Craig.Larrimore@noaa.gov) and B. Archinal
*/
bool GeoProjectionConverter::ECEFtoLL(const double ECEFMeterX, const double ECEFMeterY, const double ECEFMeterZ, double& LatDegree,  double& LongDegree, double& ElevationMeter, const GeoProjectionEllipsoid* ellipsoid) const
{
/********1*********2*********3*********4*********5*********6*********7**
 * Name:        xyz2plh
 * Version:     9602.17
 * Author:      B. Archinal (USNO)
 * Purpose:     Converts XYZ geocentric coordinates to Phi (latitude),
 *              Lambda (longitude), H (height) referred to an
 *              ellipsoid of semi-major axis A and flattening FL.
 *
 * Input:
 * -----------
 * A                semi-major axis of ellipsoid [units are of distance]
 * FL               flattening of ellipsoid [unitless]
 * xyz[]            geocentric Cartesian coordinates [units are of distance]
 *
 * Output:
 * -----------
 * plh[]            ellipsoidal coordinates of point, in geodetic latitude,
 *                  longitude east of Greenwich, and height [units for
 *                  latitude=LatDegree and longitude=plh[1] are in degrees;
 *                  height=plh[2] are distance and will be the same as
 *                  those of the input parameters]
 *
 * Local:
 * -----------
 * B                semi-minor axis of ellipsoid [same units as A]
 *
 * Global:
 * -----------
 *
 * Notes:
 * -----------
 * This routine will fail for points on the Z axis, i.e. if X= Y= 0
 * (Phi = +/- 90 degrees).
 *
 * Units of input parameters `A' and `xyz' must be the same.
 *
 * References:
 * -----------
 * Borkowski, K. M. (1989).  "Accurate algorithms to transform geocentric
 * to geodetic coordinates", *Bulletin Geodesique*, v. 63, pp. 50-56.
 *
 * Borkowski, K. M. (1987).  "Transformation of geocentric to geodetic
 * coordinates without approximations", *Astrophysics and Space Science*,
 * v. 139, n. 1, pp. 1-4.  Correction in (1988), v. 146, n. 1, p. 201.
 *
 * An equivalent formulation is recommended in the IERS Standards
 * (1995), draft.
 *
 ********1*********2*********3*********4*********5*********6*********7**
 * Modification History:
 * 9007.20, BA,  Creation
 * 9507,21, JR,  Modified for use with the page programs
 * 9602.17, MSS, Converted to C.
 ********1*********2*********3*********4*********5*********6*********7*/

  double d;
  double e;
  double f;
  double g;
  double p;
  double q;
  double r;
  double t;
  double v;
  double x= ECEFMeterX;
  double y= ECEFMeterY;
  double z= ECEFMeterZ;
  double zlong;

  double A = ellipsoid->equatorial_radius;
  double B = ellipsoid->polar_radius;

/*
 *   1.0 compute semi-minor axis and set sign to that of z in order
 *       to get sign of Phi correct
 */
  if ( z < 0.0 )
  {
    B= -B;
  }
/*
 *   2.0 compute intermediate values for latitude
 */
  r= sqrt( x*x + y*y );
  e= ( B*z - (A*A - B*B) ) / ( A*r );
  f= ( B*z + (A*A - B*B) ) / ( A*r );
/*
 *   3.0 find solution to:
 *       t^4 + 2*E*t^3 + 2*F*t - 1 = 0
 */
  p= (4.0 / 3.0) * (e*f + 1.0);
  q= 2.0 * (e*e - f*f);
  d= p*p*p + q*q;

  if( d >= 0.0 ) {
          v= pow( (sqrt( d ) - q), (1.0 / 3.0) )
           - pow( (sqrt( d ) + q), (1.0 / 3.0) );
  } else {
          v= 2.0 * sqrt( -p )
           * cos( acos( q/(p * sqrt( -p )) ) / 3.0 );
  }
/*
 *   4.0 improve v
 *       NOTE: not really necessary unless point is near pole
 */
  if( v*v < fabs(p) ) {
          v= -(v*v*v + 2.0*q) / (3.0*p);
  }
  g= (sqrt( e*e + v ) + e) / 2.0;
  t = sqrt( g*g  + (f - v*g)/(2.0*g - e) ) - g;

  LatDegree = atan( (A*(1.0 - t*t)) / (2.0*B*t) );
/*
 *   5.0 compute height above ellipsoid
 */
  ElevationMeter = (r - A*t)*cos( LatDegree ) + (z - B)*sin( LatDegree );
/*
 *   6.0 compute longitude east of Greenwich
 */
  zlong = atan2( y, x );
  if( zlong < 0.0 )
          zlong= zlong + TWO_PI;

  LongDegree = zlong;
/*
 *   7.0 convert latitude and longitude to degrees
 */
  LatDegree = LatDegree * rad2deg;
  LongDegree = LongDegree * rad2deg;

  return true;
}

/*
  * The function LLtoECEF() converts geodetic (latitude and longitude)
  * coordinates to geocentric Earth-Centered Earth-Fixed (ECEF) coordinates
  * on the provided ellipsoid
  *
  *   LatDegree        : input Latitude in decimal degrees
  *   LongDegree       : input Longitude in decimal degrees
  *   ElevationMeter   : input Elevation in meters
  *   ECEFMeterX       : output X coordinate in meters 
  *   ECEFMeterY       : output Y coordinate in meters 
  *   ECEFMeterZ       : output Z coordinate in meters 
  *
  * adapted from code by Craig Larrimore (Craig.Larrimore@noaa.gov) and C. Goad
*/
bool GeoProjectionConverter::LLtoECEF(const double LatDegree, const double LongDegree, const double ElevationMeter, double &ECEFMeterX, double &ECEFMeterY, double &ECEFMeterZ, const GeoProjectionEllipsoid* ellipsoid) const
{
/********1*********2*********3*********4*********5*********6*********7*********
 * name:            plh2xyz
 * version:         9602.20
 * written by:      C. Goad
 * purpose:         converts elliptic lat, lon, hgt to geocentric X, Y, Z
 *
 * input parameters
 * ----------------
 * A                semi-major axis of ellipsoid [units are of distance]
 * FL               flattening of ellipsoid [unitless]
 * plh[]            ellipsoidal coordinates of point, in geodetic latitude,
 *                  longitude east of Greenwich, and height [units for
 *                  latitude=plh[0] and longitude=plh[1] are in degrees;
 *                  height=plh[2] are distance and will be the same as
 *                  those of the input parameters]
 *
 * output parameters
 * -----------------
 * xyz[]            geocentric Cartesian coordinates [units are of distance]
 *
 *
 * local variables and constants
 * -----------------------------
 *
 * global variables and constants
 * ------------------------------
 *
 *
 * called by:
 * ------------------------------
 *
 * calls:
 * ------------------------------
 *
 * include files:
 * ------------------------------
 *
 * references:
 * ------------------------------
 * Escobal, "Methods of Orbit Determination", 1965, Wiley & Sons, Inc.,
 * pp. 27-29.
 *
 * comments:
 * ------------------------------
 * This routine was stripped from one called tlate which converted
 * in both directions.  A better routine (not iterative) was gotten
 * for XYZ to PLH; in turn, this was specialized and made to match.
 *
 * see also:
 * ------------------------------
 * xyz2plh
 *
 ********1*********2*********3*********4*********5*********6*********7*********
 *:modification history
 *:8301.00,  CG, Creation
 *:9406.16, MSS, Conversion to C.
 *:9602.20, MSS, Stripped plh to xyz convertion from tlate.
 ********1*********2*********3*********4*********5*********6*********7*********/

  double A = ellipsoid->equatorial_radius;
  double flatfn= ellipsoid->eccentricity_squared;
  double FL = 1.0 / ellipsoid->inverse_flattening;
  double funsq= (1.0 - FL)*(1.0 - FL);
  double g1;
  double g2;
  double lat_rad = deg2rad * LatDegree;
  double lon_rad = deg2rad * LongDegree;
  double sin_lat;

  sin_lat= sin( lat_rad );

  g1= A / sqrt( 1.0 - flatfn*sin_lat*sin_lat );
  g2= g1*funsq + ElevationMeter;
  g1= g1 + ElevationMeter;

  ECEFMeterX = g1 * cos( lat_rad );
  ECEFMeterY = ECEFMeterX * sin( lon_rad );
  ECEFMeterX = ECEFMeterX * cos( lon_rad );
  ECEFMeterZ = g2 * sin_lat;

  return true;
}

/*
  * The function AEACtoLL() converts Albers Equal Area Conic projection
  * (easting and northing) coordinates to Geodetic (latitude and longitude)
  * coordinates, according to the current ellipsoid and Albers Equal Area 
  * Conic projection parameters.
  *
  *   AEACEastingMeter  : input Easting/X in meters 
  *   AEACNorthingMeter : input Northing/Y in meters
  *   LatDegree         : output Latitude in decimal degrees
  *   LongDegree        : output Longitude in decimal degrees
  *
  * adapted from ALBERS code of U.S. Army Topographic Engineering Center
*/
bool GeoProjectionConverter::AEACtoLL(const double AEACEastingMeter, const double AEACNorthingMeter, double& LatDegree, double& LongDegree, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersAEAC* aeac) const
{
  double dy, dx;
  double rho0_MINUS_dy;
  double q, qconst, q_OVER_2;
  double rho, rho_n;
  double PHI, Delta_PHI = 1.0;
  double sin_phi;
  double es_sin, one_MINUS_SQRes_sin;
  double theta = 0.0;
  int count = 30;
  const double tolerance = 4.85e-10; /* approximately 1/1000th of an arc second or 1/10th meter */
  const double Albers_Delta_Northing = 40000000;
  const double Albers_Delta_Easting = 40000000;

  if ((AEACEastingMeter < (aeac->aeac_false_easting_meter - Albers_Delta_Easting)) || (AEACEastingMeter > aeac->aeac_false_easting_meter + Albers_Delta_Easting))
  {
    return false; /* Easting out of range  */
  }

  if ((AEACNorthingMeter < (aeac->aeac_false_northing_meter - Albers_Delta_Northing)) || (AEACNorthingMeter > aeac->aeac_false_northing_meter + Albers_Delta_Northing))
  { 
    return false; /* Northing out of range */
  }

  dy = AEACNorthingMeter - aeac->aeac_false_northing_meter;
  dx = AEACEastingMeter - aeac->aeac_false_easting_meter;
  rho0_MINUS_dy = aeac->aeac_rho0 - dy;
  rho = sqrt(dx * dx + rho0_MINUS_dy * rho0_MINUS_dy);

  if (aeac->aeac_n < 0)
  {
    rho *= -1.0;
    dy *= -1.0;
    dx *= -1.0;
    rho0_MINUS_dy *= -1.0;
  }

  if (rho != 0.0)
    theta = atan2(dx, rho0_MINUS_dy);
  rho_n = rho * aeac->aeac_n;
  q = (aeac->aeac_C - (rho_n * rho_n) / (ellipsoid->equatorial_radius * ellipsoid->equatorial_radius)) / aeac->aeac_n;
  qconst = 1.0 - ((aeac->aeac_one_MINUS_es2) / (aeac->aeac_two_es)) * log((1.0 - ellipsoid->eccentricity) / (1.0 + ellipsoid->eccentricity));
  if (fabs(fabs(qconst) - fabs(q)) > 1.0e-6)
  {
    q_OVER_2 = q / 2.0;
    if (q_OVER_2 > 1.0)
      LatDegree = PI_OVER_2;
    else if (q_OVER_2 < -1.0)
      LatDegree = -PI_OVER_2;
    else
    {
      PHI = asin(q_OVER_2);
      if (ellipsoid->eccentricity < 1.0e-10)
        LatDegree = PHI;
      else
      {
        while ((fabs(Delta_PHI) > tolerance) && count)
        {
          sin_phi = sin(PHI);
          es_sin = ellipsoid->eccentricity * sin_phi;
          one_MINUS_SQRes_sin = 1.0 - es_sin * es_sin;
          Delta_PHI = (one_MINUS_SQRes_sin * one_MINUS_SQRes_sin) / (2.0 * cos(PHI)) *
                      (q / (aeac->aeac_one_MINUS_es2) - sin_phi / one_MINUS_SQRes_sin +
                       (log((1.0 - es_sin) / (1.0 + es_sin)) / (aeac->aeac_two_es)));
          PHI += Delta_PHI;
          count --;
        }

        if (!count)
        {
          return false; /* ALBERS_NORTHING_ERROR */
        }

        LatDegree = PHI;
      }

      if (LatDegree > PI_OVER_2)  /* force distorted values to 90, -90 degrees */
        LatDegree = PI_OVER_2;
      else if (LatDegree < -PI_OVER_2)
        LatDegree = -PI_OVER_2;
    }
  }
  else
  {
    if (q >= 0.0)
      LatDegree = PI_OVER_2;
    else
      LatDegree = -PI_OVER_2;
  }
  
  LongDegree = aeac->aeac_long_meridian_radian + theta / aeac->aeac_n;

  if (LongDegree > PI)
    LongDegree -= TWO_PI;
  if (LongDegree < -PI)
    LongDegree += TWO_PI;

  if (LongDegree > PI) /* force distorted values to 180, -180 degrees */
    LongDegree = PI;
  else if (LongDegree < -PI)
    LongDegree = -PI;

  LatDegree = LatDegree * rad2deg;
  LongDegree = LongDegree * rad2deg;

  return true;
}

/*
  * The function LLtoAEAC() converts Geodetic (latitude and longitude)
  * coordinates to Albers Equal Area Conic projection (easting and
  * northing) coordinates, according to the current ellipsoid and
  * Albers Equal Area Conic projection parameters. 
  *
  *   LatDegree         : input Latitude in decimal degrees
  *   LongDegree        : input Longitude in decimal degrees
  *   AEACEastingMeter  : output Easting/X in meters 
  *   AEACNorthingMeter : output Northing/Y in meters
  *
  * adapted from ALBERS code of U.S. Army Topographic Engineering Center
*/
bool GeoProjectionConverter::LLtoAEAC(const double LatDegree, const double LongDegree, double &AEACEastingMeter, double &AEACNorthingMeter, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersAEAC* aeac) const
{
  double dlam;
  double sin_lat;
  double es_sin, one_MINUS_SQRes_sin;
  double q;
  double rho;
  double theta;
  double nq;

  double LatRadian = LatDegree*deg2rad;
  double LongRadian = LongDegree*deg2rad;

  if ((LatRadian < -PI_OVER_2) || (LatRadian > PI_OVER_2))
  {
    return false; /* Latitude out of range */
  }
  if ((LongRadian < -PI) || (LongRadian > TWO_PI))
  {
    return false; /* Longitude out of range */
  }

  dlam = LongRadian - aeac->aeac_long_meridian_radian;
  if (dlam > PI)
  {
    dlam -= TWO_PI;
  }
  else if (dlam < -PI)
  {
    dlam += TWO_PI;
  }
  sin_lat = sin(LatRadian);
  es_sin = ellipsoid->eccentricity * sin_lat;
  one_MINUS_SQRes_sin = 1.0 - es_sin * es_sin;
  q = aeac->aeac_one_MINUS_es2 * (sin_lat / (one_MINUS_SQRes_sin) - (1.0/aeac->aeac_two_es) * log((1.0 - es_sin) / (1.0 + es_sin)));
  nq = aeac->aeac_n * q;
  if (aeac->aeac_C < nq)
    rho = 0;
  else
    rho = aeac->aeac_Albers_a_OVER_n * sqrt(aeac->aeac_C - nq);

  theta = aeac->aeac_n * dlam;
  AEACEastingMeter = rho * sin(theta) + aeac->aeac_false_easting_meter;
  AEACNorthingMeter = aeac->aeac_rho0 - rho * cos(theta) + aeac->aeac_false_northing_meter;

  return true;
}

/*
  * The function OMtoLL() converts the Oblique Mercator projection
  * (easting and northing) coordinates to Geodetic (latitude and longitude)
  * coordinates, according to the current ellipsoid and Oblique Mercator 
  * projection parameters.
  *
  *   OMEastingMeter    : input Easting/X in meters 
  *   OMNorthingMeter   : input Northing/Y in meters
  *   LatDegree         : output Latitude in decimal degrees
  *   LongDegree        : output Longitude in decimal degrees
  *
  * adapted from OBLIQUE MERCATOR code of U.S. Army Topographic Engineering Center
*/
bool GeoProjectionConverter::OMtoLL(const double OMEastingMeter, const double OMNorthingMeter, double& LatDegree, double& LongDegree, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersOM* om) const
{
  return true;
}

/*
  * The function LLtoOM() converts Geodetic (latitude and longitude)
  * coordinates to the Oblique Mercator projection (easting and
  * northing) coordinates, according to the current ellipsoid and
  * Oblique Mercator projection parameters. 
  *
  *   LatDegree         : input Latitude in decimal degrees
  *   LongDegree        : input Longitude in decimal degrees
  *   OMEastingMeter    : output Easting/X in meters 
  *   OMNorthingMeter   : output Northing/Y in meters
  *
  * adapted from OBLIQUE MERCATOR code of U.S. Army Topographic Engineering Center
*/
bool GeoProjectionConverter::LLtoOM(const double LatDegree, const double LongDegree, double &OMEastingMeter, double &OMNorthingMeter, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersOM* om) const
{
  return true;
}

GeoProjectionConverter::GeoProjectionConverter()
{
  num_geo_keys = 0;
  geo_keys = 0;
  geo_ascii_params = 0;
  geo_double_params = 0;

  source_projection = 0;
  target_projection = 0;

  ellipsoid = new GeoProjectionEllipsoid();
  set_reference_ellipsoid(GEO_ELLIPSOID_WGS84);

  vertical_geokey = 0;

  coordinate_units_set[0] = false;
  coordinate_units_set[1] = false;
  coordinates2meter = 1.0;
  meter2coordinates = 1.0;
  elevation_units_set[0] = false;
  elevation_units_set[1] = false;
  elevation2meter = 1.0;
  meter2elevation = 1.0;
 
  target_precision = 0;
  target_elevation_precision = 0;

  elevation_offset_in_meter = 0.0f;
}

GeoProjectionConverter::~GeoProjectionConverter()
{
  delete ellipsoid;
  if (source_projection) delete source_projection;
  if (target_projection) delete target_projection;
}

bool GeoProjectionConverter::parse(int argc, char* argv[])
{
  int i;
  char tmp[256];
  for (i = 1; i < argc; i++)
  {
    if (argv[i][0] == '\0')
    {
      continue;
    }
    else if (strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"-help") == 0)
    {
      return true;
    }
    else if (strcmp(argv[i],"-ellipsoid") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: ellipsoid_id\n", argv[i]);
        return false;
      }
      int ellipsoid_id = atoi(argv[i+1]);
      if (set_reference_ellipsoid(ellipsoid_id, tmp))
      {
        fprintf(stderr, "using ellipsoid '%s'\n", tmp);
      }
      else
      {
        fprintf(stderr, "ERROR: ellipsoid with id %d is unknown. use one of those: \n", ellipsoid_id);
        ellipsoid_id = 1;
        while (set_reference_ellipsoid(ellipsoid_id++, tmp))
        {
          fprintf(stderr, "  %s\n", tmp);
        }
        return false;
      }
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-wgs72") == 0)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_WGS72, tmp);
      fprintf(stderr, "using ellipsoid '%s'\n", tmp);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-wgs84") == 0)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_WGS84, tmp);
      fprintf(stderr, "using ellipsoid '%s'\n", tmp);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-nad27") == 0)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD27, tmp);
      fprintf(stderr, "using ellipsoid '%s'\n", tmp);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-nad83") == 0 || strcmp(argv[i],"-grs80") == 0 || strcmp(argv[i],"-etrs89") == 0)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_NAD83, tmp);
      fprintf(stderr, "using ellipsoid '%s'\n", tmp);
      *argv[i]='\0';
    }
    else if (strncmp(argv[i],"-vertical_", 10) == 0)
    {
      if (strcmp(argv[i],"-vertical_navd88") == 0)
      {
        vertical_geokey = GEO_VERTICAL_NAVD88;
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-vertical_wgs84") == 0)
      {
        vertical_geokey = GEO_VERTICAL_WGS84;
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-vertical_navd29") == 0)
      {
        vertical_geokey = GEO_VERTICAL_NAVD29;
        *argv[i]='\0';
      }
    }
    else if (strcmp(argv[i],"-latlong") == 0 || strcmp(argv[i],"-target_latlong") == 0)
    {
      bool source = (strcmp(argv[i],"-latlong") == 0);
      set_latlong_projection(tmp, source);
      fprintf(stderr, "using %s '%s'\n", (source ? "projection" : "target projection"), tmp);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-longlat") == 0 || strcmp(argv[i],"-target_longlat") == 0)
    {
      bool source = (strcmp(argv[i],"-longlat") == 0);
      set_longlat_projection(tmp, source);
      fprintf(stderr, "using %s '%s'\n", (source ? "projection" : "target projection"), tmp);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-ecef") == 0 || strcmp(argv[i],"-target_ecef") == 0)
    {
      bool source = (strcmp(argv[i],"-ecef") == 0);
      set_ecef_projection(tmp, source);
      fprintf(stderr, "using %s '%s'\n", (source ? "projection" : "target projection"), tmp);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-utm") == 0 || strcmp(argv[i],"-target_utm") == 0)
    {
      bool source = (strcmp(argv[i],"-utm") == 0);
      if (!source && (strcmp(argv[i+1],"auto") == 0))
      {
        set_target_utm_projection(tmp);
        fprintf(stderr, "using target projection UTM '%s'\n", tmp);
      }
      else if (set_utm_projection(argv[i+1], tmp, source))
      {
        fprintf(stderr, "using %s UTM '%s'\n", (source ? "projection" : "target projection"), tmp);
      }
      else
      {
        fprintf(stderr, "ERROR: utm zone '%s' is unknown. use a format such as '11S' or '10T'\n", argv[i+1]);
        return false;
      }
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-epsg") == 0 || strcmp(argv[i],"-target_epsg") == 0)
    {
      bool source = (strcmp(argv[i],"-epsg") == 0);
      short value = (short)atoi(argv[i+1]);
      if (!set_epsg_code(value, 0, source))
      {
        fprintf(stderr, "ERROR: bad EPSG code in '%s %s'.\n", argv[i], argv[i+1]);
        print_all_epsg_codes();
        return false;
      }
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-lcc") == 0 || strcmp(argv[i],"-target_lcc") == 0)
    {
      bool source = (strcmp(argv[i],"-lcc") == 0);
      double falseEasting; sscanf(argv[i+1], "%lf", &falseEasting);
      double falseNorthing; sscanf(argv[i+2], "%lf", &falseNorthing);
      if (strcmp(argv[i+3],"survey_feet") == 0 || strcmp(argv[i+3],"surveyfeet") == 0)
      {
        set_coordinates_in_survey_feet(source);
        // the definition of the projection was in survey feet but we always calculate in meters
        falseEasting *= 0.3048006096012;
        falseNorthing *= 0.3048006096012;
      }
      else if (strcmp(argv[i+3],"feet") == 0 || strcmp(argv[i+3],"ft") == 0)
      {
        set_coordinates_in_feet(source);
        // the definition of the projection was in feet but we always calculate in meters
        falseEasting *= 0.3048;
        falseNorthing *= 0.3048;
      }
      else if (strcmp(argv[i+3],"meter") == 0 || strcmp(argv[i+3],"m") == 0)
      {
        set_coordinates_in_meter(source);
      }
      else
      {
        fprintf(stderr,"ERROR: wrong options for '-lcc'. use like shown in these examples:\n");
        fprintf(stderr,"  %s 609601.22 0 meter 33.75 -79 34.33333 36.16666\n", argv[i]);
        fprintf(stderr,"  %s 1640416.666667 0 survey_feet 47.000000 -120.833333 47.5 48.733333\n", argv[i]);
        fprintf(stderr,"  %s 1500000 0 feet 47.000000 -120.833333 47.5 48.733333\n", argv[i]);
        return false;
      }
      double latOfOriginDeg = atof(argv[i+4]);
      double longOfOriginDeg = atof(argv[i+5]);
      double firstStdParallelDeg = atof(argv[i+6]);
      double secondStdParallelDeg = atof(argv[i+7]);
      set_lambert_conformal_conic_projection(falseEasting, falseNorthing, latOfOriginDeg, longOfOriginDeg, firstStdParallelDeg, secondStdParallelDeg, tmp, source);
      fprintf(stderr, "using LCC %s '%s'\n", (source ? "projection" : "target projection"), tmp);
      *argv[i]='\0'; *argv[i+1]='\0';  *argv[i+2]='\0';  *argv[i+3]='\0';  *argv[i+4]='\0';  *argv[i+5]='\0';  *argv[i+6]='\0';  *argv[i+7]='\0'; i+=7;
    }
    else if (strcmp(argv[i],"-sp83") == 0 || strcmp(argv[i],"-target_sp83") == 0)
    {
      bool source = (strcmp(argv[i],"-sp83") == 0);
      if (set_state_plane_nad83_lcc(argv[i+1], tmp, source))
      {
        fprintf(stderr, "using %s '%s' (NAD83 LCC) '%s'\n", (source ? "state plane" : "target state plane"), argv[i+1], tmp);
      }
      else if (set_state_plane_nad83_tm(argv[i+1], tmp, source))
      {
        fprintf(stderr, "using %s '%s' (NAD83 TM) '%s'\n", (source ? "state plane" : "target state plane"), argv[i+1], tmp);
      }
      else
      {
        fprintf(stderr, "ERROR: bad state code in '%s %s'.\n", argv[i], argv[i+1]);
        print_all_state_plane_nad83_lcc();
        print_all_state_plane_nad83_tm();
        return false;
      }
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-sp27") == 0 || strcmp(argv[i],"-target_sp27") == 0)
    {
      bool source = (strcmp(argv[i],"-sp27") == 0);
      if (set_state_plane_nad27_lcc(argv[i+1], tmp, source))
      {
        fprintf(stderr, "using %s '%s' (NAD27 LCC) '%s'\n", (source ? "state plane" : "target state plane"), argv[i+1], tmp);
      }
      else if (set_state_plane_nad27_tm(argv[i+1], tmp, source))
      {
        fprintf(stderr, "using %s '%s' (NAD27 TM) '%s'\n", (source ? "state plane" : "target state plane"), argv[i+1], tmp);
      }
      else
      {
        fprintf(stderr, "ERROR: bad state code in '%s %s'.\n", argv[i], argv[i+1]);
        print_all_state_plane_nad27_lcc();
        print_all_state_plane_nad27_tm();
        return false;
      }
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-tm") == 0 || strcmp(argv[i],"-transverse_mercator") == 0 || strcmp(argv[i],"-target_tm") == 0)
    {
      bool source = (strcmp(argv[i],"-tm") == 0 || strcmp(argv[i],"-transverse_mercator") == 0);
      double falseEasting; sscanf(argv[i+1], "%lf", &falseEasting);
      double falseNorthing; sscanf(argv[i+2], "%lf", &falseNorthing);
      if (strcmp(argv[i+3],"survey_feet") == 0 || strcmp(argv[i+3],"surveyfeet") == 0)
      {
        set_coordinates_in_survey_feet(source);
        // the definition of the projection was in survey feet but we always calculate in meters
        falseEasting *= 0.3048006096012;
        falseNorthing *= 0.3048006096012;
      }
      else if (strcmp(argv[i+3],"feet") == 0 || strcmp(argv[i+3],"ft") == 0)
      {
        set_coordinates_in_feet(source);
        // the definition of the projection was in feet but we always calculate in meters
        falseEasting *= 0.3048;
        falseNorthing *= 0.3048;
      }
      else if (strcmp(argv[i+3],"meter") == 0 || strcmp(argv[i+3],"m") == 0)
      {
        set_coordinates_in_meter(source);
      }
      else
      {
        fprintf(stderr,"ERROR: wrong options for '%s'. use like shown in these examples:\n",argv[i]);
        fprintf(stderr,"  %s 500000 0 meter 0 -93 0.99996\n",argv[i]);
        fprintf(stderr,"  %s 1500000 0 feet 47 -120.833333 0.99996\n",argv[i]);
        fprintf(stderr,"  %s 1640416.666667 0 survey_feet 47 -120.833333 0.99996\n",argv[i]);
        return false;
      }
      double latOriginDeg; sscanf(argv[i+4], "%lf", &latOriginDeg);
      double longMeridianDeg; sscanf(argv[i+5], "%lf", &longMeridianDeg);
      double scaleFactor; sscanf(argv[i+6], "%lf", &scaleFactor);
      set_transverse_mercator_projection(falseEasting, falseNorthing, latOriginDeg, longMeridianDeg, scaleFactor, tmp, source);
      fprintf(stderr, "using TM %s '%s'\n", (source ? "projection" : "target projection"), tmp);
      *argv[i]='\0'; *argv[i+1]='\0';  *argv[i+2]='\0';  *argv[i+3]='\0';  *argv[i+4]='\0';  *argv[i+5]='\0';  *argv[i+6]='\0'; i+=6;
    }
    else if (strcmp(argv[i],"-aeac") == 0 || strcmp(argv[i],"-target_aeac") == 0)
    {
      bool source = (strcmp(argv[i],"-aeac") == 0);
      double falseEasting; sscanf(argv[i+1], "%lf", &falseEasting);
      double falseNorthing; sscanf(argv[i+2], "%lf", &falseNorthing);
      if (strcmp(argv[i+3],"survey_feet") == 0 || strcmp(argv[i+3],"surveyfeet") == 0)
      {
        set_coordinates_in_survey_feet(source);
        // the definition of the projection was in survey feet but we always calculate in meters
        falseEasting *= 0.3048006096012;
        falseNorthing *= 0.3048006096012;
      }
      else if (strcmp(argv[i+3],"feet") == 0 || strcmp(argv[i+3],"ft") == 0)
      {
        set_coordinates_in_feet(source);
        // the definition of the projection was in feet but we always calculate in meters
        falseEasting *= 0.3048;
        falseNorthing *= 0.3048;
      }
      else if (strcmp(argv[i+3],"meter") == 0 || strcmp(argv[i+3],"m") == 0)
      {
        set_coordinates_in_meter(source);
      }
      else
      {
        fprintf(stderr,"ERROR: wrong options for '-aeac'. use like shown in these examples:\n");
        fprintf(stderr,"  %s 609601.22 0 meter 33.75 -79 34.33333 36.16666\n", argv[i]);
        fprintf(stderr,"  %s 1640416.666667 0 survey_feet 47.000000 -120.833333 47.5 48.733333\n", argv[i]);
        fprintf(stderr,"  %s 1500000 0 feet 47.000000 -120.833333 47.5 48.733333\n", argv[i]);
        return false;
      }
      double latOfOriginDeg = atof(argv[i+4]);
      double longOfOriginDeg = atof(argv[i+5]);
      double firstStdParallelDeg = atof(argv[i+6]);
      double secondStdParallelDeg = atof(argv[i+7]);
      set_albers_equal_area_conic_projection(falseEasting, falseNorthing, latOfOriginDeg, longOfOriginDeg, firstStdParallelDeg, secondStdParallelDeg, tmp, source);
      fprintf(stderr, "using AEAC %s '%s'\n", (source ? "projection" : "target projection"), tmp);
      *argv[i]='\0'; *argv[i+1]='\0';  *argv[i+2]='\0';  *argv[i+3]='\0';  *argv[i+4]='\0';  *argv[i+5]='\0';  *argv[i+6]='\0';  *argv[i+7]='\0'; i+=7;
    }
    else if (strcmp(argv[i],"-surveyfeet") == 0 || strcmp(argv[i],"-survey_feet") == 0)
    {
      set_coordinates_in_survey_feet();
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-target_surveyfeet") == 0 || strcmp(argv[i],"-target_survey_feet") == 0)
    {
      set_coordinates_in_survey_feet(false);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-feet") == 0)
    {
      set_coordinates_in_feet();
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-target_feet") == 0)
    {
      set_coordinates_in_feet(false);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-meter") == 0)
    {
      set_coordinates_in_meter();
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-target_meter") == 0)
    {
      set_coordinates_in_meter(false);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-elevation_surveyfeet") == 0 || strcmp(argv[i],"-elevation_survey_feet") == 0)
    {
      set_elevation_in_survey_feet();
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-target_elevation_surveyfeet") == 0 || strcmp(argv[i],"-target_elevation_survey_feet") == 0)
    {
      set_elevation_in_survey_feet(false);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-elevation_feet") == 0)
    {
      set_elevation_in_feet();
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-target_elevation_feet") == 0)
    {
      set_elevation_in_feet(false);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-elevation_meter") == 0)
    {
      set_elevation_in_meter();
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-target_elevation_meter") == 0)
    {
      set_elevation_in_meter(false);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-target_precision") == 0)
    {
      set_target_precision(atof(argv[i+1]));
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-target_elevation_precision") == 0)
    {
      set_target_elevation_precision(atof(argv[i+1]));
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
  }
  return true;
}

int GeoProjectionConverter::unparse(char* string) const
{
  int n = 0;
  if (source_projection != 0)
  {
    if (source_projection->type == GEO_PROJECTION_LAT_LONG)
    {
      n += sprintf(&string[n], "-latlong ");
    }
    else if (source_projection->type == GEO_PROJECTION_LONG_LAT)
    {
      n += sprintf(&string[n], "-longlat ");
    }
    else if (source_projection->type == GEO_PROJECTION_ECEF)
    {
      n += sprintf(&string[n], "-ecef ");
    }
    else if (source_projection->geokey != 0)
    {
      n += sprintf(&string[n], "-epsg %u ", source_projection->geokey);
    }
    else if (source_projection->type == GEO_PROJECTION_UTM)
    {
      GeoProjectionParametersUTM* utm = (GeoProjectionParametersUTM*)source_projection;
      n += sprintf(&string[n], "-utm %d%c ", utm->utm_zone_number, (utm->utm_northern_hemisphere ? 'T' : 'K'));
    }
    else if (source_projection->type == GEO_PROJECTION_LCC)
    {
      GeoProjectionParametersLCC* lcc = (GeoProjectionParametersLCC*)source_projection;
      n += sprintf(&string[n], "-lcc %.10g %.10g m %.10g %.10g %.10g %.10g ", lcc->lcc_false_easting_meter, lcc->lcc_false_northing_meter, lcc->lcc_lat_origin_degree, lcc->lcc_long_meridian_degree, lcc->lcc_first_std_parallel_degree, lcc->lcc_second_std_parallel_degree);
    }
    else if (source_projection->type == GEO_PROJECTION_TM)
    {
      GeoProjectionParametersTM* tm = (GeoProjectionParametersTM*)source_projection;
      n += sprintf(&string[n], "-tm %.10g %.10g m %.10g %.10g %.10g ", tm->tm_false_easting_meter, tm->tm_false_northing_meter, tm->tm_lat_origin_degree, tm->tm_long_meridian_degree, tm->tm_scale_factor);
    }
  }
  if (ellipsoid)
  {
    n += sprintf(&string[n], "-ellipsoid %d ", ellipsoid->id);
  }
  if (coordinates2meter != 1.0)
  {
    if (coordinates2meter == 0.3048)
    {
      n += sprintf(&string[n], "-feet ");
    }
    else
    {
      n += sprintf(&string[n], "-surveyfeet ");
    }
  }
  if (elevation2meter != 1.0)
  {
    if (elevation2meter == 0.3048)
    {
      n += sprintf(&string[n], "-elevation_feet ");
    }
    else
    {
      n += sprintf(&string[n], "-elevation_surveyfeet ");
    }
  }
  if (vertical_geokey)
  {
    if (vertical_geokey == GEO_VERTICAL_NAVD88)
    {
      n += sprintf(&string[n], "-vertical_navd88 ");
    }
    else if (vertical_geokey == GEO_VERTICAL_WGS84)
    {
      n += sprintf(&string[n], "-vertical_wgs84 ");
    }
  }
  if (target_projection != 0)
  {
    if (target_projection->type == GEO_PROJECTION_LAT_LONG)
    {
      n += sprintf(&string[n], "-target_latlong ");
    }
    else if (target_projection->type == GEO_PROJECTION_LONG_LAT)
    {
      n += sprintf(&string[n], "-target_longlat ");
    }
    else if (target_projection->type == GEO_PROJECTION_ECEF)
    {
      n += sprintf(&string[n], "-target_ecef ");
    }
    else if (target_projection->geokey != 0)
    {
      n += sprintf(&string[n], "-target_epsg %u ", target_projection->geokey);
    }
    else if (target_projection->type == GEO_PROJECTION_UTM)
    {
      GeoProjectionParametersUTM* utm = (GeoProjectionParametersUTM*)target_projection;
      if (utm->utm_zone_number == -1)
      {
        n += sprintf(&string[n], "-target_utm auto ");
      }
      else
      {
        n += sprintf(&string[n], "-target_utm %d%c ", utm->utm_zone_number, (utm->utm_northern_hemisphere ? 'N' : 'K'));
      }
    }
    else if (target_projection->type == GEO_PROJECTION_LCC)
    {
      GeoProjectionParametersLCC* lcc = (GeoProjectionParametersLCC*)target_projection;
      n += sprintf(&string[n], "-target_lcc %g %g m %g %g %g %g ", lcc->lcc_false_easting_meter, lcc->lcc_false_northing_meter, lcc->lcc_lat_origin_degree, lcc->lcc_long_meridian_degree, lcc->lcc_first_std_parallel_degree, lcc->lcc_second_std_parallel_degree);
    }
    else if (target_projection->type == GEO_PROJECTION_TM)
    {
      GeoProjectionParametersTM* tm = (GeoProjectionParametersTM*)target_projection;
      n += sprintf(&string[n], "-target_tm %g %g m %g %g %g ", tm->tm_false_easting_meter, tm->tm_false_northing_meter, tm->tm_lat_origin_degree, tm->tm_long_meridian_degree, tm->tm_scale_factor);
    }
  }
  if (meter2coordinates != 1.0)
  {
    if (meter2coordinates == 1.0/0.3048)
    {
      n += sprintf(&string[n], "-target_feet ");
    }
    else
    {
      n += sprintf(&string[n], "-target_surveyfeet ");
    }
  }
  if (meter2elevation != 1.0)
  {
    if (meter2elevation == 1.0/0.3048)
    {
      n += sprintf(&string[n], "-target_elevation_feet ");
    }
    else
    {
      n += sprintf(&string[n], "-target_elevation_surveyfeet ");
    }
  }
  if (target_precision != 0.0)
  {
    n += sprintf(&string[n], "-target_precision %g ", target_precision);
  }
  if (target_elevation_precision != 0.0)
  {
    n += sprintf(&string[n], "-target_elevation_precision %g ", target_elevation_precision);
  }
  return n;
}

void GeoProjectionConverter::set_coordinates_in_survey_feet(bool source)
{
  if (source)
    coordinates2meter = 0.3048006096012;
  else
    meter2coordinates = 1.0/0.3048006096012;
  coordinate_units_set[(int)source] = true;
}

void GeoProjectionConverter::set_coordinates_in_feet(bool source)
{
  if (source)
    coordinates2meter = 0.3048;
  else
    meter2coordinates = 1.0/0.3048;
  coordinate_units_set[(int)source] = true;
}

void GeoProjectionConverter::set_coordinates_in_meter(bool source)
{
  if (source)
    coordinates2meter = 1.0;
  else
    meter2coordinates = 1.0;
  coordinate_units_set[(int)source] = true;
}

bool GeoProjectionConverter::has_coordinate_units(bool source) const
{
  return coordinate_units_set[(int)source];
}

const char* GeoProjectionConverter::get_coordinate_unit_description_string(bool abrev, bool source)
{
  if (coordinate_units_set[(int)source])
  {
    if (source)
      return (coordinates2meter == 1.0 ? (abrev ? "m" : "meter") : (coordinates2meter == 0.3048 ? (abrev ? "ft" : "feet") : (abrev ? "sft" : "surveyfeet")));
    else
      return (meter2coordinates == 1.0 ? (abrev ? "m" : "meter") : (meter2coordinates == 0.3048 ? (abrev ? "ft" : "feet") : (abrev ? "sft" : "surveyfeet")));
  }
  else
  {
    return "units";
  }
}

void GeoProjectionConverter::reset_coordinate_units(bool source)
{
  coordinate_units_set[(int)source] = false;
  if (source)
  {
    coordinates2meter = 1.0;
  }
  else
  {
    meter2coordinates = 1.0;
  }
}

void GeoProjectionConverter::set_elevation_in_survey_feet(bool source)
{
  if (source)
    elevation2meter = 0.3048006096012;
  else
    meter2elevation = 1.0/0.3048006096012;
  elevation_units_set[(int)source] = true;
}

void GeoProjectionConverter::set_elevation_in_feet(bool source)
{
  if (source)
    elevation2meter = 0.3048;
  else
    meter2elevation = 1.0/0.3048;
  elevation_units_set[(int)source] = true;
}

void GeoProjectionConverter::set_elevation_in_meter(bool source)
{
  if (source)
    elevation2meter = 1.0;
  else
    meter2elevation = 1.0;
  elevation_units_set[(int)source] = true;
}

bool GeoProjectionConverter::has_elevation_units(bool source) const
{
  return elevation_units_set[(int)source];
}

const char* GeoProjectionConverter::get_elevation_unit_description_string(bool abrev, bool source)
{
  if (elevation_units_set[(int)source])
  {
    if (source)
      return (elevation2meter == 1.0 ? (abrev ? "m" : "meter") : (abrev ? "ft" : "feet"));
    else
      return (meter2elevation == 1.0 ? (abrev ? "m" : "meter") : (abrev ? "ft" : "feet"));
  }
  else
  {
    return "units";
  }
}

void GeoProjectionConverter::reset_elevation_units(bool source)
{
  elevation_units_set[(int)source] = false;
  if (source)
  {
    elevation2meter = 1.0;
  }
  else
  {
    meter2elevation = 1.0;
  }
}

void GeoProjectionConverter::set_elevation_offset_in_meter(float elevation_offset_in_meter)
{
  this->elevation_offset_in_meter = elevation_offset_in_meter;
}

bool GeoProjectionConverter::to_lon_lat_ele(double* point) const
{
  return to_lon_lat_ele(point, point[0], point[1], point[2]);
}

bool GeoProjectionConverter::to_lon_lat_ele(const double* point, double& longitude, double& latitude, double& elevation_in_meter) const
{
  if (source_projection)
  {
    switch (source_projection->type)
    {
    case GEO_PROJECTION_UTM:
      UTMtoLL(point[0], point[1], latitude, longitude, ellipsoid, (const GeoProjectionParametersUTM*)source_projection);
      break;
    case GEO_PROJECTION_LCC:
      LCCtoLL(coordinates2meter*point[0], coordinates2meter*point[1], latitude, longitude, ellipsoid, (const GeoProjectionParametersLCC*)source_projection);
      break;
    case GEO_PROJECTION_TM:
      TMtoLL(coordinates2meter*point[0], coordinates2meter*point[1], latitude, longitude, ellipsoid, (const GeoProjectionParametersTM*)source_projection);
      break;
    case GEO_PROJECTION_LONG_LAT:
      longitude = point[0];
      latitude = point[1];
      break;
    case GEO_PROJECTION_LAT_LONG:
      longitude = point[1];
      latitude = point[0];
      break;
    case GEO_PROJECTION_ECEF:
      ECEFtoLL(coordinates2meter*point[0], coordinates2meter*point[1], coordinates2meter*point[2], latitude, longitude, elevation_in_meter, ellipsoid);
      break;
    case GEO_PROJECTION_AEAC:
      AEACtoLL(coordinates2meter*point[0], coordinates2meter*point[1], latitude, longitude, ellipsoid, (const GeoProjectionParametersAEAC*)source_projection);
      break;
    }
    elevation_in_meter = elevation2meter*point[2] + elevation_offset_in_meter;
    return true;
  }
  return false;
}

bool GeoProjectionConverter::to_target(double* point) const
{
  if (target_projection)
  {
    return to_target(point, point[0], point[1], point[2]);
  }
  return false;
}

bool GeoProjectionConverter::to_target(const double* point,  double &x, double &y, double& elevation) const
{
  if (source_projection && target_projection)
  {
    double longitude, latitude;

    switch (source_projection->type)
    {
    case GEO_PROJECTION_UTM:
      UTMtoLL(coordinates2meter*point[0], coordinates2meter*point[1], latitude, longitude, ellipsoid, (const GeoProjectionParametersUTM*)source_projection);
      break;
    case GEO_PROJECTION_LCC:
      LCCtoLL(coordinates2meter*point[0], coordinates2meter*point[1], latitude, longitude, ellipsoid, (const GeoProjectionParametersLCC*)source_projection);
      break;
    case GEO_PROJECTION_TM:
      TMtoLL(coordinates2meter*point[0], coordinates2meter*point[1], latitude, longitude, ellipsoid, (const GeoProjectionParametersTM*)source_projection);
      break;
    case GEO_PROJECTION_LONG_LAT:
      longitude = point[0];
      latitude = point[1];
      break;
    case GEO_PROJECTION_LAT_LONG:
      longitude = point[1];
      latitude = point[0];
      break;
    case GEO_PROJECTION_ECEF:
      ECEFtoLL(coordinates2meter*point[0], coordinates2meter*point[1], coordinates2meter*point[2], latitude, longitude, elevation, ellipsoid);
      break;
    case GEO_PROJECTION_AEAC:
      AEACtoLL(coordinates2meter*point[0], coordinates2meter*point[1], latitude, longitude, ellipsoid, (const GeoProjectionParametersAEAC*)source_projection);
      break;
    }

    switch (target_projection->type)
    {
    case GEO_PROJECTION_UTM:
      if (((GeoProjectionParametersUTM*)target_projection)->utm_zone_number == -1) compute_utm_zone(latitude, longitude, (GeoProjectionParametersUTM*)target_projection);
      LLtoUTM(latitude, longitude, x, y, ellipsoid, (const GeoProjectionParametersUTM*)target_projection);
      x = meter2coordinates * x;
      y = meter2coordinates * y;
      break;
    case GEO_PROJECTION_LCC:
      LLtoLCC(latitude, longitude, x, y, ellipsoid, (const GeoProjectionParametersLCC*)target_projection);
      x = meter2coordinates * x;
      y = meter2coordinates * y;
      break;
    case GEO_PROJECTION_TM:
      LLtoTM(latitude, longitude, x, y, ellipsoid, (const GeoProjectionParametersTM*)target_projection);
      x = meter2coordinates * x;
      y = meter2coordinates * y;
      break;
    case GEO_PROJECTION_LONG_LAT:
      x = longitude;
      y = latitude;
      break;
    case GEO_PROJECTION_LAT_LONG:
      x = latitude;
      y = longitude;
      break;
    case GEO_PROJECTION_ECEF:
      LLtoECEF(latitude, longitude, elevation, x, y, elevation, ellipsoid);
      x = meter2coordinates * x;
      y = meter2coordinates * y;
      elevation = meter2coordinates * elevation;
      break;
    case GEO_PROJECTION_AEAC:
      LLtoAEAC(latitude, longitude, x, y, ellipsoid, (const GeoProjectionParametersAEAC*)target_projection);
      x = meter2coordinates * x;
      y = meter2coordinates * y;
      break;
    }
    elevation = meter2elevation * (elevation2meter*point[2] + elevation_offset_in_meter);
    return true;
  }
  return false;
}

double GeoProjectionConverter::get_target_precision() const
{
  if (target_precision)
  {
    return target_precision;
  }
  if (target_projection && (target_projection->type == GEO_PROJECTION_LONG_LAT || target_projection->type == GEO_PROJECTION_LAT_LONG))
  {
    return 1e-7;
  }
  return 0.01;
}

void GeoProjectionConverter::set_target_precision(double target_precision)
{
  this->target_precision = target_precision;
}

double GeoProjectionConverter::get_target_elevation_precision() const
{
  if (target_elevation_precision)
  {
    return target_elevation_precision;
  }
  return 0.01;
}

void GeoProjectionConverter::set_target_elevation_precision(double target_elevation_precision)
{
  this->target_elevation_precision = target_elevation_precision;
}

/*
bool GeoProjectionConverter::get_img_datum_parameters(char** psDatumame, int* proNumber, int* proZone, double** proParams,) const
{
  if (ellipsoid)
  {
    if (ellipsoid->id == GEO_ELLIPSOID_WGS84)
    {
      *psDatumame = strdup("NAD27");

        if (utm->utm_northern_hemisphere)
        {
          return utm->utm_zone_number + 32600;
        }
        else
        {
          return utm->utm_zone_number + 32700;
        }
      }
        else if (ellipsoid->id == GEO_ELLIPSOID_WGS72)
        {
          if (utm->utm_northern_hemisphere)
          {
            return utm->utm_zone_number + 32200;
          }
          else
          {
            return utm->utm_zone_number + 32300;
          }
        }
        else if (ellipsoid->id == GEO_ELLIPSOID_NAD83)
        {
          if (utm->utm_northern_hemisphere)
          {
            if (3 <= utm->utm_zone_number && utm->utm_zone_number <= 23)
            {
              return utm->utm_zone_number + 26900;
            }
            else
            {
              fprintf(stderr, "get_ProjectedCSTypeGeoKey: northern UTM zone %d for NAD83 out-of-range\n", utm->utm_zone_number);
            }
          }
          else
          {
            fprintf(stderr, "get_ProjectedCSTypeGeoKey: southern UTM zone %d for NAD83 does not exist\n", utm->utm_zone_number);
          }
        }
        else if (ellipsoid->id == GEO_ELLIPSOID_NAD27)
        {
          if (utm->utm_northern_hemisphere)
          {
            if (3 <= utm->utm_zone_number && utm->utm_zone_number <= 22)
            {
              return utm->utm_zone_number + 26700;
            }
            else
            {
              fprintf(stderr, "get_ProjectedCSTypeGeoKey: northern UTM zone %d for NAD27 out-of-range\n", utm->utm_zone_number);
            }
          }
          else
          {
            fprintf(stderr, "get_ProjectedCSTypeGeoKey: southern UTM zone %d for NAD27 does not exist\n", utm->utm_zone_number);
          }
        }
        else if (ellipsoid->id == GEO_ELLIPSOID_SAD69)
        {
          if (utm->utm_northern_hemisphere)
          {
            if (18 <= utm->utm_zone_number && utm->utm_zone_number <= 22)
            {
              return utm->utm_zone_number + 29100;
            }
            else
            {
              fprintf(stderr, "get_ProjectedCSTypeGeoKey: northern UTM zone %d for SAD69 out-of-range\n", utm->utm_zone_number);
            }
          }
          else
          {
            if (17 <= utm->utm_zone_number && utm->utm_zone_number <= 25)
            {
              return utm->utm_zone_number + 29160;
            }
            else
            {
              fprintf(stderr, "get_ProjectedCSTypeGeoKey: southern UTM zone %d for SAD69 out-of-range\n", utm->utm_zone_number);
            }
          }
        }
        else if (ellipsoid->id == GEO_ELLIPSOID_Inter)
        {
          if (utm->utm_northern_hemisphere)
          {
            if (28 <= utm->utm_zone_number && utm->utm_zone_number <= 38)
            {
              return utm->utm_zone_number + 23000;
            }
            else
            {
              fprintf(stderr, "get_ProjectedCSTypeGeoKey: northern UTM zone %d for ED50 out-of-range\n", utm->utm_zone_number);
            }
          }
          else
          {
            fprintf(stderr, "get_ProjectedCSTypeGeoKey: southern UTM zone %d for ED50 does not exist\n", utm->utm_zone_number);
          }
        }
        else if (ellipsoid->id == GEO_ELLIPSOID_ID74)
        {
          if (utm->utm_northern_hemisphere)
          {
            if (46 <= utm->utm_zone_number && utm->utm_zone_number <= 53)
            {
              return utm->utm_zone_number + 23800;
            }
            else
            {
              fprintf(stderr, "get_ProjectedCSTypeGeoKey: northern UTM zone %d for ID74 out-of-range\n", utm->utm_zone_number);
            }
          }
          else
          {
            if (46 <= utm->utm_zone_number && utm->utm_zone_number <= 54)
            {
              return utm->utm_zone_number + 23840;
            }
            else
            {
              fprintf(stderr, "get_ProjectedCSTypeGeoKey: southern UTM zone %d for ID74 out-of-range\n", utm->utm_zone_number);
            }
          }
        }
        else
        {
          fprintf(stderr, "get_ProjectedCSTypeGeoKey: look-up for UTM zone %d and ellipsoid with id %d not implemented\n", utm->utm_zone_number, ellipsoid->id);
        }
        }
  psDatum->datumname
}

bool GeoProjectionConverter::get_img_projection_parameters(char** proName, int* proNumber, int* proZone, double** proParams,) const
{
  GeoProjectionParameters* projection = get_projection(source);
  if (projection)
  {
    if (projection->type == GEO_PROJECTION_UTM)
    {
      GeoProjectionParametersUTM* utm = (GeoProjectionParametersUTM*)projection;
      *proNumber = 1;
      *proZone = utm->utm_zone_number;
      *proParams[0] = 0;
    }
    else 



    }
    else
    {
    }
    if (projection->type == projection->type == GEO_PROJECTION_LAT_LONG || projection->type == GEO_PROJECTION_LONG_LAT)
    {
      *proNumber = 0;
    }
    else if (projection->type == 
  }
  else if ()
  {
  }
}
*/

bool GeoProjectionConverter::get_dtm_projection_parameters(short* horizontal_units, short* vertical_units, short* coordinate_system, short* coordinate_zone, short* horizontal_datum, short* vertical_datum, bool source)
{
  if (get_ProjLinearUnitsGeoKey() == 9001)
  {
    *horizontal_units = 1;
  }
  else if (get_ProjLinearUnitsGeoKey() == 9002)
  {
    *horizontal_units = 0;
  }
  else if (get_ProjLinearUnitsGeoKey() == 9003)
  {
    *horizontal_units = 0;
  }
  else
  {
    *horizontal_units = 2;
  }

  if (get_VerticalUnitsGeoKey() == 9001)
  {
    *vertical_units = 1;
  }
  else if (get_VerticalUnitsGeoKey() == 9002)
  {
    *vertical_units = 0;
  }
  else if (get_VerticalUnitsGeoKey() == 9003)
  {
    *vertical_units = 0;
  }
  else
  {
    *vertical_units = 2;
  }

  GeoProjectionParameters* projection = get_projection(source);
  if (projection)
  {
    if (projection->type == GEO_PROJECTION_UTM)
    {
      *coordinate_system = 2;
      GeoProjectionParametersUTM* utm = (GeoProjectionParametersUTM*)projection;
      if (utm->utm_northern_hemisphere)
        *coordinate_zone = utm->utm_zone_number;
      else
        *coordinate_zone = utm->utm_zone_number + 100;
    }
    else if ((projection->type == GEO_PROJECTION_LCC) || (projection->type == GEO_PROJECTION_TM))
    {
      unsigned short geokey = get_ProjectedCSTypeGeoKey(source);
      if ((geokey != 0) && (geokey != 32767))
      {
        switch(geokey)
        {
        case PCS_NAD83_Alabama_East:
          *coordinate_zone = GCTP_NAD83_Alabama_East;
          break;
        case PCS_NAD83_Alabama_West:
          *coordinate_zone = GCTP_NAD83_Alabama_West;
          break;
        case PCS_NAD83_Alaska_zone_1:
          *coordinate_zone = GCTP_NAD83_Alaska_zone_1;
          break;
        case PCS_NAD83_Alaska_zone_2:
          *coordinate_zone = GCTP_NAD83_Alaska_zone_2;
          break;
        case PCS_NAD83_Alaska_zone_3:
          *coordinate_zone = GCTP_NAD83_Alaska_zone_3;
          break;
        case PCS_NAD83_Alaska_zone_4:
          *coordinate_zone = GCTP_NAD83_Alaska_zone_4;
          break;
        case PCS_NAD83_Alaska_zone_5:
          *coordinate_zone = GCTP_NAD83_Alaska_zone_5;
          break;
        case PCS_NAD83_Alaska_zone_6:
          *coordinate_zone = GCTP_NAD83_Alaska_zone_6;
          break;
        case PCS_NAD83_Alaska_zone_7:
          *coordinate_zone = GCTP_NAD83_Alaska_zone_7;
          break;
        case PCS_NAD83_Alaska_zone_8:
          *coordinate_zone = GCTP_NAD83_Alaska_zone_8;
          break;
        case PCS_NAD83_Alaska_zone_9:
          *coordinate_zone = GCTP_NAD83_Alaska_zone_9;
          break;
        case PCS_NAD83_Alaska_zone_10:
          *coordinate_zone = GCTP_NAD83_Alaska_zone_10;
          break;
        case PCS_NAD83_California_1:
          *coordinate_zone = GCTP_NAD83_California_1;
          break;
        case PCS_NAD83_California_2:
          *coordinate_zone = GCTP_NAD83_California_2;
          break;
        case PCS_NAD83_California_3:
          *coordinate_zone = GCTP_NAD83_California_3;
          break;
        case PCS_NAD83_California_4:
          *coordinate_zone = GCTP_NAD83_California_4;
          break;
        case PCS_NAD83_California_5:
          *coordinate_zone = GCTP_NAD83_California_5;
          break;
        case PCS_NAD83_California_6:
          *coordinate_zone = GCTP_NAD83_California_6;
          break;
        case PCS_NAD83_Arizona_East:
          *coordinate_zone = GCTP_NAD83_Arizona_East;
          break;
        case PCS_NAD83_Arizona_Central:
          *coordinate_zone = GCTP_NAD83_Arizona_Central;
          break;
        case PCS_NAD83_Arizona_West:
          *coordinate_zone = GCTP_NAD83_Arizona_West;
          break;
        case PCS_NAD83_Arkansas_North:
          *coordinate_zone = GCTP_NAD83_Arkansas_North;
          break;
        case PCS_NAD83_Arkansas_South:
          *coordinate_zone = GCTP_NAD83_Arkansas_South;
          break;
        case PCS_NAD83_Colorado_North:
          *coordinate_zone = GCTP_NAD83_Colorado_North;
          break;
        case PCS_NAD83_Colorado_Central:
          *coordinate_zone = GCTP_NAD83_Colorado_Central;
          break;
        case PCS_NAD83_Colorado_South:
          *coordinate_zone = GCTP_NAD83_Colorado_South;
          break;
        case PCS_NAD83_Connecticut:
          *coordinate_zone = GCTP_NAD83_Connecticut;
          break;
        case PCS_NAD83_Delaware:
          *coordinate_zone = GCTP_NAD83_Delaware;
          break;
        case PCS_NAD83_Florida_East:
          *coordinate_zone = GCTP_NAD83_Florida_East;
          break;
        case PCS_NAD83_Florida_West:
          *coordinate_zone = GCTP_NAD83_Florida_West;
          break;
        case PCS_NAD83_Florida_North:
          *coordinate_zone = GCTP_NAD83_Florida_North;
          break;
        case PCS_NAD83_Hawaii_zone_1:
          *coordinate_zone = GCTP_NAD83_Hawaii_zone_1;
          break;
        case PCS_NAD83_Hawaii_zone_2:
          *coordinate_zone = GCTP_NAD83_Hawaii_zone_2;
          break;
        case PCS_NAD83_Hawaii_zone_3:
          *coordinate_zone = GCTP_NAD83_Hawaii_zone_3;
          break;
        case PCS_NAD83_Hawaii_zone_4:
          *coordinate_zone = GCTP_NAD83_Hawaii_zone_4;
          break;
        case PCS_NAD83_Hawaii_zone_5:
          *coordinate_zone = GCTP_NAD83_Hawaii_zone_5;
          break;
        case PCS_NAD83_Georgia_East:
          *coordinate_zone = GCTP_NAD83_Georgia_East;
          break;
        case PCS_NAD83_Georgia_West:
          *coordinate_zone = GCTP_NAD83_Georgia_West;
          break;
        case PCS_NAD83_Idaho_East:
          *coordinate_zone = GCTP_NAD83_Idaho_East;
          break;
        case PCS_NAD83_Idaho_Central:
          *coordinate_zone = GCTP_NAD83_Idaho_Central;
          break;
        case PCS_NAD83_Idaho_West:
          *coordinate_zone = GCTP_NAD83_Idaho_West;
          break;
        case PCS_NAD83_Illinois_East:
          *coordinate_zone = GCTP_NAD83_Illinois_East;
          break;
        case PCS_NAD83_Illinois_West:
          *coordinate_zone = GCTP_NAD83_Illinois_West;
          break;
        case PCS_NAD83_Indiana_East:
          *coordinate_zone = GCTP_NAD83_Indiana_East;
          break;
        case PCS_NAD83_Indiana_West:
          *coordinate_zone = GCTP_NAD83_Indiana_West;
          break;
        case PCS_NAD83_Iowa_North:
          *coordinate_zone = GCTP_NAD83_Iowa_North;
          break;
        case PCS_NAD83_Iowa_South:
          *coordinate_zone = GCTP_NAD83_Iowa_South;
          break;
        case PCS_NAD83_Kansas_North:
          *coordinate_zone = GCTP_NAD83_Kansas_North;
          break;
        case PCS_NAD83_Kansas_South:
          *coordinate_zone = GCTP_NAD83_Kansas_South;
          break;
        case PCS_NAD83_Kentucky_North:
          *coordinate_zone = GCTP_NAD83_Kentucky_North;
          break;
        case PCS_NAD83_Kentucky_South:
          *coordinate_zone = GCTP_NAD83_Kentucky_North;
          break;
        case PCS_NAD83_Louisiana_North:
          *coordinate_zone = GCTP_NAD83_Louisiana_North;
          break;
        case PCS_NAD83_Louisiana_South:
          *coordinate_zone = GCTP_NAD83_Louisiana_South;
          break;
        case PCS_NAD83_Maine_East:
          *coordinate_zone = GCTP_NAD83_Maine_East;
          break;
        case PCS_NAD83_Maine_West:
          *coordinate_zone = GCTP_NAD83_Maine_West;
          break;
        case PCS_NAD83_Maryland:
          *coordinate_zone = GCTP_NAD83_Maryland;
          break;
        case PCS_NAD83_Massachusetts:
          *coordinate_zone = GCTP_NAD83_Massachusetts;
          break;
        case PCS_NAD83_Massachusetts_Is:
          *coordinate_zone = GCTP_NAD83_Massachusetts_Is;
          break;
        case PCS_NAD83_Michigan_North:
          *coordinate_zone = GCTP_NAD83_Michigan_North;
          break;
        case PCS_NAD83_Michigan_Central:
          *coordinate_zone = GCTP_NAD83_Michigan_Central;
          break;
        case PCS_NAD83_Michigan_South:
          *coordinate_zone = GCTP_NAD83_Michigan_South;
          break;
        case PCS_NAD83_Minnesota_North:
          *coordinate_zone = GCTP_NAD83_Minnesota_North;
          break;
        case PCS_NAD83_Minnesota_Central:
          *coordinate_zone = GCTP_NAD83_Minnesota_Central;
          break;
        case PCS_NAD83_Minnesota_South:
          *coordinate_zone = GCTP_NAD83_Minnesota_South;
          break;
        case PCS_NAD83_Mississippi_East:
          *coordinate_zone = GCTP_NAD83_Mississippi_East;
          break;
        case PCS_NAD83_Mississippi_West:
          *coordinate_zone = GCTP_NAD83_Mississippi_West;
          break;
        case PCS_NAD83_Missouri_East:
          *coordinate_zone = GCTP_NAD83_Missouri_East;
          break;
        case PCS_NAD83_Missouri_Central:
          *coordinate_zone = GCTP_NAD83_Missouri_Central;
          break;
        case PCS_NAD83_Missouri_West:
          *coordinate_zone = GCTP_NAD83_Missouri_West;
          break;
        case PCS_NAD83_Montana:
          *coordinate_zone = GCTP_NAD83_Montana;
          break;
        case PCS_NAD83_Nebraska:
          *coordinate_zone = GCTP_NAD83_Nebraska;
          break;
        case PCS_NAD83_Nevada_East:
          *coordinate_zone = GCTP_NAD83_Nevada_East;
          break;
        case PCS_NAD83_Nevada_Central:
          *coordinate_zone = GCTP_NAD83_Nevada_Central;
          break;
        case PCS_NAD83_Nevada_West:
          *coordinate_zone = GCTP_NAD83_Nevada_West;
          break;
        case PCS_NAD83_New_Hampshire:
          *coordinate_zone = GCTP_NAD83_New_Hampshire;
          break;
        case PCS_NAD83_New_Jersey:
          *coordinate_zone = GCTP_NAD83_New_Jersey;
          break;
        case PCS_NAD83_New_Mexico_East:
          *coordinate_zone = GCTP_NAD83_New_Mexico_East;
          break;
        case PCS_NAD83_New_Mexico_Central:
          *coordinate_zone = GCTP_NAD83_New_Mexico_Central;
          break;
        case PCS_NAD83_New_Mexico_West:
          *coordinate_zone = GCTP_NAD83_New_Mexico_West;
          break;
        case PCS_NAD83_New_York_East:
          *coordinate_zone = GCTP_NAD83_New_York_East;
          break;
        case PCS_NAD83_New_York_Central:
          *coordinate_zone = GCTP_NAD83_New_York_Central;
          break;
        case PCS_NAD83_New_York_West:
          *coordinate_zone = GCTP_NAD83_New_York_West;
          break;
        case PCS_NAD83_New_York_Long_Is:
          *coordinate_zone = GCTP_NAD83_New_York_Long_Is;
          break;
        case PCS_NAD83_North_Carolina:
          *coordinate_zone = GCTP_NAD83_North_Carolina;
          break;
        case PCS_NAD83_North_Dakota_N:
          *coordinate_zone = GCTP_NAD83_North_Dakota_N;
          break;
        case PCS_NAD83_North_Dakota_S:
          *coordinate_zone = GCTP_NAD83_North_Dakota_S;
          break;
        case PCS_NAD83_Ohio_North:
          *coordinate_zone = GCTP_NAD83_Ohio_North;
          break;
        case PCS_NAD83_Ohio_South:
          *coordinate_zone = GCTP_NAD83_Ohio_South;
          break;
        case PCS_NAD83_Oklahoma_North:
          *coordinate_zone = GCTP_NAD83_Oklahoma_North;
          break;
        case PCS_NAD83_Oklahoma_South:
          *coordinate_zone = GCTP_NAD83_Oklahoma_South;
          break;
        case PCS_NAD83_Oregon_North:
          *coordinate_zone = GCTP_NAD83_Oregon_North;
          break;
        case PCS_NAD83_Oregon_South:
          *coordinate_zone = GCTP_NAD83_Oregon_South;
          break;
        case PCS_NAD83_Pennsylvania_N:
          *coordinate_zone = GCTP_NAD83_Pennsylvania_N;
          break;
        case PCS_NAD83_Pennsylvania_S:
          *coordinate_zone = GCTP_NAD83_Pennsylvania_S;
          break;
        case PCS_NAD83_Rhode_Island:
          *coordinate_zone = GCTP_NAD83_Rhode_Island;
          break;
        case PCS_NAD83_South_Carolina:
          *coordinate_zone = GCTP_NAD83_South_Carolina;
          break;
        case PCS_NAD83_South_Dakota_N:
          *coordinate_zone = GCTP_NAD83_South_Dakota_N;
          break;
        case PCS_NAD83_South_Dakota_S:
          *coordinate_zone = GCTP_NAD83_South_Dakota_S;
          break;
        case PCS_NAD83_Tennessee:
          *coordinate_zone = GCTP_NAD83_Tennessee;
          break;
        case PCS_NAD83_Texas_North:
          *coordinate_zone = GCTP_NAD83_Texas_North;
          break;
        case PCS_NAD83_Texas_North_Central:
          *coordinate_zone = GCTP_NAD83_Texas_North_Central;
          break;
        case PCS_NAD83_Texas_Central:
          *coordinate_zone = GCTP_NAD83_Texas_Central;
          break;
        case PCS_NAD83_Texas_South_Central:
          *coordinate_zone = GCTP_NAD83_Texas_South_Central;
          break;
        case PCS_NAD83_Texas_South:
          *coordinate_zone = GCTP_NAD83_Texas_South;
          break;
        case PCS_NAD83_Utah_North:
          *coordinate_zone = GCTP_NAD83_Utah_North;
          break;
        case PCS_NAD83_Utah_Central:
          *coordinate_zone = GCTP_NAD83_Utah_Central;
          break;
        case PCS_NAD83_Utah_South:
          *coordinate_zone = GCTP_NAD83_Utah_South;
          break;
        case PCS_NAD83_Vermont:
          *coordinate_zone = GCTP_NAD83_Vermont;
          break;
        case PCS_NAD83_Virginia_North:
          *coordinate_zone = GCTP_NAD83_Virginia_North;
          break;
        case PCS_NAD83_Virginia_South:
          *coordinate_zone = GCTP_NAD83_Virginia_South;
          break;
        case PCS_NAD83_Washington_North:
          *coordinate_zone = GCTP_NAD83_Washington_North;
          break;
        case PCS_NAD83_Washington_South:
          *coordinate_zone = GCTP_NAD83_Washington_South;
          break;
        case PCS_NAD83_West_Virginia_N:
          *coordinate_zone = GCTP_NAD83_West_Virginia_N;
          break;
        case PCS_NAD83_West_Virginia_S:
          *coordinate_zone = GCTP_NAD83_West_Virginia_S;
          break;
        case PCS_NAD83_Wisconsin_North:
          *coordinate_zone = GCTP_NAD83_Wisconsin_North;
          break;
        case PCS_NAD83_Wisconsin_Central:
          *coordinate_zone = GCTP_NAD83_Wisconsin_Central;
          break;
        case PCS_NAD83_Wisconsin_South:
          *coordinate_zone = GCTP_NAD83_Wisconsin_South;
          break;
        case PCS_NAD83_Wyoming_East:
          *coordinate_zone = GCTP_NAD83_Wyoming_East;
          break;
        case PCS_NAD83_Wyoming_East_Central:
          *coordinate_zone = GCTP_NAD83_Wyoming_East_Central;
          break;
        case PCS_NAD83_Wyoming_West_Central:
          *coordinate_zone = GCTP_NAD83_Wyoming_West_Central;
          break;
        case PCS_NAD83_Wyoming_West:
          *coordinate_zone = GCTP_NAD83_Wyoming_West;
          break;
        case PCS_NAD83_Puerto_Rico:
          *coordinate_zone = GCTP_NAD83_Puerto_Rico;
          break;
        default:
          *coordinate_zone = 0;
          fprintf(stderr, "state plane with geotiff tag %d not implemented\n", (int)geokey);
        }
        if (*coordinate_zone)
        {
          *horizontal_datum = 2;
          *coordinate_system = 3;
        }
      }
    }
    else
    {
      *coordinate_system = 0;
      *coordinate_zone = 0;
    }
  }
  else
  {
    *coordinate_system = 0;
    *coordinate_zone = 0;
  }

  if (ellipsoid && ellipsoid->id == GEO_ELLIPSOID_NAD27)
  {
    *horizontal_datum = 1;
  }
  else if (ellipsoid && ellipsoid->id == GEO_ELLIPSOID_NAD83)
  {
    *horizontal_datum = 2;
  }
  else if (ellipsoid && ellipsoid->id == GEO_ELLIPSOID_WGS84)
  {
    *horizontal_datum = 3;
  }
  else
  {
    *horizontal_datum = 0;
  }

  if (vertical_geokey == GEO_VERTICAL_NAVD88)
  {
    *vertical_datum = 2;
  }
  else if (vertical_geokey == GEO_VERTICAL_WGS84)
  {
    *vertical_datum = 3;
  }
  else if (vertical_geokey == GEO_VERTICAL_NAVD29)
  {
    *vertical_datum = 1;
  }
  else
  {
    *vertical_datum = 0;
  }

  return true;
}

bool GeoProjectionConverter::set_dtm_projection_parameters(short horizontal_units, short vertical_units, short coordinate_system, short coordinate_zone, short horizontal_datum, short vertical_datum, bool source)
{
  if (horizontal_units == 1)
  {
    set_ProjLinearUnitsGeoKey(9001);
  }
  else if (horizontal_units == 0)
  {
    set_ProjLinearUnitsGeoKey(9002);
  }

  if (vertical_units == 1)
  {
    set_VerticalUnitsGeoKey(9001);
  }
  else if (vertical_units == 0)
  {
    set_VerticalUnitsGeoKey(9002);
  }

  if (coordinate_system == 1)
  {
    GeoProjectionParametersUTM* utm = new GeoProjectionParametersUTM();
    utm->utm_northern_hemisphere = true;
    utm->utm_zone_number = coordinate_zone;
    utm->utm_zone_letter = 'N';
    set_projection(utm, true);
  }

  if (horizontal_datum == 1)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_NAD27);
  }
  else if (horizontal_datum == 2)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_NAD83);
  }
  else if (horizontal_datum == 3)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_WGS84);
  }

  if (vertical_datum == 2)
  {
    vertical_geokey = GEO_VERTICAL_NAVD88;
  }
  else if (vertical_datum == 3)
  {
    vertical_geokey = GEO_VERTICAL_WGS84;
  }
  else if (vertical_datum == 1)
  {
    vertical_geokey = GEO_VERTICAL_NAVD29;
  }

  return true;
}

#ifdef NOT

 /***************************************************************************/
 /* RSC IDENTIFIER: OBLIQUE MERCATOR
  *
  * ABSTRACT
  *
  *    This component provides conversions between Geodetic coordinates
  *    (latitude and longitude in radians) and Oblique Mercator
  *    projection coordinates (easting and northing in meters).
  *
  * ERROR HANDLING
  *
  *    This component checks parameters for valid values.  If an invalid value
  *    is found the error code is combined with the current error code using
  *    the bitwise or.  This combining allows multiple error codes to be
  *    returned. The possible error codes are:
  *
  *       OMERC_NO_ERROR           : No errors occurred in function
  *       OMERC_LAT_ERROR          : Latitude outside of valid range
  *                                     (-90 to 90 degrees)
  *       OMERC_LON_ERROR          : Longitude outside of valid range
  *                                     (-180 to 360 degrees)
  *       OMERC_ORIGIN_LAT_ERROR   : Origin latitude outside of valid range
  *                                     (-89 to 89 degrees)
  *       OMERC_LAT1_ERROR         : First latitude outside of valid range
  *                                     (-89 to 89 degrees, excluding 0)
  *       OMERC_LAT2_ERROR         : First latitude outside of valid range
  *                                     (-89 to 89 degrees)
  *       OMERC_LON1_ERROR         : First longitude outside of valid range
  *                                     (-180 to 360 degrees)
  *       OMERC_LON2_ERROR         : Second longitude outside of valid range
  *                                     (-180 to 360 degrees)
  *       OMERC_LAT1_LAT2_ERROR    : First and second latitudes can not be equal
  *       OMERC_DIFF_HEMISPHERE_ERROR: First and second latitudes can not be
  *                                     in different hemispheres
  *       OMERC_EASTING_ERROR      : Easting outside of valid range
  *                                     (depends on ellipsoid and projection
  *                                     parameters)
  *       OMERC_NORTHING_ERROR     : Northing outside of valid range
  *                                     (depends on ellipsoid and projection
  *                                     parameters)
  *       OMERC_A_ERROR            : Semi-major axis less than or equal to zero
  *       OMERC_INV_F_ERROR        : Inverse flattening outside of valid range
  *                                     (250 to 350)
  *       OMERC_SCALE_FACTOR_ERROR : Scale factor outside of valid
  *                                     range (0.3 to 3.0)
  *       OMERC_LON_WARNING        : Distortion will result if longitude is 90 degrees or more
  *                                     from the Central Meridian
  *
  * REUSE NOTES
  *
  *    OBLIQUE MERCATOR is intended for reuse by any application that 
  *    performs an Oblique Mercator projection or its inverse.
  *
  * REFERENCES
  *
  *    Further information on OBLIQUE MERCATOR can be found in the Reuse Manual.
  *
  *    OBLIQUE MERCATOR originated from:     U.S. Army Topographic Engineering Center
  *                                          Geospatial Information Division
  *                                          7701 Telegraph Road
  *                                          Alexandria, VA  22310-3864
  *
  * LICENSES
  *
  *    None apply to this component.
  *
  * RESTRICTIONS
  *
  *    OBLIQUE MERCATOR has no restrictions.
  *
  * ENVIRONMENT
  *
  *    OBLIQUE MERCATOR was tested and certified in the following environments:
  *
  *    1. Solaris 2.5 with GCC, version 2.8.1
  *    2. MSDOS with MS Visual C++, version 6
  *
  * MODIFICATIONS
  *
  *    Date              Description
  *    ----              -----------
  *    06-07-00          Original Code
  *    03-02-07          Original C++ Code
  *    
  *
  */
 
 
 #include "CoordinateSystem.h"
 
 
 namespace MSP
 {
   namespace CCS
   {
     class ObliqueMercatorParameters;
     class MapProjectionCoordinates;
     class GeodeticCoordinates;
 
 
     /***************************************************************************/
     /*
      *                              DEFINES
      */
 
     class ObliqueMercator : public CoordinateSystem
     {
     public:
 
       /*
        * The constructor receives the ellipsoid parameters and
        * projection parameters as inputs, and sets the corresponding state
        * variables.  If any errors occur, an exception is thrown with a description 
        * of the error.
        *
        *    ellipsoidSemiMajorAxis   : Semi-major axis of ellipsoid, in meters  (input)
        *    ellipsoidFlattening      : Flattening of ellipsoid                  (input)
        *    originLatitude           : Latitude, in radians, at which the       (input)
        *                               point scale factor is 1.0
        *    longitude1               : Longitude, in radians, of first point lying on
        *                               central line                             (input)
        *    latitude1                : Latitude, in radians, of first point lying on
        *                               central line                             (input)
        *    longitude2               : Longitude, in radians, of second point lying on
        *                               central line                             (input)
        *    latitude2                : Latitude, in radians, of second point lying on
        *                               central line                             (input)
        *    falseEasting             : A coordinate value, in meters, assigned to the
        *                               central meridian of the projection       (input)
        *    falseNorthing            : A coordinate value, in meters, assigned to the
        *                               origin latitude of the projection        (input)
        *    scaleFactor              : Multiplier which reduces distances in the
        *                               projection to the actual distance on the
        *                               ellipsoid                                (input)
        *    errorStatus              : Error status                             (output) 
        */
 
         ObliqueMercator( double ellipsoidSemiMajorAxis, double ellipsoidFlattening, double originLatitude, double longitude1, double latitude1, double longitude2, double latitude2, double falseEasting, double falseNorthing, double scaleFactor );
 
 
       ObliqueMercator( const ObliqueMercator &om );
 
 
         ~ObliqueMercator( void );
 
 
       ObliqueMercator& operator=( const ObliqueMercator &om );
 
 
       /*
        * The function getParameters returns the current ellipsoid
        * parameters and Oblique Mercator projection parameters.
        *
        *    ellipsoidSemiMajorAxis  : Semi-major axis of ellipsoid, in meters  (output)
        *    ellipsoidFlattening     : Flattening of ellipsoid                  (output)
        *    originLatitude          : Latitude, in radians, at which the       (output)
        *                              point scale factor is 1.0
        *    longitude1              : Longitude, in radians, of first point lying on
        *                              central line                           (output)
        *    latitude1               : Latitude, in radians, of first point lying on
        *                              central line                           (output)
        *    longitude2              : Longitude, in radians, of second point lying on
        *                              central line                           (output)
        *    latitude2               : Latitude, in radians, of second point lying on
        *                              central line                           (output)
        *    falseEasting            : A coordinate value, in meters, assigned to the
        *                              central meridian of the projection     (output)
        *    falseNorthing           : A coordinate value, in meters, assigned to the
        *                              origin latitude of the projection      (output)
        *    scaleFactor             : Multiplier which reduces distances in the
        *                              projection to the actual distance on the
        *                              ellipsoid                              (output)
        */
 
       ObliqueMercatorParameters* getParameters() const;
 
 
       /*
        * The function convertFromGeodetic converts geodetic (latitude and
        * longitude) coordinates to Oblique Mercator projection (easting and
        * northing) coordinates, according to the current ellipsoid and Oblique Mercator 
        * projection parameters.  If any errors occur, an exception is thrown with a description 
        * of the error.
        *
        *    longitude         : Longitude (lambda), in radians       (input)
        *    latitude          : Latitude (phi), in radians           (input)
        *    easting           : Easting (X), in meters               (output)
        *    northing          : Northing (Y), in meters              (output)
        */
 
       MSP::CCS::MapProjectionCoordinates* convertFromGeodetic( MSP::CCS::GeodeticCoordinates* geodeticCoordinates );
 
 
       /*
        * The function convertToGeodetic converts Oblique Mercator projection
        * (easting and northing) coordinates to geodetic (latitude and longitude)
        * coordinates, according to the current ellipsoid and Oblique Mercator projection
        * coordinates.  If any errors occur, an exception is thrown with a description 
        * of the error.
        *
        *    easting           : Easting (X), in meters                  (input)
        *    northing          : Northing (Y), in meters                 (input)
        *    longitude         : Longitude (lambda), in radians          (output)
        *    latitude          : Latitude (phi), in radians              (output)
        */
 
       MSP::CCS::GeodeticCoordinates* convertToGeodetic( MSP::CCS::MapProjectionCoordinates* mapProjectionCoordinates );
 
     private:
     
       /* Ellipsoid Parameters, default to WGS 84 */
       double es;
       double es_OVER_2;
       double OMerc_A;
       double OMerc_B;
       double OMerc_E;
       double OMerc_gamma;
       double OMerc_azimuth;                     /* Azimuth of central line as it crosses origin lat */
       double OMerc_Origin_Long;                 /* Longitude at center of projection */
       double cos_gamma;
       double sin_gamma;
       double sin_azimuth;  
       double cos_azimuth;
       double A_over_B;
       double B_over_A;
       double OMerc_u;                           /* Coordinates for center point (uc , vc), vc = 0 */
                                                 /* at center lat and lon */
       /* Oblique Mercator projection Parameters */
       double OMerc_Origin_Lat;                  /* Latitude of projection center, in radians */
       double OMerc_Lat_1;                       /* Latitude of first point lying on central line */
       double OMerc_Lon_1;                       /* Longitude of first point lying on central line */
       double OMerc_Lat_2;                       /* Latitude of second point lying on central line */
       double OMerc_Lon_2;                       /* Longitude of second point lying on central line */
       double OMerc_Scale_Factor;                /* Scale factor at projection center */
       double OMerc_False_Northing;              /* False northing, in meters, at projection center */
       double OMerc_False_Easting;               /* False easting, in meters, at projection center */
 
       double OMerc_Delta_Northing;
       double OMerc_Delta_Easting;
 
 
       double omercT( double lat, double e_sinlat, double e_over_2 );
 
     };
   }
 }



 /***************************************************************************/
 /* RSC IDENTIFIER: OBLIQUE MERCATOR
  *
  * ABSTRACT
  *
  *    This component provides conversions between Geodetic coordinates
  *    (latitude and longitude in radians) and Oblique Mercator
  *    projection coordinates (easting and northing in meters).
  *
  * ERROR HANDLING
  *
  *    This component checks parameters for valid values.  If an invalid value
  *    is found the error code is combined with the current error code using
  *    the bitwise or.  This combining allows multiple error codes to be
  *    returned. The possible error codes are:
  *
  *       OMERC_NO_ERROR           : No errors occurred in function
  *       OMERC_LAT_ERROR          : Latitude outside of valid range
  *                                     (-90 to 90 degrees)
  *       OMERC_LON_ERROR          : Longitude outside of valid range
  *                                     (-180 to 360 degrees)
  *       OMERC_ORIGIN_LAT_ERROR   : Origin latitude outside of valid range
  *                                     (-89 to 89 degrees)
  *       OMERC_LAT1_ERROR         : First latitude outside of valid range
  *                                     (-89 to 89 degrees, excluding 0)
  *       OMERC_LAT2_ERROR         : First latitude outside of valid range
  *                                     (-89 to 89 degrees)
  *       OMERC_LON1_ERROR         : First longitude outside of valid range
  *                                     (-180 to 360 degrees)
  *       OMERC_LON2_ERROR         : Second longitude outside of valid range
  *                                     (-180 to 360 degrees)
  *       OMERC_LAT1_LAT2_ERROR    : First and second latitudes can not be equal
  *       OMERC_DIFF_HEMISPHERE_ERROR: First and second latitudes can not be
  *                                     in different hemispheres
  *       OMERC_EASTING_ERROR      : Easting outside of valid range
  *                                     (depends on ellipsoid and projection
  *                                     parameters)
  *       OMERC_NORTHING_ERROR     : Northing outside of valid range
  *                                     (depends on ellipsoid and projection
  *                                     parameters)
  *       OMERC_A_ERROR            : Semi-major axis less than or equal to zero
  *       OMERC_INV_F_ERROR        : Inverse flattening outside of valid range
  *                                     (250 to 350)
  *       OMERC_SCALE_FACTOR_ERROR : Scale factor outside of valid
  *                                     range (0.3 to 3.0)
  *       OMERC_LON_WARNING        : Distortion will result if longitude is 90 degrees or more
  *                                     from the Central Meridian
  *
  * REUSE NOTES
  *
  *    OBLIQUE MERCATOR is intended for reuse by any application that 
  *    performs an Oblique Mercator projection or its inverse.
  *
  * REFERENCES
  *
  *    Further information on OBLIQUE MERCATOR can be found in the Reuse Manual.
  *
  *    OBLIQUE MERCATOR originated from:     U.S. Army Topographic Engineering Center
  *                                          Geospatial Information Division
  *                                          7701 Telegraph Road
  *                                          Alexandria, VA  22310-3864
  *
  * LICENSES
  *
  *    None apply to this component.
  *
  * RESTRICTIONS
  *
  *    OBLIQUE MERCATOR has no restrictions.
  *
  * ENVIRONMENT
  *
  *    OBLIQUE MERCATOR was tested and certified in the following environments:
  *
  *    1. Solaris 2.5 with GCC, version 2.8.1
  *    2. MSDOS with MS Visual C++, version 6
  *
  * MODIFICATIONS
  *
  *    Date              Description
  *    ----              -----------
  *    06-07-00          Original Code
  *    03-02-07          Original C++ Code
  *    05-11-11          BAEts28017 - Fix Oblique Mercator near poles
  *
  */
 
 
 /***************************************************************************/
 /*
  *                               INCLUDES
  */
 
 #include <math.h>
 #include "ObliqueMercator.h"
 #include "ObliqueMercatorParameters.h"
 #include "MapProjectionCoordinates.h"
 #include "GeodeticCoordinates.h"
 #include "CoordinateConversionException.h"
 #include "ErrorMessages.h"
 #include "WarningMessages.h"
 
 /*
  *    math.h     - Standard C math library
  *    ObliqueMercator.h   - Is for prototype error checking
  *    MapProjectionCoordinates.h   - defines map projection coordinates
  *    GeodeticCoordinates.h   - defines geodetic coordinates
  *    CoordinateConversionException.h - Exception handler
  *    ErrorMessages.h  - Contains exception messages
  *    WarningMessages.h  - Contains warning messages
  */
 
 
 using namespace MSP::CCS;
 
 
 /***************************************************************************/
 /*                               DEFINES 
  *
  */
 
 const double PI = 3.14159265358979323e0;  /* PI                            */
 const double PI_OVER_2 = ( PI / 2.0);                 
 const double PI_OVER_4 = ( PI / 4.0);                 
 const double TWO_PI = ( 2.0 * PI);                 
 const double MIN_SCALE_FACTOR = 0.3;
 const double MAX_SCALE_FACTOR = 3.0;
 
 
 /************************************************************************/
 /*                              FUNCTIONS     
  *
  */
 
 ObliqueMercator::ObliqueMercator( double ellipsoidSemiMajorAxis, double ellipsoidFlattening, double originLatitude, double longitude1, double latitude1, double longitude2, double latitude2, double falseEasting, double falseNorthing, double scaleFactor ) :
   CoordinateSystem(),
   es( 0.08181919084262188000 ),
   es_OVER_2( .040909595421311 ),
   OMerc_A( 6383471.9177251 ),
   OMerc_B( 1.0008420825413 ),
   OMerc_E( 1.0028158089754 ),
   OMerc_gamma( .41705894983580 ),
   OMerc_azimuth( .60940407333533 ),
   OMerc_Origin_Long( -.46732023406900 ),
   cos_gamma( .91428423352628 ),
   sin_gamma( .40507325303611 ),
   sin_azimuth( .57237890829911 ),  
   cos_azimuth( .81998925927985 ),
   A_over_B( 6378101.0302010 ),
   B_over_A( 1.5678647849335e-7 ),
   OMerc_u( 5632885.2272051 ),
   OMerc_Origin_Lat( ((45.0 * PI) / 180.0) ),
   OMerc_Lon_1( ((-5.0 * PI) / 180.0) ),
   OMerc_Lat_1( ((40.0 * PI) / 180.0) ),
   OMerc_Lon_2( ((5.0 * PI) / 180.0) ),
   OMerc_Lat_2( ((50.0 * PI) / 180.0) ),
   OMerc_False_Easting( 0.0 ),
   OMerc_False_Northing( 0.0 ),
   OMerc_Scale_Factor( 1.0 ),
   OMerc_Delta_Northing( 40000000.0 ),
   OMerc_Delta_Easting(  40000000.0 )
 {
 /*
  * The constructor receives the ellipsoid parameters and
  * projection parameters as inputs, and sets the corresponding state
  * variables.  If any errors occur, an exception is thrown with a description 
  * of the error.
  *
  *    ellipsoidSemiMajorAxis   : Semi-major axis of ellipsoid, in meters  (input)
  *    ellipsoidFlattening      : Flattening of ellipsoid                  (input)
  *    originLatitude           : Latitude, in radians, at which the       (input)
  *                               point scale factor is 1.0
  *    longitude1               : Longitude, in radians, of first point lying on
  *                               central line                             (input)
  *    latitude1                : Latitude, in radians, of first point lying on
  *                               central line                             (input)
  *    longitude2               : Longitude, in radians, of second point lying on
  *                               central line                             (input)
  *    latitude2                : Latitude, in radians, of second point lying on
  *                               central line                             (input)
  *    falseEasting             : A coordinate value, in meters, assigned to the
  *                               central meridian of the projection       (input)
  *    falseNorthing            : A coordinate value, in meters, assigned to the
  *                               origin latitude of the projection        (input)
  *    scaleFactor              : Multiplier which reduces distances in the
  *                               projection to the actual distance on the
  *                               ellipsoid                                (input)
  */
 
   double inv_f = 1 / ellipsoidFlattening;
   double es2, one_MINUS_es2;
   double cos_olat, cos_olat2;
   double sin_olat, sin_olat2, es2_sin_olat2;
   double t0, t1, t2;
   double D, D2, D2_MINUS_1, sqrt_D2_MINUS_1;
   double H, L, LH;
   double E2;
   double F, G, J, P;
   double dlon;
 
   if (ellipsoidSemiMajorAxis <= 0.0)
   { /* Semi-major axis must be greater than zero */
     throw CoordinateConversionException( ErrorMessages::semiMajorAxis );
   }
   if ((inv_f < 250) || (inv_f > 350))
   { /* Inverse flattening must be between 250 and 350 */
     throw CoordinateConversionException( ErrorMessages::ellipsoidFlattening );
   }
   if ((originLatitude <= -PI_OVER_2) || (originLatitude >= PI_OVER_2))
   { /* origin latitude out of range -  can not be at a pole */
     throw CoordinateConversionException( ErrorMessages::originLatitude );
   }
   if ((latitude1 <= -PI_OVER_2) || (latitude1 >= PI_OVER_2))
   { /* first latitude out of range -  can not be at a pole */
     throw CoordinateConversionException( ErrorMessages::latitude1 );
   }
   if ((latitude2 <= -PI_OVER_2) || (latitude2 >= PI_OVER_2))
   { /* second latitude out of range -  can not be at a pole */
     throw CoordinateConversionException( ErrorMessages::latitude2 );
   }
   if (latitude1 == 0.0)
   { /* first latitude can not be at the equator */
     throw CoordinateConversionException( ErrorMessages::latitude1 );
   }
   if (latitude1 == latitude2)
   { /* first and second latitudes can not be equal */
     throw CoordinateConversionException( ErrorMessages::latitude2 );
   }
   if (((latitude1 < 0.0) && (latitude2 > 0.0)) ||
       ((latitude1 > 0.0) && (latitude2 < 0.0)))
   { /*first and second points can not be in different hemispheres */
     throw CoordinateConversionException( ErrorMessages::omercHemisphere );
   }
   if ((longitude1 < -PI) || (longitude1 > TWO_PI))
   { /* first longitude out of range */
     throw CoordinateConversionException( ErrorMessages::longitude1 );
   }
   if ((longitude2 < -PI) || (longitude2 > TWO_PI))
   { /* first longitude out of range */
     throw CoordinateConversionException( ErrorMessages::longitude2 );
   }
   if ((scaleFactor < MIN_SCALE_FACTOR) || (scaleFactor > MAX_SCALE_FACTOR))
   { /* scale factor out of range */
     throw CoordinateConversionException( ErrorMessages::scaleFactor );
   }
 
   semiMajorAxis = ellipsoidSemiMajorAxis;
   flattening = ellipsoidFlattening;
 
   OMerc_Origin_Lat = originLatitude;
   OMerc_Lon_1 = longitude1;
   OMerc_Lat_1 = latitude1;
   OMerc_Lon_2 = longitude2;
   OMerc_Lat_2 = latitude2;
   OMerc_False_Northing = falseNorthing;
   OMerc_False_Easting = falseEasting;
   OMerc_Scale_Factor = scaleFactor;
 
   es2 = 2 * flattening - flattening * flattening;
   es = sqrt(es2);
   one_MINUS_es2 = 1 - es2;
   es_OVER_2 = es / 2.0;
 
   cos_olat = cos(OMerc_Origin_Lat);
   cos_olat2 = cos_olat * cos_olat;
   sin_olat = sin(OMerc_Origin_Lat);
   sin_olat2 = sin_olat * sin_olat;
   es2_sin_olat2 = es2 * sin_olat2;
 
   OMerc_B = sqrt(1 + (es2 * cos_olat2 * cos_olat2) / one_MINUS_es2);
   OMerc_A = (semiMajorAxis * OMerc_B * OMerc_Scale_Factor * sqrt(one_MINUS_es2)) / (1.0 - es2_sin_olat2);  
   A_over_B = OMerc_A / OMerc_B;
   B_over_A = OMerc_B / OMerc_A;
 
   t0 = omercT(OMerc_Origin_Lat, es * sin_olat, es_OVER_2);
   t1 = omercT(OMerc_Lat_1, es * sin(OMerc_Lat_1), es_OVER_2);  
   t2 = omercT(OMerc_Lat_2, es * sin(OMerc_Lat_2), es_OVER_2);  
 
   D = (OMerc_B * sqrt(one_MINUS_es2)) / (cos_olat * sqrt(1.0 - es2_sin_olat2)); 
   D2 = D * D;
   if (D2 < 1.0)
     D2 = 1.0;
   D2_MINUS_1 = D2 - 1.0;
   sqrt_D2_MINUS_1 = sqrt(D2_MINUS_1);
   if (D2_MINUS_1 > 1.0e-10)
   {
     if (OMerc_Origin_Lat >= 0.0)
       OMerc_E = (D + sqrt_D2_MINUS_1) * pow(t0, OMerc_B);
     else
       OMerc_E = (D - sqrt_D2_MINUS_1) * pow(t0, OMerc_B);
   }
   else
     OMerc_E = D * pow(t0, OMerc_B);
   H = pow(t1, OMerc_B);
   L = pow(t2, OMerc_B);
   F = OMerc_E / H;
   G = (F - 1.0 / F) / 2.0;
   E2 = OMerc_E * OMerc_E;
   LH = L * H;
   J = (E2 - LH) / (E2 + LH);
   P = (L - H) / (L + H);
 
   dlon = OMerc_Lon_1 - OMerc_Lon_2;
   if (dlon < -PI )
     OMerc_Lon_2 -= TWO_PI;
   if (dlon > PI)
     OMerc_Lon_2 += TWO_PI;
   dlon = OMerc_Lon_1 - OMerc_Lon_2;
   OMerc_Origin_Long = (OMerc_Lon_1 + OMerc_Lon_2) / 2.0 - (atan(J * tan(OMerc_B * dlon / 2.0) / P)) / OMerc_B;
 
   dlon = OMerc_Lon_1 - OMerc_Origin_Long;
   if (dlon < -PI )
     OMerc_Origin_Long -= TWO_PI;
   if (dlon > PI)
     OMerc_Origin_Long += TWO_PI;
  
   dlon = OMerc_Lon_1 - OMerc_Origin_Long;
   OMerc_gamma = atan(sin(OMerc_B * dlon) / G);
   cos_gamma = cos(OMerc_gamma);
   sin_gamma = sin(OMerc_gamma);
 
   OMerc_azimuth = asin(D * sin_gamma);
   cos_azimuth = cos(OMerc_azimuth);
   sin_azimuth = sin(OMerc_azimuth);
 
   if (OMerc_Origin_Lat >= 0)
     OMerc_u =  A_over_B * atan(sqrt_D2_MINUS_1/cos_azimuth);
   else
     OMerc_u = -A_over_B * atan(sqrt_D2_MINUS_1/cos_azimuth);
 }
 
 
 ObliqueMercator::ObliqueMercator( const ObliqueMercator &om )
 {
   semiMajorAxis = om.semiMajorAxis;
   flattening = om.flattening;
   es = om.es;     
   es_OVER_2 = om.es_OVER_2;     
   OMerc_A = om.OMerc_A;     
   OMerc_B = om.OMerc_B;     
   OMerc_E = om.OMerc_E;     
   OMerc_gamma = om.OMerc_gamma; 
   OMerc_azimuth = om.OMerc_azimuth; 
   OMerc_Origin_Long = om.OMerc_Origin_Long; 
   cos_gamma = om.cos_gamma; 
   sin_gamma = om.sin_gamma; 
   sin_azimuth = om.sin_azimuth; 
   cos_azimuth = om.cos_azimuth; 
   A_over_B = om.A_over_B; 
   B_over_A = om.B_over_A; 
   OMerc_u = om.OMerc_u; 
   OMerc_Origin_Lat = om.OMerc_Origin_Lat; 
   OMerc_Lon_1 = om.OMerc_Lon_1; 
   OMerc_Lat_1 = om.OMerc_Lat_1; 
   OMerc_Lon_2 = om.OMerc_Lon_2; 
   OMerc_Lat_2 = om.OMerc_Lat_2; 
   OMerc_False_Easting = om.OMerc_False_Easting; 
   OMerc_False_Northing = om.OMerc_False_Northing; 
   OMerc_Scale_Factor = om.OMerc_Scale_Factor; 
   OMerc_Delta_Northing = om.OMerc_Delta_Northing; 
   OMerc_Delta_Easting = om.OMerc_Delta_Easting; 
 }
 
 
 ObliqueMercator::~ObliqueMercator()
 {
 }
 
 
 ObliqueMercator& ObliqueMercator::operator=( const ObliqueMercator &om )
 {
   if( this != &om )
   {
     semiMajorAxis = om.semiMajorAxis;
     flattening = om.flattening;
     es = om.es;     
     es_OVER_2 = om.es_OVER_2;     
     OMerc_A = om.OMerc_A;     
     OMerc_B = om.OMerc_B;     
     OMerc_E = om.OMerc_E;     
     OMerc_gamma = om.OMerc_gamma; 
     OMerc_azimuth = om.OMerc_azimuth; 
     OMerc_Origin_Long = om.OMerc_Origin_Long; 
     cos_gamma = om.cos_gamma; 
     sin_gamma = om.sin_gamma; 
     sin_azimuth = om.sin_azimuth; 
     cos_azimuth = om.cos_azimuth; 
     A_over_B = om.A_over_B; 
     B_over_A = om.B_over_A; 
     OMerc_u = om.OMerc_u; 
     OMerc_Origin_Lat = om.OMerc_Origin_Lat; 
     OMerc_Lon_1 = om.OMerc_Lon_1; 
     OMerc_Lat_1 = om.OMerc_Lat_1; 
     OMerc_Lon_2 = om.OMerc_Lon_2; 
     OMerc_Lat_2 = om.OMerc_Lat_2; 
     OMerc_False_Easting = om.OMerc_False_Easting; 
     OMerc_False_Northing = om.OMerc_False_Northing; 
     OMerc_Scale_Factor = om.OMerc_Scale_Factor; 
     OMerc_Delta_Northing = om.OMerc_Delta_Northing; 
     OMerc_Delta_Easting = om.OMerc_Delta_Easting; 
   }
 
   return *this;
 }
 
 
 ObliqueMercatorParameters* ObliqueMercator::getParameters() const
 {
 /*
  * The function getParameters returns the current ellipsoid
  * parameters and Oblique Mercator projection parameters.
  *
  *    ellipsoidSemiMajorAxis  : Semi-major axis of ellipsoid, in meters  (output)
  *    ellipsoidFlattening     : Flattening of ellipsoid                  (output)
  *    originLatitude          : Latitude, in radians, at which the       (output)
  *                              point scale factor is 1.0
  *    longitude1              : Longitude, in radians, of first point lying on
  *                              central line                           (output)
  *    latitude1               : Latitude, in radians, of first point lying on
  *                              central line                           (output)
  *    longitude2              : Longitude, in radians, of second point lying on
  *                              central line                           (output)
  *    latitude2               : Latitude, in radians, of second point lying on
  *                              central line                           (output)
  *    falseEasting            : A coordinate value, in meters, assigned to the
  *                              central meridian of the projection     (output)
  *    falseNorthing           : A coordinate value, in meters, assigned to the
  *                              origin latitude of the projection      (output)
  *    scaleFactor             : Multiplier which reduces distances in the
  *                              projection to the actual distance on the
  *                              ellipsoid                              (output)
  */
 
   return new ObliqueMercatorParameters( CoordinateType::obliqueMercator, OMerc_Origin_Lat, OMerc_Lon_1, OMerc_Lat_1, OMerc_Lon_2, OMerc_Lat_2, OMerc_False_Easting, OMerc_False_Northing, OMerc_Scale_Factor );
 }
 
 
 MSP::CCS::MapProjectionCoordinates* ObliqueMercator::convertFromGeodetic( MSP::CCS::GeodeticCoordinates* geodeticCoordinates )
 {
 /*
  * The function convertFromGeodetic converts geodetic (latitude and
  * longitude) coordinates to Oblique Mercator projection (easting and
  * northing) coordinates, according to the current ellipsoid and Oblique Mercator 
  * projection parameters.  If any errors occur, an exception is thrown with a description 
  * of the error.
  *
  *    longitude         : Longitude (lambda), in radians       (input)
  *    latitude          : Latitude (phi), in radians           (input)
  *    easting           : Easting (X), in meters               (output)
  *    northing          : Northing (Y), in meters              (output)
  */
 
   double dlam, B_dlam, cos_B_dlam;
   double t, S, T, V, U;
   double Q, Q_inv;
   /* Coordinate axes defined with respect to the azimuth of the center line */
   /* Natural origin*/
   double v = 0;
   double u = 0;
 
   double longitude = geodeticCoordinates->longitude();
   double latitude = geodeticCoordinates->latitude();
 
   if ((latitude < -PI_OVER_2) || (latitude > PI_OVER_2))
   { /* Latitude out of range */
     throw CoordinateConversionException( ErrorMessages::latitude );
   }
   if ((longitude < -PI) || (longitude > TWO_PI))
   { /* Longitude out of range */
     throw CoordinateConversionException( ErrorMessages::longitude );
   }
 
   dlam = longitude - OMerc_Origin_Long;
 
   char warning[256];
   warning[0] = '\0';
   if (fabs(dlam) >= PI_OVER_2)
   { /* Distortion will result if Longitude is 90 degrees or more from the Central Meridian */
     strcat( warning, MSP::CCS::WarningMessages::longitude );
   }
 
   if (dlam > PI)
   {
     dlam -= TWO_PI;
   }
   if (dlam < -PI)
   {
     dlam += TWO_PI;
   }
 
   if (fabs(fabs(latitude) - PI_OVER_2) > 1.0e-10)
   {
     t = omercT(latitude, es * sin(latitude), es_OVER_2);  
     Q = OMerc_E / pow(t, OMerc_B);
     Q_inv = 1.0 / Q;
     S = (Q - Q_inv) / 2.0;
     T = (Q + Q_inv) / 2.0;
     B_dlam = OMerc_B * dlam;
     V = sin(B_dlam);
     U = ((-1.0 * V * cos_gamma) + (S * sin_gamma)) / T;
     if (fabs(fabs(U) - 1.0) < 1.0e-10)
     { /* Point projects into infinity */
       throw CoordinateConversionException( ErrorMessages::longitude );
     }
     else
     {
       v = A_over_B * log((1.0 - U) / (1.0 + U)) / 2.0;
       cos_B_dlam = cos(B_dlam);
       if (fabs(cos_B_dlam) < 1.0e-10)
         u = OMerc_A * B_dlam;
       // Check for longitude span > 90 degrees
       else if (fabs(B_dlam) > PI_OVER_2)
       {
         double temp = atan(((S * cos_gamma) + (V * sin_gamma)) / cos_B_dlam);
         if (temp < 0.0)
           u = A_over_B * (temp + PI);
         else
           u = A_over_B * (temp - PI);
       }
       else
         u = A_over_B * atan(((S * cos_gamma) + (V * sin_gamma)) / cos_B_dlam);
     }
   }
   else
   {
     if (latitude > 0.0)
       v = A_over_B * log(tan(PI_OVER_4 - (OMerc_gamma / 2.0)));
     else
       v = A_over_B * log(tan(PI_OVER_4 + (OMerc_gamma / 2.0)));
     u = A_over_B * latitude;
   }
 
 
   u = u - OMerc_u;
 
   double easting = OMerc_False_Easting + v * cos_azimuth + u * sin_azimuth;
   double northing = OMerc_False_Northing + u * cos_azimuth - v * sin_azimuth;
 
   return new MapProjectionCoordinates(
      CoordinateType::obliqueMercator, warning, easting, northing );
 }
 
 
 MSP::CCS::GeodeticCoordinates* ObliqueMercator::convertToGeodetic( MSP::CCS::MapProjectionCoordinates* mapProjectionCoordinates )
 {
 /*
  * The function convertToGeodetic converts Oblique Mercator projection
  * (easting and northing) coordinates to geodetic (latitude and longitude)
  * coordinates, according to the current ellipsoid and Oblique Mercator projection
  * coordinates.  If any errors occur, an exception is thrown with a description 
  * of the error.
  *
  *    easting           : Easting (X), in meters                  (input)
  *    northing          : Northing (Y), in meters                 (input)
  *    longitude         : Longitude (lambda), in radians          (output)
  *    latitude          : Latitude (phi), in radians              (output)
  */
 
   double dx, dy;
   /* Coordinate axes defined with respect to the azimuth of the center line */
   /* Natural origin*/
   double u, v;
   double Q_prime, Q_prime_inv;
   double S_prime, T_prime, V_prime, U_prime;
   double t;
   double es_sin;
   double u_B_over_A;
   double phi;
   double temp_phi = 0.0;
   int count = 60;
   double longitude, latitude;
 
   double easting  = mapProjectionCoordinates->easting();
   double northing = mapProjectionCoordinates->northing();
 
   if ((easting < (OMerc_False_Easting - OMerc_Delta_Easting)) 
       || (easting > (OMerc_False_Easting + OMerc_Delta_Easting)))
   { /* Easting out of range  */
     throw CoordinateConversionException( ErrorMessages::easting );
   }
   if ((northing < (OMerc_False_Northing - OMerc_Delta_Northing)) 
       || (northing > (OMerc_False_Northing + OMerc_Delta_Northing)))
   { /* Northing out of range */
     throw CoordinateConversionException( ErrorMessages::northing );
   }
 
   dy = northing - OMerc_False_Northing;
   dx = easting - OMerc_False_Easting;
   v = dx * cos_azimuth - dy * sin_azimuth;
   u = dy * cos_azimuth + dx * sin_azimuth;
   u = u + OMerc_u;
   Q_prime = exp(-1.0 * (v * B_over_A ));
   Q_prime_inv = 1.0 / Q_prime;
   S_prime = (Q_prime - Q_prime_inv) / 2.0;
   T_prime = (Q_prime + Q_prime_inv) / 2.0;
   u_B_over_A = u * B_over_A;
   V_prime = sin(u_B_over_A);
   U_prime = (V_prime * cos_gamma + S_prime * sin_gamma) / T_prime;
   if (fabs(fabs(U_prime) - 1.0) < 1.0e-10)
   {
     if (U_prime > 0)
       latitude = PI_OVER_2;
     else
       latitude = -PI_OVER_2;
     longitude = OMerc_Origin_Long;
   }
   else
   {
     t = pow(OMerc_E / sqrt((1.0 + U_prime) / (1.0 - U_prime)), 1.0 / OMerc_B);
     phi = PI_OVER_2 - 2.0 * atan(t);
     while (fabs(phi - temp_phi) > 1.0e-10 && count)
     {
       temp_phi = phi;
       es_sin = es * sin(phi);
       phi = PI_OVER_2 - 2.0 * atan(t * pow((1.0 - es_sin) / (1.0 + es_sin), es_OVER_2));
       count --;
     }
 
     if(!count)
       throw CoordinateConversionException( ErrorMessages::northing );
 
     latitude = phi;
     longitude = OMerc_Origin_Long - atan2((S_prime * cos_gamma - V_prime * sin_gamma), cos(u_B_over_A)) / OMerc_B;
   }
 
   if (fabs(latitude) < 2.0e-7)  /* force lat to 0 to avoid -0 degrees */
     latitude = 0.0;
   if (latitude > PI_OVER_2)  /* force distorted values to 90, -90 degrees */
     latitude = PI_OVER_2;
   else if (latitude < -PI_OVER_2)
     latitude = -PI_OVER_2;
 
   if (longitude > PI)
     longitude -= TWO_PI;
   if (longitude < -PI)
     longitude += TWO_PI;
 
   if (fabs(longitude) < 2.0e-7)  /* force lon to 0 to avoid -0 degrees */
     longitude = 0.0;
   if (longitude > PI)  /* force distorted values to 180, -180 degrees */
     longitude = PI;
   else if (longitude < -PI)
     longitude = -PI;
 
   char warning[256];
   warning[0] = '\0';
   if (fabs(longitude - OMerc_Origin_Long) >= PI_OVER_2)
   { /* Distortion results if Longitude > 90 degrees from the Central Meridian */
     strcat( warning, MSP::CCS::WarningMessages::longitude );
   }
 
   return new GeodeticCoordinates(
      CoordinateType::geodetic, warning, longitude, latitude );
 }
 
 
 double ObliqueMercator::omercT( double lat, double e_sinlat, double e_over_2 )
 {  
   return (tan(PI_OVER_4 - lat / 2.0)) / (pow((1 - e_sinlat) / (1 + e_sinlat), e_over_2));
 }
 
#endif // NOT
