****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

  las2txt:

  Converts from binary LAS/LAZ 1.0 - 1.4 to an ASCII text format
 
  For updates check the website or join the LAStools google group.
  
  https://rapidlasso.de/
  http://groups.google.com/group/lastools/

  Jochen @lastools

****************************************************************

example usage:

>> las2txt -i *.las -parse xyzit -sep comma

converts all LAS files *.las to ASCII files *.txt and places the
x, y, and z coordinate of each point as the 1st, 2nd, and 3rd
entries, the intensity as the 4th entry, and the gps_time as the
5th entry of each line. the entries are separated by commas.

>> las2txt -i lidar.las -o lidar.txt -parse xyz

converts LAS file to ASCII and places the x, y, and z coordinate
of each point at the 1st, 2nd, and 3rd entry of each line. the
entries are separated by a space.

>> las2txt -i lidar.laz -o lidar.txt -parse xyzEit -extra 0.1

converts LAZ file to ASCII and places the x, y, and z coordinate
of each point at the 1st, 2nd, and 3rd entry of each line. the
extra string "0.1" as the 4th entry and the intensity and time
of each point as the 5th and 6th entry.

>> las2txt -i lidar_1_3.las -o lidar.txt -parse WV

converts LAS 1.3 file including waveform information to ASCII. it
places the wavepacket info of each point followed by its entire
waveform into one line separated by spaces.

>> las2txt -i lidar.laz -o lidar.txt -parse txyzr -sep comma

converts LAZ file to ASCII and places the gps_time as the first
entry, the x, y, and z coordinates at the 2nd, 3rd, and 4th entry
and the number of the return as the 5th entry of each line. the
entries are separated by a comma.
 
>> las2txt -i lidar.laz -o lidar.txt -parse xyzRGB

converts LAZ file to ASCII and places the x, y, and z coordinates
at the 1st, 2nd, and 3rd entry and the r, g, and b value of the
RGB color as the 4th, 5th, and 6th entry of each line. the entries
are separated by a space. note that lidar.las should be format 1.2
or higher (because 1.0 and 1.1 do not support RGB colors).

>> las2txt -i lidar.las -o lidar.txt -parse xyzia -sep semicolon -header pound

converts LAS file to ASCII and places the x, y, and z coordinate
at the 1st, 2nd, and 3rd entry, the intensity at the 4th and the
scan angle as the 5th entry of each line. the entries are separated
by a semicolon. at the beginning of the file we print the header
information as a comment starting with a '#' symbol.

>> las2txt -i lidar.laz -o lidar.txt -parse xyzcu -sep tab -header percent

converts LAZ file to ASCII and places the x, y, and z coordinate
at the 1st, 2nd, and 3rd entry, the classification at the 4th and
the user data as the 5th entry of each line. the entries are
separated by a semicolon. at the beginning of the file we print
the header information as a comment starting with a '%' symbol.

>> las2txt -i lidar.las -o lidar.txt -parse ko

extracts only the 'k'eypoint and the 'o'verlap flags of each LiDAR
points into an ASCII file. Note that the overlap flag exists only
for LAS/LAZ files containing the LAS 1.4 point types 6 or higher. 

D:\LAStools\bin>las2txt -h
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
  -drop_first_of_many -drop_last_of_many
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
  -drop_overlap -keep_overlap
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
  -thin_with_time 0.001
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
  -change_extended_classification_from_to 6 46
Change the flags.
  -set_withheld_flag 0
  -set_synthetic_flag 1
  -set_keypoint_flag 0
  -set_extended_overlap_flag 1
Modify the extended scanner channel.
  -set_extended_scanner_channel 2
Modify the user data.
  -set_user_data 0
  -change_user_data_from_to 23 26
Modify the point source ID.
  -set_point_source 500
  -change_point_source_from_to 1023 1024
  -copy_user_data_into_point_source
  -bin_Z_into_point_source 200
  -bin_abs_scan_angle_into_point_source 2
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
LAStools (by info@rapidlasso.de) version 150526
usage:
las2txt -i test.las -parse Mxyzrna -stdout | more
las2txt -i *.las -parse xyzt
las2txt -i flight1*.las flight2*.las -parse xyziarn
las2txt -i *.las -parse xyzrn -sep comma -verbose
las2txt -i lidar.las -parse xyztE -extra 99 -o ascii.txt
las2txt -h
---------------------------------------------
The '-parse txyz' flag specifies how to format each
each line of the ASCII file. For example, 'txyzia'
means that the first number of each line should be the
gpstime, the next three numbers should be the x, y, and
z coordinate, the next number should be the intensity
and the next number should be the scan angle.
The supported entries are a - scan angle, i - intensity,
n - number of returns for given pulse, r - number of
this return, c - classification, u - user data,
p - point source ID, e - edge of flight line flag, and
d - direction of scan flag, R - red channel of RGB color,
G - green channel of RGB color, B - blue channel of RGB color,
M - the index for each point
X, Y, and Z - the unscaled, raw LAS integer coordinates
w and W - for the wavepacket information (LAS 1.3 only)
V - for the waVeform from the *.wdp file (LAS 1.3 only)
E - for an extra string. specify it with '-extra <string>'
---------------------------------------------
The '-sep space' flag specifies what separator to use. The
default is a space but 'tab', 'comma', 'colon', 'hyphen',
'dot', or 'semicolon' are other possibilities.
---------------------------------------------
The '-header pound' flag results in the header information
being printed at the beginning of the ASCII file in form of
a comment that starts with the special character '#'. Also
possible are 'percent', 'dollar', 'comma', 'star',
'colon', or 'semicolon' as that special character.

---------------

if you find bugs let me (info@rapidlasso.de) know.
