****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

  lasthin:

  A fast LIDAR thinning algorithm for LAS/LAZ/ASCII. It places
  a uniform grid over the points and within each grid cell keeps
  only the point with the lowest (or '-highest' or '-random') Z
  coordinate. When keeping '-random' points you can in addition
  specify a '-seed 232' for the random generator. You can also
  keep the point that is closest to the center of each cell with
  the option '-central'.

  Instead of removing the thinned out points from the output file
  you can also request to classify them with '-classify_as 8' or
  mark thinned out points by flagging them as '-flag_as_withheld'
  Later use the '-drop_withheld' or '-keep_withheld' filters to
  get either the thinned points or their complement. You can also
  use '-flag_as_keypoint' to flag all the points that survive the
  thinning operation.

  For adaptive thinning use '-adaptive 0.2 5.0' where 0.2 specifies
  the vertical tolerance that a TIN through the thinned points is
  allowed to deviate from the complete set of points and 5.0 the
  maximum distance between points. The default for the latter is
  10.0 if you only specify '-adaptive 0.15'.

  For 2 meter iso-contour creation you can optimize the thinning
  with '-contours 2.0'. This is in addition to deciding on a good
  step size with '-step 0.5'. It will choose the surviving points
  to be vertically as far away as possible from the contours which
  tends to result in cleaner, less jaggy countours.

  Alternatively use the ‘-percentile 40’ mode to select the point
  that is closest to the specified percentile of 40 of all point
  elevations within a grid cell. Using the more elaborate version
  ‘-percentile 40 10’ will in addition only act when there are at
  least 10 points in that cell.

  In order to process very large but sparse grids such as, for
  example, a single but very long diagonal flight line, it is
  beneficial to use the '-sparse' option to avoid exceeding the
  main memory (and start thrashing).

  It is also possible to "thicken" your points as you thin them
  to simulate a diameter for the laser beam. The '-subcircle 0.1'
  option will replicate each point 8 times in a discrete circle
  with radius 0.1 around every original input point. This makes
  sense in combination with '-highest' in order to create a nice
  set of points for subsequent CHM or DSM construction. By adding
  a second value '-subcircle 0.2 -0.05' you can lower of raise the
  z value of the 8 points on the discrete circle by the specified
  amount, here they would be 0.05 units lower than the original,
  which might be useful for subsequent tree top detection.

  You can exclude certain point classes from the thinning with
  option '-ignore_class 2' or '-ignore_class 3 4 5 6' and they
  will all be written back to the thinned file. Then the thinning 
  operation will be applied exclusively to the other points.

  Obviously you can always combine filtering, for example to keep
  only last returns ('-last_only') or to keep only two particular
  classifications ('-keep_class 2 8'), with the thinning. There
  are many other filters to choose from. Simply run 'lasthin -h'
  to lists them all.

  The grid spacing default is 1 unit and it can be changed, for
  example to 5 units, with '-step 5'.
 
  Please license from info@rapidlasso.de to use lasthin
  commercially.

  For updates check the website or join the LAStools mailing list.

  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @lastools

****************************************************************

example usage:

>> lasthin -i *.las -odix _thinned

thins all LAS files with the grid spacing default of 1 unit
and keeps the lowest point per grid cell and forms output file
names by adding appendix '_thinned' to the input file names.

>> lasthin -i *.laz -odix _thinned -olaz

same as above but with LAZ files

>> lasthin -i *.txt -iparse xyzt -oparse xyzt -odix _thinned -otxt

same as above but with ASCII files

>> lasthin -i in.las -o out.las

does LAS thinning with the grid spacing default of 1 unit
and keeps the lowest point per grid cell

>> lasthin -i in.las -o out.las -highest -step 2.0

does point thinning with a grid spacing of 2.0 units and
keeps the highest point per grid cell

>> lasthin -i in.laz -step 0.5 -o out.laz -random

does point thinning with a grid spacing of 0.5 units and
keeps a random point per grid cell

>> lasthin -i in.laz -adaptive 0.2 5.0 -o out.laz

thins out all points that can be removed from a TIN without 
deviating more than 0.2 vertical units from the original while
keeping the maximum horizontal distance to 5.0 units.

>> lasthin -i in.laz -ignore_class 1 3 4 5 6 7 9 -adaptive 0.1 -classify_as 8 -o out.laz

intends to only operate on ground points (class 8) and classifies
those points with classification code 8 that form a TIN that deviates
less than 0.1 vertical units from the original TIN while keeping the
maximum horizontal distance between points to 10.0 units.

>> lasthin -i in.laz -step 5.0 -percentile 50 20 -classify_as 8 -o out.laz -random

keeps all the points but sets the classification of the point
that is closest to the 50th percentile in z to 8. this is
done in each 5.0 unit by 5.0 unit cell with 20 or more points

>> lasthin -i in.laz -step 0.5 -o out.laz -sparse

does point thinning with a grid spacing of 0.5 units using
a sparse grid representation

>> lasthin -i in.las -o out.las -highest -step 0.5 -subcircle 0.15

does point thinning with a grid spacing of 0.5 units but
thickens every input point by adding a discrete ring of 8
new points that form a circle with radius 0.15 around the
input point before and then keeps only the highest of those
points per grid cell

>> lasthin -i in.laz -o out.laz -first_only

does LIDAR thinning with a grid spacing of 1 unit but keeps
the highest points while considering only first returns

>> lasthin -i in.las -o out.las -keep_class 2 -keep_class 3

looks only at the points classfied as 2 or 3 from in.las and
thins them with a grid spacing of 1 unit 

>> lasthin -i file_list.txt -lof -o combined.laz

looks at all the points from all the LAS or LAZ files listed
in the text file 'file_list.txt', thins them with a grid
spacing of 1 unit and outputs them compressed to 'combined.laz'

****************************************************************

overview of all tool-specific switches:

-v                                   : more info reported in console
-vv                                  : even more info reported in console
-quiet                               : nothing reported in console
-version                             : reports this tool's version number
-fail                                : fail if license expired or invalid
-gui                                 : start with files loaded into GUI
-cores 4                             : process multiple inputs on 4 cores in parallel
-ignore_class 1 3 4 5 6 7 9          : ignores points with specified classification codes
-step 1.5                            : grid cell size for thinning / classifying / flagging
                                       default is 2.0
-subcircle 0.1                       : adds a circle of 8 points at radius 0.1 around the input point
-lowest                              : thins, flags, or classifies the lowest point per cell
-highest                             : thins, flags, or classifies the highest point per cell
-random                              : thins, flags, or classifies some random point per cell
-seed 4711                           : seeds the random generator with 4711
-central                             : thins, flags, or classifies the point closest to the x/y center of each cell
-adaptive 0.1 [5.0]                  : thins, flags, or classifies points forming TIN within vertical [and horizontal] tolerance
-contours 2.0                        : thins, flags, or classifies points per cell that are as far as possible from contour intervals
-percentile 40 [15]                  : thins, flags, or classifies points closest to 40th percentile in z [if a cell has 15 or more points]
-gps_time                            : thin on GPS time instead (still in beta)
-classify_as 8                       : keep all points in file (do not thin) but classify surviving points as 8 instead
-flag_as_keypoint                    : keep all points in file (do not thin) but flag surviving points as keypoint instead
-flag_as_withheld                    : keep all points in file (do not thin) but flag surviving points as withheld instead
-sparse                              : always use hash to map points to cells internally  
-remain_buffered                     : write buffer points to output when using '-buffered 25' on-the-fly buffering  
-ilay                                : apply all LASlayers found in corresponding *.lay file on read
-ilay 3                              : apply first three LASlayers found in corresponding *.lay file on read
-ilaydir E:\my_layers                : look for corresponding *.lay file in directory E:\my_layers
-olay                                : write or append classification changes to a LASlayers *.lay file
-olaydir E:\my_layers                : write the output *.lay file in directory E:\my_layers

****************************************************************

for more info:

E:\LAStools\bin>lasthin -h
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
LAStools (by info@rapidlasso.de) version 171215 (non-profit)
usage:
lasthin -i *.las
lasthin -i *.laz -olaz
lasthin -i in.las -o out.las
lasthin -i in.las -step 0.5 -highest -o out.las
lasthin -i in.las -flag_as_withheld -o out.las
lasthin -i in.laz -ignore_class 1 3 4 5 6 7 8 9 -flag_as_keypoint -o out.laz
lasthin -i in.las -ignore_class 3 4 5 6 7 -classify_as 8 -o out.las
lasthin -i in.las -last_only -step 0.5 -sparse -o out.laz
lasthin -i in.las -ignore_class 3 4 5 -step 2 -o out.laz
lasthin -i in.las -random -seed 2 -o out.las
lasthin -i in.laz -ignore_class 1 3 4 5 6 7 9  -adaptive 0.2 -classify_as 8 -o out.laz
lasthin -i in.laz -percentile 50 20 -flag_as_keypoint  -o out.laz
lasthin -i in.laz -ignore_class 2 -subcircle 0.2 -highest -o out.laz
lasthin -h

---------------

if you find bugs let me (info@rapidlasso.de) know.
