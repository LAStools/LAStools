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

  Please license from martin@rapidlasso.com before using lascolor
  commercially. Please note that the unlicensed version will set
  intensity, gps_time, user data, and point source ID to zero,
  slightly change the LAS point order, and randomly add a tiny
  bit of white noise to the points coordinates once you exceed a
  certain number of points in the input file.

  For updates check the website or join the LAStools mailing list.

  http://rapidlasso.com/
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

Below you see the resulting changes in point type and RGB colors:

>> lasinfo LAS_18910_2010_25.laz  (original)
reporting all LAS header entries:
  file signature:             'LASF'
  file source ID:             18910
  global_encoding:            0
  project ID GUID data 1-4:   00000000-0000-0000-0000-000000000000
  version major.minor:        1.2
  system identifier:          'LAStools (c) by Martin Isenburg'
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
  system identifier:          'LAStools (c) by Martin Isenburg'
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
  system identifier:          'LAStools (c) by Martin Isenburg'
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

D:\lastools\bin>lascolor -h
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
LAStools (by martin@rapidlasso.com) version 140301 (unlicensed)
usage:
lascolor -i in.las -image ortho.tif -o out.laz
lascolor -i in.laz -image ortho.tif -dont_scale_rgb_up -odix _rgb -olaz
lascolor -i in_rgb.laz -image ortho.tif -zero_rgb -odix _new -olaz
lascolor -h

-------------

if you find bugs let me (martin@rapidlasso.com) know.
