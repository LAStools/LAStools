****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

  las2las:

  reads and writes LIDAR data in LAS/LAZ/ASCII format to filter,
  transform, project, thin, or otherwise modify its contents.
  Examples are keeping only a;; those points that are within a
  rextangle '-keep_xy 10 10 20 20' or points that are between
  a certain height '-keep_z 10 100', or dropping points that
  are a certain return '-drop_return 2', that have a scan angle
  above some threshold '-drop_scan_angle_above 5', or below some
  intensity '-drop_intensity_below 15'. Sometimes points are far
  outside the bounding box (corrupted files) and it is handy to
  remove them with '-clip_to_bounding_box'.

  It is also possible to add missing projection information to
  the LAS/LAZ file or to reproject (using the same ellipsoid)
  for example from latitude/longitude to UTM or the stateplane
  of Ohio_North or to Earth-centered Earth-fixed (ECEF). You can
  also use common EPSG codes with '-epsg 32754'. For LAS 1.4 it
  is important to '-set_ogc_wkt' which translates the GeoTIFF
  keys into an CRS string in the OGC WKT format and adds them as
  the payload of the corresponding VLR. For LAS 1.4 files you
  can use '-set_ogc_wkt_in_evlr' to put that string into the
  EVLR instead of the VLR (but we don't recommend that).

  Another typical use is extract only first (-first_only) or only
  last (-last_only) returns. Extracting the first return is the
  same as dropping all others (e.g. -drop_return 2 3 4 5).

  Or one can extract a subsequence of 1000 points (-subseq 540 1000)
  which will start at point 540.

  Finally one can also only keep or drop certain classifications.
  The option -keep_class 2 3 will keep only those points that are
  of classification 2 or 3 and the option -drop_class 2 3 will drop
  only those points. For all options run 'las2las -h'.
 
  For updates check the website or join the LAStools google group.
  
  https://rapidlasso.de/
  http://groups.google.com/group/lastools/

  Jochen @rapidlasso

****************************************************************

example usage:

>> las2las -i s1885565.laz -o out.las -sp83 OH_S -feet -elevation_feet

Adding the projection information to the file 's1885565.laz' (*). This
will not modify the points but merely change the projection VLR in the
header to contain these four geokeys:

  GeoKeyDirectoryTag version 1.1.0 number of keys 4
  - key 1024 value_offset 1 - GTModelTypeGeoKey: ModelTypeProjected
  - key 3072 value_offset 32123 - ProjectedCSTypeGeoKey: PCS_NAD83_Ohio_South
  - key 3076 value_offset 9002 - ProjLinearUnitsGeoKey: Linear_Foot
  - key 4099 value_offset 9002 - VerticalUnitsGeoKey: Linear_Foot

(*) http://www.cs.unc.edu/~isenburg/lastools/download/test/s1885565.laz

>> las2las -i s1885565.laz -o out.las -sp83 OH_S -feet -elevation_feet -target_utm auto

Reprojects the points from the Ohio_South NAD83 state plane with all units
in feet to NAD83 UTM coordinates with all units in meter and sets these four
geokeys as the projection information:

  GeoKeyDirectoryTag version 1.1.0 number of keys 4
  - key 1024 value_offset 1 - GTModelTypeGeoKey: ModelTypeProjected
  - key 3072 value_offset 26917 - ProjectedCSTypeGeoKey: PCS_NAD83_UTM_zone_17N
  - key 3076 value_offset 9001 - ProjLinearUnitsGeoKey: Linear_Meter
  - key 4099 value_offset 9001 - VerticalUnitsGeoKey: Linear_Meter

>> las2las -i s1885565.laz -o out.las -sp83 OH_S -feet -elevation_feet -target_longlat

Reprojects the points from the Ohio_South NAD83 state plane with all units
in feet to geographic coordinates with x being longitude and y latitude and
sets these three geokeys as the projection information:

  GeoKeyDirectoryTag version 1.1.0 number of keys 3
  - key 1024 value_offset 2 - GTModelTypeGeoKey: ModelTypeGeographic
  - key 2048 value_offset 4269 - GeographicTypeGeoKey: GCS_NAD83
  - key 4099 value_offset 9001 - VerticalUnitsGeoKey: Linear_Meter

>> las2las -i s1885565.laz -o out.las -sp83 OH_S -feet -elevation_feet -target_sp83 OH_N -target_survey_feet -target_elevation_survey_feet 
>> las2las -i TO_core_last_zoom.laz -o out.laz -utm 17T
>> las2las -i TO_core_last_zoom.laz -o out.laz -utm 17T -target_latlong

other variations of adding / changing projection information.

>> las2las -i *.las -last_only

processes all LAS files that match *.las and stores only the last returns
to a corresponding LAS file called *_1.las (an added '_1' in the name).

>> las2las -i *.las -olaz -keep_tile 630000 4830000 10000

keeps a 10000 by 10000 tile with a lower left coordinate of x=630000
and y=4830000 out of all LAS files that match *.las and stores each as a
compressed LAZ file *_1.laz (an added '_1' in the name).

>> las2las -i *.txt -iparse xyztiarn -keep_scan_angle -15 15

processes all ASCII files that match *.txt, parses them with "xyztiarn",
keeps all points whose scan angle is between -15 and 15, and stores them
to a corresponding LAS file called *_1.las (an added '_1' in the name).

>> las2las -i in.las -o out.las -keep_xy 630250 4834500 630500 4834750

keeps only points of in.las whose double-precision coordinates fall inside
the rectangle (630250,4834500) to (630500,4834750) and stores these points 
to out.las.

>> las2las -lof file_list.txt -merged -o out.laz -keep_circle 630000 4850000 100

keeps only those points from all files listed in the list of files file_list.txt
whose double-precision coordinates fall into the circle centered at 630000 4850000
with radius 100 and stores these points compressed to out.laz.

>> las2las -i in.las -o out.las -keep_z 10 100

keeps points of in.las whose double-precision elevations falls inside the
range 10 to 100 and stores these points to out.las.

>> las2las -i in.las -o out.laz -drop_return 1

drops all points of in.las that are designated first returns by
the value in their return_number field and stores surviving points
compressed to out.laz.

>> las2las -i in.laz -o out.las -drop_scan_angle_above 15

drops all points of compressed in.laz whose scan angle is above 15 or
below -15 and stores surviving points compressed to out.laz.

>> las2las -i in.las -o out.las -drop_intensity_below 1000 -remove_padding

drops all points of in.las whose intensity is below 1000 and stores
surviving points to out.las. in addition any additional user data after
the LAS header or after the VLR block are stripped from the file.

>> las2las -i in.laz -o out.laz -last_only

extracts all last return points from compressed in.laz and stores them
compressed to out.laz.

>> las2las -i in.las -o out.las -scale_rgb_up

multiplies all rgb values in the file with 256. this is used to scale
the rgb values from standard unsigned char range (0 ... 255) to the
unsigned short range (0 ... 65535) used in the LAS format.

>> las2las -i in.laz -o out.laz -scale_rgb_down

does the opposite with compressed input and output files

>> las2las -i in.las -o out.las -subseq 1000 2000

extracts a subsequence of points by skipping the first 1000 points and
then collecting points until 2000 points were read.

>> las2las -i in.las -o out.las -keep_class 2 -keep_class 3

extracts all points classfied as 2 or 3 from in.las and stores
them to out.las.

>> las2las -i in.las -o out.las -keep_XY 63025000 483450000 63050000 483475000

similar to '-keep_xy' but uses the integer values point.X and point.Y
that the points are stored with for the checks (and not the double
precision floating point coordinates they represent). drops all the
points of in.las that have point.X<63025000 or point.Y<483450000 or
point.X>63050000 or point.Y>483475000 and stores surviving points to
out.las (use lasinfo.exe to see the range of point.Z and point.Y).

>> las2las -i in.las -o out.las -keep_Z 1000 4000

similar to '-keep_z' but uses the integer values point.Z that the
points are stored with for the checks (and not the double-precision
floating point coordinates they represent). drops all the points
of in.las that have point.Z<1000 or point.Z>4000 and stores all
surviving points to out.las (use lasinfo.exe to see the range of
point.Z).

other commandline arguments are

-auto_reoffset                 : puts a reasonable offset in the header and translates the points accordingly
-reoffset 10000 40000 0        : puts a new offset into the header and translates the points accordingly
-rescale 0.01 0.01 0.01        : puts a new scale into the header and rescales the points accordingly
-clip_to_bounding_box          : removes all points that falls outsize the bouding box specified in the LAS header 
-repair_zero_returns           : sets return counts and number of returns that are zero to one
-start_at_point 100            : skips all points until point number 100
-start_at_point 900            : omits all points after point number 900
-subseq 20 100                 : extract a subsequence of 100 points starting from point 20
-set_point_type 0              : force point type to be 0
-set_point_size 26             : force point size to be 26
-set_global_encoding_gps_bit 1 : sets bit in global encoding field specifying Adjusted GPS Standard time stamps
-set_version 1.2               : set LAS version number to 1.2
-set_version_major 1           : set LAS major version number to 1
-set_version_minor 2           : set LAS minor version number to 2
-set_lastiling_buffer_flag 0   : sets buffer flag in LAStiling VLR (if it exists) to zero
-set_attribute_scale 0 0.1     : sets the scale of the *first* attribute in the extra bytes to 0.1 
-set_attribute_offset 1 10.0   : sets the offset of the *second* attribute in the extra bytes to 10.0 
-set_classification 0          : sets all classifications fields to zero
-set_user_data 0               : sets all user_data fields to zero
-set_ogc_wkt                   : translate GeoTIFF keys into CRS string in OGC WKT format and add it as VLR
-set_ogc_wkt_in_evlr           : same as above but adds it as LAS 1.4 EVLR instead. really not recommended!!!
-remove_padding                : remove user-defined bytes before and after the header
-remove_all_vlrs               : remove all VLRs
-remove_vlr 2                  : remove third VLR with index 2 (counting starts at 0)
-remove_vlrs_from_to 0 2       : remove the first three VLRs
-remove_all_evlrs              : remove all EVLRs
-remove_evlr 2                 : remove third EVLR with index 2 (counting starts at 0)
-remove_evlrs_from_to 0 2      : remove the first three EVLRs
-remove_tiling_vlr             : removes VLR containing tiling information created by lastile
-remove_original_vlr           : removes VLR containing original header information created by on-the-fly buffering
-add_empty_vlr uid rid desc    : add an empty VLR with given user-id(text), record-id(int) and description(text)  
-add_attribute 0 "hello" "sample attribute" 1.0 0.0 : adds a new attribute of type 0 (unsigned byte) as "extra bytes"
-unset_attribute_scale 0       : unsets the scale of the *first* attribute in the extra bytes
-unset_attribute_offset 1      : unsets the offset of the *second* attribute in the extra bytes
-move_evlrs_to_vlrs            : move all EVLRs with small enough payload to VLR section
-save_vlrs                     : saves all VLRs to a file called vlrs.vlr so they can be loaded into another file
-load_vlrs                     : loads all VLRs from a file called vlrs.vlr and adds them to each processed file
-dont_remove_empty_files       : does not remove files that have zero points remaining from disk
-clip_to_bounding_box          : kicks out all points not inside the bounding box specified by the LAS header
-week_to_adjusted              : converts time stamps from GPS week to Adjusted Standard GPS 
-adjusted_to_week              : converts time stamps from Adjusted Standard GPS to GPS week
-scale_rgb_up                  : multiplies all RGB values by 256 (to go from 8 bit to 16 bit numbers)
-scale_rgb_down                : divides all RGB values by 256 (to go from 16 bit to 8 bit numbers)
-force_RGB                     : force the use of the RGB value even if the point format does not support RGB
-wgs84                         : use datum WGS-84
-grs80                         : use datum GRS1980
-wgs72                         : use datum WGS-72
-nad83                         : use datum NAD83
-nad83_2011                    : use datum NAD83_2011
-nad83_harn                    : use datum NAD83_HARN
-nad83_csrs                    : use datum NAD83_CSRS
-nad27                         : use datum NAD27
-etrs89                        : use datum ETRS89
-gda94                         : use datum GDA94
-gda2020                       : use datum GDA2020
-osgb1936                      : use datum OSGB 1936
-utm 12T                       : input is UTM zone 12T 
-epsg 2972                     : input is EPSG code 2972 (e.g. Reseau Geodesique Francais Guyane 1995)
-sp83 CO_S                     : input is state plane NAD83 Colorado South
-sp27 SC_N                     : input is state plane NAD27 South Carolina North 
-longlat                       : input is geometric coordinates in longitude/latitude 
-latlong                       : input is geometric coordinates in latitude/longitude
-ecef                          : input is geocentric (Earth-centered Earth-fixed)
-survey_feet                   : input uses survey feet
-feet                          : input uses feet
-meter                         : input uses meter
-elevation_surveyfeet          : input uses survey feet for elevation
-elevation_feet                : input uses feet for elevation
-elevation_meter               : input uses meter for elevation
-target_utm 12T                  : output is UTM zone 12T 
-target_epsg 2193                : output is EPSG code 2193 (e.g. NZGD2000)
-target_sp83 CO_S                : output is state plane NAD83 Colorado South
-target_sp27 SC_N                : output is state plane NAD27 South Carolina North 
-target_longlat                  : output is geometric coordinates in longitude/latitude 
-target_latlong                  : output is geometric coordinates in latitude/longitude
-target_ecef                     : output is geocentric (Earth-centered Earth-fixed)
-target_survey_feet              : output uses survey feet
-target_feet                     : output uses feet
-target_meter                    : output uses meter
-target_elevation_surveyfeet     : output uses survey feet for elevation
-target_elevation_feet           : output uses feet for elevation
-target_elevation_meter          : output uses meter for elevation
-target_precision 0.001          : output uses one millimeter resolution for x and y
-target_elevation_precision 0.02 : output uses two centimeter resolution for z
-tm 609601.22 0.0 meter 33.75 -79 0.99996                 : specifies a transverse mercator projection
-tm 1804461.942257 0.0 feet 0.8203047 -2.1089395 0.99996
-lcc 609601.22 0.0 meter 33.75 -79 34.33333 36.16666      : specifies a lambertian conic confomal projection
-lcc 1640416.666667 0.0 surveyfeet 47.000000 -120.833333 47.50 48.733333
-ellipsoid 23                                             : use ellipsoid WGS-84 (specify '-ellipsoid -1' for a list)

for more info:

C:\software\LAStools\bin>las2las -h
Filter points based on their *scaled* coordinates.
  -keep_tile 631000 4834000 1000 (lowerleft_x lowerleft_y size)
  -keep_circle 630250.00 4834750.00 100 (x y radius)
  -keep_xy 630000 4834000 631000 4836000 (min_x min_y max_x max_y)
  -drop_xy 630000 4834000 631000 4836000 (min_x min_y max_x max_y)
  -keep_x 631500.50 631501.00 (min_x max_x)
  -drop_x 631500.50 631501.00 (min_x max_x)
  -drop_x_below 630000.50 (min_x)
  -drop_x_above 630500.50 (max_x)
  -keep_y 4834500.25 4834550.25 (min_y max_y)
  -drop_y 4834500.25 4834550.25 (min_y max_y)
  -drop_y_below 4834500.25 (min_y)
  -drop_y_above 4836000.75 (max_y)
  -keep_z 11.125 130.725 (min_z max_z)
  -drop_z 11.125 130.725 (min_z max_z)
  -drop_z_below 11.125 (min_z)
  -drop_z_above 130.725 (max_z)
  -keep_xyz 620000 4830000 100 621000 4831000 200 (min_x min_y min_z max_x max_y max_z)
  -drop_xyz 620000 4830000 100 621000 4831000 200 (min_x min_y min_z max_x max_y max_z)
Filter points based on their return numbering.
  -keep_first -first_only -drop_first
  -keep_last -last_only -drop_last
  -keep_second_last -drop_second_last
  -keep_first_of_many -keep_last_of_many
  -drop_first_of_many -drop_last_of_many
  -keep_middle -drop_middle
  -keep_return 1 2 3
  -drop_return 3 4
  -keep_single -drop_single
  -keep_double -drop_double
  -keep_triple -drop_triple
  -keep_quadruple -drop_quadruple
  -keep_number_of_returns 5
  -drop_number_of_returns 0
Filter points based on the scanline flags.
  -drop_scan_direction 0
  -keep_scan_direction_change
  -keep_edge_of_flight_line
Filter points based on their intensity.
  -keep_intensity 20 380
  -drop_intensity_below 20
  -drop_intensity_above 380
  -drop_intensity_between 4000 5000
Filter points based on classifications or flags.
  -keep_class 1 3 7
  -drop_class 4 2
  -keep_extended_class 43
  -drop_extended_class 129 135
  -drop_synthetic -keep_synthetic
  -drop_keypoint -keep_keypoint
  -drop_withheld -keep_withheld
  -drop_overlap -keep_overlap
Filter points based on their user data.
  -keep_user_data 1
  -drop_user_data 255
  -keep_user_data_below 50
  -keep_user_data_above 150
  -keep_user_data_between 10 20
  -drop_user_data_below 1
  -drop_user_data_above 100
  -drop_user_data_between 10 40
Filter points based on their point source ID.
  -keep_point_source 3
  -keep_point_source_between 2 6
  -drop_point_source 27
  -drop_point_source_below 6
  -drop_point_source_above 15
  -drop_point_source_between 17 21
Filter points based on their scan angle.
  -keep_scan_angle -15 15
  -drop_abs_scan_angle_above 15
  -drop_abs_scan_angle_below 1
  -drop_scan_angle_below -15
  -drop_scan_angle_above 15
  -drop_scan_angle_between -25 -23
Filter points based on their gps time.
  -keep_gps_time 11.125 130.725
  -drop_gps_time_below 11.125
  -drop_gps_time_above 130.725
  -drop_gps_time_between 22.0 48.0
Filter points based on their RGB/CIR/NIR channels.
  -keep_RGB_red 1 1
  -drop_RGB_red 5000 20000
  -keep_RGB_green 30 100
  -drop_RGB_green 2000 10000
  -keep_RGB_blue 0 0
  -keep_RGB_nir 64 127
  -keep_NDVI 0.2 0.7 -keep_NDVI_from_CIR -0.1 0.5
  -keep_NDVI_intensity_is_NIR 0.4 0.8 -keep_NDVI_green_is_NIR -0.2 0.2
Filter points based on their wavepacket.
  -keep_wavepacket 0
  -drop_wavepacket 3
Filter points based on extra attributes.
  -keep_attribute_above 0 5.0
  -drop_attribute_below 1 1.5
Filter points with simple thinning.
  -keep_every_nth 2 -drop_every_nth 3
  -keep_random_fraction 0.1
  -keep_random_fraction 0.1 4711
  -thin_with_grid 1.0
  -thin_pulses_with_time 0.0001
  -thin_points_with_time 0.000001
Boolean combination of filters.
  -filter_and
Transform coordinates.
  -translate_x -2.5
  -scale_z 0.3048
  -rotate_xy 15.0 620000 4100000 (angle + origin)
  -translate_xyz 0.5 0.5 0
  -translate_then_scale_y -0.5 1.001
  -transform_helmert -199.87,74.79,246.62
  -transform_helmert 598.1,73.7,418.2,0.202,0.045,-2.455,6.7
  -transform_affine 0.9999652,0.903571,171.67,736.26
  -switch_x_y -switch_x_z -switch_y_z
  -clamp_z_below 70.5
  -clamp_z 70.5 72.5
  -copy_attribute_into_z 0
  -add_attribute_to_z 1
  -add_scaled_attribute_to_z 1 -1.2
  -copy_intensity_into_z
  -copy_user_data_into_z
Transform raw xyz integers.
  -translate_raw_z 20
  -translate_raw_xyz 1 1 0
  -translate_raw_xy_at_random 2 2
  -clamp_raw_z 500 800
Transform intensity.
  -set_intensity 0
  -scale_intensity 2.5
  -translate_intensity 50
  -translate_then_scale_intensity 0.5 3.1
  -clamp_intensity 0 255
  -clamp_intensity_above 255
  -copy_RGB_into_intensity
  -copy_NIR_into_intensity
  -copy_attribute_into_intensity 0
  -bin_gps_time_into_intensity 0.5
Transform scan_angle.
  -set_scan_angle 0.0
  -scale_scan_angle 1.944445
  -translate_scan_angle -5
  -translate_then_scale_scan_angle -0.5 2.1
Change the return number or return count of points.
  -repair_zero_returns
  -set_return_number 1
  -set_extended_return_number 10
  -change_return_number_from_to 2 1
  -change_extended_return_number_from_to 2 8
  -set_number_of_returns 2
  -set_extended_number_of_returns 15
  -change_number_of_returns_from_to 0 2
  -change_extended_number_of_returns_from_to 8 10
Modify the classification.
  -set_classification 2
  -set_extended_classification 41
  -change_classification_from_to 2 4
  -classify_z_below_as -5.0 7
  -classify_z_above_as 70.0 7
  -classify_z_between_as 2.0 5.0 4
  -classify_intensity_above_as 200 9
  -classify_intensity_below_as 30 11
  -classify_intensity_between_as 500 900 15
  -classify_attribute_below_as 0 -5.0 7
  -classify_attribute_above_as 1 70.0 7
  -classify_attribute_between_as 1 2.0 5.0 4
  -change_extended_classification_from_to 6 46
  -move_ancient_to_extended_classification
Change the flags.
  -set_withheld_flag 0
  -set_synthetic_flag 1
  -set_keypoint_flag 0
  -set_overlap_flag 1
Modify the extended scanner channel.
  -set_scanner_channel 2
  -copy_user_data_into_scanner_channel
Modify the user data.
  -set_user_data 0
  -scale_user_data 1.5
  -change_user_data_from_to 23 26
  -change_user_data_from_to 23 26
  -copy_attribute_into_user_data 1
  -add_scaled_attribute_to_user_data 0 10.0
Modify the point source ID.
  -set_point_source 500
  -change_point_source_from_to 1023 1024
  -copy_user_data_into_point_source
  -copy_scanner_channel_into_point_source
  -merge_scanner_channel_into_point_source
  -split_scanner_channel_from_point_source
  -bin_Z_into_point_source 200
  -bin_abs_scan_angle_into_point_source 2
  -bin_gps_time_into_point_source 5.0
Transform gps_time.
  -set_gps_time 113556962.005715
  -translate_gps_time 40.50
  -adjusted_to_week
  -week_to_adjusted 1671
Transform RGB/NIR colors.
  -set_RGB 255 0 127
  -set_RGB_of_class 9 0 0 255
  -scale_RGB 2 4 2
  -scale_RGB_down (by 256)
  -scale_RGB_up (by 256)
  -switch_R_G -switch_R_B -switch_B_G
  -copy_R_into_NIR -copy_R_into_intensity
  -copy_G_into_NIR -copy_G_into_intensity
  -copy_B_into_NIR -copy_B_into_intensity
  -copy_intensity_into_NIR
  -switch_RGBI_into_CIR
  -switch_RGB_intensity_into_CIR
  -force_RGB
Transform attributes in "Extra Bytes".
  -scale_attribute 0 1.5
  -translate_attribute 1 0.2
  -copy_user_data_into_attribute 0
Supported LAS Inputs
  -i lidar.las
  -i lidar.laz
  -i lidar1.las lidar2.las lidar3.las -merged
  -i *.las - merged
  -i flight0??.laz flight1??.laz
  -i terrasolid.bin
  -i esri.shp
  -i nasa.qi
  -i lidar.txt -iparse xyzti -iskip 2 (on-the-fly from ASCII)
  -i lidar.txt -iparse xyzi -itranslate_intensity 1024
  -lof file_list.txt
  -stdin (pipe from stdin)
  -rescale 0.01 0.01 0.001
  -rescale_xy 0.01 0.01
  -rescale_z 0.01
  -reoffset 600000 4000000 0
Fast AOI Queries for LAS/LAZ with spatial indexing LAX files
  -inside min_x min_y max_x max_y
  -inside_tile ll_x ll_y size
  -inside_circle center_x center_y radius
Supported LAS Outputs
  -o lidar.las
  -o lidar.laz
  -o xyzta.txt -oparse xyzta (on-the-fly to ASCII)
  -o terrasolid.bin
  -o nasa.qi
  -odir C:\data\ground (specify output directory)
  -odix _classified (specify file name appendix)
  -ocut 2 (cut the last two characters from name)
  -olas -olaz -otxt -obin -oqfit -optx -opts (specify format)
  -stdout (pipe to stdout)
  -nil    (pipe to NULL)
LAStools (by info@rapidlasso.de) version 190711
usage:
las2las -i *.las -utm 13N
las2las -i *.laz -first_only -olaz
las2las -i *.las -drop_return 4 5 -olaz
las2las -latlong -target_utm 12T -i in.las -o out.las
las2las -i in.laz -target_epsg 2972 -o out.laz
las2las -set_point_type 0 -lof file_list.txt -merged -o out.las
las2las -remove_vlr 2 -scale_rgb_up -i in.las -o out.las
las2las -i in.las -keep_xy 630000 4834500 630500 4835000 -keep_z 10 100 -o out.las
las2las -i in.txt -iparse xyzit -keep_circle 630200 4834750 100 -oparse xyzit -o out.txt
las2las -i in.las -remove_padding -keep_scan_angle -15 15 -o out.las
las2las -i in.las -rescale 0.01 0.01 0.01 -reoffset 0 300000 0 -o out.las
las2las -i in.las -set_version 1.2 -keep_gpstime 46.5 47.5 -o out.las
las2las -i in.las -drop_intensity_below 10 -olaz -stdout > out.laz
las2las -i in.las -last_only -drop_gpstime_below 46.75 -otxt -oparse xyzt -stdout > out.txt
las2las -i in.las -remove_all_vlrs -keep_class 2 3 4 -olas -stdout > out.las
las2las -h

---------------

if you find bugs let me (info@rapidlasso.de) know.
