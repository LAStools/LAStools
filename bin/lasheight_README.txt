****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

  lasheight:

  This tool computes the height of each LAS point above the
  ground. This assumes that grounds points have already been
  classified (classification == 2) so they can be identified
  and used to construct a ground TIN. The ground points can
  also be in an separate file '-ground_points ground.las' or
  '-ground_points dtm.csv -parse ssxyz'.

  Should the input coordinates be in some representation where
  the z-axis does not correspond to elevation, such as in an
  earth-centered representation, then the file needs converted
  to, for example, a UTM projection first.

  The tool reads LIDAR in LAS/LAZ/ASCII format, triangulates
  the ground points into a TIN (or whatever point class(es)
  were selected with '-class 8' or '-classification 2 8'), and
  calculates the elevation of each point with respect to this
  TIN. By default the resulting heights are scaled with a factor
  of 10.0, quantized & clamped into an unsigned char between
  0 and 255, and stored in the "user data" field of each point.
  Change the default scale factor of 10.0 with '-scale_u 20.0'
  or disable the storage in the "user data" field with option
  '-do_not_store_in_user_data' or by storing the height above
  the ground as "extra bytes" with '-store_as_extra_bytes' as
  an 0.01 scaled signed two-byte short [cm resolution] or with
  '-store_precise_as_extra_bytes' as an 0.001 scaled signed
  four-byte integer [mm resolution].

  Alternatively - to avoid quantizing and clamping - you can
  '-replace_z' the elevation value of each point with the computed
  height. That means that afterwards all ground points will have
  an elevation of zero and all other points will have an elevation
  that equals their relative height above (or below) the ground TIN
  at their x and y location. In a sense this will "normalize" the
  elevations of points in respect to their surrounding ground truth.
  If you add the '-replace_z' option the resulting heights are *not*
  scaled with a factor of 10.0, quantized & clamped into an unsigned
  char between 0 and 255, and stored in the "user data" field of each
  point ... unless you add the explicit '-store_in_user_data' option.

  You can also use the height to change the point classification
  with '-classify_below -1.0 7' or '-classify_above 100.0 10' and
  also '-classify_between 0.5 2.0 3 -classify_between 2.0 5.0 4'.

  Another alternative is to use the computed height to eliminate
  points with a particular ground height above or below a threshold
  with the options '-drop_below 1.5' or '-drop_above 6.8'.

  You can ignore certain point classes from being dropped and/or
  classified with '-ignore_class 6 7'.

  If there are too few ground points to construct a ground TIN or
  the ground points form a degenerate triangulation no heights can
  be computed. By default the files are simply copied (and in case
  of '-replace_z' all elevations are zeroed). Alternatively these
  files can be skipped with the command-line option '-skip_files'.

  This tool can also be used to convert from Geoid to Ellipoidal
  heights. For this we need the Geoid model as a grid of points
  "geoid.txt", "geoid.las" or "geoid.xyz" whose elevations express
  the difference between the ellipsoid and the geoid. Simply run:

  lasheight -i lidar.las -ground_points geoid.txt -replace_z -odix _geoid

  When using external ground points with '-ground_points geoid.laz'
  lasheight will *not* use all points in the file but cut out a
  "generous portion" surrounding the bounding box of the points
  whose height is to be computed. That "generous cutting" may fail
  when the 'geoid.laz' file has sparsely spaced points. Using the
  '-all_ground_points' option forces lasheight to use all points. 
 
  Please license from info@rapidlasso.de before using
  lasheight commercially.

  For updates check the website or join the LAStools mailing list.

  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @lastools

****************************************************************

example usage:

>> lasheight -i *.las

computes heights for all LAS files that match '*.las' and stores
them quantized and clamped into the "user data" field (an 8-bit
unsigned char) of each point record.

>> lasheight -i *.las -class 3

the same as above but uses points with classification 3 instead
of the default 2 to create the TIN that serves as the reference
elevation against which the heights are computed.

>> lasheight -i *.laz -olaz

the same as above for LASzip-compressed input and output.

>> lasheight -i *.las -replace_z

replaces the z coordinate of each point with the computed height
to avoid quantizing and clamping their value at the expense of
losing the original z elevation values.

>> lasheight -i lidar.las -o brush.las -drop_below 1.0 -drop_above 3.0

kepps only those points who are between 1 and 3 units above the
ground.

>> lasheight -i lidar.las -o heights.txt -oparse u

stores the heights to a file called 'heights.txt' as quantized
and clamped "unsigned chars".

>> lasheight -i lidar.las -replace_z -o heights.txt -oparse z

stores the heights to a file called 'heights.txt' as floating-
point values of the same precision as the original elevations.

****************************************************************

overview of all tool-specific switches:

-v                                   : more info reported in console
-vv                                  : even more info reported in console
-quiet                               : nothing reported in console
-wait                                : wait for <ENTER> in the console at end of process
-version                             : reports this tool's version number
-fail                                : fail if license expired or invalid
-gui                                 : start with files loaded into GUI
-cores 4                             : process multiple inputs on 4 cores in parallel
-ignore_class 1 3 4 5 6 7 9          : ignores points with specified classification codes
-ground_points external.laz          : use points from external file as ground points
-parse ssxysz                        : in case external ground point file is in ASCII format use this parse string to access x, y, and z
-all_ground_points                   : use all external ground points instead of generously clipping them to bounding box of current file
-class 2 8                           : specifies to use points with class 2 *and* with class 8 as ground points
-drop_below -0.2                     : drop points that are 0.2 units vertically below Delaunay TIN of ground points
-drop_above 60.0                     : drop points that are 60.0 units vertically above Delaunay TIN of ground points
-drop_between 11.0 12.0              : drop points that are between 11.0 and 12.0 units vertically above TIN of ground points
-classify_below -0.5 7               : classify points that are 0.5 units vertically below Delaunay TIN of ground points as 7
-classify_above 60.0 7               : classify points that are 60.0 units vertically above Delaunay TIN of ground points as 7
-classify_between 0.5 2.0 4          : classify points that are between 0.5 and 2.0 units vertically above TIN of ground points as 4
-replace_z                           : store heights to z coordinate (instead of in dm in user_data field). original elevations are lost.
-store_as_extra_bytes                : store height with cm precision as short in "extra bytes" (instead of in dm in user_data field)
-store_precise_as_extra_bytes        : store height with mm precision as int in "extra bytes" (instead of in dm in user_data field)
-store_in_user_data                  : store also in user_data field (even when storing to z coordinate or as extra bytes)
-scale_u 0.2                         : scale height quantized to 20 cm increments in user_data field (instead of default dm = 10 cm)
-do_not_store_in_user_data           : do not modify the user_data field at all
-skip_files                          : skip (instead of the default copy) files that have an insufficient number of ground points
-remain_buffered                     : write buffer points to output when using '-buffered 25' on-the-fly buffering  
-dont_remove_empty_files             : do not remove files that have zero points remaining from disk
-ilay                                : apply all LASlayers found in corresponding *.lay file on read
-ilay 3                              : apply first three LASlayers found in corresponding *.lay file on read
-ilaydir E:\my_layers                : look for corresponding *.lay file in directory E:\my_layers
-olay                                : write or append classification changes to a LASlayers *.lay file
-olaydir E:\my_layers                : write the output *.lay file in directory E:\my_layers

****************************************************************

for more info:

E:\LAStools\bin>lasheight -h
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
  -copy_RGB_into_intensity
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
  -copy_R_into_NIR -copy_R_into_intensity
  -copy_G_into_NIR -copy_G_into_intensity
  -copy_B_into_NIR -copy_B_into_intensity
  -copy_intensity_into_NIR
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
LAStools (by info@rapidlasso.de) version 171215 (academic)
usage:
lasheight -i *.las -v
lasheight -i *.laz -olaz -quiet
lasheight -i *.las -classification 2 8 -replace_z
lasheight -i *.las -ground_points ground.las -replace_z
lasheight -i *.las -ground_points geoid.xyz -replace_z
lasheight -i in.las -o out.las -drop_below 1.5
lasheight -i in.las -o out.txt -drop_above 6.5 -oparse xyzu
lasheight -i in.las -o out.las -drop_between 0.5 2.5
lasheight -i in.laz -o out.laz -classify_below -1 7
lasheight -i in.laz -o out.laz -classify_above 100 10
lasheight -i in.laz -o out.laz -classify_between 0.5 2 3 -classify_between 2 5 4 -ignore_class 6
lasheight -i in.laz -o out.laz -replace_z
lasheight -i in.las -o out.txt -replace_z -oparse z
lasheight -i tiles_ground\*.laz -store_as_extra_bytes -odir tiles_height -olaz -cores 4
lasheight -i tiles_ground\*.laz -store_precise_as_extra_bytes -odir tiles_height -olaz -cores 4
lasheight -h

---------------

if you find bugs let me (info@rapidlasso.de) know.
