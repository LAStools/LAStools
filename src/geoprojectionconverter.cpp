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

    (c) 2007-2017, martin isenburg, rapidlasso - fast tools to catch reality

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
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#if defined(_MSC_VER) && \
    (_MSC_FULL_VER >= 150000000)
#define LASCopyString _strdup
#else
#define LASCopyString strdup
#endif

static const double PI = 3.141592653589793238462643383279502884197169;
static const double TWO_PI = PI * 2;
static const double PI_OVER_2 = PI / 2;
static const double PI_OVER_4 = PI / 4;
static const double EPSILON = 0.0000000001;
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
static const int GEO_PROJECTION_HOM      = 7;
static const int GEO_PROJECTION_OS       = 8;
static const int GEO_PROJECTION_NONE     = 9;

class ReferenceEllipsoid
{
public:
  ReferenceEllipsoid(int id, char const* name, double equatorialRadius, double eccentricitySquared, double inverseFlattening)
  {
    this->id = id;
    this->name = name;
    this->equatorialRadius = equatorialRadius;
    this->eccentricitySquared = eccentricitySquared;
    this->inverseFlattening = inverseFlattening;
  }
  int id;
  char const* name;
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
  ReferenceEllipsoid( 4, "Bessel 1841 (Namibia) ", 6377483.865, 0.006674372, 299.1528128),
  ReferenceEllipsoid( 5, "Clarke 1866", 6378206.4, 0.006768658, 294.9786982),
  ReferenceEllipsoid( 6, "Clarke 1880", 6378249.145, 0.006803511, 293.465),
  ReferenceEllipsoid( 7, "Everest 1830", 6377276.345, 0.006637847, 300.8017),
  ReferenceEllipsoid( 8, "Fischer 1960 (Mercury) ", 6378166, 0.006693422, 298.3),
  ReferenceEllipsoid( 9, "Fischer 1968", 6378150, 0.006693422, 298.3),
  ReferenceEllipsoid( 10, "GRS 1967", 6378160, 0.006694605, 298.247167427),
  ReferenceEllipsoid( 11, "GRS 1980", 6378137, 0.00669438002290, 298.257222101),
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
  StatePlaneLCC(short geokey, char const* zone, double falseEastingMeter, double falseNorthingMeter, double latOriginDegree, double longMeridianDegree, double firstStdParallelDegree, double secondStdParallelDegree)
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
  char const* zone;
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
  StatePlaneTM(short geokey, char const* zone, double falseEastingMeter, double falseNorthingMeter, double latOriginDegree, double longMeridianDegree, double scaleFactor)
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
  char const* zone;
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

static const short EPSG_CH1903_LV03 = 21781;
static const short EPSG_EOV_HD72 = 23700;

bool GeoProjectionConverter::set_projection_from_geo_keys(int num_geo_keys, const GeoProjectionGeoKeys* geo_keys, char* geo_ascii_params, double* geo_double_params, char* description)
{
  bool user_defined_ellipsoid = false;
  int user_defined_projection = 0;
  int offsetProjStdParallel1GeoKey = -1;
  int offsetProjStdParallel2GeoKey = -1;
  int offsetProjNatOriginLatGeoKey = -1;
  int offsetProjNatOriginLongGeoKey = -1;
  int offsetProjFalseEastingGeoKey = -1;
  int offsetProjFalseNorthingGeoKey = -1;
  int offsetProjCenterLatGeoKey = -1;
  int offsetProjCenterLongGeoKey = -1;
  int offsetProjFalseOriginLongGeoKey = -1;
  int offsetProjFalseOriginLatGeoKey = -1;
  int offsetProjFalseOriginEastingGeoKey = -1;
  int offsetProjFalseOriginNorthingGeoKey = -1;
  int offsetProjScaleAtNatOriginGeoKey = -1;
  int offsetProjScaleAtCenterGeoKey = -1;
  int offsetProjAzimuthAngleGeoKey = -1;
  int offsetProjRectifiedGridAngleGeoKey = -1;
  bool has_projection = false;
  int ellipsoid = -1;
  int datum_code = -1;
  int gcs_code = -1;

  this->num_geo_keys = num_geo_keys;
  if (this->geo_keys) delete [] this->geo_keys;
  if (num_geo_keys)
  {
    this->geo_keys = new GeoProjectionGeoKeys[num_geo_keys];
    memcpy(this->geo_keys, geo_keys, sizeof(GeoProjectionGeoKeys)*num_geo_keys);
  }
  else
  {
    this->geo_keys = 0;
  }
  this->geo_ascii_params = geo_ascii_params;
  this->geo_double_params = geo_double_params;

  for (int i = 0; i < num_geo_keys; i++)
  {
    switch (geo_keys[i].key_id)
    {
    case 1024: // GTModelTypeGeoKey
      has_projection = set_GTModelTypeGeoKey(geo_keys[i].value_offset, description);
      break;
    case 3072: // ProjectedCSTypeGeoKey
      has_projection = set_ProjectedCSTypeGeoKey(geo_keys[i].value_offset, description);
      break;
    case 3076: // ProjLinearUnitsGeoKey
      set_ProjLinearUnitsGeoKey(geo_keys[i].value_offset);
      break;
    case 4099: // VerticalUnitsGeoKey
      set_VerticalUnitsGeoKey(geo_keys[i].value_offset);
      break;
    case 4096: // VerticalCSTypeGeoKey
      set_VerticalCSTypeGeoKey(geo_keys[i].value_offset);
      break;
    case 2048: // GeographicTypeGeoKey
      switch (geo_keys[i].value_offset)
      {
      case 32767: // user-defined GCS
        user_defined_ellipsoid = true;
        break;
      case 4326: // GCS_WGS_84
        gcs_code = GEO_GCS_WGS84;
        break;
      case 4269: // GCS_NAD83
        gcs_code = GEO_GCS_NAD83;
        break;
      case 4322: // GCS_WGS_72
        gcs_code = GEO_GCS_WGS72;
        break;
      case 4267: // GCS_NAD27
        gcs_code = GEO_GCS_NAD27;
        break;
      case 4283: // GCS_GDA94
        gcs_code = GEO_GCS_GDA94;
        break;
      case 7844: // GCS_GDA2020
        gcs_code = GEO_GCS_GDA2020;
        break;
      case 4140: // Datum_NAD83_CSRS
      case 4617: // Datum_NAD83_CSRS
        datum_code = GEO_GCS_NAD83_CSRS;
        break;
      case 4759: // NAD83_2007
      case 4893: // NAD83_2007_3D
        gcs_code = GEO_GCS_NAD83_NSRS2007;
        break;
      case 4957: // NAD83_HARN_3D
      case 4152: // NAD83_HARN
        gcs_code = GEO_GCS_NAD83_HARN;
        break;
      case 6783: // CORS96
      case 6782: // CORS96 3D
        gcs_code = GEO_GCS_NAD83_CORS96;
        break;
      case 6318: // NAD83_2011
      case 6319: // NAD83_2011_3D
        gcs_code = GEO_GCS_NAD83_2011;
        break;
      case 6322: // NAD83_PA11
        gcs_code = GEO_GCS_NAD83_PA11;
        break;
      case 4030: // GCSE_WGS84 (unknown datum based on WGS 84 ellipsoid)
        ellipsoid = GEO_ELLIPSOID_WGS84;
        break;
      case 4019: // GCSE_GRS1980 (unknown datum based on GRS1980 ellipsoid)
        ellipsoid = GEO_ELLIPSOID_GRS1980;
        break;
      case 4619: // GCS_SWEREF99
        gcs_code = GEO_GCS_SWEREF99;
        break;
      case 4167: // GCS_NZGD2000
        gcs_code = GEO_GCS_NZGD2000;
        break;
      case 4001: // GCSE_Airy1830
        ellipsoid = GEO_ELLIPSOID_AIRY;
        break;
      case 4002: // GCSE_AiryModified1849
        ellipsoid = 16;
        break;
      case 4003: // GCSE_AustralianNationalSpheroid
        ellipsoid = 2;
        break;
      case 4004: // GCSE_Bessel1841
      case 4005: // GCSE_Bessel1841Modified
        ellipsoid = GEO_ELLIPSOID_BESSEL_1841;
        break;
      case 4006: // GCSE_BesselNamibia
        ellipsoid = GEO_ELLIPSOID_BESSEL_NAMIBIA;
        break;
      case 4008: // GCSE_Clarke1866 (unknown datum based on Clarke1866 ellipsoid)
      case 4009: // GCSE_Clarke1866Michigan (unknown datum based on Clarke1866Michigan ellipsoid)
        ellipsoid = GEO_ELLIPSOID_CLARKE1866;
        break;
      case 4010: // GCSE_Clarke1880_Benoit
      case 4011: // GCSE_Clarke1880_IGN
      case 4012: // GCSE_Clarke1880_RGS
      case 4013: // GCSE_Clarke1880_Arc
      case 4014: // GCSE_Clarke1880_SGA1922
      case 4034: // GCSE_Clarke1880
        ellipsoid = GEO_ELLIPSOID_CLARKE1880;
        break;
      case 4015: // GCSE_Everest1830_1937Adjustment
      case 4016: // GCSE_Everest1830_1967Definition
      case 4017: // GCSE_Everest1830_1975Definition
        ellipsoid = 7;
        break;
      case 4018: // GCSE_Everest1830Modified
        ellipsoid = 17;
        break;
      case 4020: // GCSE_Helmert1906
        ellipsoid = 12;
        break;
      case 4022: // GCSE_International1924
      case 4023: // GCSE_International1967
        ellipsoid = GEO_ELLIPSOID_INTERNATIONAL;
        break;
      case 4024: // GCSE_Krassowsky1940
        ellipsoid = GEO_ELLIPSOID_KRASSOWSKY;
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
      case 6326: // Datum_WGS84
        datum_code = GEO_GCS_WGS84;
        break;
      case 6269: // Datum_North_American_Datum_1983
        datum_code = GEO_GCS_NAD83;
        break;
      case 6322: // Datum_WGS72
        datum_code = GEO_GCS_WGS72;
        break;
      case 6267: // Datum_North_American_Datum_1927
        datum_code = GEO_GCS_NAD27;
        break;
      case 6283: // Datum_Geocentric_Datum_of_Australia_1994
        datum_code = GEO_GCS_GDA94;
        break;
      case 9844: // Datum_Geocentric_Datum_of_Australia_2020
        datum_code = GEO_GCS_GDA94;
        break;
      case 6140: // Datum_NAD83_CSRS
        datum_code = GEO_GCS_NAD83_CSRS;
        break;
      case 6030: // DatumE_WGS84
        ellipsoid = GEO_ELLIPSOID_WGS84;
        break;
      case 6019: // DatumE_GRS1980
        ellipsoid = GEO_ELLIPSOID_GRS1980;
        break;
      case 6167: // Datum_SWEREF99
        datum_code = GEO_GCS_SWEREF99;
        break;
      case 6619: // Datum_NZGD2000
        datum_code = GEO_GCS_NZGD2000;
        break;
      case 6202: // Datum_Australian_Geodetic_Datum_1966
      case 6203: // Datum_Australian_Geodetic_Datum_1984
        ellipsoid = 2;
        break;
      case 6001: // DatumE_Airy1830
        ellipsoid = GEO_ELLIPSOID_AIRY;
        break;
      case 6002: // DatumE_AiryModified1849
        ellipsoid = 16;
        break;
      case 6003: // DatumE_AustralianNationalSpheroid
        ellipsoid = 2;
        break;
      case 6004: // DatumE_Bessel1841
      case 6005: // DatumE_BesselModified
        ellipsoid = GEO_ELLIPSOID_BESSEL_1841;
        break;
      case 6006: // DatumE_BesselNamibia
        ellipsoid = GEO_ELLIPSOID_BESSEL_NAMIBIA;
        break;
      case 6008: // DatumE_Clarke1866
      case 6009: // DatumE_Clarke1866Michigan
        ellipsoid = GEO_ELLIPSOID_CLARKE1866;
        break;
      case 6010: // DatumE_Clarke1880_Benoit
      case 6011: // DatumE_Clarke1880_IGN
      case 6012: // DatumE_Clarke1880_RGS
      case 6013: // DatumE_Clarke1880_Arc
      case 6014: // DatumE_Clarke1880_SGA1922
      case 6034: // DatumE_Clarke1880
        ellipsoid = GEO_ELLIPSOID_CLARKE1880;
        break;
      case 6015: // DatumE_Everest1830_1937Adjustment
      case 6016: // DatumE_Everest1830_1967Definition
      case 6017: // DatumE_Everest1830_1975Definition
        ellipsoid = 7;
        break;
      case 6018: // DatumE_Everest1830Modified
        ellipsoid = 17;
        break;
      case 6020: // DatumE_Helmert1906
        ellipsoid = 12;
        break;
      case 6022: // DatumE_International1924
      case 6023: // DatumE_International1967
        ellipsoid = GEO_ELLIPSOID_INTERNATIONAL;
        break;
      case 6024: // DatumE_Krassowsky1940
        ellipsoid = GEO_ELLIPSOID_KRASSOWSKY;
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
      ellipsoid = set_GeogEllipsoidGeoKey(geo_keys[i].value_offset);
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
      case 16: // CT_ObliqueStereographic
        user_defined_projection = 16;
        break;
      case 2: // CT_TransvMercator_Modified_Alaska
        fprintf(stderr, "ProjCoordTransGeoKey: CT_TransvMercator_Modified_Alaska not implemented\n");
        break;
      case 3: // CT_ObliqueMercator
        user_defined_projection = 3;
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
    case 3078: // ProjStdParallel1GeoKey
      offsetProjStdParallel1GeoKey = geo_keys[i].value_offset;
      break;
    case 3079: // ProjStdParallel2GeoKey
      offsetProjStdParallel2GeoKey = geo_keys[i].value_offset;
      break;
    case 3080 : // ProjNatOriginLongGeoKey
      offsetProjNatOriginLongGeoKey = geo_keys[i].value_offset;
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
    case 3084: // ProjFalseOriginLongGeoKey
      offsetProjFalseOriginLongGeoKey = geo_keys[i].value_offset;
      break;
    case 3085: // ProjFalseOriginLatGeoKey
      offsetProjFalseOriginLatGeoKey = geo_keys[i].value_offset;
      break;
    case 3086: // ProjFalseOriginEastingGeoKey
      offsetProjFalseOriginEastingGeoKey = geo_keys[i].value_offset;
      break;
    case 3087: // ProjFalseOriginNorthingGeoKey
      offsetProjFalseOriginNorthingGeoKey = geo_keys[i].value_offset;
      break;
    case 3088: // ProjCenterLongGeoKey
      offsetProjCenterLongGeoKey = geo_keys[i].value_offset;
      break;
    case 3089: // ProjCenterLatGeoKey
      offsetProjCenterLatGeoKey = geo_keys[i].value_offset;
      break;
    case 3092: // ProjScaleAtNatOriginGeoKey
      offsetProjScaleAtNatOriginGeoKey = geo_keys[i].value_offset;
      break;
    case 3093: // ProjScaleAtCenterGeoKey
      offsetProjScaleAtCenterGeoKey = geo_keys[i].value_offset;
      break;
    case 3094: // ProjAzimuthAngleGeoKey
      offsetProjAzimuthAngleGeoKey = geo_keys[i].value_offset;
      break;
    case 3096: // ProjRectifiedGridAngleGeoKey
      offsetProjRectifiedGridAngleGeoKey = geo_keys[i].value_offset;
      break;
    }
  }

  if (gcs_code != -1)
  {
    set_gcs(gcs_code);
  }
  else if (ellipsoid != -1)
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
      if (((offsetProjFalseEastingGeoKey >= 0) || (offsetProjFalseOriginEastingGeoKey >= 0)) &&
          ((offsetProjFalseNorthingGeoKey >= 0) || (offsetProjFalseOriginNorthingGeoKey >= 0)) &&
          ((offsetProjNatOriginLatGeoKey >= 0) || (offsetProjFalseOriginLatGeoKey >= 0)) &&
          ((offsetProjCenterLongGeoKey >= 0) || (offsetProjNatOriginLongGeoKey >= 0) || (offsetProjFalseOriginLongGeoKey >= 0)) &&
          (offsetProjStdParallel1GeoKey >= 0) &&
          (offsetProjStdParallel2GeoKey >= 0))
      {
        double falseEastingMeter = ((offsetProjFalseEastingGeoKey >= 0) ? geo_double_params[offsetProjFalseEastingGeoKey] * coordinates2meter : geo_double_params[offsetProjFalseOriginEastingGeoKey] * coordinates2meter);
        double falseNorthingMeter = ((offsetProjFalseNorthingGeoKey >= 0) ? geo_double_params[offsetProjFalseNorthingGeoKey] * coordinates2meter : geo_double_params[offsetProjFalseOriginNorthingGeoKey] * coordinates2meter);
        double latOriginDeg = ((offsetProjNatOriginLatGeoKey >= 0) ? geo_double_params[offsetProjNatOriginLatGeoKey] : geo_double_params[offsetProjFalseOriginLatGeoKey]);
        double longOriginDeg = ((offsetProjCenterLongGeoKey >= 0) ? geo_double_params[offsetProjCenterLongGeoKey] : ((offsetProjNatOriginLongGeoKey >= 0) ? geo_double_params[offsetProjNatOriginLongGeoKey] : geo_double_params[offsetProjFalseOriginLongGeoKey]));
        if ((longOriginDeg == 0.0) && (offsetProjNatOriginLongGeoKey >= 0)) longOriginDeg = geo_double_params[offsetProjNatOriginLongGeoKey];
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
    else if (user_defined_projection == 11)
    {
      if ((offsetProjFalseEastingGeoKey >= 0) &&
          (offsetProjFalseNorthingGeoKey >= 0) &&
          (offsetProjNatOriginLatGeoKey >= 0) &&
          ((offsetProjCenterLongGeoKey >= 0) || (offsetProjNatOriginLongGeoKey >= 0)) &&
          (offsetProjStdParallel1GeoKey >= 0) &&
          (offsetProjStdParallel2GeoKey >= 0))
      {
        double falseEastingMeter = geo_double_params[offsetProjFalseEastingGeoKey] * coordinates2meter;
        double falseNorthingMeter = geo_double_params[offsetProjFalseNorthingGeoKey] * coordinates2meter;
        double latOriginDeg = geo_double_params[offsetProjNatOriginLatGeoKey];
        double longOriginDeg = ((offsetProjCenterLongGeoKey >= 0) ? geo_double_params[offsetProjCenterLongGeoKey] : geo_double_params[offsetProjNatOriginLongGeoKey]);
        if ((longOriginDeg == 0.0) && (offsetProjNatOriginLongGeoKey >= 0)) longOriginDeg = geo_double_params[offsetProjNatOriginLongGeoKey];
        double firstStdParallelDeg = geo_double_params[offsetProjStdParallel1GeoKey];
        double secondStdParallelDeg = geo_double_params[offsetProjStdParallel2GeoKey];
        set_albers_equal_area_conic_projection(falseEastingMeter, falseNorthingMeter, latOriginDeg, longOriginDeg, firstStdParallelDeg, secondStdParallelDeg);
        if (description)
        {
          sprintf(description, "generic albers equal area");
        }
        has_projection = true;
      }
    }
    else if (user_defined_projection == 16)
    {
      if ((offsetProjFalseEastingGeoKey >= 0) &&
          (offsetProjFalseNorthingGeoKey >= 0) &&
          (offsetProjNatOriginLatGeoKey >= 0) &&
          ((offsetProjCenterLongGeoKey >= 0) || (offsetProjNatOriginLongGeoKey >= 0)) &&
          (offsetProjScaleAtNatOriginGeoKey >= 0))
      {
        double falseEastingMeter = geo_double_params[offsetProjFalseEastingGeoKey] * coordinates2meter;
        double falseNorthingMeter = geo_double_params[offsetProjFalseNorthingGeoKey] * coordinates2meter;
        double latOriginDeg = geo_double_params[offsetProjNatOriginLatGeoKey];
        double longMeridianDeg = ((offsetProjCenterLongGeoKey >= 0) ? geo_double_params[offsetProjCenterLongGeoKey] : geo_double_params[offsetProjNatOriginLongGeoKey]);
        double scaleFactor = geo_double_params[offsetProjScaleAtNatOriginGeoKey];
        set_oblique_stereographic_projection(falseEastingMeter, falseNorthingMeter, latOriginDeg, longMeridianDeg, scaleFactor);
        if (description)
        {
          sprintf(description, "generic oblique stereographic");
        }
        has_projection = true;
      }
    }
    else if (user_defined_projection == 3)
    {
      if ((offsetProjFalseEastingGeoKey >= 0) &&
          (offsetProjFalseNorthingGeoKey >= 0) &&
          (offsetProjCenterLatGeoKey >= 0) &&
          (offsetProjCenterLongGeoKey >= 0) &&
          (offsetProjAzimuthAngleGeoKey >= 0) &&
          (offsetProjScaleAtCenterGeoKey >= 0))
      {
        double falseEastingMeter = geo_double_params[offsetProjFalseEastingGeoKey] * coordinates2meter;
        double falseNorthingMeter = geo_double_params[offsetProjFalseNorthingGeoKey] * coordinates2meter;
        double latCenterDeg = geo_double_params[offsetProjCenterLatGeoKey];
        double longCenterDeg = geo_double_params[offsetProjCenterLongGeoKey];
        double azimuthDeg = geo_double_params[offsetProjAzimuthAngleGeoKey];
        double rectifiedGridAngleDeg = (offsetProjRectifiedGridAngleGeoKey >= 0 ? geo_double_params[offsetProjRectifiedGridAngleGeoKey] : azimuthDeg);
        double scaleFactor = geo_double_params[offsetProjScaleAtCenterGeoKey];
        set_hotine_oblique_mercator_projection(falseEastingMeter, falseNorthingMeter, latCenterDeg, longCenterDeg, azimuthDeg, rectifiedGridAngleDeg, scaleFactor);
        if (description)
        {
          sprintf(description, "generic hotine oblique mercator");
        }
        has_projection = true;
      }
    }
  }

  return has_projection;
}

bool GeoProjectionConverter::get_geo_keys_from_projection(int& num_geo_keys, GeoProjectionGeoKeys** geo_keys, int& num_geo_double_params, double** geo_double_params, bool source)
{
  num_geo_keys = 0;
  num_geo_double_params = 0;
  GeoProjectionParameters* projection = (source ? source_projection : target_projection);
  if (projection)
  {
    unsigned short vertical_geokey = get_VerticalCSTypeGeoKey();
    if (projection->type == GEO_PROJECTION_UTM || projection->type == GEO_PROJECTION_LCC || projection->type == GEO_PROJECTION_TM || projection->type == GEO_PROJECTION_AEAC || projection->type == GEO_PROJECTION_HOM || projection->type == GEO_PROJECTION_OS)
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

        num_geo_keys = 12 + (vertical_geokey ? 1 : 0);
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

        // GCS used with custom LLC projection
        (*geo_keys)[10].key_id = 2048; // GeographicTypeGeoKey
        (*geo_keys)[10].tiff_tag_location = 0;
        (*geo_keys)[10].count = 1;
        (*geo_keys)[10].value_offset = get_GeographicTypeGeoKey();

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
      else if (projection->type == GEO_PROJECTION_TM)
      {
        GeoProjectionParametersTM* tm = (GeoProjectionParametersTM*)projection;

        num_geo_keys = 11 + (vertical_geokey ? 1 : 0);
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

        // GCS used with custom TM projection
        (*geo_keys)[9].key_id = 2048; // GeographicTypeGeoKey
        (*geo_keys)[9].tiff_tag_location = 0;
        (*geo_keys)[9].count = 1;
        (*geo_keys)[9].value_offset = get_GeographicTypeGeoKey();

        // vertical units
        (*geo_keys)[10].key_id = 4099; // VerticalUnitsGeoKey
        (*geo_keys)[10].tiff_tag_location = 0;
        (*geo_keys)[10].count = 1;
        (*geo_keys)[10].value_offset = get_VerticalUnitsGeoKey(source);

        if (vertical_geokey)
        {
          // vertical datum
          (*geo_keys)[11].key_id = 4096; // VerticalCSTypeGeoKey
          (*geo_keys)[11].tiff_tag_location = 0;
          (*geo_keys)[11].count = 1;
          (*geo_keys)[11].value_offset = vertical_geokey;
        }
        return true;
      }
      else if (projection->type == GEO_PROJECTION_AEAC)
      {
        GeoProjectionParametersAEAC* aeac = (GeoProjectionParametersAEAC*)projection;

        num_geo_keys = 12 + (vertical_geokey ? 1 : 0);
        (*geo_keys) = (GeoProjectionGeoKeys*)malloc(sizeof(GeoProjectionGeoKeys)*num_geo_keys);
        num_geo_double_params = 6;
        (*geo_double_params) = (double*)malloc(sizeof(double)*num_geo_double_params);

        // projected coordinates
        (*geo_keys)[0].key_id = 1024; // GTModelTypeGeoKey
        (*geo_keys)[0].tiff_tag_location = 0;
        (*geo_keys)[0].count = 1;
        (*geo_keys)[0].value_offset = 1; // ModelTypeProjected

        // user-defined custom AEAC projection
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
        (*geo_double_params)[2] = aeac->aeac_longitude_of_center_degree;

        (*geo_keys)[7].key_id = 3081; // ProjCenterLatGeoKey
        (*geo_keys)[7].tiff_tag_location = 34736;
        (*geo_keys)[7].count = 1;
        (*geo_keys)[7].value_offset = 3;
        (*geo_double_params)[3] = aeac->aeac_latitude_of_center_degree;

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

        // GCS used with custom AEAC projection
        (*geo_keys)[10].key_id = 2048; // GeographicTypeGeoKey
        (*geo_keys)[10].tiff_tag_location = 0;
        (*geo_keys)[10].count = 1;
        (*geo_keys)[10].value_offset = get_GeographicTypeGeoKey();

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
      else if (projection->type == GEO_PROJECTION_HOM)
      {
        GeoProjectionParametersHOM* hom = (GeoProjectionParametersHOM*)projection;

        num_geo_keys = 13 + (vertical_geokey ? 1 : 0);
        (*geo_keys) = (GeoProjectionGeoKeys*)malloc(sizeof(GeoProjectionGeoKeys)*num_geo_keys);
        num_geo_double_params = 7;
        (*geo_double_params) = (double*)malloc(sizeof(double)*num_geo_double_params);

        // projected coordinates
        (*geo_keys)[0].key_id = 1024; // GTModelTypeGeoKey
        (*geo_keys)[0].tiff_tag_location = 0;
        (*geo_keys)[0].count = 1;
        (*geo_keys)[0].value_offset = 1; // ModelTypeProjected

        // user-defined custom HOM projection
        (*geo_keys)[1].key_id = 3072; // ProjectedCSTypeGeoKey
        (*geo_keys)[1].tiff_tag_location = 0;
        (*geo_keys)[1].count = 1;
        (*geo_keys)[1].value_offset = 32767; // user-defined

        // which projection do we use
        (*geo_keys)[2].key_id = 3075; // ProjCoordTransGeoKey
        (*geo_keys)[2].tiff_tag_location = 0;
        (*geo_keys)[2].count = 1;
        (*geo_keys)[2].value_offset = 3; // CT_ObliqueMercator

        // which units do we use
        (*geo_keys)[3].key_id = 3076; // ProjCoordTransGeoKey
        (*geo_keys)[3].tiff_tag_location = 0;
        (*geo_keys)[3].count = 1;
        (*geo_keys)[3].value_offset = get_ProjLinearUnitsGeoKey(source);

        // here come the 7 double parameters

        (*geo_keys)[4].key_id = 3088; // ProjCenterLongGeoKey
        (*geo_keys)[4].tiff_tag_location = 34736;
        (*geo_keys)[4].count = 1;
        (*geo_keys)[4].value_offset = 0;
        (*geo_double_params)[0] = hom->hom_longitude_of_center_degree;

        (*geo_keys)[5].key_id = 3089; // ProjCenterLatGeoKey
        (*geo_keys)[5].tiff_tag_location = 34736;
        (*geo_keys)[5].count = 1;
        (*geo_keys)[5].value_offset = 1;
        (*geo_double_params)[1] = hom->hom_latitude_of_center_degree;

        (*geo_keys)[6].key_id = 3094; // ProjAzimuthAngleGeoKey
        (*geo_keys)[6].tiff_tag_location = 34736;
        (*geo_keys)[6].count = 1;
        (*geo_keys)[6].value_offset = 2;
        (*geo_double_params)[2] = hom->hom_azimuth_degree;

        (*geo_keys)[7].key_id = 3096; // ProjRectifiedGridAngleGeoKey
        (*geo_keys)[7].tiff_tag_location = 34736;
        (*geo_keys)[7].count = 1;
        (*geo_keys)[7].value_offset = 3;
        (*geo_double_params)[3] = hom->hom_rectified_grid_angle_degree;

        (*geo_keys)[8].key_id = 3093; // ProjScaleAtCenterGeoKey
        (*geo_keys)[8].tiff_tag_location = 34736;
        (*geo_keys)[8].count = 1;
        (*geo_keys)[8].value_offset = 4;
        (*geo_double_params)[4] = hom->hom_scale_factor;

        (*geo_keys)[9].key_id = 3082; // ProjFalseEastingGeoKey
        (*geo_keys)[9].tiff_tag_location = 34736;
        (*geo_keys)[9].count = 1;
        (*geo_keys)[9].value_offset = 5;
        if (source)
          (*geo_double_params)[5] = hom->hom_false_easting_meter / coordinates2meter;
        else
          (*geo_double_params)[5] = hom->hom_false_easting_meter * meter2coordinates;

        (*geo_keys)[10].key_id = 3083; // ProjFalseNorthingGeoKey
        (*geo_keys)[10].tiff_tag_location = 34736;
        (*geo_keys)[10].count = 1;
        (*geo_keys)[10].value_offset = 6;
        if (source)
          (*geo_double_params)[6] = hom->hom_false_northing_meter / coordinates2meter;
        else
          (*geo_double_params)[6] = hom->hom_false_northing_meter * meter2coordinates;

        // GCS used with custom HOM projection
        (*geo_keys)[11].key_id = 2048; // GeographicTypeGeoKey
        (*geo_keys)[11].tiff_tag_location = 0;
        (*geo_keys)[11].count = 1;
        (*geo_keys)[11].value_offset = get_GeographicTypeGeoKey();

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
        fprintf(stderr, "get_geo_keys_from_projection for generic projection not implemented\n");
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

      // GCS used with latitude/longitude coordinates
      (*geo_keys)[1].key_id = 2048; // GeographicTypeGeoKey
      (*geo_keys)[1].tiff_tag_location = 0;
      (*geo_keys)[1].count = 1;
      (*geo_keys)[1].value_offset = get_GeographicTypeGeoKey();

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

      // GCS used with earth centered coodinates
      (*geo_keys)[1].key_id = 2048; // GeographicTypeGeoKey
      (*geo_keys)[1].tiff_tag_location = 0;
      (*geo_keys)[1].count = 1;
      (*geo_keys)[1].value_offset = get_GeographicTypeGeoKey();
      return true;
    }
  }
  return false;
}

static FILE* open_geo_file(const char* program_name, bool pcs=true, bool vertical=false)
{
  FILE* file = 0;

  // create path to 'pcs.csv', 'gcs.csv', or 'vertcs.csv' file

  #define MAX_GEO_PATH_LENGTH 4096
  int path_len = 0;
  char path[MAX_GEO_PATH_LENGTH];

#ifdef _WIN32
  if (program_name)
  {
    GetModuleFileName(GetModuleHandle(program_name),(LPTSTR)path, MAX_GEO_PATH_LENGTH);
    path_len = (int)strlen(path);
  }
  else
  {
    path[path_len] = '.';
    path_len = 1;
  }
#else //_WIN32
  path_len = readlink("/proc/self/exe", path, MAX_GEO_PATH_LENGTH);
#endif //_WIN32

  while ((path_len > 0) && (path[path_len] != '\\') && (path[path_len] != '/') && (path[path_len] != ':')) path_len--;
  path[path_len] = '/'; path_len++;
  path[path_len] = 's'; path_len++;
  path[path_len] = 'e'; path_len++;
  path[path_len] = 'r'; path_len++;
  path[path_len] = 'f'; path_len++;
  path[path_len] = '/'; path_len++;
  path[path_len] = 'g'; path_len++;
  path[path_len] = 'e'; path_len++;
  path[path_len] = 'o'; path_len++;
  path[path_len] = '/'; path_len++;
  if (vertical)
  {
    path[path_len] = 'v'; path_len++;
    path[path_len] = 'e'; path_len++;
    path[path_len] = 'r'; path_len++;
    path[path_len] = 't'; path_len++;
    path[path_len] = 'c'; path_len++;
    path[path_len] = 's'; path_len++;
  }
  else
  {
    path[path_len] = (pcs ? 'p' : 'g'); path_len++;
    path[path_len] = 'c'; path_len++;
    path[path_len] = 's'; path_len++;
  }
  path[path_len] = '.'; path_len++;
  path[path_len] = 'c'; path_len++;
  path[path_len] = 's'; path_len++;
  path[path_len] = 'v'; path_len++;
  path[path_len] = '\0';

  file = fopen(path, "r");

  return file;
}

bool get_unit_from_ogc_wkt(const char* ogc_wkt, double* value)
{
  const char* unit = strstr(ogc_wkt, "UNIT[");
  if (unit)
  {
    int len = (int)strlen(ogc_wkt);
    int curr = (int)((unit - ogc_wkt) + 5);
    while (curr < len)
    {
      if (ogc_wkt[curr] == '[')
      {
        return false;
      }
      else if (ogc_wkt[curr] == ']')
      {
        return false;
      }
      else if (ogc_wkt[curr] == ',')
      {
        curr++;
        if (sscanf(&ogc_wkt[curr], "%lf", value) == 1)
        {
//          fprintf(stderr, "unit %f\n", *value);
          return true;
        }
      }
      curr++;
    }
  }
  return false;
}

bool get_parameter_from_ogc_wkt(const char* ogc_wkt, const char* name, double* value)
{
  const char* para = strstr(ogc_wkt, name);
  if (para)
  {
    int len = (int)strlen(ogc_wkt);
    int curr = (int)((para - ogc_wkt) + strlen(name));
    while (curr < len)
    {
      if (ogc_wkt[curr] == '[')
      {
        return false;
      }
      else if (ogc_wkt[curr] == ']')
      {
        return false;
      }
      else if (ogc_wkt[curr] == ',')
      {
        curr++;
        if (sscanf(&ogc_wkt[curr], "%lf", value) == 1)
        {
//          fprintf(stderr, "%s %f\n", name, *value);
          return true;
        }
      }
      curr++;
    }
  }
  return false;
}

bool GeoProjectionConverter::set_projection_from_ogc_wkt(const char* ogc_wkt, char* description)
{
/*
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
  int datum = -1;
*/

  // this very first version only checks for the EPSG code of projection

  // check if we have vertical datum (e.g. string contains a VERT_CS)

  const char* vertcs = strstr(ogc_wkt, "VERT_CS[");

  if (vertcs)
  {
    vertical_geokey = 0;
    int len = (int)strlen(ogc_wkt);
    // see if we can find an AUTHORITY containing the EPSG code
    int open_bracket = 1;
    int curr = (int)((vertcs - ogc_wkt) + 8);
    while ((curr < len) && open_bracket)
    {
      if (ogc_wkt[curr] == '[')
      {
        open_bracket++;
      }
      else if (ogc_wkt[curr] == ']')
      {
        open_bracket--;
      }
      else if (open_bracket == 2)
      {
        if (ogc_wkt[curr] == 'A')
        {
          if (strncmp(&ogc_wkt[curr], "AUTHORITY", 9) == 0)
          {
            curr += 9;
            const char* epsg = strstr(&ogc_wkt[curr], "\"EPSG\"");
            if (epsg)
            {
              curr = (int)((epsg - ogc_wkt) + 6);
              while ((curr < len) && ogc_wkt[curr] != ',')
              {
                curr++;
              }
              curr++;
              while ((curr < len) && ogc_wkt[curr] != '\"')
              {
                curr++;
              }
              curr++;
              int code = -1;
              if (sscanf(&ogc_wkt[curr], "%d", &code) == 1)
              {
                vertical_geokey = code;
                open_bracket++; // because the one from "AUTHORITY[" was not counted
              }
            }
          }
        }
      }
      else if (open_bracket == 1)
      {
        if (ogc_wkt[curr] == 'U')
        {
          if (strncmp(&ogc_wkt[curr], "UNIT[", 5) == 0)
          {
            curr += 5;
            while ((curr < len) && ogc_wkt[curr] != ',') // skip description
            {
              curr++;
            }
            curr++;
            double unit;
            if (sscanf(&ogc_wkt[curr], "%lf", &unit) == 1)
            {
              if (unit == 1.0)
              {
                set_elevation_in_meter();
              }
              else if (fabs(unit-0.3048006096012192) < 0.000000001)
              {
                set_elevation_in_survey_feet();
              }
              else
              {
                set_elevation_in_feet();
              }
            }
          }
        }
      }
      curr++;
    }
  }

  // check if we have a projection (e.g. string contains a PROJCS)

  const char* projcs = strstr(ogc_wkt, "PROJCS[");

  if (projcs)
  {
    int len = (int)strlen(ogc_wkt);
    // if we can find an AUTHORITY containing the EPSG code we are done
    int open_bracket = 1;
    int curr = (int)((projcs - ogc_wkt) + 7);
    while ((curr < len) && open_bracket)
    {
      if (ogc_wkt[curr] == '[')
      {
        open_bracket++;
      }
      else if (ogc_wkt[curr] == ']')
      {
        open_bracket--;
      }
      else if (open_bracket == 1)
      {
        if (ogc_wkt[curr] == 'A')
        {
          if (strncmp(&ogc_wkt[curr], "AUTHORITY", 9) == 0)
          {
            curr += 9;
            const char* epsg = strstr(&ogc_wkt[curr], "\"EPSG\"");
            if (epsg)
            {
              curr = (int)((epsg - ogc_wkt) + 6);
              while ((curr < len) && ogc_wkt[curr] != ',')
              {
                curr++;
              }
              curr++;
              while ((curr < len) && ogc_wkt[curr] != '\"')
              {
                curr++;
              }
              curr++;
              int code = -1;
              if (sscanf(&ogc_wkt[curr], "%d", &code) == 1)
              {
                return set_epsg_code(code, description);
              }
            }
          }
        }
      }
      curr++;
    }

    // otherwise try to find the PROJECTION and all its parameters

    const char* proj = strstr(projcs, "PROJECTION[");

    if (proj)
    {
      int open_bracket = 1;
      int curr = (int)((proj - ogc_wkt) + 11);

      while (curr < len)
      {
        if (ogc_wkt[curr] == '[')
        {
          open_bracket++;
        }
        else if (ogc_wkt[curr] == ']')
        {
          open_bracket--;
        }
        else if (open_bracket == 1)
        {
          if (ogc_wkt[curr] == '\"')
          {
            curr++;
            if (strncmp(&ogc_wkt[curr], "Lambert_Conformal_Conic", 23) == 0)
            {
              double false_easting;
              double false_northing;
              double unit = 1.0;
              double latitude_of_origin;
              double central_meridian;
              double standard_parallel_1;
              double standard_parallel_2;
              if (!get_parameter_from_ogc_wkt(&ogc_wkt[curr], "false_easting", &false_easting) && !get_parameter_from_ogc_wkt(&ogc_wkt[curr], "False_Easting", &false_easting)) return false;
              if (!get_parameter_from_ogc_wkt(&ogc_wkt[curr], "false_northing", &false_northing) && !get_parameter_from_ogc_wkt(&ogc_wkt[curr], "False_Northing", &false_northing)) return false;
              if (!get_parameter_from_ogc_wkt(&ogc_wkt[curr], "latitude_of_origin", &latitude_of_origin) && !get_parameter_from_ogc_wkt(&ogc_wkt[curr], "Latitude_Of_Origin", &latitude_of_origin)) return false;
              if (!get_parameter_from_ogc_wkt(&ogc_wkt[curr], "central_meridian", &central_meridian) && !get_parameter_from_ogc_wkt(&ogc_wkt[curr], "Central_Meridian", &central_meridian)) return false;
              if (!get_parameter_from_ogc_wkt(&ogc_wkt[curr], "standard_parallel_1", &standard_parallel_1) && !get_parameter_from_ogc_wkt(&ogc_wkt[curr], "Standard_Parallel_1", &standard_parallel_1)) return false;
              if (!get_parameter_from_ogc_wkt(&ogc_wkt[curr], "standard_parallel_2", &standard_parallel_2) && !get_parameter_from_ogc_wkt(&ogc_wkt[curr], "Standard_Parallel_2", &standard_parallel_2)) return false;
              get_unit_from_ogc_wkt(&ogc_wkt[curr], &unit);
              set_lambert_conformal_conic_projection(unit*false_easting, unit*false_northing, latitude_of_origin, central_meridian, standard_parallel_1, standard_parallel_2);
              return true;
            }
            else if (strncmp(&ogc_wkt[curr], "Transverse_Mercator", 19) == 0)
            {
              double false_easting;
              double false_northing;
              double unit = 1.0;
              double latitude_of_origin;
              double central_meridian;
              double scale_factor;
              if (!get_parameter_from_ogc_wkt(&ogc_wkt[curr], "false_easting", &false_easting) && !get_parameter_from_ogc_wkt(&ogc_wkt[curr], "False_Easting", &false_easting)) return false;
              if (!get_parameter_from_ogc_wkt(&ogc_wkt[curr], "false_northing", &false_northing) && !get_parameter_from_ogc_wkt(&ogc_wkt[curr], "False_Northing", &false_northing)) return false;
              if (!get_parameter_from_ogc_wkt(&ogc_wkt[curr], "latitude_of_origin", &latitude_of_origin) && !get_parameter_from_ogc_wkt(&ogc_wkt[curr], "Latitude_Of_Origin", &latitude_of_origin)) return false;
              if (!get_parameter_from_ogc_wkt(&ogc_wkt[curr], "central_meridian", &central_meridian) && !get_parameter_from_ogc_wkt(&ogc_wkt[curr], "Central_Meridian", &central_meridian)) return false;
              if (!get_parameter_from_ogc_wkt(&ogc_wkt[curr], "scale_factor", &scale_factor) && !get_parameter_from_ogc_wkt(&ogc_wkt[curr], "Scale_Factor", &scale_factor)) return false;
              get_unit_from_ogc_wkt(&ogc_wkt[curr], &unit);
              set_transverse_mercator_projection(unit*false_easting, unit*false_northing, latitude_of_origin, central_meridian, scale_factor);
              return true;
            }
            else if (strncmp(&ogc_wkt[curr], "Albers_Conic_Equal_Area", 23) == 0)
            {
              double false_easting;
              double false_northing;
              double unit = 1.0;
              double latitude_of_center;
              double longitude_of_center;
              double standard_parallel_1;
              double standard_parallel_2;
              if (!get_parameter_from_ogc_wkt(&ogc_wkt[curr], "false_easting", &false_easting) && !get_parameter_from_ogc_wkt(&ogc_wkt[curr], "False_Easting", &false_easting)) return false;
              if (!get_parameter_from_ogc_wkt(&ogc_wkt[curr], "false_northing", &false_northing) && !get_parameter_from_ogc_wkt(&ogc_wkt[curr], "False_Northing", &false_northing)) return false;
              if (!get_parameter_from_ogc_wkt(&ogc_wkt[curr], "latitude_of_center", &latitude_of_center) && !get_parameter_from_ogc_wkt(&ogc_wkt[curr], "Latitude_Of_Center", &latitude_of_center)) return false;
              if (!get_parameter_from_ogc_wkt(&ogc_wkt[curr], "longitude_of_center", &longitude_of_center) && !get_parameter_from_ogc_wkt(&ogc_wkt[curr], "Longitude_Of_Center", &longitude_of_center)) return false;
              if (!get_parameter_from_ogc_wkt(&ogc_wkt[curr], "standard_parallel_1", &standard_parallel_1) && !get_parameter_from_ogc_wkt(&ogc_wkt[curr], "Standard_Parallel_1", &standard_parallel_1)) return false;
              if (!get_parameter_from_ogc_wkt(&ogc_wkt[curr], "standard_parallel_2", &standard_parallel_2) && !get_parameter_from_ogc_wkt(&ogc_wkt[curr], "Standard_Parallel_2", &standard_parallel_2)) return false;
              get_unit_from_ogc_wkt(&ogc_wkt[curr], &unit);
              set_albers_equal_area_conic_projection(unit*false_easting, unit*false_northing, latitude_of_center, longitude_of_center, standard_parallel_1, standard_parallel_2);
              return true;
            }
            else
            {
              return false;
            }
          }
        }
        curr++;
      }
    }
  }
  else
  {
      // check if the string contains a GEOCCS
    const char* geoccs = strstr(ogc_wkt, "GEOCCS[");
  }

  return false;
}

static char* get_epsg_name_from_pcs_file(const char* program_name, short value)
{
  FILE* file = open_geo_file(program_name, true);
  if (file == 0)
  {
    return 0;
  }
  char* epsg_name = 0;
  int epsg_code = 0;
  char line[2048];
  while (fgets(line, 2048, file))
  {
    if (sscanf(line, "%d,", &epsg_code) == 1)
    {
      if (epsg_code == value)
      {
        char* name;
        int run = 0;;
        // skip until first comma
        while (line[run] != ',') run++;
        run++;
        // maybe name is in parentheses
        if (line[run] == '\"')
        {
          // remove opening parentheses
          run++;
          // this is where the name starts
          name = &line[run];
          run++;
          // skip until closing parentheses
          while (line[run] != '\"') run++;
          // this is where the name ends
          line[run] = '\0';
        }
        else
        {
          // this is where the name starts
          name = &line[run];
          // skip until second comma
          while (line[run] != ',') run++;
          // this is where the name ends
          line[run] = '\0';
        }
        // copy the name
        epsg_name = LASCopyString(name);
        break;
      }
    }
  }
  fclose(file);
  return epsg_name;
}

static int print_ogc_wkt_spheroid(char* string, short spheroid_code)
{
  int n = 0;

  if (spheroid_code == GEO_SPHEROID_WGS84)
  {
    n = sprintf(string, "SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],");
  }
  else if (spheroid_code == GEO_SPHEROID_GRS80)
  {
    n = sprintf(string, "SPHEROID[\"GRS 1980\",6378137,298.257222101,AUTHORITY[\"EPSG\",\"7019\"]],");
  }
  else if (spheroid_code == GEO_SPHEROID_WGS72)
  {
    n = sprintf(string, "SPHEROID[\"WGS 72\",6378135,298.26,AUTHORITY[\"EPSG\",\"7043\"]],");
  }
  else if (spheroid_code == GEO_SPHEROID_GRS67)
  {
    n = sprintf(string, "SPHEROID[\"GRS 1967\",6378160,298.247167427,AUTHORITY[\"EPSG\",\"7036\"]],");
  }
  else if (spheroid_code == GEO_SPHEROID_CLARKE1866)
  {
    n = sprintf(string, "SPHEROID[\"Clarke 1866\",6378206.4,294.9786982139006,AUTHORITY[\"EPSG\",\"7008\"]],");
  }
  else if (spheroid_code == GEO_SPHEROID_INTERNATIONAL)
  {
    n = sprintf(string, "SPHEROID[\"International 1924\",6378388,297,AUTHORITY[\"EPSG\",\"7022\"]],");
  }
  else if (spheroid_code == GEO_SPHEROID_BESSEL1841)
  {
    n = sprintf(string, "SPHEROID[\"Bessel 1841\",6377397.155,299.1528128,AUTHORITY[\"EPSG\",\"7004\"]],");
  }
  else if (spheroid_code == GEO_SPHEROID_AIRY)
  {
    n = sprintf(string, "SPHEROID[\"Airy 1830\",6377563.396,299.3249646,AUTHORITY[\"EPSG\",\"7001\"]],");
  }
  return n;
}

static int print_ogc_wkt_datum(char* string, const char* datum_name, short datum_code, short spheroid_code)
{
  int n = 0;

  n += sprintf(&string[n], "DATUM[\"%s\",", datum_name);
  n += print_ogc_wkt_spheroid(&string[n], spheroid_code);
  n += sprintf(&string[n], "AUTHORITY[\"EPSG\",\"%d\"]],", datum_code);

  return n;
}

static int print_ogc_wkt_geogcs(char* string, const char* gcs_name, short gcs_code, const char* datum_name, short datum_code, short spheroid_code)
{
  int n = 0;

  n += sprintf(&string[n], "GEOGCS[\"%s\",", gcs_name);
  n += print_ogc_wkt_datum(&string[n], datum_name, datum_code, spheroid_code);
  n += sprintf(&string[n], "PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"%d\"]],", gcs_code);

  return n;
}

/* non-zero *ogc_wkt returns char[] that becomes property of caller. dealloc with free() */
bool GeoProjectionConverter::get_ogc_wkt_from_projection(int& len, char** ogc_wkt, bool source)
{
  GeoProjectionParameters* projection = (source ? source_projection : target_projection);
  if (projection)
  {
    int n = 0;
    char* string = (char*)malloc(4096);
    memset(string, 0, 4096);
    // maybe geocentric
    if (projection->type == GEO_PROJECTION_ECEF)
    {
      n += sprintf(&string[n], "GEOCCS[\"WGS 84\",DATUM[\"World Geodetic System 1984\",SPHEROID[\"WGS 84\",6378137.0,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0.0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"m\",1.0],AXIS[\"Geocentric X\",OTHER],AXIS[\"Geocentric Y\",EAST],AXIS[\"Geocentric Z\",NORTH],AUTHORITY[\"EPSG\",\"4978\"]]");
    }
    else
    {
      // if not geographic we have a projection
      if ((projection->type != GEO_PROJECTION_LAT_LONG) && (projection->type != GEO_PROJECTION_LONG_LAT))
      {
        int len = (int)strlen(projection->name);
        char* epsg_name = 0;
        if (len == 0)
        {
          epsg_name = get_epsg_name_from_pcs_file(argv_zero, projection->geokey);
        }
        else
        {
          len += (int)strlen(gcs_name) + 16;
          epsg_name = (char*)malloc(len);
          sprintf(epsg_name, "%s / %s", gcs_name, projection->name);
        }
        // maybe output a compound CRS
        if ((vertical_geokey == GEO_VERTICAL_NAVD88) || (vertical_geokey == GEO_VERTICAL_NGVD29) || (vertical_geokey == GEO_VERTICAL_CGVD2013) || (vertical_geokey == GEO_VERTICAL_EVRF2007) || (vertical_geokey == GEO_VERTICAL_CGVD28) || (vertical_geokey == GEO_VERTICAL_DVR90) || (vertical_geokey == GEO_VERTICAL_NN2000) || (vertical_geokey == GEO_VERTICAL_NN54) || (vertical_geokey == GEO_VERTICAL_DHHN92) || (vertical_geokey == GEO_VERTICAL_DHHN2016) || (vertical_geokey == GEO_VERTICAL_NZVD2016) )
        {
          n += sprintf(&string[n], "COMPD_CS[\"%s + ", epsg_name);

          if (vertical_geokey == GEO_VERTICAL_NAVD88)
          {
            n += sprintf(&string[n], "NAVD88");
            if (vertical_geoid)
            {
              if (vertical_geoid == GEO_VERTICAL_NAVD88_GEOID12B)
              {
                n += sprintf(&string[n], " height - Geoid12B");
              }
              else if (vertical_geoid ==  GEO_VERTICAL_NAVD88_GEOID12A)
              {
                n += sprintf(&string[n], " height - Geoid12A");
              }
              else if (vertical_geoid ==  GEO_VERTICAL_NAVD88_GEOID12)
              {
                n += sprintf(&string[n], " height - Geoid12");
              }
              else if (vertical_geoid ==  GEO_VERTICAL_NAVD88_GEOID09)
              {
                n += sprintf(&string[n], " height - Geoid09");
              }
              else if (vertical_geoid ==  GEO_VERTICAL_NAVD88_GEOID06)
              {
                n += sprintf(&string[n], " height - Geoid06");
              }
              else if (vertical_geoid ==  GEO_VERTICAL_NAVD88_GEOID03)
              {
                n += sprintf(&string[n], " height - Geoid03");
              }
              else if (vertical_geoid ==  GEO_VERTICAL_NAVD88_GEOID99)
              {
                n += sprintf(&string[n], " height - Geoid99");
              }
              else if (vertical_geoid == GEO_VERTICAL_NAVD88_GEOID96)
              {
                n += sprintf(&string[n], " height - Geoid96");
              }
            }
          }
          else if (vertical_geokey == GEO_VERTICAL_NGVD29)
          {
            n += sprintf(&string[n], "NGVD29");
          }
          else if (vertical_geokey == GEO_VERTICAL_CGVD2013)
          {
            n += sprintf(&string[n], "CGVD2013");
          }
          else if (vertical_geokey == GEO_VERTICAL_EVRF2007)
          {
            n += sprintf(&string[n], "EVRF2007");
          }
          else if (vertical_geokey == GEO_VERTICAL_CGVD28)
          {
            n += sprintf(&string[n], "CGVD28");
          }
          else if (vertical_geokey == GEO_VERTICAL_DVR90)
          {
            n += sprintf(&string[n], "DVR90");
          }
          else if (vertical_geokey == GEO_VERTICAL_NN2000)
          {
            n += sprintf(&string[n], "NN2000");
          }
          else if (vertical_geokey == GEO_VERTICAL_NN54)
          {
            n += sprintf(&string[n], "NN54");
          }
          else if (vertical_geokey == GEO_VERTICAL_DHHN92)
          {
            n += sprintf(&string[n], "DHHN92");
          }
          else if (vertical_geokey == GEO_VERTICAL_DHHN2016)
          {
            n += sprintf(&string[n], "DHHN2016");
          }
          else if (vertical_geokey == GEO_VERTICAL_NZVD2016)
          {
            n += sprintf(&string[n], "NZVD2016");
          }

          if (source)
          {
            if (elevation2meter == 1.0)
            {
              n += sprintf(&string[n], "\",");
            }
            else if (elevation2meter == 0.3048)
            {
              n += sprintf(&string[n], " (ft)\",");
            }
            else
            {
              n += sprintf(&string[n], " (ftUS)\",");
            }
          }
          else
          {
            if (meter2elevation == 1.0)
            {
              n += sprintf(&string[n], "\",");
            }
            else if (meter2elevation == 0.3048)
            {
              n += sprintf(&string[n], " (ft)\",");
            }
            else
            {
              n += sprintf(&string[n], " (ftUS)\",");
            }
          }
        }
        n += sprintf(&string[n], "PROJCS[\"%s\",", epsg_name);
        free(epsg_name);
      }
      // which datum
      if (gcs_code == GEO_GCS_NAD83_2011)
      {
        n += sprintf(&string[n], "GEOGCS[\"NAD83(2011)\",DATUM[\"NAD_1983_2011\",SPHEROID[\"GRS 1980\",6378137,298.257222101,AUTHORITY[\"EPSG\",\"7019\"]],AUTHORITY[\"EPSG\",\"1116\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"6318\"]],");
      }
      else
      {
        n += print_ogc_wkt_geogcs(&string[n], gcs_name, gcs_code, datum_name, datum_code, spheroid_code);
      }
      // maybe geographic (long/lat)
      if (projection->type == GEO_PROJECTION_LONG_LAT)
      {
        n--; // remove comma
        n--; // remove bracket
        n += sprintf(&string[n], ",AXIS[\"Longitude\",EAST],AXIS[\"Latitude\",NORTH]]");
      }
      else if (projection->type == GEO_PROJECTION_LAT_LONG) // or maybe geographic with reversed coordinates
      {
        n--; // remove comma
        n--; // remove bracket
        n += sprintf(&string[n], ",AXIS[\"Latitude\",NORTH],AXIS[\"Longitude\",EAST]]");
      }
      else // some real projection
      {
        if (projection->type == GEO_PROJECTION_UTM)
        {
          GeoProjectionParametersUTM* utm = (GeoProjectionParametersUTM*)projection;
          n += sprintf(&string[n], "PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",%d],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",%d],", -183 + 6 * utm->utm_zone_number, (utm->utm_northern_hemisphere ? 0 : 10000000));
        }
        else if (projection->type == GEO_PROJECTION_LCC)
        {
          GeoProjectionParametersLCC* lcc = (GeoProjectionParametersLCC*)projection;
          n += sprintf(&string[n], "PROJECTION[\"Lambert_Conformal_Conic_2SP\"],PARAMETER[\"standard_parallel_1\",%.15g],PARAMETER[\"standard_parallel_2\",%.15g],PARAMETER[\"latitude_of_origin\",%.15g],PARAMETER[\"central_meridian\",%.15g],", lcc->lcc_first_std_parallel_degree, lcc->lcc_second_std_parallel_degree, lcc->lcc_lat_origin_degree, lcc->lcc_long_meridian_degree);
          if (source)
          {
            n += sprintf(&string[n], "PARAMETER[\"false_easting\",%.15g],PARAMETER[\"false_northing\",%.15g],", lcc->lcc_false_easting_meter/coordinates2meter, lcc->lcc_false_northing_meter/coordinates2meter);
          }
          else
          {
            n += sprintf(&string[n], "PARAMETER[\"false_easting\",%.15g],PARAMETER[\"false_northing\",%.15g],", lcc->lcc_false_easting_meter*meter2coordinates, lcc->lcc_false_northing_meter*meter2coordinates);
          }
        }
        else if (projection->type == GEO_PROJECTION_TM)
        {
          GeoProjectionParametersTM* tm = (GeoProjectionParametersTM*)projection;
          n += sprintf(&string[n], "PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",%.15g],PARAMETER[\"central_meridian\",%.15g],PARAMETER[\"scale_factor\",%.15g],", tm->tm_lat_origin_degree, tm->tm_long_meridian_degree, tm->tm_scale_factor);
          if (source)
          {
            n += sprintf(&string[n], "PARAMETER[\"false_easting\",%.15g],PARAMETER[\"false_northing\",%.15g],", tm->tm_false_easting_meter/coordinates2meter, tm->tm_false_northing_meter/coordinates2meter);
          }
          else
          {
            n += sprintf(&string[n], "PARAMETER[\"false_easting\",%.15g],PARAMETER[\"false_northing\",%.15g],", tm->tm_false_easting_meter*meter2coordinates, tm->tm_false_northing_meter*meter2coordinates);
          }
        }
        else if (projection->type == GEO_PROJECTION_AEAC)
        {
          GeoProjectionParametersAEAC* aeac = (GeoProjectionParametersAEAC*)projection;
          n += sprintf(&string[n], "PROJECTION[\"Albers_Conic_Equal_Area\"],PARAMETER[\"standard_parallel_1\",%.15g],PARAMETER[\"standard_parallel_2\",%.15g],PARAMETER[\"latitude_of_center\",%.15g],PARAMETER[\"longitude_of_center\",%.15g],", aeac->aeac_first_std_parallel_degree, aeac->aeac_second_std_parallel_degree, aeac->aeac_latitude_of_center_degree, aeac->aeac_longitude_of_center_degree);
          if (source)
          {
            n += sprintf(&string[n], "PARAMETER[\"false_easting\",%.15g],PARAMETER[\"false_northing\",%.15g],", aeac->aeac_false_easting_meter/coordinates2meter, aeac->aeac_false_northing_meter/coordinates2meter);
          }
          else
          {
            n += sprintf(&string[n], "PARAMETER[\"false_easting\",%.15g],PARAMETER[\"false_northing\",%.15g],", aeac->aeac_false_easting_meter*meter2coordinates, aeac->aeac_false_northing_meter*meter2coordinates);
          }
        }
        else if (projection->type == GEO_PROJECTION_HOM)
        {
          GeoProjectionParametersHOM* hom = (GeoProjectionParametersHOM*)projection;
          n += sprintf(&string[n], "PROJECTION[\"Hotine_Oblique_Mercator\"],PARAMETER[\"latitude_of_center\",%.15g],PARAMETER[\"longitude_of_center\",%.15g],PARAMETER[\"azimuth\",%.15g],PARAMETER[\"rectified_grid_angle\",%.15g],PARAMETER[\"scale_factor\",%.15g],", hom->hom_latitude_of_center_degree, hom->hom_longitude_of_center_degree, hom->hom_azimuth_degree, hom->hom_rectified_grid_angle_degree , hom->hom_scale_factor);
          if (source)
          {
            n += sprintf(&string[n], "PARAMETER[\"false_easting\",%.15g],PARAMETER[\"false_northing\",%.15g],", hom->hom_false_easting_meter/coordinates2meter, hom->hom_false_northing_meter/coordinates2meter);
          }
          else
          {
            n += sprintf(&string[n], "PARAMETER[\"false_easting\",%.15g],PARAMETER[\"false_northing\",%.15g],", hom->hom_false_easting_meter*meter2coordinates, hom->hom_false_northing_meter*meter2coordinates);
          }
        }
        else if (projection->type == GEO_PROJECTION_OS)
        {
          GeoProjectionParametersOS* os = (GeoProjectionParametersOS*)projection;
          n += sprintf(&string[n], "PROJECTION[\"Oblique_Stereographic\"],PARAMETER[\"latitude_of_origin\",%.15g],PARAMETER[\"central_meridian\",%.15g],PARAMETER[\"scale_factor\",%.15g],", os->os_lat_origin_degree, os->os_long_meridian_degree, os->os_scale_factor);
          if (source)
          {
            n += sprintf(&string[n], "PARAMETER[\"false_easting\",%.15g],PARAMETER[\"false_northing\",%.15g],", os->os_false_easting_meter/coordinates2meter, os->os_false_northing_meter/coordinates2meter);
          }
          else
          {
            n += sprintf(&string[n], "PARAMETER[\"false_easting\",%.15g],PARAMETER[\"false_northing\",%.15g],", os->os_false_easting_meter*meter2coordinates, os->os_false_northing_meter*meter2coordinates);
          }
        }
        else
        {
          free(string);
          len = 0;
          *ogc_wkt = 0;
          return false;
        }
        // units
        if (source)
        {
          if (coordinates2meter == 1.0)
          {
            n += sprintf(&string[n], "UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],");
          }
          else if (coordinates2meter == 0.3048)
          {
            n += sprintf(&string[n], "UNIT[\"foot\",0.3048,AUTHORITY[\"EPSG\",\"9002\"]]");
          }
          else
          {
            n += sprintf(&string[n], "UNIT[\"US survey foot\",0.3048006096012192,AUTHORITY[\"EPSG\",\"9003\"]],");
          }
        }
        else
        {
          if (meter2coordinates == 1.0)
          {
            n += sprintf(&string[n], "UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],");
          }
          else if (meter2coordinates == 0.3048)
          {
            n += sprintf(&string[n], "UNIT[\"foot\",0.3048,AUTHORITY[\"EPSG\",\"9002\"]]");
          }
          else
          {
            n += sprintf(&string[n], "UNIT[\"US survey foot\",0.3048006096012192,AUTHORITY[\"EPSG\",\"9003\"]],");
          }
        }
        // axis
        n += sprintf(&string[n], "AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH],");
        // authority
        if (projection->geokey == 0)
        {
          projection->geokey = get_ProjectedCSTypeGeoKey(source);
        }
        if (projection->geokey)
        {
          n += sprintf(&string[n], "AUTHORITY[\"EPSG\",\"%d\"]]", projection->geokey);
        }
        else
        {
          n += sprintf(&string[n], "]");
        }
      }
      if ((vertical_geokey == GEO_VERTICAL_NAVD88) || (vertical_geokey == GEO_VERTICAL_NGVD29) || (vertical_geokey == GEO_VERTICAL_CGVD2013) || (vertical_geokey == GEO_VERTICAL_EVRF2007) || (vertical_geokey == GEO_VERTICAL_CGVD28) || (vertical_geokey == GEO_VERTICAL_DVR90) || (vertical_geokey == GEO_VERTICAL_NN2000) || (vertical_geokey == GEO_VERTICAL_NN54) || (vertical_geokey == GEO_VERTICAL_DHHN92) || (vertical_geokey == GEO_VERTICAL_DHHN2016) || (vertical_geokey == GEO_VERTICAL_NZVD2016))
      {
        // comma for compound CRS
        n += sprintf(&string[n], ",");
        // add VERT_CS info
        if (vertical_geokey == GEO_VERTICAL_NAVD88)
        {
          n += sprintf(&string[n], "VERT_CS[\"NAVD88");
          if (vertical_geoid)
          {
            if (vertical_geoid == GEO_VERTICAL_NAVD88_GEOID12B)
            {
              n += sprintf(&string[n], " height - Geoid12B");
            }
            else if (vertical_geoid ==  GEO_VERTICAL_NAVD88_GEOID12A)
            {
              n += sprintf(&string[n], " height - Geoid12A");
            }
            else if (vertical_geoid ==  GEO_VERTICAL_NAVD88_GEOID12)
            {
              n += sprintf(&string[n], " height - Geoid12");
            }
            else if (vertical_geoid ==  GEO_VERTICAL_NAVD88_GEOID09)
            {
              n += sprintf(&string[n], " height - Geoid09");
            }
            else if (vertical_geoid ==  GEO_VERTICAL_NAVD88_GEOID06)
            {
              n += sprintf(&string[n], " height - Geoid06");
            }
            else if (vertical_geoid ==  GEO_VERTICAL_NAVD88_GEOID03)
            {
              n += sprintf(&string[n], " height - Geoid03");
            }
            else if (vertical_geoid ==  GEO_VERTICAL_NAVD88_GEOID99)
            {
              n += sprintf(&string[n], " height - Geoid99");
            }
            else if (vertical_geoid == GEO_VERTICAL_NAVD88_GEOID96)
            {
              n += sprintf(&string[n], " height - Geoid96");
            }
          }
          n += sprintf(&string[n], "\",VERT_DATUM[\"North American Vertical Datum 1988\",2005,AUTHORITY[\"EPSG\",\"5103\"]],");
        }
        else if (vertical_geokey == GEO_VERTICAL_NGVD29)
        {
          n += sprintf(&string[n], "VERT_CS[\"NGVD29\",VERT_DATUM[\"National Geodetic Vertical Datum 1929\",2005,AUTHORITY[\"EPSG\",\"5102\"]],");
        }
        else if (vertical_geokey == GEO_VERTICAL_CGVD2013)
        {
          n += sprintf(&string[n], "VERT_CS[\"CGVD2013\",VERT_DATUM[\"Canadian Geodetic Vertical Datum of 2013\",2005,AUTHORITY[\"EPSG\",\"1127\"]],");
        }
        else if (vertical_geokey == GEO_VERTICAL_EVRF2007)
        {
          n += sprintf(&string[n], "VERT_CS[\"EVRF2007\",VERT_DATUM[\"European Vertical Reference Frame 2007\",2005,AUTHORITY[\"EPSG\",\"5215\"]],");
        }
        else if (vertical_geokey == GEO_VERTICAL_CGVD28)
        {
          n += sprintf(&string[n], "VERT_CS[\"CGVD28\",VERT_DATUM[\"Canadian Geodetic Vertical Datum of 1928\",2005,AUTHORITY[\"EPSG\",\"5114\"]],");
        }
        else if (vertical_geokey == GEO_VERTICAL_DVR90)
        {
          n += sprintf(&string[n], "VERT_CS[\"DVR90\",VERT_DATUM[\"Dansk Vertikal Reference 1990\",2005,AUTHORITY[\"EPSG\",\"5206\"]],");
        }
        else if (vertical_geokey == GEO_VERTICAL_NN2000)
        {
          n += sprintf(&string[n], "VERT_CS[\"NN2000\",VERT_DATUM[\"Norway Normal Null 2000\",2005,AUTHORITY[\"EPSG\",\"1096\"]],");
        }
        else if (vertical_geokey == GEO_VERTICAL_NN54)
        {
          n += sprintf(&string[n], "VERT_CS[\"NN54\",VERT_DATUM[\"Norway Normal Null 1954\",2005,AUTHORITY[\"EPSG\",\"5174\"]],");
        }
        else if (vertical_geokey == GEO_VERTICAL_DHHN92)
        {
          n += sprintf(&string[n], "VERT_CS[\"DHHN92\",VERT_DATUM[\"Deutsches Haupthoehennetz 1992\",2005,AUTHORITY[\"EPSG\",\"5783\"]],");
        }
        else if (vertical_geokey == GEO_VERTICAL_DHHN2016)
        {
          n += sprintf(&string[n], "VERT_CS[\"DHHN2016\",VERT_DATUM[\"Deutsches Haupthoehennetz 2016\",2005,AUTHORITY[\"EPSG\",\"7837\"]],");
        }
        else if (vertical_geokey == GEO_VERTICAL_NZVD2016)
        {
          n += sprintf(&string[n], "VERT_CS[\"NZVD2016\",VERT_DATUM[\"New Zealand Vertical Datum 2016\",2005,AUTHORITY[\"EPSG\",\"7839\"]],");
        }
        if (source)
        {
          if (elevation2meter == 1.0)
          {
            n += sprintf(&string[n], "UNIT[\"metre\",1.0,AUTHORITY[\"EPSG\",\"9001\"]],");
          }
          else if (elevation2meter == 0.3048)
          {
            n += sprintf(&string[n], "UNIT[\"foot\",0.3048,AUTHORITY[\"EPSG\",\"9002\"]],");
          }
          else
          {
            n += sprintf(&string[n], "UNIT[\"US survey foot\",0.3048006096012192,AUTHORITY[\"EPSG\",\"9003\"]],");
          }
        }
        else
        {
          if (meter2elevation == 1.0)
          {
            n += sprintf(&string[n], "UNIT[\"metre\",1.0,AUTHORITY[\"EPSG\",\"9001\"]],");
          }
          else if (meter2elevation == 0.3048)
          {
            n += sprintf(&string[n], "UNIT[\"foot\",0.3048,AUTHORITY[\"EPSG\",\"9002\"]],");
          }
          else
          {
            n += sprintf(&string[n], "UNIT[\"US survey foot\",0.3048006096012192,AUTHORITY[\"EPSG\",\"9003\"]],");
          }
        }
        if (vertical_geokey == GEO_VERTICAL_NAVD88)
        {
          n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"%d\"]]", ((source && (elevation2meter == 1.0)) || (!source && (meter2elevation == 1.0)) ? 5703 : 6360));
        }
        else if (vertical_geokey == GEO_VERTICAL_NGVD29)
        {
          n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"5702\"]]");
        }
        else if (vertical_geokey == GEO_VERTICAL_CGVD2013)
        {
          n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"6647\"]]");
        }
        else if (vertical_geokey == GEO_VERTICAL_EVRF2007)
        {
          n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"5621\"]]");
        }
        else if (vertical_geokey == GEO_VERTICAL_CGVD28)
        {
          n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"5713\"]]");
        }
        else if (vertical_geokey == GEO_VERTICAL_DVR90)
        {
          n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"5799\"]]");
        }
        else if (vertical_geokey == GEO_VERTICAL_NN2000)
        {
          n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"5941\"]]");
        }
        else if (vertical_geokey == GEO_VERTICAL_NN54)
        {
          n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"5776\"]]");
        }
        else if (vertical_geokey == GEO_VERTICAL_DHHN92)
        {
          n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"5783\"]]");
        }
        else if (vertical_geokey == GEO_VERTICAL_DHHN2016)
        {
          n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"7837\"]]");
        }
        else if (vertical_geokey == GEO_VERTICAL_NZVD2016)
        {
          n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"7839\"]]");
        }
        // close bracket for compound CRS
        n += sprintf(&string[n], "]");
      }
    }
    len = n+1;
    *ogc_wkt = string;
    return true;
  }
  len = 0;
  *ogc_wkt = 0;
  return false;
}

static int print_prj_spheroid(char* string, short spheroid_code)
{
  int n = 0;

  if (spheroid_code == GEO_SPHEROID_WGS84)
  {
    n = sprintf(string, "SPHEROID[\"WGS 84\",6378137,298.257223563]");
  }
  else if (spheroid_code == GEO_SPHEROID_GRS80)
  {
    n = sprintf(string, "SPHEROID[\"GRS 1980\",6378137,298.257222101]");
  }
  else if (spheroid_code == GEO_SPHEROID_WGS72)
  {
    n = sprintf(string, "SPHEROID[\"WGS 72\",6378135,298.26]");
  }
  else if (spheroid_code == GEO_SPHEROID_GRS67)
  {
    n = sprintf(string, "SPHEROID[\"GRS 1967\",6378160,298.247167427]");
  }
  else if (spheroid_code == GEO_SPHEROID_CLARKE1866)
  {
    n = sprintf(string, "SPHEROID[\"Clarke 1866\",6378206.4,294.9786982139006]");
  }
  else if (spheroid_code == GEO_SPHEROID_INTERNATIONAL)
  {
    n = sprintf(string, "SPHEROID[\"International 1924\",6378388,297]");
  }
  else if (spheroid_code == GEO_SPHEROID_BESSEL1841)
  {
    n = sprintf(string, "SPHEROID[\"Bessel 1841\",6377397.155,299.1528128]");
  }
  else if (spheroid_code == GEO_SPHEROID_AIRY)
  {
    n = sprintf(string, "SPHEROID[\"Airy 1830\",6377563.396,299.3249646]");
  }
  return n;
}

static int print_prj_datum(char* string, const char* datum_name, short datum_code, short spheroid_code)
{
  int n = 0;

  if (datum_code == (GEO_GCS_WGS84 + 2000))
  {
    n += sprintf(&string[n], "DATUM[\"D_WGS_1984\",");
  }
  else
  {
    n += sprintf(&string[n], "DATUM[\"D_%s\",", datum_name);
  }
  n += print_prj_spheroid(&string[n], spheroid_code);
  n += sprintf(&string[n], "],");

  return n;
}

static int print_prj_geogcs(char* string, const char* gcs_name, short gcs_code, const char* datum_name, short datum_code, short spheroid_code)
{
  int n = 0;

  n += sprintf(&string[n], "GEOGCS[\"%s\",", gcs_name);
  n += print_prj_datum(&string[n], datum_name, datum_code, spheroid_code);
  n += sprintf(&string[n], "PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],");

  return n;
}

/* non-zero *prj returns char[] that becomes property of caller. dealloc with free() */
bool GeoProjectionConverter::get_prj_from_projection(int& len, char** prj, bool source)
{
  GeoProjectionParameters* projection = (source ? source_projection : target_projection);
  if (projection)
  {
    int n = 0;
    char* string = (char*)malloc(4096);
    memset(string, 0, 4096);
    // maybe geocentric
    if (projection->type == GEO_PROJECTION_ECEF)
    {
      n += sprintf(&string[n], "GEOCCS[\"WGS 84\",DATUM[\"World Geodetic System 1984\",SPHEROID[\"WGS 84\",6378137.0,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0.0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"m\",1.0],AXIS[\"Geocentric X\",OTHER],AXIS[\"Geocentric Y\",EAST],AXIS[\"Geocentric Z\",NORTH],AUTHORITY[\"EPSG\",\"4978\"]]");
    }
    else
    {
      // if not geographic we have a projection
      if ((projection->type != GEO_PROJECTION_LAT_LONG) && (projection->type != GEO_PROJECTION_LONG_LAT))
      {
        if (strlen(projection->name) == 0)
        {
          char* epsg_name = get_epsg_name_from_pcs_file(argv_zero, projection->geokey);
          if (epsg_name)
          {
            n += sprintf(&string[n], "PROJCS[\"%s\",", epsg_name);
            free(epsg_name);
          }
        }
        else
        {
          n += sprintf(&string[n], "PROJCS[\"%s\",", projection->name);
        }
      }
      // which datum
      if (gcs_code == GEO_GCS_WGS84)
      {
        n += sprintf(&string[n], "GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],");
      }
      else if ((gcs_code == GEO_GCS_NAD83) || (gcs_code == GEO_GCS_NAD83_HARN) || (gcs_code == GEO_GCS_NAD83_CSRS) || (gcs_code == GEO_GCS_NAD83_PA11) || (gcs_code == GEO_GCS_NAD83_2011))
      {
        if (gcs_code == GEO_GCS_NAD83)
        {
          n += sprintf(&string[n], "GEOGCS[\"GCS_North_American_1983\",DATUM[\"D_North_American_1983\",");
        }
        else if (gcs_code == GEO_GCS_NAD83_HARN)
        {
          n += sprintf(&string[n], "GEOGCS[\"NAD83(HARN)\",DATUM[\"D_North_American_1983_HARN\",");
        }
        else if (gcs_code == GEO_GCS_NAD83_CSRS)
        {
          n += sprintf(&string[n], "GEOGCS[\"NAD83(CSRS)\",DATUM[\"D_NAD83_Canadian_Spatial_Reference_System\",");
        }
        else if (gcs_code == GEO_GCS_NAD83_PA11)
        {
          n += sprintf(&string[n], "GEOGCS[\"NAD83(PA11)\",DATUM[\"D_North_American_1983_PA11\",");
        }
        else
        {
          n += sprintf(&string[n], "GEOGCS[\"NAD83(2011)\",DATUM[\"D_North_American_1983_2011\",");
        }
        n += sprintf(&string[n], "SPHEROID[\"GRS 1980\",6378137,298.257222101]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],");
      }
      else
      {
        n += print_prj_geogcs(&string[n], gcs_name, gcs_code, datum_name, datum_code, spheroid_code);
      }
      // maybe geographic (long/lat)
      if (projection->type == GEO_PROJECTION_LONG_LAT)
      {
        n--; // remove comma
        n--; // remove bracket
        n += sprintf(&string[n], ",AXIS[\"Longitude\",EAST],AXIS[\"Latitude\",NORTH]]");
      }
      else if (projection->type == GEO_PROJECTION_LAT_LONG) // or maybe geographic with reversed coordinates
      {
        n--; // remove comma
        n--; // remove bracket
        n += sprintf(&string[n], ",AXIS[\"Latitude\",NORTH],AXIS[\"Longitude\",EAST]]");
      }
      else // some real projection
      {
        if (projection->type == GEO_PROJECTION_UTM)
        {
          GeoProjectionParametersUTM* utm = (GeoProjectionParametersUTM*)projection;
          n += sprintf(&string[n], "PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",%d],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",%d],", -183 + 6 * utm->utm_zone_number, (utm->utm_northern_hemisphere ? 0 : 10000000));
        }
        else if (projection->type == GEO_PROJECTION_LCC)
        {
          GeoProjectionParametersLCC* lcc = (GeoProjectionParametersLCC*)projection;
          n += sprintf(&string[n], "PROJECTION[\"Lambert_Conformal_Conic_2SP\"],PARAMETER[\"standard_parallel_1\",%.15g],PARAMETER[\"standard_parallel_2\",%.15g],PARAMETER[\"latitude_of_origin\",%.15g],PARAMETER[\"central_meridian\",%.15g],", lcc->lcc_first_std_parallel_degree, lcc->lcc_second_std_parallel_degree, lcc->lcc_lat_origin_degree, lcc->lcc_long_meridian_degree);
          if (source)
          {
            n += sprintf(&string[n], "PARAMETER[\"false_easting\",%.15g],PARAMETER[\"false_northing\",%.15g],", lcc->lcc_false_easting_meter/coordinates2meter, lcc->lcc_false_northing_meter/coordinates2meter);
          }
          else
          {
            n += sprintf(&string[n], "PARAMETER[\"false_easting\",%.15g],PARAMETER[\"false_northing\",%.15g],", lcc->lcc_false_easting_meter*meter2coordinates, lcc->lcc_false_northing_meter*meter2coordinates);
          }
        }
        else if (projection->type == GEO_PROJECTION_TM)
        {
          GeoProjectionParametersTM* tm = (GeoProjectionParametersTM*)projection;
          n += sprintf(&string[n], "PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",%.15g],PARAMETER[\"central_meridian\",%.15g],PARAMETER[\"scale_factor\",%.15g],", tm->tm_lat_origin_degree, tm->tm_long_meridian_degree, tm->tm_scale_factor);
          if (source)
          {
            n += sprintf(&string[n], "PARAMETER[\"false_easting\",%.15g],PARAMETER[\"false_northing\",%.15g],", tm->tm_false_easting_meter/coordinates2meter, tm->tm_false_northing_meter/coordinates2meter);
          }
          else
          {
            n += sprintf(&string[n], "PARAMETER[\"false_easting\",%.15g],PARAMETER[\"false_northing\",%.15g],", tm->tm_false_easting_meter*meter2coordinates, tm->tm_false_northing_meter*meter2coordinates);
          }
        }
        else if (projection->type == GEO_PROJECTION_AEAC)
        {
          GeoProjectionParametersAEAC* aeac = (GeoProjectionParametersAEAC*)projection;
          n += sprintf(&string[n], "PROJECTION[\"Albers_Conic_Equal_Area\"],PARAMETER[\"standard_parallel_1\",%.15g],PARAMETER[\"standard_parallel_2\",%.15g],PARAMETER[\"latitude_of_center\",%.15g],PARAMETER[\"longitude_of_center\",%.15g],", aeac->aeac_first_std_parallel_degree, aeac->aeac_second_std_parallel_degree, aeac->aeac_latitude_of_center_degree, aeac->aeac_longitude_of_center_degree);
          if (source)
          {
            n += sprintf(&string[n], "PARAMETER[\"false_easting\",%.15g],PARAMETER[\"false_northing\",%.15g],", aeac->aeac_false_easting_meter/coordinates2meter, aeac->aeac_false_northing_meter/coordinates2meter);
          }
          else
          {
            n += sprintf(&string[n], "PARAMETER[\"false_easting\",%.15g],PARAMETER[\"false_northing\",%.15g],", aeac->aeac_false_easting_meter*meter2coordinates, aeac->aeac_false_northing_meter*meter2coordinates);
          }
        }
        else if (projection->type == GEO_PROJECTION_HOM)
        {
          GeoProjectionParametersHOM* hom = (GeoProjectionParametersHOM*)projection;
          n += sprintf(&string[n], "PROJECTION[\"Hotine_Oblique_Mercator\"],PARAMETER[\"latitude_of_center\",%.15g],PARAMETER[\"longitude_of_center\",%.15g],PARAMETER[\"azimuth\",%.15g],PARAMETER[\"rectified_grid_angle\",%.15g],PARAMETER[\"scale_factor\",%.15g],", hom->hom_latitude_of_center_degree, hom->hom_longitude_of_center_degree, hom->hom_azimuth_degree, hom->hom_rectified_grid_angle_degree , hom->hom_scale_factor);
          if (source)
          {
            n += sprintf(&string[n], "PARAMETER[\"false_easting\",%.15g],PARAMETER[\"false_northing\",%.15g],", hom->hom_false_easting_meter/coordinates2meter, hom->hom_false_northing_meter/coordinates2meter);
          }
          else
          {
            n += sprintf(&string[n], "PARAMETER[\"false_easting\",%.15g],PARAMETER[\"false_northing\",%.15g],", hom->hom_false_easting_meter*meter2coordinates, hom->hom_false_northing_meter*meter2coordinates);
          }
        }
        else if (projection->type == GEO_PROJECTION_OS)
        {
          GeoProjectionParametersOS* os = (GeoProjectionParametersOS*)projection;
          n += sprintf(&string[n], "PROJECTION[\"Oblique_Stereographic\"],PARAMETER[\"latitude_of_origin\",%.15g],PARAMETER[\"central_meridian\",%.15g],PARAMETER[\"scale_factor\",%.15g],", os->os_lat_origin_degree, os->os_long_meridian_degree, os->os_scale_factor);
          if (source)
          {
            n += sprintf(&string[n], "PARAMETER[\"false_easting\",%.15g],PARAMETER[\"false_northing\",%.15g],", os->os_false_easting_meter/coordinates2meter, os->os_false_northing_meter/coordinates2meter);
          }
          else
          {
            n += sprintf(&string[n], "PARAMETER[\"false_easting\",%.15g],PARAMETER[\"false_northing\",%.15g],", os->os_false_easting_meter*meter2coordinates, os->os_false_northing_meter*meter2coordinates);
          }
        }
        else
        {
          free(string);
          len = 0;
          *prj = 0;
          return false;
        }
        // units
        if (source)
        {
          if (coordinates2meter == 1.0)
          {
            n += sprintf(&string[n], "UNIT[\"Meter\",1]");
          }
          else if (coordinates2meter == 0.3048)
          {
            n += sprintf(&string[n], "UNIT[\"foot\",0.3048]");
          }
          else
          {
            n += sprintf(&string[n], "UNIT[\"US survey foot\",0.3048006096012192]");
          }
        }
        else
        {
          if (meter2coordinates == 1.0)
          {
            n += sprintf(&string[n], "UNIT[\"Meter\",1]");
          }
          else if (meter2coordinates == 0.3048)
          {
            n += sprintf(&string[n], "UNIT[\"foot\",0.3048]");
          }
          else
          {
            n += sprintf(&string[n], "UNIT[\"US survey foot\",0.3048006096012192]");
          }
        }
        n += sprintf(&string[n], "]");
      }
    }
    if ((vertical_geokey == GEO_VERTICAL_NAVD88) || (vertical_geokey == GEO_VERTICAL_NGVD29) || (vertical_geokey == GEO_VERTICAL_CGVD2013) || (vertical_geokey == GEO_VERTICAL_EVRF2007) || (vertical_geokey == GEO_VERTICAL_CGVD28) || (vertical_geokey == GEO_VERTICAL_DVR90) || (vertical_geokey == GEO_VERTICAL_NN2000) || (vertical_geokey == GEO_VERTICAL_NN54) || (vertical_geokey == GEO_VERTICAL_DHHN92) || (vertical_geokey == GEO_VERTICAL_DHHN2016) || (vertical_geokey == GEO_VERTICAL_NZVD2016))
    {
      if (vertical_geokey == GEO_VERTICAL_NAVD88)
      {
        n += sprintf(&string[n], "VERT_CS[\"NAVD88\",VERT_DATUM[\"North American Vertical Datum 1988\",2005,AUTHORITY[\"EPSG\",\"5103\"]],");
      }
      else if (vertical_geokey == GEO_VERTICAL_NGVD29)
      {
        n += sprintf(&string[n], "VERT_CS[\"NGVD29\",VERT_DATUM[\"National Geodetic Vertical Datum 1929\",2005,AUTHORITY[\"EPSG\",\"5102\"]],");
      }
      else if (vertical_geokey == GEO_VERTICAL_CGVD2013)
      {
        n += sprintf(&string[n], "VERT_CS[\"CGVD2013\",VERT_DATUM[\"Canadian Geodetic Vertical Datum of 2013\",2005,AUTHORITY[\"EPSG\",\"1127\"]],");
      }
      else if (vertical_geokey == GEO_VERTICAL_EVRF2007)
      {
        n += sprintf(&string[n], "VERT_CS[\"EVRF2007\",VERT_DATUM[\"European Vertical Reference Frame 2007\",2005,AUTHORITY[\"EPSG\",\"5215\"]],");
      }
      else if (vertical_geokey == GEO_VERTICAL_CGVD28)
      {
        n += sprintf(&string[n], "VERT_CS[\"CGVD28\",VERT_DATUM[\"Canadian Geodetic Vertical Datum of 1928\",2005,AUTHORITY[\"EPSG\",\"5114\"]],");
      }
      else if (vertical_geokey == GEO_VERTICAL_DVR90)
      {
        n += sprintf(&string[n], "VERT_CS[\"DVR90\",VERT_DATUM[\"Dansk Vertikal Reference 1990\",2005,AUTHORITY[\"EPSG\",\"5206\"]],");
      }
      else if (vertical_geokey == GEO_VERTICAL_NN2000)
      {
        n += sprintf(&string[n], "VERT_CS[\"NN2000\",VERT_DATUM[\"Norway Normal Null 2000\",2005,AUTHORITY[\"EPSG\",\"1096\"]],");
      }
      else if (vertical_geokey == GEO_VERTICAL_NN54)
      {
        n += sprintf(&string[n], "VERT_CS[\"NN54\",VERT_DATUM[\"Norway Normal Null 1954\",2005,AUTHORITY[\"EPSG\",\"5174\"]],");
      }
      else if (vertical_geokey == GEO_VERTICAL_DHHN92)
      {
        n += sprintf(&string[n], "VERT_CS[\"DHHN92\",VERT_DATUM[\"Deutsches Haupthoehennetz 1992\",2005,AUTHORITY[\"EPSG\",\"5783\"]],");
      }
      else if (vertical_geokey == GEO_VERTICAL_DHHN2016)
      {
        n += sprintf(&string[n], "VERT_CS[\"DHHN2016\",VERT_DATUM[\"Deutsches Haupthoehennetz 2016\",2005,AUTHORITY[\"EPSG\",\"7837\"]],");
      }
      else if (vertical_geokey == GEO_VERTICAL_NZVD2016)
      {
        n += sprintf(&string[n], "VERT_CS[\"NZVD2016\",VERT_DATUM[\"New Zealand Vertical Datum 2016\",2005,AUTHORITY[\"EPSG\",\"7839\"]],");
      }
      if (source)
      {
        if (elevation2meter == 1.0)
        {
          n += sprintf(&string[n], "UNIT[\"metre\",1.0],");
        }
        else if (elevation2meter == 0.3048)
        {
          n += sprintf(&string[n], "UNIT[\"foot\",0.3048],");
        }
        else
        {
          n += sprintf(&string[n], "UNIT[\"US survey foot\",0.3048006096012192],");
        }
      }
      else
      {
        if (meter2elevation == 1.0)
        {
          n += sprintf(&string[n], "UNIT[\"metre\",1.0],");
        }
        else if (meter2elevation == 0.3048)
        {
          n += sprintf(&string[n], "UNIT[\"foot\",0.3048],");
        }
        else
        {
          n += sprintf(&string[n], "UNIT[\"US survey foot\",0.3048006096012192],");
        }
      }
      if (vertical_geokey == GEO_VERTICAL_NAVD88)
      {
        n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"%d\"]]", ((source && (elevation2meter == 1.0)) || (!source && (meter2elevation == 1.0)) ? 5703 : 6360));
      }
      else if (vertical_geokey == GEO_VERTICAL_NGVD29)
      {
        n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"5702\"]]");
      }
      else if (vertical_geokey == GEO_VERTICAL_CGVD2013)
      {
        n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"6647\"]]");
      }
      else if (vertical_geokey == GEO_VERTICAL_EVRF2007)
      {
        n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"5621\"]]");
      }
      else if (vertical_geokey == GEO_VERTICAL_CGVD28)
      {
        n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"5713\"]]");
      }
      else if (vertical_geokey == GEO_VERTICAL_DVR90)
      {
        n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"5799\"]]");
      }
      else if (vertical_geokey == GEO_VERTICAL_NN2000)
      {
        n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"5941\"]]");
      }
      else if (vertical_geokey == GEO_VERTICAL_NN54)
      {
        n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"5776\"]]");
      }
      else if (vertical_geokey == GEO_VERTICAL_DHHN92)
      {
        n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"5783\"]]");
      }
      else if (vertical_geokey == GEO_VERTICAL_DHHN2016)
      {
        n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"7837\"]]");
      }
      else if (vertical_geokey == GEO_VERTICAL_NZVD2016)
      {
        n += sprintf(&string[n], "AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"7837\"]]");
      }
    }
    len = n+1;
    *prj = string;
    return true;
  }
  len = 0;
  *prj = 0;
  return false;
}

/* non-zero *proj4 returns char[] that becomes property of caller. dealloc with free() */
bool GeoProjectionConverter::get_proj4_string_from_projection(int& len, char** proj4, bool source)
{
  GeoProjectionParameters* projection = (source ? source_projection : target_projection);
  if (projection)
  {
    int n = 0;
    char* string = (char*)malloc(1024);
    memset(string, 0, 1024);
    if (projection->type == GEO_PROJECTION_UTM)
    {
      GeoProjectionParametersUTM* utm = (GeoProjectionParametersUTM*)projection;
      n += sprintf(&string[n], "+proj=utm +zone=%d%s ", utm->utm_zone_number, (utm->utm_northern_hemisphere ? "" : " +south"));
    }
    else if (projection->type == GEO_PROJECTION_LCC)
    {
      GeoProjectionParametersLCC* lcc = (GeoProjectionParametersLCC*)projection;
      n += sprintf(&string[n], "+proj=lcc +lat_1=%.15g +lat_2=%.15g +lat_0=%.15g +lon_0=%.15g +x_0=%.15g +y_0=%.15g ", lcc->lcc_first_std_parallel_degree, lcc->lcc_second_std_parallel_degree, lcc->lcc_lat_origin_degree, lcc->lcc_long_meridian_degree, lcc->lcc_false_easting_meter, lcc->lcc_false_northing_meter);
    }
    else if (projection->type == GEO_PROJECTION_TM)
    {
      GeoProjectionParametersTM* tm = (GeoProjectionParametersTM*)projection;
      n += sprintf(&string[n], "+proj=tmerc +lat_0=%.15g +lon_0=%.15g +k=%.15g +x_0=%.15g +y_0=%.15g ", tm->tm_lat_origin_degree, tm->tm_long_meridian_degree, tm->tm_scale_factor, tm->tm_false_easting_meter, tm->tm_false_northing_meter);
    }
    else if (projection->type == GEO_PROJECTION_AEAC)
    {
      GeoProjectionParametersAEAC* aeac = (GeoProjectionParametersAEAC*)projection;
      n += sprintf(&string[n], "+proj=albers +lat_1=%.15g +lat_2=%.15g +lat_0=%.15g +lon_0=%.15g +x_0=%.15g +y_0=%.15g ", aeac->aeac_first_std_parallel_degree, aeac->aeac_second_std_parallel_degree, aeac->aeac_latitude_of_center_degree, aeac->aeac_longitude_of_center_degree, aeac->aeac_false_easting_meter, aeac->aeac_false_northing_meter);
    }
    else if (projection->type == GEO_PROJECTION_HOM)
    {
      GeoProjectionParametersHOM* hom = (GeoProjectionParametersHOM*)projection;
      n += sprintf(&string[n], "+proj=omerc +lat_0=%.15g +lonc=%.15g +alpha=%.15g +gamma=%.15g +k=%.15g +x_0=%.15g +y_0=%.15g ", hom->hom_latitude_of_center_degree, hom->hom_longitude_of_center_degree, hom->hom_azimuth_degree, hom->hom_rectified_grid_angle_degree, hom->hom_scale_factor, hom->hom_false_easting_meter, hom->hom_false_northing_meter);
    }
    else if (projection->type == GEO_PROJECTION_OS)
    {
      GeoProjectionParametersOS* os = (GeoProjectionParametersOS*)projection;
      n += sprintf(&string[n], "+proj=sterea +lat_0=%.15g +lon_0=%.15g +k=%.15g +x_0=%.15g +y_0=%.15g ", os->os_lat_origin_degree, os->os_long_meridian_degree, os->os_scale_factor, os->os_false_easting_meter, os->os_false_northing_meter);
    }
    else if (projection->type == GEO_PROJECTION_LONG_LAT)
    {
      n += sprintf(&string[n], "+proj=longlat ");
    }
    else if (projection->type == GEO_PROJECTION_LAT_LONG)
    {
      n += sprintf(&string[n], "+proj=latlong ");
    }
    else if (projection->type == GEO_PROJECTION_ECEF)
    {
      n += sprintf(&string[n], "+proj=geocent ");
    }
    else
    {
      free(string);
      len = 0;
      *proj4 = 0;
      return false;
    }
    if (gcs_code == GEO_GCS_WGS84)
    {
      n += sprintf(&string[n], "+datum=WGS84 ");
    }
    else if ((gcs_code == GEO_GCS_NAD83) || (gcs_code == GEO_GCS_NAD83_HARN) || (gcs_code == GEO_GCS_NAD83_CSRS)|| (gcs_code == GEO_GCS_NAD83_PA11) || (gcs_code == GEO_GCS_NAD83_2011) || (gcs_code == GEO_GCS_NAD83_NSRS2007))
    {
      n += sprintf(&string[n], "+datum=NAD83 ");
    }
    else if (gcs_code == GEO_GCS_WGS72)
    {
      n += sprintf(&string[n], "+datum=WGS72 ");
    }
    else if (gcs_code == GEO_GCS_NAD27)
    {
      n += sprintf(&string[n], "+datum=NAD27 ");
    }
    else if (ellipsoid->id == GEO_ELLIPSOID_WGS84)
    {
      n += sprintf(&string[n], "+ellps=WGS84 ");
    }
    else if (ellipsoid->id == GEO_ELLIPSOID_GRS1980)
    {
      n += sprintf(&string[n], "+datum=NAD83 ");
    }
    else if (ellipsoid->id == GEO_ELLIPSOID_WGS72)
    {
      n += sprintf(&string[n], "+ellps=WGS72 ");
    }
    else if (ellipsoid->id == GEO_ELLIPSOID_CLARKE1866)
    {
      n += sprintf(&string[n], "+datum=NAD27 ");
    }
    else if (ellipsoid->id == GEO_ELLIPSOID_BESSEL_1841)
    {
      n += sprintf(&string[n], "+datum=bessel ");
    }
    else
    {
      free(string);
      len = 0;
      *proj4 = 0;
      return false;
    }
    if (has_coordinate_units(source))
    {
      if (source)
      {
        if (coordinates2meter == 1.0)
        {
          n += sprintf(&string[n], "+units=m ");
        }
        else
        {
          n += sprintf(&string[n], "+to_meter=%.15g ", coordinates2meter);
        }
      }
      else
      {
        if (meter2coordinates == 1.0)
        {
          n += sprintf(&string[n], "+units=m ");
        }
        else
        {
          n += sprintf(&string[n], "+to_meter=%.15g ", meter2coordinates);
        }
      }
    }
    n += sprintf(&string[n], "+no_defs");
    len = n;
    *proj4 = string;
    return true;
  }
  len = 0;
  *proj4 = 0;
  return false;
}

bool GeoProjectionConverter::set_GTModelTypeGeoKey(short value, char* description)
{
  if (value == 2) // ModelTypeGeographic
  {
    return set_longlat_projection(description);
  }
  else if (value == 3) // ModelTypeGeocentric
  {
    return set_ecef_projection(description);
  }
  else if (value == 0) // ModelTypeUndefined
  {
    return set_no_projection(description);
  }
  return false;
}

short GeoProjectionConverter::get_GTModelTypeGeoKey() const
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
  if (source_projection)
  {
    if ((source_projection->type == GEO_PROJECTION_LONG_LAT) || (source_projection->type == GEO_PROJECTION_LAT_LONG))
    {
      return 2; // ModelTypeGeographic
    }
    if ((source_projection->type == GEO_PROJECTION_LCC) || (source_projection->type == GEO_PROJECTION_TM) || (source_projection->type == GEO_PROJECTION_AEAC) || (source_projection->type == GEO_PROJECTION_HOM) || (source_projection->type == GEO_PROJECTION_OS))
    {
      return 1; // ModelTypeProjected
    }
  }
  return 0; // ModelTypeUndefined
}

short GeoProjectionConverter::get_GTRasterTypeGeoKey() const
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

short GeoProjectionConverter::get_GeographicTypeGeoKey() const
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
  // if not in geo keys maybe GCS code was set
  if (gcs_code != -1)
  {
    return gcs_code;
  }
  switch (ellipsoid->id)
  {
  case GEO_ELLIPSOID_AIRY: // GCSE_Airy1830
    return 4001;
  case 2: // GCSE_AustralianNationalSpheroid
    return 4003;
  case GEO_ELLIPSOID_BESSEL_1841: // GCSE_Bessel1841
    return 4004;
  case GEO_ELLIPSOID_BESSEL_NAMIBIA: // GCSE_BesselNamibia
    return 4006;
  case GEO_ELLIPSOID_CLARKE1866: // GCS_NAD27
    return 4267;
  case GEO_ELLIPSOID_CLARKE1880: // GCSE_Clarke1880
    return 4034;
  case GEO_ELLIPSOID_GRS1980: // GCS_NAD83
    return 4269;
  case 12: // GCSE_Helmert1906
    return 4020;
  case GEO_ELLIPSOID_INTERNATIONAL: // GCSE_International1924
    return 4022;
  case GEO_ELLIPSOID_KRASSOWSKY: // GCSE_Krassowsky1940
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

short GeoProjectionConverter::get_GeogGeodeticDatumGeoKey() const
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
  // if not in geo keys derive from GCS code
  if (gcs_code != -1)
  {
    return gcs_code + 2000;
  }
  return 0;
}

short GeoProjectionConverter::get_GeogPrimeMeridianGeoKey() const
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

short GeoProjectionConverter::get_GeogLinearUnitsGeoKey() const
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

double GeoProjectionConverter::get_GeogLinearUnitSizeGeoKey() const
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

short GeoProjectionConverter::get_GeogAngularUnitsGeoKey() const
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
}

double GeoProjectionConverter::get_GeogAngularUnitSizeGeoKey() const
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

double GeoProjectionConverter::get_GeogSemiMajorAxisGeoKey() const
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

double GeoProjectionConverter::get_GeogSemiMinorAxisGeoKey() const
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

double GeoProjectionConverter::get_GeogInvFlatteningGeoKey() const
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

short GeoProjectionConverter::get_GeogAzimuthUnitsGeoKey() const
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
}

double GeoProjectionConverter::get_GeogPrimeMeridianLongGeoKey() const
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

short GeoProjectionConverter::get_ProjectedCSTypeGeoKey(bool source) const
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
        if (gcs_code == GEO_GCS_NAD83)
        {
          if (utm->utm_northern_hemisphere)
          {
            if ((1 <= utm->utm_zone_number) && (utm->utm_zone_number <= 23))
            {
              return 26900 + utm->utm_zone_number;
            }
            else if ((59 <= utm->utm_zone_number) && (utm->utm_zone_number <= 60))
            {
              return 3313 + utm->utm_zone_number;
            }
          }
        }
        else if (gcs_code == GEO_GCS_NAD83_CSRS)
        {
          if (utm->utm_northern_hemisphere)
          {
            if ((7 <= utm->utm_zone_number) && (utm->utm_zone_number <= 22))
            {
              unsigned short epsg_codes_NAD83_CSRS[] = { 3154, 3155, 3156, 3157, 2955, 2956, 2957, 3158, 3159, 3160, 2958, 2959, 2960, 2961, 2962, 3761 };
              return epsg_codes_NAD83_CSRS[utm->utm_zone_number - 7];
            }
          }
        }
        else if (gcs_code == GEO_GCS_NAD83_2011)
        {
          if (utm->utm_northern_hemisphere)
          {
            if ((1 <= utm->utm_zone_number) && (utm->utm_zone_number <= 19))
            {
              return 6329 + utm->utm_zone_number;
            }
            else if ((59 <= utm->utm_zone_number) && (utm->utm_zone_number <= 60))
            {
              return 6269 + utm->utm_zone_number;
            }
          }
        }
        else if (gcs_code == GEO_GCS_NAD83_NSRS2007)
        {
          if (utm->utm_northern_hemisphere)
          {
            if ((1 <= utm->utm_zone_number) && (utm->utm_zone_number <= 19))
            {
              return 3707 + utm->utm_zone_number;
            }
            else if ((59 <= utm->utm_zone_number) && (utm->utm_zone_number <= 60))
            {
              return 3647 + utm->utm_zone_number;
            }
          }
        }
        else if (gcs_code == GEO_GCS_NAD83_HARN)
        {
          if (utm->utm_northern_hemisphere)
          {
            if ((10 <= utm->utm_zone_number) && (utm->utm_zone_number <= 19))
            {
              return 3730 + utm->utm_zone_number;
            }
            else if ((4 <= utm->utm_zone_number) && (utm->utm_zone_number <= 5))
            {
              return 3746 + utm->utm_zone_number;
            }
          }
          else
          {
            if (utm->utm_zone_number == 2)
            {
              return 2195;
            }
          }
        }
        else if (gcs_code == GEO_GCS_NAD83_CSRS)
        {
          if (utm->utm_northern_hemisphere)
          {
            if ((7 <= utm->utm_zone_number) && (utm->utm_zone_number <= 22))
            {
              return 6643 + utm->utm_zone_number;
            }
          }
        }
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
        else if (ellipsoid->id == GEO_ELLIPSOID_GRS1980)
        {
          if (utm->utm_northern_hemisphere)
          {
            if ((3 <= utm->utm_zone_number) && (utm->utm_zone_number <= 23))
            {
              return utm->utm_zone_number + 26900;
            }
            else if ((28 <= utm->utm_zone_number) && (utm->utm_zone_number <= 38))
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
            if (gcs_code == GEO_GCS_GDA94)
            {
              if ((48 <= utm->utm_zone_number) && (utm->utm_zone_number <= 58))
              {
                return utm->utm_zone_number + 28300;
              }
              else
              {
                fprintf(stderr, "get_ProjectedCSTypeGeoKey: southern MGA zone %d for GDA94 does not exist\n", utm->utm_zone_number);
              }
            }
            else if (gcs_code == GEO_GCS_GDA2020)
            {
              if ((46 <= utm->utm_zone_number) && (utm->utm_zone_number <= 59))
              {
                return utm->utm_zone_number + 7800;
              }
              else
              {
                fprintf(stderr, "get_ProjectedCSTypeGeoKey: southern MGA zone %d for GDA2020 does not exist\n", utm->utm_zone_number);
              }
            }
            else
            {
              fprintf(stderr, "get_ProjectedCSTypeGeoKey: southern UTM zone %d for NAD83 does not exist\n", utm->utm_zone_number);
            }
          }
        }
        else if (ellipsoid->id == GEO_ELLIPSOID_CLARKE1866)
        {
          if (utm->utm_northern_hemisphere)
          {
            if ((3 <= utm->utm_zone_number) && (utm->utm_zone_number <= 22))
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
        else if (ellipsoid->id == GEO_ELLIPSOID_GRS1967)
        {
          if (utm->utm_northern_hemisphere)
          {
            if ((18 <= utm->utm_zone_number) && (utm->utm_zone_number <= 22))
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
        else if (ellipsoid->id == GEO_ELLIPSOID_INTERNATIONAL)
        {
          if (utm->utm_northern_hemisphere)
          {
            if ((28 <= utm->utm_zone_number) && (utm->utm_zone_number <= 38))
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
            if ((46 <= utm->utm_zone_number) && (utm->utm_zone_number <= 53))
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
            if ((46 <= utm->utm_zone_number) && (utm->utm_zone_number <= 54))
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
    else if (projection->type == GEO_PROJECTION_HOM)
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
    else if (projection->type == GEO_PROJECTION_OS)
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

int GeoProjectionConverter::set_GeogEllipsoidGeoKey(short value)
{
  int ellipsoid_id = -1;

  switch (value)
  {
  case 7030: // Ellipse_WGS_84
    ellipsoid_id = GEO_ELLIPSOID_WGS84;
    break;
  case 7019: // Ellipse_GRS_1980
    ellipsoid_id = GEO_ELLIPSOID_GRS1980;
    break;
  case 7043: // Ellipse_WGS_72
    ellipsoid_id = GEO_ELLIPSOID_WGS72;
    break;
  case 7001: // Ellipse_Airy_1830
    ellipsoid_id = GEO_ELLIPSOID_AIRY;
    break;
  case 7002: // Ellipse_Airy_Modified_1849
    ellipsoid_id = 16;
    break;
  case 7003: // Ellipse_Australian_National_Spheroid
    ellipsoid_id = 2;
    break;
  case 7004: // Ellipse_Bessel_1841
  case 7005: // Ellipse_Bessel_Modified
    ellipsoid_id = GEO_ELLIPSOID_BESSEL_1841;
    break;
  case 7006: // Ellipse_Bessel_Namibia
  case 7046: // Ellipse_Bessel_Namibia
    ellipsoid_id = GEO_ELLIPSOID_BESSEL_NAMIBIA;
    break;
  case 7008: // Ellipse_Clarke_1866
  case 7009: // Ellipse_Clarke_1866_Michigan
    ellipsoid_id = GEO_ELLIPSOID_CLARKE1866;
    break;
  case 7010: // Ellipse_Clarke1880_Benoit
  case 7011: // Ellipse_Clarke1880_IGN
  case 7012: // Ellipse_Clarke1880_RGS
  case 7013: // Ellipse_Clarke1880_Arc
  case 7014: // Ellipse_Clarke1880_SGA1922
  case 7034: // Ellipse_Clarke1880
    ellipsoid_id = GEO_ELLIPSOID_CLARKE1880;
    break;
  case 7015: // Ellipse_Everest1830_1937Adjustment
  case 7016: // Ellipse_Everest1830_1967Definition
  case 7017: // Ellipse_Everest1830_1975Definition
    ellipsoid_id = 7;
    break;
  case 7018: // Ellipse_Everest1830Modified
    ellipsoid_id = 17;
    break;
  case 7020: // Ellipse_Helmert1906
    ellipsoid_id = 12;
    break;
  case 7021: // Ellipse_IndonesianNational1974
    ellipsoid_id = GEO_ELLIPSOID_ID74;
    break;
  case 7022: // Ellipse_International1924
  case 7023: // Ellipse_International1967
    ellipsoid_id = GEO_ELLIPSOID_INTERNATIONAL;
    break;
  case 7024: // Ellipse_Krassowsky1940
    ellipsoid_id = GEO_ELLIPSOID_KRASSOWSKY;
    break;
  }
  return ellipsoid_id;
}

short GeoProjectionConverter::get_GeogEllipsoidGeoKey() const
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
  case GEO_ELLIPSOID_BESSEL_1841: // Ellipse_Bessel_1841
    return 7004;
  case GEO_ELLIPSOID_BESSEL_NAMIBIA: // Ellipse_Bessel_Namibia
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

bool GeoProjectionConverter::set_ProjLinearUnitsGeoKey(short value, bool source)
{
  switch (value)
  {
  case 9001: // Linear_Meter
    set_coordinates_in_meter(source);
    break;
  case 9002: // Linear_Foot
    set_coordinates_in_feet(source);
    break;
  case 9003: // Linear_Foot_US_Survey
    set_coordinates_in_survey_feet(source);
    break;
  default:
    fprintf(stderr, "set_ProjLinearUnitsGeoKey: look-up for %d not implemented\n", value);
    return false;
  }
  return true;
}

short GeoProjectionConverter::get_ProjLinearUnitsGeoKey(bool source) const
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

bool GeoProjectionConverter::set_VerticalUnitsGeoKey(short value)
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
    return false;
  }
  return true;
}

short GeoProjectionConverter::get_VerticalUnitsGeoKey(bool source) const
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

bool GeoProjectionConverter::set_VerticalCSTypeGeoKey(short value, char* description)
{
  if (value == GEO_VERTICAL_WGS84)
  {
    vertical_geokey = GEO_VERTICAL_WGS84;
    if (description) sprintf(description, "WGS 84 Ellipsoid");
    return true;
  }
  else if (value == GEO_VERTICAL_NAVD88)
  {
    vertical_geokey = GEO_VERTICAL_NAVD88;
    if (description) sprintf(description, "North American Vertical Datum 1988");
    return true;
  }
  else if (value == GEO_VERTICAL_DHHN2016)
  {
    vertical_geokey = GEO_VERTICAL_DHHN2016;
    if (description) sprintf(description, "Deutsches Haupthoehennetz 2016");
    return true;
  }
  else if (value == GEO_VERTICAL_NZVD2016)
  {
    vertical_geokey = GEO_VERTICAL_NZVD2016;
    if (description) sprintf(description, "New Zealand Vertical Datum 2016");
    return true;
  }
  else if (value == GEO_VERTICAL_DHHN92)
  {
    vertical_geokey = GEO_VERTICAL_DHHN92;
    if (description) sprintf(description, "Deutsches Haupthoehennetz 1992");
    return true;
  }
  else if (value == GEO_VERTICAL_CGVD2013)
  {
    vertical_geokey = GEO_VERTICAL_CGVD2013;
    if (description) sprintf(description, "Canadian Geodetic Vertical Datum of 2013");
    return true;
  }
  else if (value == GEO_VERTICAL_NN2000)
  {
    vertical_geokey = GEO_VERTICAL_NN2000;
    if (description) sprintf(description, "Norway Normal Null 2000");
    return true;
  }
  else if (value == GEO_VERTICAL_NN54)
  {
    vertical_geokey = GEO_VERTICAL_NN54;
    if (description) sprintf(description, "Norway Normal Null 1954");
    return true;
  }
  else if (value == GEO_VERTICAL_EVRF2007)
  {
    vertical_geokey = GEO_VERTICAL_EVRF2007;
    if (description) sprintf(description, "European Vertical Reference Frame 2007");
    return true;
  }
  else if (value == GEO_VERTICAL_DVR90)
  {
    vertical_geokey = GEO_VERTICAL_DVR90;
    if (description) sprintf(description, "Dansk Vertikal Reference 1990");
    return true;
  }
  else if (value == GEO_VERTICAL_NGVD29)
  {
    vertical_geokey = GEO_VERTICAL_NGVD29;
    if (description) sprintf(description, "National Geodetic Vertical Datum 1929");
    return true;
  }
  else if (value == GEO_VERTICAL_CGVD28)
  {
    vertical_geokey = GEO_VERTICAL_CGVD28;
    if (description) sprintf(description, "Canadian Geodetic Vertical Datum of 1928");
    return true;
  }
  else if ((5000 <= value) && (value <= 5099)) // [5000, 5099] = EPSG Ellipsoid Vertical CS Codes
  {
    vertical_geokey = value;    
    if (description) sprintf(description, "Some Ellipsoid Vertical Datum");
    return true;
  }
  else
  {
    // try to look it up in 'vertcs.csv' file
    FILE* file = open_geo_file(argv_zero, true, true);
    if (file == 0)
    {
      fprintf(stderr, "ERROR: cannot open 'vertcs.csv' file. maybe your LAStools distribution\n");
      fprintf(stderr, "       has no .\\LAStools\\bin\\serf\\geo\\vertcs.csv file. download the\n");
      fprintf(stderr, "       latest version at http://lastools.org/download/LAStools.zip\n");
      return false;
    }
    int epsg_code = 0;
    char line[2048];
    while (fgets(line, 2048, file))
    {
      if (sscanf(line, "%d,", &epsg_code) == 1)
      {
        if (epsg_code == value)
        {
          // no need to read this file any further
          fclose(file);
          file = 0;
          // parse the current line
          char* name;
          int dummy, units, run = 0;;
          // skip until first comma
          while (line[run] != ',') run++;
          run++;
          // maybe name is in parentheses
          if (line[run] == '\"')
          {
            // remove opening parentheses
            run++;
            // this is where the name starts
            name = &line[run];
            run++;
            // skip until closing parentheses
            while (line[run] != '\"') run++;
            // this is where the name ends
            line[run] = '\0';
            run++;
          }
          else
          {
            // this is where the name starts
            name = &line[run];
            // skip until second comma
            while (line[run] != ',') run++;
            // this is where the name ends
            line[run] = '\0';
          }
          if (description) sprintf(description, name);
          run++;
          // skip two commas
          while (line[run] != ',') run++;
          run++;
          while (line[run] != ',') run++;
          run++;
          // scan
          if (sscanf(&line[run], "%d,%d", &units, &dummy) != 2)
          {
            fprintf(stderr, "failed to scan units from '%s'", line);
            return false;
          }
          if (!set_VerticalUnitsGeoKey(units))
          {
            fprintf(stderr, "units %d of EPSG code %d not implemented.\n", units, value);
            return false;
          }
          vertical_geokey = value;
          return true;
        }
      }
    }
    fprintf(stderr, "EPSG code %d not found in 'vertcs.csv' file\n", value);
    fclose(file);
    file = 0;
  }
  fprintf(stderr, "set_VerticalCSTypeGeoKey: look-up for %d not implemented\012", value);
  return false;
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

bool GeoProjectionConverter::set_gcs(short code, char* description)
{
  gcs_code = code;

  if (code == GEO_GCS_WGS84)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_WGS84);
    sprintf(gcs_name, "WGS 84");
    datum_code = gcs_code + 2000;
    sprintf(datum_name, "World_Geodetic_System_1984");
    spheroid_code = GEO_SPHEROID_WGS84;
  }
  else if (code == GEO_GCS_ETRS89)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_GRS1980);
    sprintf(gcs_name, "ETRS89");
    datum_code = gcs_code + 2000;
    sprintf(datum_name, "European_Terrestrial_Reference_System_1989");
    spheroid_code = GEO_SPHEROID_GRS80;
  }
  else if (code == GEO_GCS_NAD83)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_GRS1980);
    sprintf(gcs_name, "NAD83");
    datum_code = gcs_code + 2000;
    sprintf(datum_name, "North_American_Datum_1983");
    spheroid_code = GEO_SPHEROID_GRS80;
  }
  else if (code == GEO_GCS_GDA94)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_GRS1980);
    sprintf(gcs_name, "GDA94");
    datum_code = gcs_code + 2000;
    sprintf(datum_name, "Geocentric_Datum_of_Australia_1994");
    spheroid_code = GEO_SPHEROID_GRS80;
  }
  else if (code == GEO_GCS_GDA2020)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_GRS1980);
    sprintf(gcs_name, "GDA2020");
    datum_code = gcs_code + 2000;
    sprintf(datum_name, "Geocentric_Datum_of_Australia_2020");
    spheroid_code = GEO_SPHEROID_GRS80;
  }
  else if (code == GEO_GCS_WGS72)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_WGS72);
    sprintf(gcs_name, "WGS 72");
    datum_code = gcs_code + 2000;
    sprintf(datum_name, "World_Geodetic_System_1972");
    spheroid_code = GEO_SPHEROID_WGS72;
  }
  else if (code == GEO_GCS_NAD83_HARN)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_GRS1980);
    sprintf(gcs_name, "NAD83(HARN)");
    datum_code = gcs_code + 2000;
    sprintf(datum_name, "NAD83_High_Accuracy_Regional_Network");
    spheroid_code = GEO_SPHEROID_GRS80;
  }
  else if (code == GEO_GCS_NAD83_CSRS)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_GRS1980);
    sprintf(gcs_name, "NAD83(CSRS)");
    datum_code = 6140;
    sprintf(datum_name, "NAD83_Canadian_Spatial_Reference_System");
    spheroid_code = GEO_SPHEROID_GRS80;
  }
  else if (code == GEO_GCS_NAD83_PA11)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_GRS1980);
    sprintf(gcs_name, "NAD83(PA11)");
  }
  else if (code == GEO_GCS_NAD83_2011)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_GRS1980);
    sprintf(gcs_name, "NAD83(2011)");
  }
  else if (code == GEO_GCS_NZGD2000)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_GRS1980);
    sprintf(gcs_name, "NZGD2000");
    datum_code = gcs_code + 2000;
    sprintf(datum_name, "New_Zealand_Geodetic_Datum_2000");
    spheroid_code = GEO_SPHEROID_GRS80;
  }
  else if (code == GEO_GCS_NAD27)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_CLARKE1866);
    sprintf(gcs_name, "NAD27");
    datum_code = gcs_code + 2000;
    sprintf(datum_name, "North_American_Datum_1927");
    spheroid_code = GEO_ELLIPSOID_CLARKE1866;
  }
  else if (code == GEO_GCS_NAD83_NSRS2007)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_GRS1980);
    sprintf(gcs_name, "NAD83(NSRS2007)");
    datum_code = gcs_code + 2000;
    sprintf(datum_name, "NAD83_National_Spatial_Reference_System_2007");
    spheroid_code = GEO_SPHEROID_GRS80;
  }
  else if (code == GEO_GCS_WGS72BE)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_WGS72);
    sprintf(gcs_name, "WGS 72BE");
    datum_code = gcs_code + 2000;
    sprintf(datum_name, "WGS_1972_Transit_Broadcast_Ephemeris");
    spheroid_code = GEO_SPHEROID_WGS72;
  }
  else if (code == GEO_GCS_SAD69)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_GRS1967);
    sprintf(gcs_name, "SAD69");
    datum_code = gcs_code + 2000;
    sprintf(datum_name, "South_American_Datum_1969");
    spheroid_code = GEO_ELLIPSOID_GRS1967;
  }
  else if (code == GEO_GCS_HD72)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_GRS1967);
    sprintf(gcs_name, "HD72");
    datum_code = gcs_code + 2000;
    sprintf(datum_name, "Hungarian_Datum_1972");
    spheroid_code = GEO_ELLIPSOID_GRS1967;
  }
  else if (code == GEO_GCS_CH1903)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_BESSEL_1841);
    sprintf(gcs_name, "CH1903");
    datum_code = gcs_code + 2000;
    sprintf(datum_name, "CH1903");
    spheroid_code = GEO_SPHEROID_BESSEL1841;
  }
  else if (code == GEO_GCS_OSGB1936)
  {
    set_reference_ellipsoid(GEO_ELLIPSOID_AIRY);
    sprintf(gcs_name, "OSGB1936");
    datum_code = gcs_code + 2000;
    sprintf(datum_name, "OSGB1936");
    spheroid_code = GEO_ELLIPSOID_AIRY;
  }
  else
  {
    // try to look it up in 'gcs.csv' file
    FILE* file = open_geo_file(argv_zero, false);
    if (file == 0)
    {
      fprintf(stderr, "ERROR: cannot open 'gcs.csv' file. maybe your LAStools distribution\n");
      fprintf(stderr, "       has no .\\LAStools\\bin\\serf\\geo\\gcs.csv file. download the\n");
      fprintf(stderr, "       latest version at http://lastools.org/download/LAStools.zip\n");
      return false;
    }
    int value = 0;
    char line[2048];
    bool done = false;
    while (fgets(line, 2048, file))
    {
      if (sscanf(line, "%d,", &value) == 1)
      {
        if (code == value)
        {
          const char* gname;
          int run = 0;;
          // skip until first comma
          while (line[run] != ',') run++;
          run++;
          // maybe name is in parentheses
          if (line[run] == '\"')
          {
            // remove opening parentheses
            run++;
            // this is where the name starts
            gname = &line[run];
            run++;
            // skip until closing parentheses
            while (line[run] != '\"') run++;
            // this is where the name ends
            line[run] = '\0';
            run++;
          }
          else
          {
            // this is where the name starts
            gname = &line[run];
            // skip until second comma
            while (line[run] != ',') run++;
            // this is where the name ends
            line[run] = '\0';
          }
          run++;
          // get datum code
          int dcode;
          if (sscanf(&line[run], "%d,", &dcode) != 1)
          {
            break;
          }
          // skip until third comma
          while (line[run] != ',') run++;
          run++;
          // get datum name
          const char* dname;
          // maybe name is in parentheses
          if (line[run] == '\"')
          {
            // remove opening parentheses
            run++;
            // this is where the name starts
            dname = &line[run];
            run++;
            // skip until closing parentheses
            while (line[run] != '\"') run++;
            // this is where the name ends
            line[run] = '\0';
            run++;
          }
          else
          {
            // this is where the name starts
            dname = &line[run];
            // skip until fourth comma
            while (line[run] != ',') run++;
            // this is where the name ends
            line[run] = '\0';
          }
          run++;
          // skip until fifth comma
          while (line[run] != ',') run++;
          run++;
          // skip until sixth comma
          while (line[run] != ',') run++;
          run++;
          // get ellipsoid code
          if (sscanf(&line[run], "%d,", &value) == 1)
          {
            int ellipsoid_id = set_GeogEllipsoidGeoKey(value);
            if (ellipsoid_id != -1)
            {
//              fprintf(stderr, "set ellipsoid %d for EPSG code %d for '%s'\n", ellipsoid_id, value, gname);
              set_reference_ellipsoid(ellipsoid_id);
              sprintf(gcs_name, "%s", gname);
              datum_code = dcode;
              sprintf(datum_name, "%s", dname);
              spheroid_code = value;
              done = true;
              break;
            }
            else
            {
              fprintf(stderr, "ERROR: ellipsoid with EPSG code %d for '%s' not supported\n", value, gname);
              break;
            }
          }
        }
      }
    }
    fclose(file);
    if (!done)
    {
      if (description) sprintf(description, "unknown");
      return false;
    }
  }
  if (description) sprintf(description, "%s", gcs_name);
  return true;
}

const char* GeoProjectionConverter::get_gcs_name() const
{
  return gcs_name;
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
      source_projection->datum = gcs_code;
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
      target_projection->datum = gcs_code;
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

bool GeoProjectionConverter::set_ecef_projection(char* description, bool source, const char* name)
{
  GeoProjectionParameters* ecef = new GeoProjectionParameters();
  ecef->type = GEO_PROJECTION_ECEF;
  if (name)
  {
    sprintf(ecef->name, "%.255s", name);
  }
  else
  {
    sprintf(ecef->name, "earth-centered earth-fixed");
  }
  set_projection(ecef, source);

  if (description)
  {
    sprintf(description, "%s", ecef->name);
  }
  return true;
}

bool GeoProjectionConverter::set_utm_projection(char* zone, char* description, bool source, const char* name, bool is_mga)
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
  if (name)
  {
    sprintf(utm->name, "%.255s", name);
  }
  else
  {
    sprintf(utm->name, "%s zone %d%s", (is_mga ? "MGA" : "UTM"), zone_number, (utm->utm_northern_hemisphere ? "N" : "S"));
  }
  utm->utm_long_origin = (zone_number - 1) * 6 - 180 + 3; // + 3 puts origin in middle of zone
  set_projection(utm, source);
  if (description)
  {
    sprintf(description, "%.255s", utm->name);
  }
  return true;
}

bool GeoProjectionConverter::set_utm_projection(int zone, bool northern, char* description, bool source, const char* name, bool is_mga)
{
  GeoProjectionParametersUTM* utm = new GeoProjectionParametersUTM();
  utm->type = GEO_PROJECTION_UTM;
  utm->utm_zone_number = zone;
  utm->utm_zone_letter = ' ';
  utm->utm_northern_hemisphere = northern;
  if (name)
  {
    sprintf(utm->name, "%.255s", name);
  }
  else
  {
    sprintf(utm->name, "%s / %s %d%s", gcs_name, (is_mga ? "MGA" : "UTM"), zone, (utm->utm_northern_hemisphere ? "N" : "S"));
  }
  utm->utm_long_origin = (zone - 1) * 6 - 180 + 3;  // + 3 puts origin in middle of zone
  set_projection(utm, source);
  if (description)
  {
    sprintf(description, "%.255s", utm->name);
  }
  return true;
}

bool GeoProjectionConverter::set_target_utm_projection(char* description, const char* name)
{
  GeoProjectionParametersUTM* utm = new GeoProjectionParametersUTM();
  utm->type = GEO_PROJECTION_UTM;
  utm->utm_zone_number = -1;
  if (name)
  {
    sprintf(utm->name, "%.255s", name);
  }
  else
  {
    sprintf(utm->name, "auto select");
  }
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
void GeoProjectionConverter::set_lambert_conformal_conic_projection(double falseEastingMeter, double falseNorthingMeter, double latOriginDegree, double longMeridianDegree, double firstStdParallelDegree, double secondStdParallelDegree, char* description, bool source, const char* name)
{
  GeoProjectionParametersLCC* lcc = new GeoProjectionParametersLCC();
  lcc->type = GEO_PROJECTION_LCC;
  if (name)
  {
    sprintf(lcc->name, "%.255s", name);
  }
  else
  {
    sprintf(lcc->name, "Lambert Conformal Conic");
  }
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
void GeoProjectionConverter::set_transverse_mercator_projection(double falseEastingMeter, double falseNorthingMeter, double latOriginDegree, double longMeridianDegree, double scaleFactor, char* description, bool source, const char* name)
{
  GeoProjectionParametersTM* tm = new GeoProjectionParametersTM();
  tm->type = GEO_PROJECTION_TM;
  if (name)
  {
    sprintf(tm->name, "%.255s", name);
  }
  else
  {
    sprintf(tm->name, "Transverse Mercator");
  }
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
// latCenterDegree & longCenterDegree are the "center" latitiude and
// longitude in decimal degrees of the area being projected. All coordinates
// will be calculated in meters relative to this point on the earth.
//
// firstStdParallelDegree & secondStdParallelDegree are the two lines of
// longitude in decimal degrees (that is they run east-west) that define
// where the "cone" intersects the earth. They bracket the area being projected.
void GeoProjectionConverter::set_albers_equal_area_conic_projection(double falseEastingMeter, double falseNorthingMeter, double latCenterDegree, double longCenterDegree, double firstStdParallelDegree, double secondStdParallelDegree, char* description, bool source, const char* name)
{
  GeoProjectionParametersAEAC* aeac = new GeoProjectionParametersAEAC();
  aeac->type = GEO_PROJECTION_AEAC;
  if (name)
  {
    sprintf(aeac->name, "%.255s", name);
  }
  else
  {
    sprintf(aeac->name, "Albers Equal Area Conic");
  }
  aeac->aeac_false_easting_meter = falseEastingMeter;
  aeac->aeac_false_northing_meter = falseNorthingMeter;
  aeac->aeac_latitude_of_center_degree = latCenterDegree;
  aeac->aeac_longitude_of_center_degree = longCenterDegree;
  aeac->aeac_first_std_parallel_degree = firstStdParallelDegree;
  aeac->aeac_second_std_parallel_degree = secondStdParallelDegree;
  aeac->aeac_latitude_of_center_radian = deg2rad*aeac->aeac_latitude_of_center_degree;
  aeac->aeac_longitude_of_center_radian = deg2rad*aeac->aeac_longitude_of_center_degree;
  aeac->aeac_first_std_parallel_radian = deg2rad*aeac->aeac_first_std_parallel_degree;
  aeac->aeac_second_std_parallel_radian = deg2rad*aeac->aeac_second_std_parallel_degree;
  set_projection(aeac, source);
  compute_aeac_parameters(source);
  if (description)
  {
    sprintf(description, "false east/north: %g/%g [m], center lat/long: %g/%g, parallel 1st/2nd: %g/%g", aeac->aeac_false_easting_meter, aeac->aeac_false_northing_meter, aeac->aeac_latitude_of_center_degree, aeac->aeac_longitude_of_center_degree, aeac->aeac_first_std_parallel_degree, aeac->aeac_second_std_parallel_degree);
  }
}

// Configure an Oblique Mercator Projection
//
// The function set_hotine_oblique_mercator_projection() receives the Hotine
// Oblique Mercator projection parameters as inputs and sets the corresponding
// state variables.
//
// falseEastingMeter & falseNorthingMeter are just an offset in meters added
// to the final coordinate calculated.
//
// latCenterDegree & longCenterDegree are the "center" latitiude and
// longitude in decimal degrees of the area being projected. All coordinates
// will be calculated in meters relative to this point on the earth.
//
void GeoProjectionConverter::set_hotine_oblique_mercator_projection(double falseEastingMeter, double falseNorthingMeter, double latCenterDegree, double longCenterDegree, double azimuthDegree, double rectifiedGridAngleDegree, double scaleFactor, char* description, bool source, const char* name)
{
  GeoProjectionParametersHOM* hom = new GeoProjectionParametersHOM();
  hom->type = GEO_PROJECTION_HOM;
  if (name)
  {
    sprintf(hom->name, "%.255s", name);
  }
  else
  {
    sprintf(hom->name, "Hotine Oblique Mercator");
  }
  hom->hom_false_easting_meter = falseEastingMeter;
  hom->hom_false_northing_meter = falseNorthingMeter;
  hom->hom_latitude_of_center_degree = latCenterDegree;
  hom->hom_longitude_of_center_degree = longCenterDegree;
  hom->hom_azimuth_degree = azimuthDegree;
  hom->hom_rectified_grid_angle_degree = rectifiedGridAngleDegree;
  hom->hom_scale_factor = scaleFactor;
  hom->hom_latitude_of_center_radian = deg2rad*hom->hom_latitude_of_center_degree;
  hom->hom_longitude_of_center_radian = deg2rad*hom->hom_longitude_of_center_degree;
  hom->hom_azimuth_radian = deg2rad*hom->hom_azimuth_degree;
  hom->hom_rectified_grid_angle_radian = deg2rad*hom->hom_rectified_grid_angle_degree;
  set_projection(hom, source);
  compute_hom_parameters(source);
  if (description)
  {
    sprintf(description, "false east/north: %g/%g [m], center lat/long: %g/%g, azimuth: %g angle: %g scale: %g", hom->hom_false_easting_meter, hom->hom_false_northing_meter, hom->hom_latitude_of_center_degree, hom->hom_longitude_of_center_degree, hom->hom_azimuth_degree, hom->hom_rectified_grid_angle_degree, hom->hom_scale_factor);
  }
}

/*
  * The function set_oblique_stereographic_projection() receives the Oblique
  * Stereographic projection parameters as input and sets the corresponding
  * state variables.
  * falseEastingMeter   : Easting/X in meters at the center of the projection
  * falseNorthingMeter  : Northing/Y in meters at the center of the projection
  * latOriginDegree     : Latitude in decimal degree at the origin of the projection
  * longMeridianDegree  : Longitude n decimal degree at the center of the projection
  * scaleFactor         : Projection scale factor
*/
void GeoProjectionConverter::set_oblique_stereographic_projection(double falseEastingMeter, double falseNorthingMeter, double latOriginDegree, double longMeridianDegree, double scaleFactor, char* description, bool source, const char* name)
{
  GeoProjectionParametersOS* os = new GeoProjectionParametersOS();
  os->type = GEO_PROJECTION_OS;
  if (name)
  {
    sprintf(os->name, "%.255s", name);
  }
  else
  {
    sprintf(os->name, "Oblique Stereographic");
  }
  os->os_false_easting_meter = falseEastingMeter;
  os->os_false_northing_meter = falseNorthingMeter;
  os->os_lat_origin_degree = latOriginDegree;
  os->os_long_meridian_degree = longMeridianDegree;
  os->os_scale_factor = scaleFactor;
  os->os_lat_origin_radian = deg2rad*os->os_lat_origin_degree;
  os->os_long_meridian_radian = deg2rad*os->os_long_meridian_degree;
  set_projection(os, source);
  compute_os_parameters(source);
  if (description)
  {
    sprintf(description, "false east/north: %g/%g [m], origin lat/meridian long: %g/%g, scale: %g", os->os_false_easting_meter, os->os_false_northing_meter, os->os_lat_origin_degree, os->os_long_meridian_degree, os->os_scale_factor);
  }
}

static double unit2decdeg(double length, int unit)
{
  if (unit == 9102)
  {
    return length;
  }
  else if (unit == 9110)
  {
    double decimal, fraction, mins, secs;
    if (length > 0)
    {
      decimal = floor(length);
      fraction = floor(((length - decimal) * 100.0 * 100000.0) + 0.5) / 100000.0;
      mins = floor(fraction);
      secs = floor(((fraction - mins) * 100.0 * 100000.0) + 0.5) / 100000.0;
      decimal += ((mins / 60.0) + (secs/3600.0));
    }
    else
    {
      decimal = ceil(length);
      fraction = floor(((decimal - length) * 100.0 * 100000.0) + 0.5) / 100000.0;
      mins = floor(fraction);
      secs = floor(((fraction - mins) * 100.0 * 100000.0) + 0.5) / 100000.0;
      decimal -= ((mins / 60.0) + (secs/3600.0));
    }
//    fprintf(stderr, "length: %.15g fraction %.15g decimal: %.15g mins: %.15g secs: %.15g\n", length, fraction, decimal, mins, secs);
    return decimal;
  }
  else if (unit == 9105)
  {
    return length * 0.9;
  }
  fprintf(stderr, "ERROR: unit %d not known\n", unit);
  return 0.0;
}

static double unit2meter(double length, int unit)
{
  if (unit == 9001)
  {
    return length;
  }
  else if (unit == 9002)
  {
    return length*0.3048;
  }
  else if (unit == 9003)
  {
    return length*0.3048006096012;
  }
  fprintf(stderr, "ERROR: unit %d not known\n", unit);
  return 0.0;
}

bool GeoProjectionConverter::set_epsg_code(short value, char* description, bool source)
{
  int ellipsoid = -1;
  int gcs = -1;
  bool utm_northern = false;
  int utm_zone = -1;
  bool is_mga = false;
  bool longlat = false;
  bool ecef = false;

  if ((value >= 32601) && (value <= 32660)) // PCS_WGS84_UTM_zone_1N - PCS_WGS84_UTM_zone_60N
  {
    utm_northern = true; utm_zone = value-32600;
    gcs = GEO_GCS_WGS84;
  }
  else if ((value >= 32701) && (value <= 32760)) // PCS_WGS84_UTM_zone_1S - PCS_WGS84_UTM_zone_60S
  {
    utm_northern = false; utm_zone = value-32700;
    gcs = GEO_GCS_WGS84;
  }
  else if ((value >= 25828) && (value <= 25838)) // PCS_ETRS89_UTM_zone_28N - PCS_ETRS89_UTM_zone_38N
  {
    utm_northern = true; utm_zone = value-25800;
    gcs = GEO_GCS_ETRS89;
  }
  else if ((value >= 26903) && (value <= 26923)) // PCS_NAD83_UTM_zone_3N - PCS_NAD83_UTM_zone_23N
  {
    utm_northern = true; utm_zone = value-26900;
    gcs = GEO_GCS_NAD83;
  }
  else if ((value >= 3154) && (value <= 3160)) // NAD83(CSRS) / UTM zone 7N - NAD83(CSRS) / UTM zone 16N
  {
    utm_northern = true; utm_zone = (value < 3158 ? value - 3154 + 7 : value - 3158 + 14);
    gcs = GEO_GCS_NAD83_CSRS;
  }
  else if ((value >= 7846) && (value <= 7859)) // PCS_GDA2020_MGA_zone_46S - PCS_GDA2020_MGA_zone_59S
  {
    utm_northern = false; utm_zone = value-7800; is_mga = true;
    gcs = GEO_GCS_GDA2020;
  } 
  else if ((value >= 28348) && (value <= 28358)) // PCS_GDA94_MGA_zone_48 - PCS_GDA94_MGA_zone_58
  {
    utm_northern = false; utm_zone = value-28300; is_mga = true;
    gcs = GEO_GCS_GDA94;
  }
  else if ((value >= 29118) && (value <= 29122)) // PCS_SAD69_UTM_zone_18N - PCS_SAD69_UTM_zone_22N
  {
    utm_northern = true; utm_zone = value-29100;
    gcs = GEO_GCS_SAD69;
  }
  else if ((value >= 29177) && (value <= 29185)) // PCS_SAD69_UTM_zone_17S - PCS_SAD69_UTM_zone_25S
  {
    utm_northern = false; utm_zone = value-29160;
    gcs = GEO_GCS_SAD69;
  }
  else if ((value >= 32201) && (value <= 32260)) // PCS_WGS72_UTM_zone_1N - PCS_WGS72_UTM_zone_60N
  {
    utm_northern = true; utm_zone = value-32200;
    gcs = GEO_GCS_WGS72;
  }
  else if ((value >= 32301) && (value <= 32360)) // PCS_WGS72_UTM_zone_1S - PCS_WGS72_UTM_zone_60S
  {
    utm_northern = false; utm_zone = value-32300;
    gcs = GEO_GCS_WGS72;
  }
  else if ((value >= 32401) && (value <= 32460)) // PCS_WGS72BE_UTM_zone_1N - PCS_WGS72BE_UTM_zone_60N
  {
    utm_northern = true; utm_zone = value-32400;
    gcs = GEO_GCS_WGS72BE;
  }
  else if ((value >= 32501) && (value <= 32560)) // PCS_WGS72BE_UTM_zone_1S - PCS_WGS72BE_UTM_zone_60S
  {
    utm_northern = false; utm_zone = value-32500;
    gcs = GEO_GCS_WGS72BE;
  }
  else if ((value >= 26703) && (value <= 26723)) // PCS_NAD27_UTM_zone_3N - PCS_NAD27_UTM_zone_23N
  {
    utm_northern = true; utm_zone = value-26700;
    gcs = GEO_GCS_NAD27;
  }
  else switch (value)
  {
  case 4326: // WGS 84 longlat
    longlat = true;
    gcs = GEO_GCS_WGS84;
    break;
  case 4978: // WGS 84 ecef
    ecef = true;
    gcs = GEO_GCS_WGS84;
    break;
  case EPSG_EOV_HD72:     // should really be Hotine_Oblique_Mercator (but is special case with 90 degree angle that reduces to TM)
    set_gcs(GEO_GCS_HD72);
    set_transverse_mercator_projection(650000.0, 200000.0, 47.14439372222222, 19.04857177777778, 0.99993, 0, source, "EOV / HD72 / Hungarian National Grid");
    set_geokey(value, source);
    set_coordinates_in_meter(source);
    if (description) sprintf(description, "%s", gcs_name);
    return true;
  case EPSG_CH1903_LV03:  // should really be Hotine_Oblique_Mercator (but is special case with 90 degree angle that reduces to TM)
    set_gcs(GEO_GCS_CH1903);
    set_transverse_mercator_projection(600000.0, 200000.0, 46.95240555555556, 7.439583333333333, 1.0, 0, source, "CH1903 / LV03");
    set_geokey(value, source);
    set_coordinates_in_meter(source);
    if (description) sprintf(description, "%s", gcs_name);
    return true;
  default:
    // try to look it up in 'pcs.csv' file
    FILE* file = open_geo_file(argv_zero, true);
    if (file == 0)
    {
      fprintf(stderr, "ERROR: cannot open 'pcs.csv' file. maybe your LAStools distribution\n");
      fprintf(stderr, "       has no .\\LAStools\\bin\\serf\\geo\\pcs.csv file. download the\n");
      fprintf(stderr, "       latest version at http://lastools.org/download/LAStools.zip\n");
      return false;
    }
    int epsg_code = 0;
    char line[2048];
    while (fgets(line, 2048, file))
    {
      if (sscanf(line, "%d,", &epsg_code) == 1)
      {
        if (epsg_code == value)
        {
          // found the code. no need to read file any further
          fclose(file);
          file = 0;
          // parse the current line
          char* name;
          int dummy, units, gcs, transform, run = 0;;
          // skip until first comma
          while (line[run] != ',') run++;
          run++;
          // maybe name is in parentheses
          if (line[run] == '\"')
          {
            // remove opening parentheses
            run++;
            // this is where the name starts
            name = &line[run];
            run++;
            // skip until closing parentheses
            while (line[run] != '\"') run++;
            // this is where the name ends
            line[run] = '\0';
            run++;
          }
          else
          {
            // this is where the name starts
            name = &line[run];
            // skip until second comma
            while (line[run] != ',') run++;
            // this is where the name ends
            line[run] = '\0';
          }
          run++;
          // scan
          if (sscanf(&line[run], "%d,%d,%d,%d,%d", &units, &gcs, &dummy, &transform, &dummy) != 5)
          {
            fprintf(stderr, "failed to scan units, gcs, and transform from '%s'", line);
            return false;
          }
          if (!set_ProjLinearUnitsGeoKey(units, source))
          {
            fprintf(stderr, "units %d of EPSG code %d not implemented.\n", units, value);
            return false;
          }
          if (!set_gcs(gcs))
          {
            fprintf(stderr, "GCS %d of EPSG code %d not implemented.\n", gcs, value);
            return false;
          }
          // skip eight commas
          while (line[run] != ',') run++;
          run++;
          while (line[run] != ',') run++;
          run++;
          while (line[run] != ',') run++;
          run++;
          while (line[run] != ',') run++;
          run++;
          while (line[run] != ',') run++;
          run++;
          while (line[run] != ',') run++;
          run++;
          while (line[run] != ',') run++;
          run++;
          while (line[run] != ',') run++;
          run++;
          if (transform == 9807) // CT_TransverseMercator
          {
            double latitude_of_origin;
            int unit_latitude_of_origin;
            double central_meridian;
            int unit_central_meridian;
            double scale_factor;
            double false_easting;
            int unit_false_easting;
            double false_northing;
            int unit_false_northing;
            if (sscanf(&line[run], "%lf,%d,%d,%lf,%d,%d,%lf,%d,%d,%lf,%d,%d,%lf,%d", &latitude_of_origin, &unit_latitude_of_origin, &dummy, &central_meridian, &unit_central_meridian, &dummy, &scale_factor, &dummy, &dummy, &false_easting, &unit_false_easting, &dummy, &false_northing, &unit_false_northing) != 14)
            {
              fprintf(stderr, "failed to scan TM parameters from '%s'\n", line);
              return false;
            }
            double latitude_of_origin_decdeg = unit2decdeg(latitude_of_origin, unit_latitude_of_origin);
            double central_meridian_decdeg = unit2decdeg(central_meridian, unit_central_meridian);
            double false_easting_meter = unit2meter(false_easting, unit_false_easting);
            double false_northing_meter = unit2meter(false_northing, unit_false_northing);
            set_transverse_mercator_projection(false_easting_meter, false_northing_meter, latitude_of_origin_decdeg, central_meridian_decdeg, scale_factor, 0, source, name);
            set_geokey(value, source);
            if (description) sprintf(description, "%s", name);
            return true;
          }
          else if (transform == 9802) // CT_LambertConfConic_2SP
          {
            double latitude_of_origin;
            int unit_latitude_of_origin;
            double central_meridian;
            int unit_central_meridian;
            double standard_parallel_1;
            int unit_standard_parallel_1;
            double standard_parallel_2;
            int unit_standard_parallel_2;
            double false_easting;
            int unit_false_easting;
            double false_northing;
            int unit_false_northing;
            if (sscanf(&line[run], "%lf,%d,%d,%lf,%d,%d,%lf,%d,%d,%lf,%d,%d,%lf,%d,%d,%lf,%d", &latitude_of_origin, &unit_latitude_of_origin, &dummy, &central_meridian, &unit_central_meridian, &dummy, &standard_parallel_1, &unit_standard_parallel_1, &dummy, &standard_parallel_2, &unit_standard_parallel_2, &dummy, &false_easting, &unit_false_easting, &dummy, &false_northing, &unit_false_northing) != 17)
            {
              fprintf(stderr, "failed to scan LCC(2SP) parameters from '%s'\n", line);
              return false;
            }
            double latitude_of_origin_decdeg = unit2decdeg(latitude_of_origin, unit_latitude_of_origin);
            double central_meridian_decdeg = unit2decdeg(central_meridian, unit_central_meridian);
            double standard_parallel_1_decdeg = unit2decdeg(standard_parallel_1, unit_standard_parallel_1);
            double standard_parallel_2_decdeg = unit2decdeg(standard_parallel_2, unit_standard_parallel_2);
            double false_easting_meter = unit2meter(false_easting, unit_false_easting);
            double false_northing_meter = unit2meter(false_northing, unit_false_northing);
            set_lambert_conformal_conic_projection(false_easting_meter, false_northing_meter, latitude_of_origin_decdeg, central_meridian_decdeg, standard_parallel_1_decdeg, standard_parallel_2_decdeg, 0, source, name);
            set_geokey(value, source);
            if (description) sprintf(description, "%s", name);
            return true;
          }
          else if (transform == 9801) // CT_LambertConfConic_1SP
          {
            double latitude_of_natural_origin;
            int unit_latitude_of_natural_origin;
            double longitude_of_natural_origin;
            int unit_longitude_of_natural_origin;
            double scale_factor_at_natural_origin;
            double false_easting;
            int unit_false_easting;
            double false_northing;
            int unit_false_northing;

            if (sscanf(&line[run], "%lf,%d,%d,%lf,%d,%d,%lf,%d,%d,%lf,%d,%d,%lf,%d", &latitude_of_natural_origin, &unit_latitude_of_natural_origin, &dummy, &longitude_of_natural_origin, &unit_longitude_of_natural_origin, &dummy, &scale_factor_at_natural_origin, &dummy, &dummy, &false_easting, &unit_false_easting, &dummy, &false_northing, &unit_false_northing) != 14)
            {
              fprintf(stderr, "failed to scan LCC(1SP) parameters from '%s'\n", line);
              return false;
            }
            double latitude_of_natural_origin_decdeg = unit2decdeg(latitude_of_natural_origin, unit_latitude_of_natural_origin);
            double longitude_of_natural_origin_decdeg = unit2decdeg(longitude_of_natural_origin, unit_longitude_of_natural_origin);
            double false_easting_meter = unit2meter(false_easting, unit_false_easting);
            double false_northing_meter = unit2meter(false_northing, unit_false_northing);
/*
            fprintf(stderr, "Lambert Conic Conformal (1SP)\n");
            fprintf(stderr, "Latitude of natural origin:  %.10g\n", latitude_of_natural_origin_decdeg);
            fprintf(stderr, "Longitude of natural origin: %.10g\n", longitude_of_natural_origin_decdeg);
            fprintf(stderr, "Scale factor at natural origin: %.10g\n", scale_factor_at_natural_origin);
            fprintf(stderr, "False easting:  %.10g\n", false_easting_meter);
            fprintf(stderr, "False northing: %.10g\n", false_northing_meter);
*/
            if (scale_factor_at_natural_origin != 1.0) fprintf(stderr, "\nWARNING: current implementation for Lambert Conic Conformal (1SP) ignores scale factor %.10g and uses 1.0 instead\n", scale_factor_at_natural_origin);
            set_lambert_conformal_conic_projection(false_easting_meter, false_northing_meter, latitude_of_natural_origin_decdeg, longitude_of_natural_origin_decdeg, latitude_of_natural_origin, latitude_of_natural_origin, 0, source, name);
            set_geokey(value, source);
            if (description) sprintf(description, "%s", name);
            return true;
          }
          else if (transform == 9822) // CT_AlbersEqualArea
          {
            double latitude_of_center;
            int unit_latitude_of_center;
            double longitude_of_center;
            int unit_longitude_of_center;
            double standard_parallel_1;
            int unit_standard_parallel_1;
            double standard_parallel_2;
            int unit_standard_parallel_2;
            double false_easting;
            int unit_false_easting;
            double false_northing;
            int unit_false_northing;
            if (sscanf(&line[run], "%lf,%d,%d,%lf,%d,%d,%lf,%d,%d,%lf,%d,%d,%lf,%d,%d,%lf,%d", &latitude_of_center, &unit_latitude_of_center, &dummy, &longitude_of_center, &unit_longitude_of_center, &dummy, &standard_parallel_1, &unit_standard_parallel_1, &dummy, &standard_parallel_2, &unit_standard_parallel_2, &dummy, &false_easting, &unit_false_easting, &dummy, &false_northing, &unit_false_northing) != 17)
            {
              fprintf(stderr, "failed to scan AEAC parameters from '%s'\n", line);
              return false;
            }
            double latitude_of_center_decdeg = unit2decdeg(latitude_of_center, unit_latitude_of_center);
            double longitude_of_center_decdeg = unit2decdeg(longitude_of_center, unit_longitude_of_center);
            double standard_parallel_1_decdeg = unit2decdeg(standard_parallel_1, unit_standard_parallel_1);
            double standard_parallel_2_decdeg = unit2decdeg(standard_parallel_2, unit_standard_parallel_2);
            double false_easting_meter = unit2meter(false_easting, unit_false_easting);
            double false_northing_meter = unit2meter(false_northing, unit_false_northing);
            set_albers_equal_area_conic_projection(false_easting_meter, false_northing_meter, latitude_of_center_decdeg, longitude_of_center_decdeg, standard_parallel_1_decdeg, standard_parallel_2_decdeg, 0, source, name);
            set_geokey(value, source);
            if (description) sprintf(description, "%s", name);
            return true;
          }
          else if (transform == 9812) // CT_HotineObliqueMercator
          {
            double false_easting;
            int unit_false_easting;
            double false_northing;
            int unit_false_northing;
            double latitude_of_center;
            int unit_latitude_of_center;
            double longitude_of_center;
            int unit_longitude_of_center;
            double azimuth;
            int unit_azimuth;
            double rectified_grid_angle;
            int unit_rectified_grid_angle;
            double scale_factor;
            if (sscanf(&line[run], "%lf,%d,%d,%lf,%d,%d,%lf,%d,%d,%lf,%d,%d,%lf,%d,%d,%lf,%d,%d,%lf", &false_easting, &unit_false_easting, &dummy, &false_northing, &unit_false_northing, &dummy, &latitude_of_center, &unit_latitude_of_center, &dummy, &longitude_of_center, &unit_longitude_of_center, &dummy,&azimuth, &unit_azimuth, &dummy, &rectified_grid_angle, &unit_rectified_grid_angle, &dummy, &scale_factor) != 19)
            {
              fprintf(stderr, "failed to scan HOM parameters from '%s'\n", line);
              return false;
            }
            double false_easting_meter = unit2meter(false_easting, unit_false_easting);
            double false_northing_meter = unit2meter(false_northing, unit_false_northing);
            double latitude_of_center_decdeg = unit2decdeg(latitude_of_center, unit_latitude_of_center);
            double longitude_of_center_decdeg = unit2decdeg(longitude_of_center, unit_longitude_of_center);
            double azimuth_decdeg = unit2decdeg(azimuth, unit_azimuth);
            double rectified_grid_angle_decdeg = unit2decdeg(rectified_grid_angle, unit_rectified_grid_angle);
            set_hotine_oblique_mercator_projection(false_easting_meter, false_northing_meter, latitude_of_center_decdeg, longitude_of_center_decdeg, azimuth_decdeg, rectified_grid_angle_decdeg, scale_factor, 0, source, name);
            set_geokey(value, source);
            if (description) sprintf(description, "%s", name);
            return true;
          }
          else if (transform == 9809) // CT_ObliqueStereographic
          {
            double latitude_of_origin;
            int unit_latitude_of_origin;
            double central_meridian;
            int unit_central_meridian;
            double scale_factor;
            double false_easting;
            int unit_false_easting;
            double false_northing;
            int unit_false_northing;
            if (sscanf(&line[run], "%lf,%d,%d,%lf,%d,%d,%lf,%d,%d,%lf,%d,%d,%lf,%d", &latitude_of_origin, &unit_latitude_of_origin, &dummy, &central_meridian, &unit_central_meridian, &dummy, &scale_factor, &dummy, &dummy, &false_easting, &unit_false_easting, &dummy, &false_northing, &unit_false_northing) != 14)
            {
              fprintf(stderr, "failed to scan OS parameters from '%s'\n", line);
              return false;
            }
            double latitude_of_origin_decdeg = unit2decdeg(latitude_of_origin, unit_latitude_of_origin);
            double central_meridian_decdeg = unit2decdeg(central_meridian, unit_central_meridian);
            double false_easting_meter = unit2meter(false_easting, unit_false_easting);
            double false_northing_meter = unit2meter(false_northing, unit_false_northing);
            set_oblique_stereographic_projection(false_easting_meter, false_northing_meter, latitude_of_origin_decdeg, central_meridian_decdeg, scale_factor, 0, source, name);
            set_geokey(value, source);
            if (description) sprintf(description, "%s", name);
            return true;
          }
          else
          {
            fprintf(stderr, "transform %d of EPSG code %d not implemented.\n", transform, value);
            return false;
          }
        }
      }
    }
    fprintf(stderr, "EPSG code %d not found in 'pcs.csv' file\n", value);
    fclose(file);
    file = 0;
    return false;
  }

  if (gcs != -1)
  {
    set_gcs(gcs);
  }
  else
  {
    if (utm_zone != -1)
    {
      ellipsoid = GEO_GCS_WGS84;
    }
    if (ellipsoid != -1)
    {
      set_reference_ellipsoid(ellipsoid);
    }
  }

  if (utm_zone != -1)
  {
    if (set_utm_projection(utm_zone, utm_northern, description, source, 0, is_mga))
    {
      set_geokey(value, source);
      return true;
    }
  }

  if (longlat)
  {
    if (set_longlat_projection(description, source))
    {
      set_geokey(value, source);
      return true;
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

const char* GeoProjectionConverter::get_state_plane_nad27_lcc_zone(int i) const
{
  if (state_plane_lcc_nad27_list[i].zone)
  {
    return state_plane_lcc_nad27_list[i].zone;
  }
  return 0;
}

bool GeoProjectionConverter::set_state_plane_nad27_lcc(const char* zone, char* description, bool source, const char* name)
{
  int i = 0;
  while (state_plane_lcc_nad27_list[i].zone)
  {
    if (strcmp(zone, state_plane_lcc_nad27_list[i].zone) == 0)
    {
      return set_epsg_code(state_plane_lcc_nad27_list[i].geokey, description, source);
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

bool GeoProjectionConverter::set_state_plane_nad83_lcc(const char* zone, char* description, bool source, const char* name)
{
  int i = 0;
  while (state_plane_lcc_nad83_list[i].zone)
  {
    if (strcmp(zone, state_plane_lcc_nad83_list[i].zone) == 0)
    {
      return set_epsg_code(state_plane_lcc_nad83_list[i].geokey, description, source);
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

bool GeoProjectionConverter::set_state_plane_nad27_tm(const char* zone, char* description, bool source, const char* name)
{
  int i = 0;
  while (state_plane_tm_nad27_list[i].zone)
  {
    if (strcmp(zone, state_plane_tm_nad27_list[i].zone) == 0)
    {
      return set_epsg_code(state_plane_tm_nad27_list[i].geokey, description, source);
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

bool GeoProjectionConverter::set_state_plane_nad83_tm(const char* zone, char* description, bool source, const char* name)
{
  int i = 0;
  while (state_plane_tm_nad83_list[i].zone)
  {
    if (strcmp(zone, state_plane_tm_nad83_list[i].zone) == 0)
    {
      return set_epsg_code(state_plane_tm_nad83_list[i].geokey, description, source);
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

  double sin_lat = sin(aeac->aeac_latitude_of_center_radian);
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

void GeoProjectionConverter::compute_hom_parameters(bool source)
{
  GeoProjectionParametersHOM* hom = (GeoProjectionParametersHOM*)(source ? source_projection : target_projection);

  if (!hom || hom->type != GEO_PROJECTION_HOM) return;

	double esp = ellipsoid->eccentricity * sin(hom->hom_latitude_of_center_radian);
	double esp2 = esp * esp;
	hom->hom_B = sqrt(1.0 + ellipsoid->eccentricity_squared * pow(cos(hom->hom_latitude_of_center_radian), 4) / (1.0 - ellipsoid->eccentricity_squared));
	hom->hom_A = ellipsoid->equatorial_radius * hom->hom_B * hom->hom_scale_factor * sqrt(1.0 - ellipsoid->eccentricity_squared) / (1.0 - esp2);
  double t = tan(PI/4 - hom->hom_latitude_of_center_radian/2) / pow((1.0 - esp) / (1.0 + esp), ellipsoid->eccentricity/2);
  double t0 = (t < 0 ? 0 : t);
	double D = hom->hom_B * sqrt(1.0 - ellipsoid->eccentricity_squared) / cos(hom->hom_latitude_of_center_radian) / sqrt(1.0 - esp2);
  double F, D2;
	if (D < 1.0)
  {
    D2 = 1.0;
    F = D;
  }
	else
  {
    D2 = D * D;
    if (hom->hom_latitude_of_center_radian < 0.0)
    {
      F = D - sqrt(D2 - 1.0);
    }
    else
    {
      F = D + sqrt(D2 - 1.0);
    }
  }
	hom->hom_H = F * pow(t0, hom->hom_B);
	hom->hom_g0 = asin(sin(hom->hom_azimuth_radian) / D);
	hom->hom_l0 = (F - (1.0 / F)) / 2.0 * tan(hom->hom_g0);
	if (fabs(hom->hom_l0 - 1.0) < EPSILON)
  {
    hom->hom_l0 = 1.0;
  }
	hom->hom_l0 = hom->hom_longitude_of_center_radian - (asin(hom->hom_l0) / hom->hom_B);
}

static double srat(double esinp, double exp)
{
  return pow((1.0 - esinp) / (1.0 + esinp), exp);
}

void GeoProjectionConverter::compute_os_parameters(bool source)
{
  GeoProjectionParametersOS* os = (GeoProjectionParametersOS*)(source ? source_projection : target_projection);

  if (!os || os->type != GEO_PROJECTION_OS) return;

  double sphi = sin(os->os_lat_origin_radian);
  double cphi = cos(os->os_lat_origin_radian);
  cphi *= cphi;

  os->os_R2 = 2.0 * sqrt(1.0 - ellipsoid->eccentricity_squared) / (1.0 - ellipsoid->eccentricity_squared * sphi * sphi);
  os->os_C = sqrt(1.0 + ellipsoid->eccentricity_squared * cphi * cphi / (1.0 - ellipsoid->eccentricity_squared));
  os->os_phic0 = asin(sphi / os->os_C);
  os->os_sinc0 = sin(os->os_phic0);
  os->os_cosc0 = cos(os->os_phic0);
  os->os_ratexp = 0.5 * os->os_C * ellipsoid->eccentricity;
  os->os_K = tan(0.5 * os->os_phic0 + PI/4) / (pow(tan(0.5 * os->os_lat_origin_radian + PI/4), os->os_C) * srat(ellipsoid->eccentricity * sphi, os->os_ratexp));
  os->os_gf = os->os_scale_factor * ellipsoid->equatorial_radius;
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
  
  sprintf(utm->name, "UTM %d%s", utm->utm_zone_number, (utm->utm_northern_hemisphere ? "N" : "S"));

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
//  s = sin(ftphi);
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
 *   6.0 compute longitude
 */
  zlong = atan2( y, x );

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

  LongDegree = aeac->aeac_longitude_of_center_radian + theta / aeac->aeac_n;

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

  dlam = LongRadian - aeac->aeac_longitude_of_center_radian;
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
  * The function HOMtoLL() converts the Hotine Oblique Mercator projection
  * (easting and northing) coordinates to Geodetic (latitude and longitude)
  * coordinates, according to the current ellipsoid and Oblique Mercator
  * projection parameters.
  *
  *   HOMEastingMeter    : input Easting/X in meters
  *   HOMNorthingMeter   : input Northing/Y in meters
  *   LatDegree         : output Latitude in decimal degrees
  *   LongDegree        : output Longitude in decimal degrees
  *
  * Formulas: OGP 373-7-2 Geomatics Guidance Note 7, part 2, July 2012
*/
bool GeoProjectionConverter::HOMtoLL(const double HOMEastingMeter, const double HOMNorthingMeter, double& LatDegree, double& LongDegree, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersHOM* hom) const
{
	double v = (HOMEastingMeter - hom->hom_false_easting_meter) * cos(hom->hom_rectified_grid_angle_radian) - (HOMNorthingMeter - hom->hom_false_northing_meter) * sin(hom->hom_rectified_grid_angle_radian);
	double u = (HOMNorthingMeter - hom->hom_false_northing_meter) * cos(hom->hom_rectified_grid_angle_radian) + (HOMEastingMeter - hom->hom_false_easting_meter) * sin(hom->hom_rectified_grid_angle_radian);
  double Qp = exp(-hom->hom_B * v/hom->hom_A);
	double Sp = (Qp - (1.0 / Qp)) / 2.0;
	double Vp = sin(hom->hom_B * u / hom->hom_A);
	double Up = (Vp * cos(hom->hom_g0) + Sp * sin(hom->hom_g0)) / (Qp + (1.0 / Qp)) * 2.0;
	double tp = pow(hom->hom_H / sqrt((1.0 + Up) / (1.0 - Up)), 1.0/hom->hom_B);
	double x = PI / 2.0 - 2.0 * atan(tp);
	double e2 = ellipsoid->eccentricity_squared;
	double e4 = e2 * e2;
	double e6 = e4 * e2;
	double e8 = e4 * e4;
	double LatRadian = x+sin(2*x)*(e2/2+5*e4/24+e6/12+13*e8/360)+sin(4*x)*(7*e4/48+29*e6/240+811*e8/11520)+sin(6*x)*(7*e6/120+81*e8/1120)+sin(8*x)*(4279*e8/161280);
	double LongRadian = hom->hom_l0 - atan2(Sp * cos(hom->hom_g0) - Vp * sin(hom->hom_g0), cos(hom->hom_B * u / hom->hom_A)) / hom->hom_B;
	LatDegree = rad2deg * LatRadian;
	LongDegree = rad2deg * LongRadian;
  return true;
}

/*
  * The function LLtoHOM() converts Geodetic (latitude and longitude)
  * coordinates to the Hotine Oblique Mercator projection (easting and
  * northing) coordinates, according to the current ellipsoid and
  * Oblique Mercator projection parameters.
  *
  *   LatDegree         : input Latitude in decimal degrees
  *   LongDegree        : input Longitude in decimal degrees
  *   HOMEastingMeter    : output Easting/X in meters
  *   HOMNorthingMeter   : output Northing/Y in meters
  *
  * Formulas: OGP 373-7-2 Geomatics Guidance Note 7, part 2, July 2012
*/
bool GeoProjectionConverter::LLtoHOM(const double LatDegree, const double LongDegree, double &OMEastingMeter, double &OMNorthingMeter, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersHOM* hom) const
{
  double lat = LatDegree * deg2rad;
  double lon = LongDegree * deg2rad;

/*
  double e = ellipsoid->eccentricity;
  double esq = ellipsoid->eccentricity_squared;
  double latc = hom->hom_latitude_of_center_radian;
  double a = ellipsoid->equatorial_radius;
  double kc = hom->hom_scale_factor;
  double alphac = hom->hom_azimuth_radian;
  double lonc = hom->hom_longitude_of_center_radian;
  double gammac = hom->hom_rectified_grid_angle_radian;
  double FE = hom->hom_false_easting_meter;
  double FN = hom->hom_false_northing_meter;

  // constants for the projection

  double B = sqrt(1.0 + (esq * pow(cos(latc),4) / (1 - esq )));
  double A = a * B * kc * sqrt(1.0 - esq) / ( 1.0 - esq * pow(sin(latc),2));
  double to = tan(PI/4.0 - (latc/2)) / pow(((1.0 - e*sin(latc)) / (1.0 + e*sin(latc))),e/2);
  double D = B * sqrt(1.0 - esq) / (cos(latc) * sqrt( 1 - esq * pow(sin(latc),2)));
  double D2;
  if (D < 1) 
    D2 = 1;
  else
    D2 = D*D; 
  double F;
  if (latc < 0.0)
    F = D - sqrt(D2 - 1.0);
  else
    F = D + sqrt(D2 - 1.0);
  double H = F*pow(to,B);
  double G = (F - (1.0/F)) / 2;
  double gammao = asin(sin(alphac) / D);
  double lonO = lonc - (asin(G*tan(gammao))) / B;

  // forward: compute (E,N) from a given (lat,lon) :

  double t = tan(PI/4 - lat/2) / pow((1.0 - e * sin(lat)) / (1 + e * sin(lat)),e/2);
  double Q = H / pow(t,B);
  double S = (Q - 1.0 / Q) / 2;
  double T = (Q + 1.0 / Q) / 2;
  double V = sin(B * (lon - lonO));
  double U = (-V * cos(gammao) + S * sin(gammao)) / T;
  double v = A * log((1.0 - U) / (1.0 + U)) / (2 * B);
  double u = A * atan((S * cos(gammao) + V * sin(gammao)) / cos(B * (lon - lonO))) / B;
  OMEastingMeter = v * cos(gammac) + u * sin(gammac) + FE;
  OMNorthingMeter = u * cos(gammac) - v * sin(gammac) + FN;
*/

  double t = tan(PI/4 - lat/2) / pow((1.0 - ellipsoid->eccentricity * sin(lat)) / (1 + ellipsoid->eccentricity * sin(lat)), ellipsoid->eccentricity/2);
  double Q = hom->hom_H / pow(t,hom->hom_B);
  double S = (Q - 1.0 / Q) / 2;
  double T = (Q + 1.0 / Q) / 2;
  double V = sin(hom->hom_B * (lon - hom->hom_l0));
  double U = (-V * cos(hom->hom_g0) + S * sin(hom->hom_g0)) / T;
  double v = hom->hom_A * log((1.0 - U) / (1.0 + U)) / (2 * hom->hom_B);
  double u = hom->hom_A * atan((S * cos(hom->hom_g0) + V * sin(hom->hom_g0)) / cos(hom->hom_B * (lon - hom->hom_l0))) / hom->hom_B;
  OMEastingMeter = v * cos(hom->hom_rectified_grid_angle_radian) + u * sin(hom->hom_rectified_grid_angle_radian) + hom->hom_false_easting_meter;
  OMNorthingMeter = u * cos(hom->hom_rectified_grid_angle_radian) - v * sin(hom->hom_rectified_grid_angle_radian) + hom->hom_false_northing_meter;
  return true;
}

/*
  * The function OStoLL() converts the Oblique Stereographic projection
  * (easting and northing) coordinates to Geodetic (latitude and longitude)
  * coordinates, according to the current ellipsoid and Oblique Stereographic
  * projection parameters.
  *
  *   OSEastingMeter    : input Easting/X in meters
  *   OSNorthingMeter   : input Northing/Y in meters
  *   LatDegree         : output Latitude in decimal degrees
  *   LongDegree        : output Longitude in decimal degrees
  *
  * formulas from "Oblique Stereographic Alternative" by Gerald Evenden and Rueben Schulz
*/
bool GeoProjectionConverter::OStoLL(const double OSEastingMeter, const double OSNorthingMeter, double& LatDegree,  double& LongDegree, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersOS* os) const
{
  double x = (OSEastingMeter - os->os_false_easting_meter) / os->os_gf;
  double y = (OSNorthingMeter - os->os_false_northing_meter) / os->os_gf;

  double rho = hypot(x, y);
  if (fabs(rho) < 1.0e-6)
  {
    x = 0.0;
    y = os->os_phic0;
  }
  else
  {
    double ce = 2.0 * atan2(rho, os->os_R2);
    double sinc = sin(ce);
    double cosc = cos(ce);
    x = atan2(x * sinc, rho * (os->os_cosc0 * cosc) - (y * os->os_sinc0 * sinc));
    y = (cosc * os->os_sinc0) + (y * sinc * os->os_cosc0 / rho);
    if (fabs(y) >= 1.0)
    {
      y = (y < 0.0) ? -PI/2.0 : PI/2.0;
    }
    else
    {
      y = asin(y);
    }
  }
  x /= os->os_C;
  double num = pow(tan(0.5 * y + PI/4)/os->os_K, 1.0/os->os_C);
  for (int i=15;;)
  {
    double phi = 2.0 * atan(num * srat(ellipsoid->eccentricity * sin(y), -0.5 * ellipsoid->eccentricity)) - PI/2;
    if (fabs(phi - y) < 1.0e-14)
    {
      break;
    }
    y = phi;
    if (--i < 0)
    {
      return false;
    }
  }

  LatDegree = rad2deg*y;
  LongDegree = rad2deg*(x + os->os_long_meridian_radian);
  return true;
}

/*
  * The function LLtoOS() converts Geodetic (latitude and longitude)
  * coordinates to the Oblique Stereographic projection (easting and
  * northing) coordinates, according to the current ellipsoid and
  * Oblique Stereographic projection parameters.
  *
  *   LatDegree         : input Latitude in decimal degrees
  *   LongDegree        : input Longitude in decimal degrees
  *   OSEastingMeter    : output Easting/X in meters
  *   OSNorthingMeter   : output Northing/Y in meters
  *
  * formulas from "Oblique Stereographic Alternative" by Gerald Evenden and Rueben Schulz
*/
bool GeoProjectionConverter::LLtoOS(const double LatDegree, const double LongDegree, double& OSEastingMeter,  double& OSNorthingMeter, const GeoProjectionEllipsoid* ellipsoid, const GeoProjectionParametersOS* os) const
{
  double y = LatDegree*deg2rad;
  double x = (LongDegree - os->os_long_meridian_degree)*deg2rad;

  y = 2.0 * atan(os->os_K * pow(tan(0.5 * y + PI/4), os->os_C) * srat(ellipsoid->eccentricity * sin(y), os->os_ratexp)) - PI/2;
  x *= os->os_C;
  double sinc = sin(y);
  double cosc = cos(y);
  double cosl = cos(x);
  double k = os->os_R2 / (1.0 + os->os_sinc0 * sinc + os->os_cosc0 * cosc * cosl);
  x = k * cosc * sin(x);
  y = k * (os->os_cosc0 * sinc - os->os_sinc0 * cosc * cosl);

  OSEastingMeter = x * os->os_gf + os->os_false_easting_meter;
  OSNorthingMeter = y * os->os_gf + os->os_false_northing_meter;
  return true;
}

GeoProjectionConverter::GeoProjectionConverter()
{
  argv_zero = 0;

  num_geo_keys = 0;
  geo_keys = 0;
  geo_ascii_params = 0;
  geo_double_params = 0;

  source_projection = 0;
  target_projection = 0;

  ellipsoid = new GeoProjectionEllipsoid();
  set_gcs(GEO_GCS_WGS84);
  vertical_geokey = 0;
  vertical_geoid = 0;

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
  if (argv_zero) free(argv_zero);
  if (geo_keys) delete [] geo_keys;
  delete ellipsoid;
  if (source_projection) delete source_projection;
  if (target_projection) delete target_projection;
}

bool GeoProjectionConverter::parse(int argc, char* argv[])
{
  int i;
  char tmp[256];
  bool verbose = false;

  if (argv_zero) free(argv_zero);
  argv_zero = LASCopyString(argv[0]);

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
    else if (strcmp(argv[i],"-v") == 0 || strcmp(argv[i],"-verbose") == 0)
    {
      verbose = true;
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
        if (verbose) fprintf(stderr, "using ellipsoid '%s'\n", tmp);
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
      set_gcs(GEO_GCS_WGS72, tmp);
      if (verbose) fprintf(stderr, "using ellipsoid '%s'\n", tmp);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-wgs84") == 0)
    {
      set_gcs(GEO_GCS_WGS84, tmp);
      if (verbose) fprintf(stderr, "using datum '%s' with ellipsoid '%s'\n", tmp, get_ellipsoid_name());
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-grs80") == 0)
    {
      set_reference_ellipsoid(GEO_ELLIPSOID_GRS1980, tmp);
      if (verbose) fprintf(stderr, "using ellipsoid '%s'\n", tmp);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-nad27") == 0)
    {
      set_gcs(GEO_GCS_NAD27, tmp);
      if (verbose) fprintf(stderr, "using datum '%s' with ellipsoid '%s'\n", tmp, get_ellipsoid_name());
      *argv[i]='\0';
    }
    else if (strncmp(argv[i],"-nad83", 6) == 0)
    {
      if (strcmp(argv[i],"-nad83") == 0)
      {
        set_gcs(GEO_GCS_NAD83, tmp);
        if (verbose) fprintf(stderr, "using datum '%s' with ellipsoid '%s'\n", tmp, get_ellipsoid_name());
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-nad83_2011") == 0)
      {
        set_gcs(GEO_GCS_NAD83_2011, tmp);
        if (verbose) fprintf(stderr, "using datum '%s' with ellipsoid '%s'\n", tmp, get_ellipsoid_name());
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-nad83_harn") == 0)
      {
        set_gcs(GEO_GCS_NAD83_HARN, tmp);
        if (verbose) fprintf(stderr, "using datum '%s' with ellipsoid '%s'\n", tmp, get_ellipsoid_name());
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-nad83_csrs") == 0)
      {
        set_gcs(GEO_GCS_NAD83_CSRS, tmp);
        if (verbose) fprintf(stderr, "using datum '%s' with ellipsoid '%s'\n", tmp, get_ellipsoid_name());
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-nad83_pa11") == 0)
      {
        set_gcs(GEO_GCS_NAD83_PA11, tmp);
        if (verbose) fprintf(stderr, "using datum '%s' with ellipsoid '%s'\n", tmp, get_ellipsoid_name());
        *argv[i]='\0';
      }
      else
      {
        fprintf(stderr,"ERROR: unknown datum '%s'.\n", argv[i]);
        return false;
      }
    }
    else if (strcmp(argv[i],"-gda94") == 0)
    {
      set_gcs(GEO_GCS_GDA94, tmp);
      if (verbose) fprintf(stderr, "using datum '%s' with ellipsoid '%s'\n", tmp, get_ellipsoid_name());
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-gda2020") == 0)
    {
      set_gcs(GEO_GCS_GDA2020, tmp);
      if (verbose) fprintf(stderr, "using datum '%s' with ellipsoid '%s'\n", tmp, get_ellipsoid_name());
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-etrs89") == 0)
    {
      set_gcs(GEO_GCS_ETRS89, tmp);
      if (verbose) fprintf(stderr, "using datum '%s' with ellipsoid '%s'\n", tmp, get_ellipsoid_name());
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-osgb1936") == 0)
    {
      set_gcs(GEO_GCS_OSGB1936, tmp);
      if (verbose) fprintf(stderr, "using datum '%s' with ellipsoid '%s'\n", tmp, get_ellipsoid_name());
      *argv[i]='\0';
    }
    else if (strncmp(argv[i],"-vertical_", 10) == 0)
    {
      if (strncmp(argv[i],"-vertical_navd88", 16) == 0)
      {
        vertical_geokey = GEO_VERTICAL_NAVD88;
        if (strcmp(argv[i] + 16,"") == 0)
        {
          vertical_geoid = 0; // none
        }
        else if (strcmp(argv[i] + 16,"_geoid12b") == 0)
        {
          vertical_geoid = GEO_VERTICAL_NAVD88_GEOID12B;
        }
        else if (strcmp(argv[i] + 16,"_geoid12a") == 0)
        {
          vertical_geoid = GEO_VERTICAL_NAVD88_GEOID12A;
        }
        else if (strcmp(argv[i] + 16,"_geoid12") == 0)
        {
          vertical_geoid = GEO_VERTICAL_NAVD88_GEOID12;
        }
        else if (strcmp(argv[i] + 16,"_geoid09") == 0)
        {
          vertical_geoid = GEO_VERTICAL_NAVD88_GEOID09;
        }
        else if (strcmp(argv[i] + 16,"_geoid06") == 0)
        {
          vertical_geoid = GEO_VERTICAL_NAVD88_GEOID06;
        }
        else if (strcmp(argv[i] + 16,"_geoid03") == 0)
        {
          vertical_geoid = GEO_VERTICAL_NAVD88_GEOID03;
        }
        else if (strcmp(argv[i] + 16,"_geoid99") == 0)
        {
          vertical_geoid = GEO_VERTICAL_NAVD88_GEOID99;
        }
        else if (strcmp(argv[i] + 16,"_geoid96") == 0)
        {
          vertical_geoid = GEO_VERTICAL_NAVD88_GEOID96;
        }
        else
        {
          fprintf(stderr, "WARNING: unknown specialization of NAVD88 '%s'\n", argv[i] + 16);
        }
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-vertical_wgs84") == 0)
      {
        vertical_geokey = GEO_VERTICAL_WGS84;
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-vertical_ngvd29") == 0 || strcmp(argv[i],"-vertical_navd29") == 0)
      {
        vertical_geokey = GEO_VERTICAL_NGVD29;
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-vertical_cgvd2013") == 0)
      {
        vertical_geokey = GEO_VERTICAL_CGVD2013;
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-vertical_evrf2007") == 0)
      {
        vertical_geokey = GEO_VERTICAL_EVRF2007;
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-vertical_cgvd28") == 0)
      {
        vertical_geokey = GEO_VERTICAL_CGVD28;
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-vertical_dvr90") == 0)
      {
        vertical_geokey = GEO_VERTICAL_DVR90;
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-vertical_nn2000") == 0)
      {
        vertical_geokey = GEO_VERTICAL_NN2000;
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-vertical_nn54") == 0)
      {
        vertical_geokey = GEO_VERTICAL_NN54;
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-vertical_dhhn92") == 0)
      {
        vertical_geokey = GEO_VERTICAL_DHHN92;
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-vertical_dhhn2016") == 0)
      {
        vertical_geokey = GEO_VERTICAL_DHHN2016;
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-vertical_nzvd2016") == 0)
      {
        vertical_geokey = GEO_VERTICAL_NZVD2016;
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-vertical_epsg") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: EPSG code\n", argv[i]);
          return false;
        }
        unsigned int code = 0;
        if (sscanf(argv[i+1], "%u", &code) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: EPSG code but '%s' is not a valid code\n", argv[i], argv[i+1]);
          return false;
        }
        if (code > 32767)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: EPSG code but %u is not a valid code\n", argv[i], code);
          return false;
        }
        if (!set_VerticalCSTypeGeoKey(code))
        {
          fprintf(stderr, "ERROR: unknown vertical EPSG code in '%s %s'.\n", argv[i], argv[i+1]);
          return false;
        }
        else
        {
          if (verbose) fprintf(stderr, "using vertical EPSG code %d\n", code);
        }
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
      }
    }
    else if (strcmp(argv[i],"-latlong") == 0 || strcmp(argv[i],"-target_latlong") == 0)
    {
      bool source = (strcmp(argv[i],"-latlong") == 0);
      set_latlong_projection(tmp, source);
      if (verbose) fprintf(stderr, "using %s '%s'\n", (source ? "projection" : "target projection"), tmp);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-longlat") == 0 || strcmp(argv[i],"-target_longlat") == 0)
    {
      bool source = (strcmp(argv[i],"-longlat") == 0);
      set_longlat_projection(tmp, source);
      if (verbose) fprintf(stderr, "using %s '%s'\n", (source ? "projection" : "target projection"), tmp);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-ecef") == 0 || strcmp(argv[i],"-target_ecef") == 0)
    {
      bool source = (strcmp(argv[i],"-ecef") == 0);
      set_ecef_projection(tmp, source);
      if (verbose) fprintf(stderr, "using %s '%s'\n", (source ? "projection" : "target projection"), tmp);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-utm") == 0 || strcmp(argv[i],"-target_utm") == 0)
    {
      bool source = (strcmp(argv[i],"-utm") == 0);
      if (!source && (strcmp(argv[i+1],"auto") == 0))
      {
        set_target_utm_projection(tmp);
        if (verbose) fprintf(stderr, "using target projection UTM '%s'\n", tmp);
      }
      else
      {
        if (argv[i+1][1] == 'n')
        {
          argv[i+1][1] = 'U';
          argv[i+1][2] = '\0';
        }
        else if (argv[i+1][1] == 's')
        {
          argv[i+1][1] = 'L';
          argv[i+1][2] = '\0';
        }
        else if (argv[i+1][2] == 'n')
        {
          argv[i+1][2] = 'U';
          argv[i+1][3] = '\0';
        }
        else if (argv[i+1][2] == 's')
        {
          argv[i+1][2] = 'L';
          argv[i+1][3] = '\0';
        }
        if (set_utm_projection(argv[i+1], tmp, source))
        {
          if (verbose) fprintf(stderr, "using %s UTM '%s'\n", (source ? "projection" : "target projection"), tmp);
        }
        else
        {
          fprintf(stderr, "ERROR: utm zone '%s' is unknown. use '32north', '55south', '10n', '56s', '17U', or '49L'\n", argv[i+1]);
          return false;
        }
      }
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-epsg") == 0 || strcmp(argv[i],"-target_epsg") == 0)
    {
      bool source = (strcmp(argv[i],"-epsg") == 0);
      unsigned int code = 0;
      if (sscanf(argv[i+1], "%u", &code) != 1)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: EPSG code but '%s' is not a valid code\n", argv[i], argv[i+1]);
        return false;
      }
      if (code > 32767)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: EPSG code but %u is not a valid code\n", argv[i], code);
        return false;
      }
      if (!set_epsg_code((short)code, 0, source))
      {
        fprintf(stderr, "ERROR: unknown EPSG code in '%s %s'.\n", argv[i], argv[i+1]);
        return false;
      }
      else
      {
        if (verbose) fprintf(stderr, "using %s EPSG code %d\n", (source ? "projection" : "target projection"), code);
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
      if (verbose) fprintf(stderr, "using LCC %s '%s'\n", (source ? "projection" : "target projection"), tmp);
      *argv[i]='\0'; *argv[i+1]='\0';  *argv[i+2]='\0';  *argv[i+3]='\0';  *argv[i+4]='\0';  *argv[i+5]='\0';  *argv[i+6]='\0';  *argv[i+7]='\0'; i+=7;
    }
    else if (strcmp(argv[i],"-sp83") == 0 || strcmp(argv[i],"-target_sp83") == 0)
    {
      bool source = (strcmp(argv[i],"-sp83") == 0);
      if (set_state_plane_nad83_lcc(argv[i+1], tmp, source))
      {
        if (verbose) fprintf(stderr, "using %s '%s' (NAD83 LCC) '%s'\n", (source ? "state plane" : "target state plane"), argv[i+1], tmp);
      }
      else if (set_state_plane_nad83_tm(argv[i+1], tmp, source))
      {
        if (verbose) fprintf(stderr, "using %s '%s' (NAD83 TM) '%s'\n", (source ? "state plane" : "target state plane"), argv[i+1], tmp);
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
        if (verbose) fprintf(stderr, "using %s '%s' (NAD27 LCC) '%s'\n", (source ? "state plane" : "target state plane"), argv[i+1], tmp);
      }
      else if (set_state_plane_nad27_tm(argv[i+1], tmp, source))
      {
        if (verbose) fprintf(stderr, "using %s '%s' (NAD27 TM) '%s'\n", (source ? "state plane" : "target state plane"), argv[i+1], tmp);
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
      if (verbose) fprintf(stderr, "using TM %s '%s'\n", (source ? "projection" : "target projection"), tmp);
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
      double latOfCenterDeg = atof(argv[i+4]);
      double longOfCenterDeg = atof(argv[i+5]);
      double firstStdParallelDeg = atof(argv[i+6]);
      double secondStdParallelDeg = atof(argv[i+7]);
      set_albers_equal_area_conic_projection(falseEasting, falseNorthing, latOfCenterDeg, longOfCenterDeg, firstStdParallelDeg, secondStdParallelDeg, tmp, source);
      if (verbose) fprintf(stderr, "using AEAC %s '%s'\n", (source ? "projection" : "target projection"), tmp);
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
    if (source_projection->geokey != 0)
    {
      // EPSG codes are simplest ...

      n += sprintf(&string[n], "-epsg %u ", source_projection->geokey);
    }
    else
    {
      // ... or more complex as a composite of switches

      if (gcs_code)
      {
        if (gcs_code == GEO_GCS_NAD83)
        {
          n += sprintf(&string[n], "-nad83 ");
        }
        else if (gcs_code == GEO_ELLIPSOID_GRS1980)
        {
          n += sprintf(&string[n], "-grs80 ");
        }
        else if (gcs_code == GEO_GCS_WGS84)
        {
          n += sprintf(&string[n], "-wgs84 ");
        }
        else if (gcs_code == GEO_GCS_NAD83_2011)
        {
          n += sprintf(&string[n], "-nad83_2011 ");
        }
        else if (gcs_code == GEO_GCS_NAD83_HARN)
        {
          n += sprintf(&string[n], "-nad83_harn ");
        }
        else if (gcs_code == GEO_GCS_NAD83_CSRS)
        {
          n += sprintf(&string[n], "-nad83_csrs ");
        }
        else if (gcs_code == GEO_GCS_NAD83_PA11)
        {
          n += sprintf(&string[n], "-nad83_pa11 ");
        }
        else if (gcs_code == GEO_GCS_ETRS89)
        {
          n += sprintf(&string[n], "-etrs89 ");
        }
        else if (gcs_code == GEO_GCS_GDA94)
        {
          n += sprintf(&string[n], "-gda94 ");
        }
        else if (gcs_code == GEO_GCS_GDA2020)
        {
          n += sprintf(&string[n], "-gda2020 ");
        }
        else if (gcs_code == GEO_GCS_WGS72)
        {
          n += sprintf(&string[n], "-wgs72 ");
        }
        else if (gcs_code == GEO_GCS_NAD27)
        {
          n += sprintf(&string[n], "-nad27 ");
        }
        else if (gcs_code == GEO_GCS_OSGB1936)
        {
          n += sprintf(&string[n], "-osgb1936 ");
        }
        else if (ellipsoid)
        {
          n += sprintf(&string[n], "-ellipsoid %d ", ellipsoid->id);
        }
      }
      else if (ellipsoid)
      {
        n += sprintf(&string[n], "-ellipsoid %d ", ellipsoid->id);
      }

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
      else if (source_projection->type == GEO_PROJECTION_UTM)
      {
        GeoProjectionParametersUTM* utm = (GeoProjectionParametersUTM*)source_projection;
        n += sprintf(&string[n], "-utm %d%c ", utm->utm_zone_number, (utm->utm_northern_hemisphere ? 'n' : 's'));
      }
      else if (source_projection->type == GEO_PROJECTION_LCC)
      {
        GeoProjectionParametersLCC* lcc = (GeoProjectionParametersLCC*)source_projection;
        n += sprintf(&string[n], "-lcc %lf %lf m %lf %lf %lf %lf ", lcc->lcc_false_easting_meter, lcc->lcc_false_northing_meter, lcc->lcc_lat_origin_degree, lcc->lcc_long_meridian_degree, lcc->lcc_first_std_parallel_degree, lcc->lcc_second_std_parallel_degree);
      }
      else if (source_projection->type == GEO_PROJECTION_TM)
      {
        GeoProjectionParametersTM* tm = (GeoProjectionParametersTM*)source_projection;
        n += sprintf(&string[n], "-tm %lf %lf m %lf %lf %lf ", tm->tm_false_easting_meter, tm->tm_false_northing_meter, tm->tm_lat_origin_degree, tm->tm_long_meridian_degree, tm->tm_scale_factor);
      }
      else if (source_projection->type == GEO_PROJECTION_AEAC)
      {
        GeoProjectionParametersAEAC* aeac = (GeoProjectionParametersAEAC*)source_projection;
        n += sprintf(&string[n], "-aeac %lf %lf m %lf %lf %lf %lf ", aeac->aeac_false_easting_meter, aeac->aeac_false_northing_meter, aeac->aeac_latitude_of_center_degree, aeac->aeac_longitude_of_center_degree, aeac->aeac_first_std_parallel_degree, aeac->aeac_second_std_parallel_degree);
      }
    }
  }
  if (has_coordinate_units(true))
  {
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
  }
  if (has_elevation_units(true))
  {
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
    else if (vertical_geokey == GEO_VERTICAL_CGVD2013)
    {
      n += sprintf(&string[n], "-vertical_cgvd2013 ");
    }
    else if (vertical_geokey == GEO_VERTICAL_EVRF2007)
    {
      n += sprintf(&string[n], "-vertical_evrf2007 ");
    }
    else if (vertical_geokey == GEO_VERTICAL_CGVD28)
    {
      n += sprintf(&string[n], "-vertical_cgvd28 ");
    }
    else if (vertical_geokey == GEO_VERTICAL_DVR90)
    {
      n += sprintf(&string[n], "-vertical_dvr90 ");
    }
    else if (vertical_geokey == GEO_VERTICAL_NGVD29)
    {
      n += sprintf(&string[n], "-vertical_ngvd29 ");
    }
    else if (vertical_geokey == GEO_VERTICAL_NN2000)
    {
      n += sprintf(&string[n], "-vertical_nn2000 ");
    }
    else if (vertical_geokey == GEO_VERTICAL_NN54)
    {
      n += sprintf(&string[n], "-vertical_nn54 ");
    }
    else if (vertical_geokey == GEO_VERTICAL_DHHN92)
    {
      n += sprintf(&string[n], "-vertical_dhhn92 ");
    }
    else if (vertical_geokey == GEO_VERTICAL_DHHN2016)
    {
      n += sprintf(&string[n], "-vertical_dhhn2016 ");
    }
    else if (vertical_geokey == GEO_VERTICAL_NZVD2016)
    {
      n += sprintf(&string[n], "-vertical_nzvd2016 ");
    }
    else
    {
      n += sprintf(&string[n], "-vertical_epsg %d ", vertical_geokey);
    }
  }
  if (target_projection != 0)
  {
    if (target_projection->geokey != 0)
    {
      n += sprintf(&string[n], "-target_epsg %u ", target_projection->geokey);
    }
    else if (target_projection->type == GEO_PROJECTION_LAT_LONG)
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
      n += sprintf(&string[n], "-target_lcc %lf %lf m %lf %lf %lf %lf ", lcc->lcc_false_easting_meter, lcc->lcc_false_northing_meter, lcc->lcc_lat_origin_degree, lcc->lcc_long_meridian_degree, lcc->lcc_first_std_parallel_degree, lcc->lcc_second_std_parallel_degree);
    }
    else if (target_projection->type == GEO_PROJECTION_TM)
    {
      GeoProjectionParametersTM* tm = (GeoProjectionParametersTM*)target_projection;
      n += sprintf(&string[n], "-target_tm %lf %lf m %lf %lf %lf ", tm->tm_false_easting_meter, tm->tm_false_northing_meter, tm->tm_lat_origin_degree, tm->tm_long_meridian_degree, tm->tm_scale_factor);
    }
    else if (target_projection->type == GEO_PROJECTION_AEAC)
    {
      GeoProjectionParametersAEAC* aeac = (GeoProjectionParametersAEAC*)target_projection;
      n += sprintf(&string[n], "-target_aeac %lf %lf m %lf %lf %lf %lf ", aeac->aeac_false_easting_meter, aeac->aeac_false_northing_meter, aeac->aeac_latitude_of_center_degree, aeac->aeac_longitude_of_center_degree, aeac->aeac_first_std_parallel_degree, aeac->aeac_second_std_parallel_degree);
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
    n += sprintf(&string[n], "-target_precision %lf ", target_precision);
  }
  if (target_elevation_precision != 0.0)
  {
    n += sprintf(&string[n], "-target_elevation_precision %lf ", target_elevation_precision);
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

const char* GeoProjectionConverter::get_coordinate_unit_description_string(bool abrev, bool source) const
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
    case GEO_PROJECTION_OS:
      OStoLL(coordinates2meter*point[0], coordinates2meter*point[1], latitude, longitude, ellipsoid, (const GeoProjectionParametersOS*)source_projection);
      break;
    case GEO_PROJECTION_HOM:
      HOMtoLL(coordinates2meter*point[0], coordinates2meter*point[1], latitude, longitude, ellipsoid, (const GeoProjectionParametersHOM*)source_projection);
      break;
    }

    elevation_in_meter = elevation2meter*point[2] + elevation_offset_in_meter;
    return true;
  }
  return false;
}

bool GeoProjectionConverter::check_horizontal_datum_before_reprojection()
{
  if (source_projection && target_projection)
  {
    if (source_projection->datum == target_projection->datum)
    {
      return true;
    }
    else if (source_projection->datum == 0)
    {
      fprintf(stderr, "WARNING: horizontal datum of source unspecified. assuming same as target.\n");
      source_projection->datum = target_projection->datum;
      return true;
    }
    else if (target_projection->datum == 0)
    {
//      fprintf(stderr, "WARNING: horizontal datum of target unspecified. assuming same as source.\n");
      target_projection->datum = source_projection->datum;
      return true;
    }
    else
    {
      fprintf(stderr, "SERIOUS WARNING: horizontal datum of source and target incompatible.\n");
      return false;
    }
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
    case GEO_PROJECTION_OS:
      OStoLL(coordinates2meter*point[0], coordinates2meter*point[1], latitude, longitude, ellipsoid, (const GeoProjectionParametersOS*)source_projection);
      break;
    case GEO_PROJECTION_HOM:
      HOMtoLL(coordinates2meter*point[0], coordinates2meter*point[1], latitude, longitude, ellipsoid, (const GeoProjectionParametersHOM*)source_projection);
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
    case GEO_PROJECTION_OS:
      LLtoOS(latitude, longitude, x, y, ellipsoid, (const GeoProjectionParametersOS*)target_projection);
      x = meter2coordinates * x;
      y = meter2coordinates * y;
      break;
    case GEO_PROJECTION_HOM:
      LLtoHOM(latitude, longitude, x, y, ellipsoid, (const GeoProjectionParametersHOM*)target_projection);
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
      *psDatumame = LASCopyString("NAD27");

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
        else if (ellipsoid->id == GEO_ELLIPSOID_CLARKE1866)
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
        else if (ellipsoid->id == GEO_ELLIPSOID_GRS1967)
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
        else if (ellipsoid->id == GEO_ELLIPSOID_INTERNATIONAL)
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
  if (get_ProjLinearUnitsGeoKey(source) == 9001)
  {
    *horizontal_units = 1;
  }
  else if (get_ProjLinearUnitsGeoKey(source) == 9002)
  {
    *horizontal_units = 0;
  }
  else if (get_ProjLinearUnitsGeoKey(source) == 9003)
  {
    *horizontal_units = 0;
  }
  else
  {
    *horizontal_units = 2;
  }

  if (get_VerticalUnitsGeoKey(source) == 9001)
  {
    *vertical_units = 1;
  }
  else if (get_VerticalUnitsGeoKey(source) == 9002)
  {
    *vertical_units = 0;
  }
  else if (get_VerticalUnitsGeoKey(source) == 9003)
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

  if (gcs_code == GEO_GCS_NAD27)
  {
    *horizontal_datum = 1;
  }
  else if ((gcs_code == GEO_GCS_NAD83) || (gcs_code == GEO_GCS_NAD83_HARN) || (gcs_code == GEO_GCS_NAD83_CSRS) || (gcs_code == GEO_GCS_NAD83_PA11) || (gcs_code == GEO_GCS_NAD83_2011) || (gcs_code == GEO_GCS_NAD83_NSRS2007))
  {
    *horizontal_datum = 2;
  }
  else if (gcs_code == GEO_GCS_WGS84)
  {
    *horizontal_datum = 3;
  }
  else if (ellipsoid && ellipsoid->id == GEO_ELLIPSOID_CLARKE1866)
  {
    *horizontal_datum = 1;
  }
  else if (ellipsoid && ellipsoid->id == GEO_ELLIPSOID_GRS1980)
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
  else if (vertical_geokey == GEO_VERTICAL_NGVD29)
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
    set_ProjLinearUnitsGeoKey(9001, source);
  }
  else if (horizontal_units == 0)
  {
    set_ProjLinearUnitsGeoKey(9002, source);
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
    set_gcs(GEO_GCS_NAD27);
  }
  else if (horizontal_datum == 2)
  {
    set_gcs(GEO_GCS_NAD83);
  }
  else if (horizontal_datum == 3)
  {
    set_gcs(GEO_GCS_WGS84);
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
    vertical_geokey = GEO_VERTICAL_NGVD29;
  }

  return true;
}
