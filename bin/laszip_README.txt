****************************************************************

  laszip:

  Compresses and uncompresses LiDAR data stored in binary LAS
  format (1.0 - 1.4) in a completely lossless manner to the
  compressed LAZ format.

  This tool comes with an LGPL license. The LASzip compression
  source code is embedded inside LASlib, the LGPL-licensed API
  that is used by all LAStools. The source code is also available
  at http://laszip.org packaged for integration into libLAS.

  The laszip compressor also includes full waveform compression,
  namely the WPD part of LAS 1.3 files. Compress/decompress the
  WPD part to and from a WPZ file via '-waveforms'. This makes
  only sense for point types 4 and 5. It is expected that the
  waveforms are sequential in the file. The waveform compressor
  is not fully supported yet.

  For updates check the website or join the LAStools mailing list.

  http://laszip.org/
  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @rapidlasso

****************************************************************

example usage:

>> laszip *.las

compresses all LAS files in the current folder overwriting any
existing file.

>> laszip *.laz

decompresses all LAZ files in the current folder overwriting any
existing file.

>> laszip *.txt -iparse xyztairn

parses and compresses all ASCII files in the current folder.

>> laszip lidar.las

compresses the LAS file 'lidar.las' to the LAZ file 'lidar.laz'
overwriting any existing file.

>> laszip lidar.laz

decompresses the LAZ file 'lidar.laz' to the LAS file 'lidar.las'
overwriting any existing file.

>> laszip -i lidar.las -o lidar_comp.laz

compresses the LAS file 'lidar.las' to the LAZ file 'lidar_comp.laz'

>> laszip -i data\flight*.las -merged -o merged.laz

merges all the LAS files that match the wild card 'data\flight*.las'
and compresses them to the LAZ file 'merged.laz'

>> laszip -lof file_list.txt

compresses (or uncompresses) all LAS/TXT (or LAZ) files listed in the
the text file 'file_list.txt'

>> laszip -lof file_list.txt -merged -o merged.laz

merges all the LAS/LAZ files listed in the text file 'file_list.txt'
and compresses them to the LAZ file 'merged.laz'

>> laszip -i lidar.txt -iparse xyziRGB -itranslate_intensity 2047

parses the ASCII file 'lidar.txt' with parse string 'xyziRGB' and
converts it on-the-fly to LAS while translating the intensity with
an offset of 2047 and compresses it to the LAZ file 'lidar.laz'

>> laszip -i *.txt -iparse xyzt -iscale_intensity 65535

parses all ASCII file ending in *.txt with parse string 'xyzt' and
converts them on-the-fly to LAS while scaling the intensity with a
multiplier of 65535 and compresses it to a properly named LAZ file.

>> laszip -i *.txt -iparse xyzt -iscale_intensity 65535 -merged -o merged.laz

same as above but all matching ASCII files are merged into one LAS
file.

for more info:

D:\software\LAStools\bin>laszip -h
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
  -copy_attribute_into_user_data 1
  -add_scaled_attribute_to_user_data 0 10.0
Modify the point source ID.
  -set_point_source 500
  -change_point_source_from_to 1023 1024
  -map_point_source map_file.txt
  -copy_user_data_into_point_source
  -copy_scanner_channel_into_point_source
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
LAStools (by martin@rapidlasso.com) version 190927
usage:
laszip -i lidar.las
laszip -i lidar.laz
laszip -i lidar.las -nil
laszip -i lidar.laz -size
laszip -i lidar.laz -check
laszip -i *.las
laszip -i *.laz
laszip -i *.las -odir compressed
laszip -i *.laz -odir uncompressed
laszip -i *.las -odir compressed -cores 4
laszip -i *.laz -odir uncompressed -cores 4
laszip -i lidar.las -o lidar_zipped.laz
laszip -i lidar.laz -o lidar_unzipped.las
laszip -i lidar.las -stdout -olaz > lidar.laz
laszip -stdin -o lidar.laz < lidar.las
laszip -i *.txt -iparse xyztiarn
laszip -i las14.las -compatible -o las14compatible.laz
laszip -i las14.laz -compatible -o las14compatible.laz
laszip -i las14compatible.laz -remain_compatible -o las14compatible.las
laszip -h

---------------

if you find bugs let me (martin.isenburg@rapidlasso.com) know.