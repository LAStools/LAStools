****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

  lasprobe:

  This tool probes the elevation of the LIDAR for a given x and y
  location and reports it to a text file or to stdout.

  The tool reads LIDAR in LAS/LAZ/ASCII format, triangulates the
  relevant points into a TIN. For classified data sets containing
  a mix of ground and vegetation/building points it is imperative
  to specify the points which should be used for this calclation
  (i.e. usually '-keep_class 2' or '-keep_class 2 8').

  The tool collects all LiDAR points that are within 'step' meters
  of the probed location. This can be changed with the '-step 2'
  parameter which would shrink this circle to a radius to 2 meters.

  If the LiDAR is spatially indexed (i.e. a *.lax file exists) this
  collection of points will be accelerated significanly. For repeat
  probing running lasindex before lasprobe is recommended.

  The output report defaults to stdout unless you specify an output
  file with '-o report.txt'. Standard output is only the elevation
  value, unless you add '-xyz' to the command line so that the x abd

  For updates check the website or join the LAStools mailing list.

  https://rapidlasso.de/
  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools.guy/
  http://linkedin.com/groups?gid=4408378

  Martin @lastools

****************************************************************

example usage:

>> lasprobe -i ..\data\fusa.laz -probe 277760.00 6122260
52.14

probes all LiDAR points including buildings, wires and vegetation
and outputs the computed elevation "52.14" to stdout

----------------------------------------------------------------

>> lasprobe -i ..\data\fusa.laz -probe 277760.00 6122260 -o mist.txt

probes all LiDAR points including buildings, wires and vegetation
and outputs the computed elevation "52.14" to text file 'mist.txt'

----------------------------------------------------------------

>> lasprobe -i ..\data\fusa.laz -probe 277760.00 6122260 -xyz
277760.00 6122260.00 52.14

probes all LiDAR points including buildings, wires and vegetation
and outputs the input probe coordinates together with the computed
elevation "52.14" to stdout

----------------------------------------------------------------

>> lasprobe -i ..\data\fusa.laz -probe 277760.00 6122260 -xyz -o mist.txt

probes all LiDAR points including buildings, wires and vegetation
and outputs the input probe coordinates together with the computed
elevation "52.14" to text file 'mist.txt'

----------------------------------------------------------------

>> lasprobe -i ..\data\fusa.laz -keep_class 2 -probe 277760.00 6122260
42.66

probes only LiDAR with classification code 2 (aka ground) and
outputs the computed elevation "42.66" to stdout

----------------------------------------------------------------

>> lasprobe -i ..\data\fusa.laz -keep_class 2 -probe 277760.00 6122260
277760.00 6122260 42.66

probes only LiDAR with classification code 2 (aka ground) and
outputs the input probe coordinates together with the computed
 elevation "42.66" to stdout

----------------------------------------------------------------

>> lasprobe -i ..\data\fusa.laz -probe 277760.00 6122250
WARNING: sampling infinite TIN triangle suggests insufficient LIDAR coverage.
52.20

probes all LiDAR points but - as the WARNING tells us - at a 
location that is (slightly) outside the LiDAR coverage. This
suggests that the computed elevation "52.20" is not as stable
of a value as an elevation probed more inside the LiDAR.

In case your probe is well inside the LiDAR coverage you may
need to increase your '-step 5' default to '-step 10' or more

----------------------------------------------------------------

>> lasprobe -i ..\data\fusa.laz -keep_class 2 -probe 277812.23 6122332.31
WARNING: none of 277573 points covered probe. all filtered or too far.

probes only the ground points at the center location of a huge building
such that all ground points are farther away than 5 meters.

----------------------------------------------------------------

>> lasprobe -i ..\data\fusa.laz -keep_class 2 -probe 277812.23 6122332.31
WARNING: none of 277573 points covered probe. all filtered or too far.

>> lasprobe -i ..\data\fusa.laz -keep_class 2 -probe 277812.23 6122332.31 -step 10
WARNING: none of 277573 points covered probe. all filtered or too far.

>> lasprobe -i ..\data\fusa.laz -keep_class 2 -probe 277812.23 6122332.31 -step 20
WARNING: sampling infinite TIN triangle suggests insufficient LIDAR coverage.
45.26

>> lasprobe -i ..\data\fusa.laz -keep_class 2 -probe 277812.23 6122332.31 -step 25
45.28

probes only the ground points at the center location of a huge building
such that all ground points are farther away than 5 and also 10 meters. 
At a step of 20 meters the circle of LiDAR points still does not fully
surround the probed location. At a step of 25 meters we get a stable
reading for the elevation, but recall that this elevation is now in the
center of the large police station interpolated from the grounds points
outside of of building

for more info:

E:\software\LAStools\bin>lasprobe -h
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
  -clamp_RGB_to_8bit
  -set_NIR 65535
  -scale_NIR 2
  -scale_NIR_down (by 256)
  -scale_NIR_up (by 256)
  -scale_NIR_to_8bit (only scales down 16 bit values)
  -scale_NIR_to_16bit (only scales up 8 bit values)
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
Transform using "LASregisters".
  -copy_attribute_into_register 0 0
  -scale_register 0 1.5
  -translate_register 1 10.7
  -add_registers 0 1 3
  -multiply_registers 0 1 2
  -copy_intensity_into_register 0
  -copy_R_into_register 1
  -copy_G_into_register 2
  -copy_B_into_register 3
  -copy_NIR_into_register 4
  -copy_register_into_intensity 1
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
LAStools (by info@rapidlasso.de) version 210628
usage:
lasprobe -i fusa.laz -probe 277760.00 6122260
lasprobe -i fusa.laz -keep_class 2 -probe 277760.00 6122260
lasprobe -i fusa.laz -keep_class 2 -probe 277812.23 6122332.31 -step 10
lasprobe -i fusa.laz -keep_class 2 -probe 277812.23 6122332.31 -step 25
lasprobe -i fusa.laz -keep_class 2 -probe 277760.00 6122260 -xyz
lasprobe -i fusa.laz -keep_class 2 -probe 277760.00 6122260 -o probe.txt
lasprobe -i fusa.laz -keep_class 2 -probe 277760.00 6122260 -o probe.txt -xyz
lasprobe -h

---------------

if you find bugs let me (info@rapidlasso.de) know.
