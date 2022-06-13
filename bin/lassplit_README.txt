****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

  lassplit:

  Splits the input file(s) into several output files based on
  various parameters. By default lassplit split a combined LAS
  file into its original, individual flight lines by splitting
  based on the point source ID of the points. Other options are
  to split '-by_classification', '-by_gps_time_interval 100', 
  '-by_x_interval 15.5', '-by_y_interval 2.5', '-by_z_interval 11.5',
  '-by_intensity_interval 10', '-by_scan_angle_interval 5',  
  '-by_user_data_interval 16', '-by_attribute_interval 0 0.5' or
  '-by_attribute_interval 1 1.0' 

  Instead of splitting LAS/LAZ/BIN/ASCII files based on these
  listed attributes they can also be split into many numbered
  files that each contain the same number of points (except for
  the last one) with the '-split 100000000' option, which would
  split each time after 100 million points were written.

  The header information from the first file (e.g. VLRs, scale
  factor offset, ...) is used for each written output file while
  these LAS header attributes are updated:

    * number_of_point_records
    * number_of_points_by_return[5]
    * max_x, min_x, max_y, min_y, max_z, and min_z

  In case the '-merged' option is used, it may become necessary
  to change the x_scale_factor, y_scale_factor, z_scale_factor
  and/or the x_offset, y_offset, z_offset, values. In order to
  have more control over this process the user can reset those
  in the command line with

     -rescale 0.01 0.01 0.001
     -reoffset 600000 4000000 0

  For updates check the website or join the LAStools google group.
  
  https://rapidlasso.de/
  http://groups.google.com/group/lastools/

****************************************************************
see also:
  lasmerge - Merge or split lidar data files by number of points
****************************************************************

example usage:

>> lassplit -i lidar.las -olaz

splits the file "lidar.las" based on the point source ID into
files named "lidar.XXXXX.laz" where XXXXX corresponds to the
point source ID.

>> lassplit -i *.las -olas

splits all files that match "*.las" based on the point source
ID into files named "*.XXXXX.las" where XXXXX corresponds to
the point source ID.

>> lassplit -i *.laz -olaz -recover_flightlines

this is a special option for the case that the LiDAR tiles do
not have the point source ID correctly populated but have a proper
GPS time stamp per point. here lassplit recovers the most likely
flightlines based on conituity (or rather gaps) in the GPS times.

>> lassplit -i *.las -merged -o flightlines.laz

merges files that match "*.las" and splits the result based on
the point source ID into files named "flightlines.XXXXX.laz" where
XXXXX corresponds to the point source ID.

>> lassplit -i *.las -merged -o chopped.las -digits 2 -split 10000000

merges all *.las files into one and then splits the result into
several output files that contain ten million points each and that
are called chopped.00.las, chopped.01.las, chopped.02.las, ...

>> lassplit -i big.txt -iparse xyztp -o split.laz -split 500000000

split the ASCII file "big.txt" that could, for example, contain 25
billion points one and then split it into several compressed
output LAZ files that contain 500 million points each that are
called split.00000.laz, split.00001.laz, split.00002.laz, ...

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
-digits 5                            : number of digits to use for naming of split files
                                     : default: 7
-split 20000000                      : split into containing 20 million points each
-by_classification                   : split based on the classification code
-by_gps_time_interval 2.0            : split points into intervals of 2 seconds
-by_intensity_interval 64            : split points based on intensty intervals of 64
-by_x_interval 2.0                   : split points based on x coordinate intervals of 2.0
-by_y_interval 3.0                   : split points based on y coordinate intervals of 3.0
-by_z_interval 10.0                  : split points based on z coordinate intervals of 10.0
-by_scan_angle_interval 2.5          : split points based on scan angle intervals of 2.5
-by_user_data_interval 16            : split points based on user data intervals of 16
-by_attribute_interval 0 1.0         : split points based on attribute with index in intervals of 1.0
-recover_flightlines                 : split points after recovering flight lines from 5 second GPS time interruptions
-recover_flightlines_interval 20     : split points after recovering flight lines from 20 second GPS time interruptions
-ilay                                : apply all LASlayers found in corresponding *.lay file on read
-ilay 3                              : apply first three LASlayers found in corresponding *.lay file on read
-ilaydir E:\my_layers                : look for corresponding *.lay file in directory E:\my_layers

****************************************************************

for more info:

E:\LAStools\bin>lassplit -h
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
  -drop_RGB_red 5000 20000
  -keep_RGB_green 30 100
  -drop_RGB_green 2000 10000
  -keep_RGB_blue 0 0
  -keep_RGB_nir 64 127
  -keep_RGB_greenness 200 65535
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
  -map_intensity map_file.txt
  -copy_RGB_into_intensity
  -copy_NIR_into_intensity
  -copy_attribute_into_intensity 0
  -bin_gps_time_into_intensity 0.5
Transform scan_angle.
  -set_scan_angle 0.0
  -scale_scan_angle 1.944445
  -translate_scan_angle -5
  -translate_then_scale_scan_angle -0.5 2.1
Transform register.
  -copy_attribute_into_register 0 0
  -scale_register 0 1.5
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
  -copy_user_data_into_classification
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
  -map_user_data map_file.txt
  -copy_scanner_channel_into_user_data
  -copy_attribute_into_user_data 1
  -add_scaled_attribute_to_user_data 0 10.0
Modify the point source ID.
  -set_point_source 500
  -change_point_source_from_to 1023 1024
  -map_point_source map_file.txt
  -copy_user_data_into_point_source
  -copy_scanner_channel_into_point_source
  -copy_attribute_into_point_source 0
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
  -scale_RGB_to_8bit (only scales down 16 bit values)
  -scale_RGB_to_16bit (only scales up 8 bit values)
  -switch_R_G -switch_R_B -switch_B_G
  -copy_R_into_NIR -copy_R_into_intensity
  -copy_G_into_NIR -copy_G_into_intensity
  -copy_B_into_NIR -copy_B_into_intensity
  -copy_intensity_into_NIR
  -switch_RGBI_into_CIR
  -switch_RGB_intensity_into_CIR
Transform attributes in "Extra Bytes".
  -scale_attribute 0 1.5
  -translate_attribute 1 0.2
  -copy_user_data_into_attribute 0
  -copy_z_into_attribute 0
  -map_attribute_into_RGB 0 map_height_to_RGB.txt
Ignore points based on classifications.
  -ignore_class 7
  -ignore_class 0 1 7 33
Ignore points based on return type.
  -ignore_first -ignore_first_of_many
  -ignore_last -ignore_last_of_many
  -ignore_intermediate
  -ignore_single
Ignore points based on flags.
  -ignore_synthetic -ignore_keypoint
  -ignore_withheld -ignore_overlap
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
LAStools (by info@rapidlasso.de) version 211206 (commercial)
usage:
lassplit -i *.las
lassplit -i *.laz -merged -o flightlines.las
lassplit -i *.laz -merged -recover_flightlines -o flightlines.las
lassplit -i lidar.laz -by_gps_time_interval 5.0 -o segments.laz
lassplit -i lidar.laz -by_intensity_interval 32 -o intensities.laz
lassplit -i lidar.laz -by_classification -o slices.laz
lassplit -i lidar.laz -by_user_data_interval 8 -o slices.laz
lassplit -i forest.laz -by_attribute_interval 0 1.0 -o trees.laz
lassplit -i lidar.laz -by_x_interval 5.0 -o slices_x.laz
lassplit -i lidar.laz -by_y_interval 2.5 -o slices_y.laz
lassplit -i lidar.laz -by_z_interval 1.0 -o slices_z.laz
lassplit -i *.las -merged -split 100000000 -digits 2
lassplit -h

---------------

if you find bugs let us (support@rapidlasso.com) know.
