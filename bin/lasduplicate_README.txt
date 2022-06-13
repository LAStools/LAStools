****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

  lasduplicate:

  Removes all duplicate points from a LAS/LAZ/ASCII file. In the
  default mode those are xy-duplicate points that have identical
  x and y coordinates. The first point survives, all subsequent
  duplicates are removed. It is also possible to keep the lowest
  points amongst all xy-duplicates via '-lowest_z'.
  
  It is also possible to remove only xyz-duplicates points that
  have all x, y and z coordinates identical via '-unique_xyz'.

  Another option is to identify '-nearby 0.005' points into one so
  that multiple returns within a specified distance become a single
  return. Given all pairs of points p1 and p2 (with p1 being before
  p2 in the file), point p2 will not be part of the output if these
  three conditions on their quantized coordinates are true

    (1)   |p1.qx - p2.qx| <= 1
    (2)   |p1.qy - p2.qy| <= 1
    (3)   |p1.qz - p2.qz| <= 1 

  after quantizing them as follows based on the '-nearby d' value 

      p1.qx = round(p1.x / d)     p2.qx = round(p2.x / d)
      p1.qy = round(p1.y / d)     p2.qy = round(p2.y / d)
      p1.qz = round(p1.z / d)     p2.qz = round(p2.z / d)

  The special option '-single_returns' was added particularly to
  reconstruct the single versus multiple return information for
  the (unfortunate) case that the LiDAR points were delivered in
  two separate files with some points appearing in both. These
  LiDAR points may be split into all first return and all last
  returns or into all first returns and all ground returns. See
  the example below how to deal with this case correctly.

  The removed points can also be recorded to a LAZ file with the
  option '-record_removed'.

  Please license from info@rapidlasso.de to use
  lasduplicate commercially.

  For updates check the website or join the LAStools google group.
  
  https://rapidlasso.de/
  http://groups.google.com/group/lastools/

****************************************************************
see also:
  lascopy - Copy point properties from different sources to a target
****************************************************************

example usage:

>> lasduplicate -i *.las

removes all duplicates from all LAS file matching '*.las' and stores
each result to a corresponding LAS file '*_1.las'.

>> lasduplicate -i *.las -lowest_z

same as above but keeps the duplicate with the lowest z coordinate.

>> lasduplicate -i *.txt -iparse xyziat -otxt -oparse xyziat

removes all duplicates from all ASCII file matching '*.txt' that are
then parsed with 'xyziat' and stores each result to a corresponding
ASCII file '*_1.txt'.

>> lasduplicate -i in.las -o out.las

removes all duplicates from the LAS file 'in.las' and stores the
result to the LAS file 'out.las'.

>> lasduplicate -i in.las -o out.las -v

same as above but reports every removed point in the stderr.

>> lasduplicate -i in.laz -nearby 0.005 -o out.laz 

removes all duplicates and nearby points from fulfilling the criteria
desribed above from the LAS file 'in.laz' and stores the result to the
LAS file 'out.laz'.

>> lasduplicate -i in.laz -o out.laz

removes all duplicates from the LASzip compressed file 'in.laz'
and stores the result to the LASzip compressed 'out.laz'.

>> lasduplicate -i in.txt -iparse xyzai -o out.txt -oparse xyzai 

removes all duplicates from the ASCII file 'in.txt' and stores
the result to the ASCII file 'out.txt'.

>> lasduplicate -lof files.txt -merged -o merged.laz

removes all duplicates from the merged points of all LAS/LAZ files
listed in 'files.txt' and stores the merged result to the LASzip
compressed file 'merged.laz'.

>> lasduplicate -i in.las ^
                -unique_xyz -record_removed ^
                -o out.laz

removes all xyz duplicates from in.las and stores the xyz unique
points to out.laz and all removed xyz duplicates to out_removed.laz

>> lasduplicate -lof files.txt -iparse xyzcirn -olas

same as above but for a list of ASCII files, without merging them,
and with storing each result to a LAS file.

>> lasduplicate -i first.txt -iparse xyzi ^
                -set_return_number 1 -set_number_of_returns 2 ^
                -unique_xyz ^
                -o first.laz
>> lasduplicate -i last.txt -iparse xyzi ^
                -set_return_number 2 -set_number_of_returns 2 ^
                -unique_xyz ^
                -o last.laz
>> lasduplicate -i first.laz -i last.laz -merged ^
                -single_returns ^
                -o final.laz

first removes all xyz unique duplicates from an ASCII text file
containing only first returns and marks them all as the first
of two returns, then removes all xyz unique duplicates from an
ASCII text file containing only last returns and marks them all
as the second of two returns. finally merges the first.laz and
the last.laz file on-the-fly, removes all xy unique duplicates
and marks the sole survivor as the only return of one.

****************************************************************

overview of all tool-specific switches:

-v                    : more info reported in console
-vv                   : even more info reported in console
-quiet                : nothing reported in console
-version              : reports this tool's version number
-fail                 : fail if license expired or invalid
-gui                  : start with files loaded into GUI
-cores 4              : process multiple inputs on 4 cores in parallel
-unique_xyz           : remove only xyz-duplicates where x, y and z coordinate are identical
-nearby 0.1           : remove all points that are "nearby" (see rules above) an earlier point
-lowest_z             : remove those xy-duplicates that have higher z coordinates
-flag_as_withheld     : instead of removing points set their withheld flag to true
-single_returns       : mark single returns (used for converting first.txt and last.txt ASCII files to LAS)
-record_removed       : record all removed points into a separate file with appendix "_removed"
-hash_set             : use a different internal data structure (i.e. hash_set) for finding duplicates
-hash_map             : use a different internal data structure (i.e. hash_map) for finding duplicates
-two_pass             : find duplicates in two instead of one pass over the file
-remain_buffered      : write buffer points to output when using '-buffered 25' on-the-fly buffering  
-ilay                 : apply all LASlayers found in corresponding *.lay file on read
-ilay 3               : apply first three LASlayers found in corresponding *.lay file on read
-ilaydir E:\my_layers : look for corresponding *.lay file in directory E:\my_layers
-olay                 : write or append classification changes to a LASlayers *.lay file
-olaydir E:\my_layers : write the output *.lay file in directory E:\my_layers

****************************************************************

for more info:

E:\LAStools\bin>lasduplicate -h
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
lasduplicate -i *.las
lasduplicate -i tile.laz -o out.laz -flag_as_withheld
lasduplicate -i *.txt -iparse xyzit -otxt -oparse xyzit
lasduplicate -i in.las -lowest_z -o out.las
lasduplicate -i *.laz -unique_xyz -odix _unique -olaz
lasduplicate -i *.laz -nearby 0.01 -odix _nearby01 -olaz
lasduplicate -i in.laz -unique_xyz -record_removed -o out.laz
lasduplicate -i tiles\*.laz -single_returns -odir fixed\ -olaz
lasduplicate -i in.txt -iparse xyzit -o out.txt -oparse xyzit
lasduplicate -i in.las -nil
lasduplicate -h

---------------

if you find bugs let me (info@rapidlasso.de) know.
