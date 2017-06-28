****************************************************************

  lasheight:

  This tool computes the height of each LAS point above the
  ground. This assumes that grounds points have already been
  classified (classification == 2) so they can be identified
  and used to construct a ground TIN. The ground points can
  also be in an separate file '-ground_points ground.las' or
  '-ground_points dtm.csv -parse ssxyz'.

  Should the input coordinates be in some representation where
  the z-axis does not correspond to elevation, such as in an
  earth-centered representation, then the file needs converted
  to, for example, a UTM projection first.

  The tool reads LIDAR in LAS/LAZ/ASCII format, triangulates
  the ground points into a TIN (or whatever point class(es)
  were selected with '-class 8' or '-classification 2 8'), and
  calculates the elevation of each point with respect to this
  TIN. By default the resulting heights are scaled with a factor
  of 10.0, quantized & clamped into an unsigned char between
  0 and 255, and stored in the "user data" field of each point.
  Change the default scale factor of 10.0 with '-scale_u 20.0'
  or disable the storage in the "user data" field with option
  '-do_not_store_in_user_data' or by storing the height above
  the ground as "extra bytes" with '-store_as_extra_bytes' as
  an 0.01 scaled signed two-byte short [cm resolution] or with
  '-store_precise_as_extra_bytes' as an 0.001 scaled signed
  four-byte integer [mm resolution].

  Alternatively - to avoid quantizing and clamping - you can
  '-replace_z' the elevation value of each point with the computed
  height. That means that afterwards all ground points will have
  an elevation of zero and all other points will have an elevation
  that equals their relative height above (or below) the ground TIN
  at their x and y location. In a sense this will "normalize" the
  elevations of points in respect to their surrounding ground truth.
  If you add the '-replace_z' option the resulting heights are *not*
  scaled with a factor of 10.0, quantized & clamped into an unsigned
  char between 0 and 255, and stored in the "user data" field of each
  point ... unless you add the explicit '-store_in_user_data' option.

  You can also use the height to change the point classification
  with '-classify_below -1.0 7' or '-classify_above 100.0 10' and
  also '-classify_between 0.5 2.0 3 -classify_between 2.0 5.0 4'.

  Another alternative is to use the computed height to eliminate
  points with a particular ground height above or below a threshold
  with the options '-drop_below 1.5' or '-drop_above 6.8'.

  You can ignore certain point classes from being dropped and/or
  classified with '-ignore_class 6 7'.

  If there are too few ground points to construct a ground TIN or
  the ground points form a degenerate triangulation no heights can
  be computed. By default the files are simply copied (and in case
  of '-replace_z' all elevations are zeroed). Alternatively these
  files can be skipped with the command-line option '-skip_files'.

  This tool can also be used to convert from Geoid to Ellipoidal
  heights. For this we need the Geoid model as a grid of points
  "geoid.txt", "geoid.las" or "geoid.xyz" whose elevations express
  the difference between the ellipsoid and the geoid. Simply run:

  lasheight -i lidar.las -ground_points geoid.txt -replace_z -odix _geoid

  When using external ground points with '-ground_points geoid.laz'
  lasheight will *not* use all points in the file but cut out a
  "generous portion" surrounding the bounding box of the points
  whose height is to be computed. That "generous cutting" may fail
  when the 'geoid.laz' file has sparsely spaced points. Using the
  '-all_ground_points' option forces lasheight to use all points. 
 
  Please license from martin.isenburg@rapidlasso.com before using
  lasheight commercially.

  For updates check the website or join the LAStools mailing list.

  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @lastools

****************************************************************

example usage:

>> lasheight -i *.las

computes heights for all LAS files that match '*.las' and stores
them quantized and clamped into the "user data" field (an 8-bit
unsigned char) of each point record.

>> lasheight -i *.las -class 3

the same as above but uses points with classification 3 instead
of the default 2 to create the TIN that serves as the reference
elevation against which the heights are computed.

>> lasheight -i *.laz -olaz

the same as above for LASzip-compressed input and output.

>> lasheight -i *.las -replace_z

replaces the z coordinate of each point with the computed height
to avoid quantizing and clamping their value at the expense of
losing the original z elevation values.

>> lasheight -i lidar.las -o brush.las -drop_below 1.0 -drop_above 3.0

kepps only those points who are between 1 and 3 units above the
ground.

>> lasheight -i lidar.las -o heights.txt -oparse u

stores the heights to a file called 'heights.txt' as quantized
and clamped "unsigned chars".

>> lasheight -i lidar.las -replace_z -o heights.txt -oparse z

stores the heights to a file called 'heights.txt' as floating-
point values of the same precision as the original elevations.

for more info:

C:\lastools\bin>lasheight -h

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
LAStools (by martin@rapidlasso.com) version 140221 (unlicensed)
usage:
lasheight -i *.las -v
lasheight -i *.laz -olaz
lasheight -i *.las -class 8 -replace_z
lasheight -i *.las -ground_points ground.las -replace_z
lasheight -i *.las -ground_points geoid.xyz -replace_z
lasheight -i in.las -o out.las -drop_below 1.5
lasheight -i in.las -o out.txt -drop_above 6.5 -oparse xyzu
lasheight -i in.las -o out.las -drop_between 0.5 2.5
lasheight -i in.las -o out.las -classify_below -1 7
lasheight -i in.las -o out.las -classify_above 100 10
lasheight -i in.las -o out.las -classify_between 0.5 2 3 -classify_between 2 5 4 -ignore_class 6
lasheight -i in.las -o out.las -replace_z
lasheight -i in.las -o out.txt -replace_z -oparse z
lasheight -h

---------------

if you find bugs let me (martin.isenburg@rapidlasso.com) know.
