****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

  lasclassify:

  This tool classifies buildings and high vegetation (i.e. trees)
  in LAS/LAZ files. This tool requires that the bare-earth points
  have already been identified (e.g. with lasground) and that the
  elevation of each point above the ground was already computed
  with lasheight (which stores this height in the 'user data' field
  of each point). In case the height above ground is stored as an
  additional attribute in the "extra bytes" you can specify how to
  find it with '-height_in_attribute 0' on '-height_in_attribute 2'.

  The tool essentially tries to find neighboring points that are
  at least 2 meter (or 6 feet) above the ground and form '-planar 0.1'
  (= roofs) or '-rugged 0.4' (= trees) regions. You can change the
  above the ground threshold with '-ground_offset 5'.

  If your data is very noisy the tool have trouble finding planar
  regions. Try playing with the '-planar 0.1' default. Often the
  flight lines are not properly aligned which will also destroy
  planarity. Here you may be better served to process your flight
  lines separately from another.

  It is also important to tell the tool whether the horizontal
  and vertical units are meters (which is assumed by default)
  or '-feet' or '-elevation_feet'. Should the LAS file contain
  projection information then there is no need to specify this
  explicitly. If the input coordinates are in an earth-centered
  or a longlat representation, the file needs converted to, for
  example, a UTM projection first.

  The experienced user can fine-tune the algorithm by specifing
  a threshold until which points are considered planar with
  '-planar 0.2'. This would roughly correspond to a standard
  deviation of up to 0.2 meters that neighboring points can have
  from the planar region they share. The default is 0.1 meters.

  A too low point density will usually cause lasclassify to fail.
  with 'lasinfo -i lidar.las -cd' you can check if you have at
  least 2 pulses per square meter which is the minimum that is
  needed for somewhat reliable roof detection. If you have less
  than 2 pulses per square meter you can enlargen the planes with
  which lasclassify is searching with '-step 4' to 4 by 4 meters,
  for example. The default is 2 meters.

  Please license from info@rapidlasso.de to use lasclassify
  commercially. Please note that the unlicensed version will set
  intensity, gps_time, user data, and point source ID to zero,
  slightly change the LAS point order, and randomly add a tiny
  bit of white noise to the points coordinates.

  For updates check the website or join the LAStools mailing list.

  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @lastools

****************************************************************

example usage:

>> lasground -i lidar.las -o lidar_with_bare_earth.las -city
>> lasheight -i lidar_with_bare_earth.las -o lidar_with_heights.las
>> lasclassify -i lidar_with_heights.las -o lidar_classified.las

finds the ground points with lasground, computes the height of each
point with lasheight, and classifies buildings and high vegetation
with the default settings.

>> lasground -i lidar.las -o lidar_with_bare_earth.las -city -feet -elevation_feet
>> lasheight -i lidar_with_bare_earth.las -o lidar_with_heights.las
>> lasclassify -i lidar_with_heights.las -o lidar_classified.las -feet -elevation_feet

the same as above for LIDAR where both horizontal and vertical units
are in feet instead of in meters (meters are assumed by default unless
there is projection information in the LAS file saying otherwise).

>> lasclassify -i *.las

classifies all LAS files with the default settings (the LAS files need
to already have ground points classified and point heigths computed).

>> lasclassify -i *.laz

classifies all LAZ files with the default settings (the LAZ files need
to already have ground points classified and point heigths computed).

>> lasclassify -i *.laz -planar 0.2

experimental. same as above but more points will be joined into roofs.

****************************************************************

overview of all tool-specific switches:

-v                                   : more info reported in console
-vv                                  : even more info reported in console
-quiet                               : nothing reported in console
-version                             : reports this tool's version number
-fail                                : fail if license expired or invalid
-gui                                 : start with files loaded into GUI
-cores 4                             : process multiple inputs on 4 cores in parallel
-ignore_class 7 18                   : ignores points with specified classification codes
-ignore_first                        : ignores first returns
-ignore_last                         : ignores last returns
-ignore_intermediate                 : ignores intermediate returns
-step 1.5                            : grid cell size for planar / non-planar analysis
                                       default is 2.0
-ground_offset 1.5                   : only points that are 1.5 meters above ground are considered
                                       default is 2.0
-planar 0.09                         : grid cell points up to this standard deviation are potential roofs
                                       default is 0.1
-rugged 0.3                          : grid cell points above this standard deviation are potential vegetation
                                       default is 0.4
-sub                                 : number of substep used in analysis
                                       default ranges from 2 to 8 depending on value of requested '-step'
-small_buildings                     : don't discard overly small buildings
-small_trees                         : don't discard overly small trees
-no_gutters                          : don't try to complete roof along edges
-wide_gutters                        : try harder to complete roof along edges
-keep_overhang                       : don't remove vegetation points that overhang building and are close in height
-height_in_attribute 0               : use height above ground stored in extra attribute 0 instead of from user data
-remain_buffered                     : write buffer points to output when using '-buffered 25' on-the-fly buffering  
-ilay                                : apply all LASlayers found in corresponding *.lay file on read
-ilay 3                              : apply first three LASlayers found in corresponding *.lay file on read
-ilaydir E:\my_layers                : look for corresponding *.lay file in directory E:\my_layers
-olay                                : write or append classification changes to a LASlayers *.lay file
-olaydir E:\my_layers                : write the output *.lay file in directory E:\my_layers

****************************************************************

for more info:

E:\LAStools\bin>lasclassify -h
Filter points based on their coordinates.
  -keep_tile 631000 4834000 1000 (ll_x ll_y size)
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
  -keep_RGB_green 30 100
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
  -switch_x_y -switch_x_z -switch_y_z
  -clamp_z_below 70.5
  -clamp_z 70.5 72.5
  -copy_attribute_into_z 0
  -copy_intensity_into_z
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
  -copy_NIR_into_intensity
Transform scan_angle.
  -scale_scan_angle 1.944445
  -translate_scan_angle -5
  -translate_then_scale_scan_angle -0.5 2.1
Change the return number or return count of points.
  -repair_zero_returns
  -set_return_number 1
  -set_extended_return_number 10
  -change_return_number_from_to 2 1
  -set_number_of_returns 2
  -set_number_of_returns 15
  -change_number_of_returns_from_to 0 2
Modify the classification.
  -set_classification 2
  -set_extended_classification 0
  -change_classification_from_to 2 4
  -classify_z_below_as -5.0 7
  -classify_z_above_as 70.0 7
  -classify_z_between_as 2.0 5.0 4
  -classify_intensity_above_as 200 9
  -classify_intensity_below_as 30 11
  -classify_intensity_between_as 500 900 15
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
Modify the point source ID.
  -set_point_source 500
  -change_point_source_from_to 1023 1024
  -copy_user_data_into_point_source
  -copy_scanner_channel_into_point_source
  -merge_scanner_channel_into_point_source
  -split_scanner_channel_from_point_source
  -bin_Z_into_point_source 200
  -bin_abs_scan_angle_into_point_source 2
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
  -copy_RGB_into_intensity
  -copy_R_into_NIR -copy_R_into_intensity
  -copy_G_into_NIR -copy_G_into_intensity
  -copy_B_into_NIR -copy_B_into_intensity
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
  -olas -olaz -otxt -obin -oqfit (specify format)
  -stdout (pipe to stdout)
  -nil    (pipe to NULL)
LAStools (by info@rapidlasso.de) version 171030 (academic)
usage:
lasclassify -i in.las -o out.laz
lasclassify -i in.laz -o out.las -feet -elevation_feet
lasclassify -i in.laz -o out.las -verbose -planar 0.2
lasclassify -i in.laz -o out.las -verbose -planar 0.15 -ground_offset 1.5 -wide_gutters
lasclassify -i *.las
lasclassify -i *.laz -verbose -feet -elevation_feet
lasclassify -h

---------------

if you find bugs let me (info@rapidlasso.de) know.
