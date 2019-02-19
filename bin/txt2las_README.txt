****************************************************************

  txt2las:

  Converts LIDAR data from a standard ASCII format into the more
  efficient binary LAS/LAZ/BIN representations. You can request 
  a particular point type with '-set_point_type 6'.

  Reads also directy from *.gz, *.zip, *.rar, and *.7z files if
  the corresponding gzip.exe, unzip.exe, unrar.exe, and 7z.exe
  are in the same folder.

  Allows adding a VLR to the header with projection information.

  If your input text file is PTS or PTX format you can preserve
  the extra header information of these files. Simply add the
  appropriate '-ipts' or '-iptx' switch to the command line which
  will store this in a VLR. You can later reconstruct the PTS or
  PTX files with 'las2las' or 'las2txt' by adding the corresponding
  '-opts' or '-optx' option to the command line.

  Also allows adding additional attributes to LAS/LAZ files using
  the "Extra Bytes" concept with '-add_attribute'.

  It is also possible to pipe the ASCII into txt2las. For this you
  will need to add both '-stdin' and '-itxt' to the command-line.
 
  For updates check the website or join the LAStools mailing list.

  http://rapidlasso.com/LAStools
  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/LAStools
  http://facebook.com/LAStools
  http://linkedin.com/groups?gid=4408378

  Martin @rapidlasso

****************************************************************

example usage:

>> txt2las -i lidar.txt.gz -o lidar.las -parse ssxyz

converts a gzipped ASCII file and uses the 3rd, 4th, and 5th entry
of each line as the x, y, and z coordinate of each point

>> txt2las -i lidar.zip -o lidar.laz -parse ssxyz -utm 14T

same as above for a zipped ASCII file but produces compressed LAZ
and adds projection info for utm zone with wgs84 

>> txt2las -i lidar.txt -o lidar.laz -parse xyzai -scale_scan_angle 57.3 -scale_intensity 65535

also reads the 4th entry as the scan angle and multiplies it by 57.3
(radian to angle) and the 5th entry as the intensity and multiplies
it by 65535 (converts range [0.0 .. 1.0] to range [0 .. 65535]. then
produces a compressed LAZ file.

>> txt2las -skip 3 -i lidar.txt.gz -o lidar.las -parse txyzsa -sp83 OH_N

converts a gzipped ASCII file and uses the 1st entry of each line
as the gps time, the 3rd, 4th, and 5th entry as the x, y, and z
coordinate of each point, and the 6th entry as the scan angle. it
skips the first three lines of the ASCII data file and adds projection
info for state plane ohio north with nad83. 

>> txt2las -i lidar.txt.gz -o lidar.laz -parse xyzRGB -set_scale 0.001 0.001 0.001 -set_offset 500000 4000000 0

converts a gzipped ASCII file and uses the 1st 2nd, and 3rd entry
of each line as the x, y, and z coordinate of each point, and the
4th, 5th, and 6th entry as the RGB color. the created compressed
LAZ file will have a precision of 0.001 0.001 0.001 and an offset
of 500000 4000000 0

>> txt2las -i LiDAR.txt -parse xyz0irnt -add_attribute 3 "echo width" "of returning waveform [ns]" 0.1 -o LiDAR.laz

4590235.448 5436373.539 600.528 9.8 35 1 2 215277.271327
4590235.524 5436373.642 600.257 9.8 35 2 2 215277.271327
4590238.788 5436378.064 591.820 5.7 89 1 1 215277.271333
4590238.710 5436377.908 595.356 5.9 18 1 3 215277.271340
4590239.164 5436378.529 593.746 5.4 30 2 3 215277.271340
4590239.633 5436379.171 592.083 4.8 15 3 3 215277.271340

Given 'LiDAR.txt' contains ASCII as above then '-parse xyz0irnt' stores
the first three columns to x y and z and the 4th column as an additional
attribute in the "Extra Bytes". The first argument of '-add_attribute'
specifies the data type, the following two strings are the name and the
description of the "Extra Bytes", and the final 0.1 means that the number
should be stored with a scale of 0.1. Hence the value 9.8 will be stored
as an unsigned short 98 and the value 5.3 will be stored as an unsigned
short 53. 

>> txt2las -i LiDAR2.txt -parse xyz0i1rnt ^
           -add_attribute 3 "echo width" "of returning waveform [ns]" 0.1 ^
	   -add_attribute 3 "corrected intensity" "uniform across flightlines" ^
           -o LiDAR2.laz

4590235.448 5436373.539 600.528 9.8 35 44 1 2 215277.271327
4590235.524 5436373.642 600.257 9.8 35 39 2 2 215277.271327
4590238.788 5436378.064 591.820 5.7 89 99 1 1 215277.271333
4590238.710 5436377.908 595.356 5.9 18 27 1 3 215277.271340
4590239.164 5436378.529 593.746 5.4 30 38 2 3 215277.271340
4590239.633 5436379.171 592.083 4.8 15 23 3 3 215277.271340

Same as before but with a second additional attribute marked by '1' in the
parse string and specified by a second occurance of the '-add_attribute'
option. These are unsigned integers numbers so no scale value is required.

****************************************************************

overview of all tool-specific switches:

-v                                      : more info reported in console
-vv                                     : even more info reported in console
-quiet                                  : nothing reported in console
-version                                : reports this tool's version number
-gui                                    : start with files loaded into GUI
-cores 4                                : process multiple inputs on 4 cores in parallel
-set_point_type 6                       : use point type 6 of LAS 1.4 instead of point type 1 of LAS 1.2
-set_version 1.4                        : force version 1.4 (even if point type 0, 1, 2, or 3 are used)
-set_scale 0.05 0.05 0.001              : quantize ASCII points with 5 cm in x and y and 1 mm in z
-set_offset 500000 2000000 0            : use offset 500000 2000000 0 instead of auto choosing one
-set_file_creation 67 2011              : set file creation date to specified dayofyear / year
-set_system_identifier "Riegl Q680i"    : set system identifier to specified 31 character string
-set_generating_software "LAStools"     : set generating software to specified 31 character string
-set_global_encoding 1                  : set global encoding in LAS header to 1
-progress 10000000                      : report progress every 10 million points

****************************************************************

for more info:

E:\LAStools\bin>txt2las -h
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
LAStools (by martin@rapidlasso.com) version 180131
Supported ASCII Inputs:
  -i lidar.txt
  -i lidar.txt.gz
  -i lidar.zip
  -i lidar.rar
  -i lidar.7z
  -stdin (pipe from stdin)
usage:
txt2las -parse tsxyz -i lidar.txt.gz
txt2las -parse xyzairn -i lidar.zip -utm 17T -vertical_navd88 -olaz -set_classification 2 -quiet
unzip -p lidar.zip | txt2las -parse xyz -stdin -o lidar.las -longlat -elevation_survey_feet
txt2las -i lidar.zip -parse txyzar -scale_scan_angle 57.3 -o lidar.laz
txt2las -skip 5 -parse xyz -i lidar.rar -set_file_creation 28 2011 -o lidar.las
txt2las -parse xyzsst -verbose -set_scale 0.001 0.001 0.001 -i lidar.txt
txt2las -parse xsysz -set_scale 0.1 0.1 0.01 -i lidar.txt.gz -sp83 OH_N -feet
las2las -parse tsxyzRGB -i lidar.txt -set_version 1.2 -scale_intensity 65535 -o lidar.las
txt2las -h
---------------------------------------------
The '-parse tsxyz' flag specifies how to interpret
each line of the ASCII file. For example, 'tsxyzssa'
means that the first number is the gpstime, the next
number should be skipped, the next three numbers are
the x, y, and z coordinate, the next two should be
skipped, and the next number is the scan angle.
The other supported entries are i - intensity,
n - number of returns of given pulse, r - number
of return, c - classification, u - user data, and
p - point source ID, e - edge of flight line flag, and
d - direction of scan flag, R - red channel of RGB
color, G - green channel, B - blue channel, I - NIR channel,
l - scanner channel, o - overlap flag, h - withheld
flag, k - keypoint flag, g - synthetic flag, 0 - first
additional attribute specified, 1 - second additional
attribute specified, 2 - third ...
---------------------------------------------
Other parameters are
'-set_scale 0.05 0.05 0.001'
'-set_offset 500000 2000000 0'
'-set_file_creation 67 2011'
'-set_system_identifier "Riegl 500,000 Hz"'
'-set_generating_software "LAStools"'
'-utm 14T'
'-sp83 CA_I -feet -elevation_survey_feet'
'-longlat -elevation_feet'

---------------

if you find bugs let me (martin.isenburg@rapidlasso.com) know.
