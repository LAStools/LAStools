****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

  lascolor:

  This tool colors LiDAR points based on imagery that is usually
  an ortho-photo. The tool computes into which pixel a LAS point
  is falling and then sets the RGB values accordingly. Currently
  only the TIF format is supported. The world coordinates need
  to be either in GeoTIFF tags or in an accompanying *.tfw file.

  By default the 8 bit RGB colors of the TIF file will be scaled
  up to 16 bits in order to comform the LAS specification. Use the
  option '-dont_scale_rgb_up' to avoid this automatic up-scaling.

  By default the LiDAR points falling outside of the image or that
  fall into a no-data area of the image will not be colored. Use
  option '-zero_rgb' to set their color to (0/0/0) instead.

  It is also possible to only copy a '-grey' band into all three
  RGB channels or a single color '-red', '-green' or '-blue' into
  the corresponding channel. If the input image has more than one
  band use '-band 2' to specify which one to copy from. By default
  this will be band 0.

  It is also possible to copy '-nir' either from a single band
  image or from a multi-band image and then '-band 3' or else is
  needed. Instead of copying into the NIR band of point types 8
  or 10 the NIR value can also be stored as '-intensity'.

  With '-rgbnir' and a 4-band RGBNIR input image it is possible
  to copy all 4 bands at once into either RGB and the NIR channel
  or into the RGB and the intensity fields by adding '-intensity'  

  For storing to the NIR channel the input LAS file already needs
  to be LAS 1.4. If needed use las2las to upgraded to a LAS 1.4
  file prior to running lascolor.

  You can run lascolor on an entire folder of LAS/LAZ files with
  corresponding (identical file name but different file extension) 
  image files by using the '-imagedir ortho' switch *instead* of
  the '-image some_file.tif'. If corresponding TIF files are in the
  same folder as the LAS/LAZ files then you can simply omit both.
  Here running on multiple '-cores 4' tends to give you a nice
  speed-up. 

  Instead of coloring points it is also possible to classify them
  based on the pixel they fall into. Use '-classify_as 10' to give
  all points that fall into a black pixel of a gray or a color image
  the classification code 10. Alternatively use one of the options
  '-classify_not_black', '-classify_white', or '-classify_not_white'
  to color pixels that are not black, white, or not white.
  
  Please license from info@rapidlasso.de before using lascolor
  commercially. Please note that the unlicensed version will set
  intensity, gps_time, user data, and point source ID to zero,
  slightly change the LAS point order, and randomly add a tiny
  bit of white noise to the points coordinates once you exceed a
  certain number of points in the input file.

  For updates check the website or join the LAStools mailing list.

  https://rapidlasso.de/
  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @rapidlasso

****************************************************************

example usage:

>> lascolor -i LAS_18910_2010_25.laz ^
            -image cir_18910_2010_tiled_16bit.tif ^
            -odix _rgb -olaz

>> lascolor -i LAS_18910_2010_25.laz ^
            -image cir_18910_2010_2_scanline_8bit.tif ^
            -odix _rgb1 -olaz

>> lascolor -i LAS_18910_2010_25.laz ^
            -image cir_18910_2010_2_tiled_8bit.tif ^
            -odix _rgb2 -olaz

>> lascolor -i point_clouds\*.laz ^
            -imagedir ortho_photos ^
            -odir colored_point_clouds -olaz ^
            -cores 4

Below you see the resulting changes in point type and RGB colors:

>> lasinfo LAS_18910_2010_25.laz  (original)
reporting all LAS header entries:
  file signature:             'LASF'
  file source ID:             18910
  global_encoding:            0
  project ID GUID data 1-4:   00000000-0000-0000-0000-000000000000
  version major.minor:        1.2
  system identifier:          'LAStools (c) by rapidlasso GmbH'
  generating software:        'lasduplicate (130506) commercia'
  file creation day/year:     276/2013
  header size:                227
  offset to point data:       227
  number var. length records: 0
  point data format:          0
  point data record length:   20
  number of point records:    486674
  number of points by return: 486674 0 0 0 0
  scale factor x y z:         0.01 0.01 0.01
  offset x y z:               700000 200000 0
  min x y z:                  703149.14 248150.21 647.87
  max x y z:                  703849.14 248850.21 798.71
LASzip compression (version 2.1r0 c2 50000): POINT10 2
reporting minimum and maximum for all LAS point record entries ...
  X     314914     384914
  Y    4815021    4885021
  Z      64787      79871
  intensity 45 99
  edge_of_flight_line 0 0
  scan_direction_flag 0 0
  number_of_returns_of_given_pulse 1 1
  return_number                    1 1
  classification      0     0
  scan_angle_rank     0     0
  user_data           0     0
  point_source_ID    11    22
overview over number of returns of given pulse: 486674 0 0 0 0 0 0
histogram of classification of points:
          486674  Created, never classified (0)

>> lasinfo -i LAS_18910_2010_25_rgb.laz    (colored with 16 bit image)
reporting all LAS header entries:
  file signature:             'LASF'
  file source ID:             18910
  global_encoding:            0
  project ID GUID data 1-4:   00000000-0000-0000-0000-000000000000
  version major.minor:        1.2
  system identifier:          'LAStools (c) by rapidlasso GmbH'
  generating software:        'lascolor (131214) academic'
  file creation day/year:     276/2013
  header size:                227
  offset to point data:       227
  number var. length records: 0
  point data format:          2
  point data record length:   26
  number of point records:    486674
  number of points by return: 486674 0 0 0 0
  scale factor x y z:         0.01 0.01 0.01
  offset x y z:               700000 200000 0
  min x y z:                  703149.14 248150.21 647.87
  max x y z:                  703849.14 248850.21 798.71
LASzip compression (version 2.2r0 c2 50000): POINT10 2 RGB12 2
reporting minimum and maximum for all LAS point record entries ...
  X     314914     384914
  Y    4815021    4885021
  Z      64787      79871
  intensity 45 99
  edge_of_flight_line 0 0
  scan_direction_flag 0 0
  number_of_returns_of_given_pulse 1 1
  return_number                    1 1
  classification      0     0
  scan_angle_rank     0     0
  user_data           0     0
  point_source_ID    11    22
  Color R 0 19603
        G 0 27297
        B 0 31113
overview over number of returns of given pulse: 486674 0 0 0 0 0 0
histogram of classification of points:
          486674  Created, never classified (0)

>> lasinfo -i LAS_18910_2010_25_rgb1.laz    (colored with 8 bit image)
reporting all LAS header entries:
  file signature:             'LASF'
  file source ID:             18910
  global_encoding:            0
  project ID GUID data 1-4:   00000000-0000-0000-0000-000000000000
  version major.minor:        1.2
  system identifier:          'LAStools (c) by rapidlasso GmbH'
  generating software:        'lascolor (131214) academic'
  file creation day/year:     276/2013
  header size:                227
  offset to point data:       227
  number var. length records: 0
  point data format:          2
  point data record length:   26
  number of point records:    486674
  number of points by return: 486674 0 0 0 0
  scale factor x y z:         0.01 0.01 0.01
  offset x y z:               700000 200000 0
  min x y z:                  703149.14 248150.21 647.87
  max x y z:                  703849.14 248850.21 798.71
LASzip compression (version 2.2r0 c2 50000): POINT10 2 RGB12 2
reporting minimum and maximum for all LAS point record entries ...
  X     314914     384914
  Y    4815021    4885021
  Z      64787      79871
  intensity 45 99
  edge_of_flight_line 0 0
  scan_direction_flag 0 0
  number_of_returns_of_given_pulse 1 1
  return_number                    1 1
  classification      0     0
  scan_angle_rank     0     0
  user_data           0     0
  point_source_ID    11    22
  Color R 0 65280
        G 0 65280
        B 0 65280
overview over number of returns of given pulse: 486674 0 0 0 0 0 0
histogram of classification of points:
          486674  Created, never classified (0)

****************************************************************

overview of all tool-specific switches:

-v                                   : more info reported in console
-vv                                  : even more info reported in console
-quiet                               : nothing reported in console
-version                             : reports this tool's version number
-fail                                : fail if license expired or invalid
-gui                                 : start with files loaded into GUI
-cores 4                             : process multiple inputs on 4 cores in parallel
-ignore_class 0 1 3 5 6 7 9          : ignores points with specified classification codes
-ignore_extended_class 42 43 45 67   : ignores points with specified extended classification codes
-ignore_single                       : ignores single returns
-ignore_first                        : ignores first returns
-ignore_last                         : ignores last returns
-ignore_first_of_many                : ignores first returns (but only those of multi-returns)
-ignore_intermediate                 : ignores intermediate returns
-ignore_last_of_many                 : ignores last returns (but only those of multi-returns)
-image ortophoto.tif                 : specifies *single* image file to be used for coloring (all) input file(s)
-imagedir ortophotos                 : specifies directory where correspondingly named image files are located
-dont_scale_rgb_up                   : copy RGBI values as they are (don't default upscale from 8 to 16 bit)
-zero_rgb                            : any LiDAR points not covered by the image receive the color black
-rgb                                 : default setting. copies the first three bands from image to LiDAR points
-band 2                              : specifies which band to copy for single band operations
-red                                 : copies the specified '-band 2' into the red (R) channels
-green                               : copies the specified '-band 0' into the green (G) channels
-blue                                : copies the specified '-band 2' into the blue (B) channels
-nir                                 : copies the specified '-band 1' into the NIR channel
-rgbnir                              : copies the first four bands of the image into the RGBNIR fields of the LAS file
-gray or -grey                       : copies the specified '-band 2' into all three RGB channels
-intensity                           : copies the specified '-band 1' into the intensity field
-classify_as 10                      : classifies all points falling into a black pixel as 10
-classify_black                      :            all points falling into a black pixels [default]
-classify_non_black                  :            all points falling into a non-black pixels
-classify_white                      :            all points falling into a white pixels
-classify_non_white                  :            all points falling into a non-white pixels
-ilay                                : apply all LASlayers found in corresponding *.lay file on read
-ilay 3                              : apply first three LASlayers found in corresponding *.lay file on read
-ilaydir E:\my_layers                : look for corresponding *.lay file in directory E:\my_layers
-olay                                : write or append classification changes to a LASlayers *.lay file
-olaydir E:\my_layers                : write the output *.lay file in directory E:\my_layers

****************************************************************

e:\LAStools\bin>lascolor -h
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
  -transform_helmert -199.87,74.79,246.62
  -transform_helmert 598.1,73.7,418.2,0.202,0.045,-2.455,6.7
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
  -copy_attribute_into_intensity 0
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
LAStools (by info@rapidlasso.de) version 180330 (commercial)
usage:
lascolor -i in.las -image ortho.tif -o out.laz
lascolor -i in.laz -image ortho.tif -dont_scale_rgb_up -odix _rgb -olaz
lascolor -i in_rgb.laz -image ortho.tif -zero_rgb -odix _new -olaz
lascolor -h

e:\LAStools\bin>lascolor -h
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
  -transform_helmert -199.87,74.79,246.62
  -transform_helmert 598.1,73.7,418.2,0.202,0.045,-2.455,6.7
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
  -copy_attribute_into_intensity 0
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
LAStools (by info@rapidlasso.de) version 180330 (commercial)
usage:
lascolor -i in.las -image ortho.tif -o out.laz
lascolor -i in.laz -image ortho.tif -dont_scale_rgb_up -odix _rgb -olaz
lascolor -i in_rgb.laz -image ortho.tif -zero_rgb -odix _new -olaz
lascolor -i lidar\*.laz -imagedir ortho -odir colored_lidar -olaz -cores 4
lascolor -i lidar\*.laz -odir colored_lidar -olaz -cores 4
lascolor -h

-------------

if you find bugs let me (info@rapidlasso.de) know.
