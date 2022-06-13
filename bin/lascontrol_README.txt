****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

  lascontrol:

  This tool computes the height of the LIDAR at certain x and y
  control points locations and reports the height difference in
  respect to the control points' elevation.

  The tool reads LIDAR in LAS/LAZ/ASCII format, triangulates the
  relevant points into a TIN. For classified data sets containing
  a mix of ground and vegetation/building points it is imperative
  to specified the points against which the reference measurements
  are to be taken (i.e. usually '-keep_class 2').

  The tool collects for each ground control point all LiDAR points
  that fall into a 3 by 3 grid of cells surrounding the control
  point. By default the size of each of the nine grid cells is 5
  meter by 5 meter so that the total patch surrounding the control
  point that needs to be covered by LiDAR for that control point
  to get evaluated is 15 meter by 15 meter. This can be changed
  with the '-step 2' parameter which would shrink each of the nine
  cells to 2 meter by 2 meter.

  If the horizontal units are in feet this needs to be specified in
  the command-line to make sure that a wide enough patch of LiDAR
  points is used around each control point (i.e. '-feet'). If the
  LIDAR file contains proper projection information this can be
  automatically detected by the tool.
 
  The output report defaults to stdout unless you specify an output
  file with '-cp_out report.txt'.

  One handy feature is the option to apply the resulting average error
  as a vertical adjustment to all of the input files. This can be done
  with the '-adjust_z' command switch and should go hand in hand with
  specifying where the output should go with '-odir ...' or '-odix ...'

  Especially for the '-adjust_z' it is sometimes useful to not let all
  control points contribute to the error calculation. Via the options
  '-cp_ignore_diff_above 1.5' and '-cp_ignore_diff_below -0.5' it is
  possible to ignore control points that are too high or too low above
  the LiDAR from the calculation that also computes the average error.

  Please license from info@rapidlasso.de to use lascontrol
  commercially.

  For updates check the website or join the LAStools mailing list.

  https://rapidlasso.de/
  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @lastools

****************************************************************

example usage:

>> lascontrol -i *.laz -cp cp.csv -cp_out report.txt -keep_class 2 8 -adjust_z -odix _adjusted

assumes the x/y/z coordinates of the control points are stored as
the 1nd/2nd/3rd entry on each line of the 'cp.csv' file and only
points with ground and keypoint classification (class 2 or 8)
are used to construct the reference TIN. all LAZ files that match
'*.laz' get merged on the fly into one to construct the reference
TIN. the output is written to the file 'report.txt' and the average
error is used to adjust the z coordinate of all LAZ files that are
then written to files with the same name and appendix "_adjusted"

>> lascontrol -i *.las -cp cp.csv -keep_class 2 8 -parse sssxyz

assumes the x/y/z coordinates of the control points are stored as
the 4th/5th/6rth entry on each line of the 'cp.csv' file and only
points with ground and mass-point classification (class 2 or 8)
are used to construct the reference TIN. all LAS files that match
'*.las' get merged on the fly into one. output goes to stdout.

>> lascontrol -i lidar.las -cp cp.csv -keep_class 2 -parse sxyz

assumes the x/y/z coordinates of the control points are stored as
the 2nd/3rd/4th entry on each line of the 'cp.csv' file and only
points with ground classification (class = 2) are used to construct
the reference TIN. output goes to stdout.

>> lascontrol -i lidar.laz -cp cp.csv -keep_class 2 -parse xsysz -cp_out cp_out.csv

assumes the x/y/z coordinates of the control points are stored as
the 1st/3rd/5th entry on each line of the 'cp.csv' file and only
points with ground classification (class = 2) are used to construct
the reference TIN. output goes to 'cp_out.csv'.

>> lascontrol -i *.laz -cp cp.txt -skip 3 -keep_class 2

skips the first three lines of the file 'cp.txt' and assumes the
x/y/z coordinates of the control points are stored as 1st/2nd/3rd
entry on each line. all LAS files that match '*.laz' get merged
on the fly into one. output goes to stdout.

>> lascontrol -i flight1*.txt -iparse sxyz -iskip 2 -cp cp.txt -parse sssxyz -skip 1  -cp_out cp_out.csv

skips the first lines of the file 'cp.txt' and assumes the x/y/z
coordinates of the control points are stored as the 4th/5th/6th
entry on each line. the LIDAR points get merged from all ASCII
files that match the wildcard 'flight1*.txt'. the first 2 lines
of each ASCII file are skipped and the x/y/z coordinates of the
LIDAR points are taked from the nd/3rd/4th entry on each line and
the output goes to 'cp_out.csv'.

for more info:

C:\lastools\bin>lascontrol -h
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
LAStools (by info@rapidlasso.de) version 140301 (unlicensed)
usage:
lascontrol -i lidar.las -cp cp.txt -keep_class 2
lascontrol -i lidar.las -cp cp.csv -keep_class 2 8 -parse sxyz -step 2
lascontrol -i lidar.laz -cp cp.csv -keep_class 2 8 -parse xsysz -cp_out cp_out.csv
lascontrol -i *.las -cp cp.txt -skip 3 -keep_class 2
lascontrol -i lidar.txt -iparse cssxyz -cp cp.txt -parse ssxyz -skip 3 -keep_class 2 8
lascontrol -i flight1*.txt -iparse sxyz -iskip 2 -cp cp.txt -parse sssxyz -skip 1
lascontrol -h

---------------

if you find bugs let me (info@rapidlasso.de) know.
