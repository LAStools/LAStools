****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

  lasmerge:
  
  Merge many files into one or split one file into many.

  The tool reads multiple LIDAR data files in LAS/LAZ/ASCII 
  format and merges them into a single LAS/LAZ/ASCII file. 
  The filenames can either be provided one by one, with one 
  or more wildcards, or in form of a text file.

  Rather than merging the merged LAS/LAZ/ASCII files into a single
  LAS/LAZ/ASCII file they can also be split into many numbered
  files that each contain the same number of points (except the
  last one) with the '-split 500000000' option, which would split
  after 500 million points were written.

  All the header information of the first file provided is used
  including variable and user_defined headers. but some records
  are updated by integrating the corresponding information from
  other headers. these are:

    * number_of_point_records
    * number_of_points_by_return[5]
    * max_x, min_x, max_y, min_y, max_z, and min_z

  In addition x_scale_factor, y_scale_factor, z_scale_factor may
  need to be increased to accommodate a possibly larger bounding
  box. The user can also reset those in the command line with

     -rescale 0.01 0.01 0.001

  Similarly a new offset can be specified

     -reoffset 600000 4000000 0

  For updates check the website or join the LAStools google group.
  
  https://rapidlasso.de/
  http://groups.google.com/group/lastools/

****************************************************************
see also:
  lassplit - Merge or split lidar data files by properties
****************************************************************

example usage:

>> lasmerge -i *.las -o out.las

merge all *.las files into one out.las file. 

>> lasmerge -i *.las -o out0000.las -split 1000000000

merge all *.las files into one and then split it into several
output files that contain one billion points each and that are
called out0000.las, out0001.las, out0002.las, out0003.las, ...

>> lasmerge -i big.txt -iparse xyzt -o out000.laz -split 500000000

split the text file big.txt that could, for example, contain 25
billion points one and then split it into several compressed
output LAZ files that contain 500 million points each that are
called out000.laz, out001.laz, out002.laz, out003.laz, ...

>> lasmerge -i *.txt -iparse xyztiarn -o out.las

merge all *.txt files, parsed with 'xyztiarn', into one out.las file. 

>> lasmerge -i in1.las in2.las in3.las -o out.las

merge the three inX.las files into one out.las file. 

>> lasmerge -lof file_list.txt -o out.laz

merges all LAS files listed in the text file into one out.laz file

>> lasmerge -lof file_list.txt -o out.las -rescale 0.01 0.01 0.001 -v

merges the file and stores the LIDAR points with 0.01 accuracy 
in x y and z and prints out control information

>> lasmerge -lof file_list.txt -o out.las -rescale 0.01 0.01 0.01 -reoffset 600000 4000000 0 -v

same but with a different accuracy for z and an offset

for more info:

C:\lastools\bin>lasmerge -h
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
LAStools (by info@rapidlasso.de) version 140301
usage:
lasmerge -i *.las -o out.las
lasmerge -lof lasfiles.txt -o out.las
lasmerge -i *.las -o out0000.laz -split 1000000000
lasmerge -i file1.las file2.las file3.las -o out.las
lasmerge -i file1.las file2.las -reoffset 600000 4000000 0 -olas > out.las
lasmerge -lof lasfiles.txt -rescale 0.01 0.01 0.01 -v -o out.las
lasmerge -h

---------------

if you find bugs let me (info@rapidlasso.de) know.
