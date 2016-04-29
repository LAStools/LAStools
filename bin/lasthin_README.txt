****************************************************************

  lasthin:

  A simple LIDAR thinning algorithm for LAS/LAZ/ASCII. It places
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

  In order to process very large but sparse grids such as, for
  example, a single but very long diagonal flight line, it is
  beneficial to use the '-sparse' option to avoid exceeding the
  main memory (and start thrashing).

  It is also possible to "thicken" your points as you thin them
  to simulate a diameter for the laser beam. The '-subcircle 0.1'
  option will replicate each point 8 times in a discrete circle
  with radius 0.1 around every original input point. This makes
  sense in combination with '-highest' in order to create a nice
  set of points for subsequent CHM or DSM construction.

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
 
  Please license from martin.isenburg@rapidlasso.com to use lasthin
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

>> lasthin -i *.las

thins all LAS files with the grid spacing default of 1 unit
and keeps the lowest point per grid cell

>> lasthin -i *.laz -olaz

same but with LAZ files

>> lasthin -i *.txt -iparse xyzt -otxt -oparse xyzt

same but with ASCII files

>> lasthin -i in.las -o out.las

does LAS thinning with the grid spacing default of 1 unit
and keeps the lowest point per grid cell

>> lasthin -i in.las -o out.las -highest -step 2.0

does point thinning with a grid spacing of 2.0 units and
keeps the highest point per grid cell

>> lasthin -i in.laz -step 0.5 -o out.laz -random

does point thinning with a grid spacing of 0.5 units and
keeps a random point per grid cell

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

for more info:
D:\lastools\bin>lasthin -h
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
Filter points based on their return number.
  -first_only -keep_first -drop_first
  -last_only -keep_last -drop_last
  -keep_middle -drop_middle
  -keep_return 1 2 3
  -drop_return 3 4
  -keep_single -drop_single
  -keep_double -drop_double
  -keep_triple -drop_triple
  -keep_quadruple -drop_quadruple
  -keep_quintuple -drop_quintuple
Filter points based on the scanline flags.
  -drop_scan_direction 0
  -scan_direction_change_only
  -edge_of_flight_line_only
Filter points based on their intensity.
  -keep_intensity 20 380
  -drop_intensity_below 20
  -drop_intensity_above 380
  -drop_intensity_between 4000 5000
Filter points based on their classification.
  -keep_class 1 3 7
  -drop_class 4 2
  -drop_synthetic -keep_synthetic
  -drop_keypoint -keep_keypoint
  -drop_withheld -keep_withheld
Filter points based on their user data.
  -keep_user_data 1
  -drop_user_data 255
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
  -drop_scan_angle_below -15
  -drop_scan_angle_above 15
  -drop_scan_angle_between -25 -23
Filter points based on their gps time.
  -keep_gps_time 11.125 130.725
  -drop_gps_time_below 11.125
  -drop_gps_time_above 130.725
  -drop_gps_time_between 22.0 48.0
Filter points based on their wavepacket.
  -keep_wavepacket 0
  -drop_wavepacket 3
Filter points with simple thinning.
  -keep_every_nth 2
  -keep_random_fraction 0.1
  -thin_with_grid 1.0
Transform coordinates.
  -translate_x -2.5
  -scale_z 0.3048
  -rotate_xy 15.0 620000 4100000 (angle + origin)
  -translate_xyz 0.5 0.5 0
  -translate_then_scale_y -0.5 1.001
  -switch_x_y -switch_x_z -switch_y_z
  -clamp_z_below 70.5
  -clamp_z 70.5 72.5
Transform raw xyz integers.
  -translate_raw_z 20
  -translate_raw_xyz 1 1 0
  -clamp_raw_z 500 800
Transform intensity.
  -scale_intensity 2.5
  -translate_intensity 50
  -translate_then_scale_intensity 0.5 3.1
  -clamp_intensity 0 255
  -clamp_intensity_above 255
Transform scan_angle.
  -scale_scan_angle 1.944445
  -translate_scan_angle -5
  -translate_then_scale_scan_angle -0.5 2.1
Change the return number or return count of points.
  -repair_zero_returns
  -set_return_number 1
  -change_return_number_from_to 2 1
  -set_number_of_returns 2
  -change_number_of_returns_from_to 0 2
Modify the classification.
  -set_classification 2
  -change_classification_from_to 2 4
  -classify_z_below_as -5.0 7
  -classify_z_above_as 70.0 7
  -classify_z_between_as 2.0 5.0 4
  -classify_intensity_above_as 200 9
  -classify_intensity_below_as 30 11
Modify the user data.
  -set_user_data 0
  -change_user_data_from_to 23 26
Modify the point source ID.
  -set_point_source 500
  -change_point_source_from_to 1023 1024
  -quantize_Z_into_point_source 200
Transform gps_time.
  -translate_gps_time 40.50
  -adjusted_to_week
  -week_to_adjusted 1671
Transform RGB colors.
  -scale_rgb_down (by 256)
  -scale_rgb_up (by 256)
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
LAStools (by martin@rapidlasso.com) version 140527
usage:
lasthin -i *.las
lasthin -i *.laz -olaz
lasthin -i in.las -o out.las
lasthin -i in.las -step 0.5 -highest -o out.las
lasthin -i in.las -last_only -step 0.5 -sparse -o out.laz
lasthin -i in.las -ignore_class 3 4 5 -step 2 -o out.laz
lasthin -i in.las -random -seed 2 -o out.las
lasthin -i in.laz -ignore_class 2 -subcircle 0.2 -highest -o out.laz
lasthin -h

---------------

if you find bugs let me (martin.isenburg@rapidlasso.com) know.
