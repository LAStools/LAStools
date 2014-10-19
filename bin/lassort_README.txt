****************************************************************

  lassort:

  sorts the points of a LAS file into z-order arranged cells of
  a square quad tree and saves them into LAS or LAZ format. This
  is useful to bucket together returns from different swaths or
  to merge first and last returns that were stored in separate
  files.

  For standard LAS/LAZ files one simply chooses a -bucket_size
  to specify the resolution of finest quad tree cell. A bucket
  size of, for example, 5 creates 5x5 unit buckets. The z-order
  traversal of the quad tree creates implicit "finalization tags"
  that can later be used for streaming processing. 

  For LAS/LAZ files that are part of a tiling that was created
  with lastile it is beneficial to specify the resolution via
  the number of -levels of subtiling this tile. This has the
  advantage that both the tiling and the subtiling can be used
  during streaming processing.

  Another option is -average to coarsen the resolution of the
  quadtree until the average number of points per cell is as
  specified.

  The square quad tree used by lassort can (eventually) be
  exploited by "streaming TIN" generation code to seamlessly
  Delaunay triangulate large LAS/LAZ files (or large amounts
  of LAS/LAZ tiles) in a highly memory-efficient fashion. For
  that purpose, lassort either adds (or updates) a small VLR
  to the header the generated LAS/LAZ file.

  Large amounts of LAS data should first be sorted into tiles
  with lastile - which operates out-of-core - because lassort
  does its bucket sort in memory.

  Alternatively lassort can sort a LAS/LAZ file in GPS time order
  or in a point source ID order (or first sort by point source
  IDs and then by time).

  Please license from martin.isenburg@rapidlasso.com to use lassort
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

>> lassort *.las 

z-orders all LAS files with a default bucket size. 

>> lassort flight1*.las flight2*.las -gps_time

sorts all LAS files by their GPS time

>> lassort *.las -olaz -point_source

sorts all LAS files by their point source ID and stores them compressed

>> lassort *.laz -olaz -point_source -gps_time

sorts all LAZ files first by their point source ID and then by their
GPS time and stores them compressed

>> lassort *.txt -iparse xyzt -otxt -oparse xyzt   

z-orders all ASCII files with a default bucket size. 

>> lassort lidar.las sorted.las -v

z-orders the points from lidar.las with a default bucket size. 

>> lassort -i tile.las -o tile_subtiled.las -levels 3 -v

z-orders the points from tile tile.las with 3 subtiling levels. 

>> lassort -i tile.las -o tile_subtiled.las -average 1000 -v

z-orders the points from tile tile.las with as many subtiling
levels as required to get an average of 1000 points per bucket.

>> lassort -i lidar.las -o lidar_sorted.las -bucket_size 2 -v

z-orders the points from lidar.las with bucket size 2. 

for more info:

C:\lastools\bin>lassort -h
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
lassort *.las
lassort *.las -levels 5
lassort *.laz -bucket_size 10 -olaz
lassort *.txt -iparse xyzt -otxt -oparse xyzt
lassort lidar.las sorted.las
lassort -i tile.las -o tile_subtiled.las -levels 3 -v
lassort -i tile.las -o tile_subtiled.las -average 1000 -v
lassort -i lidar.las -o lidar_sorted.las -bucket_size 5
lassort -i lidar.las -o lidar_sorted.las -bucket_size 2 -v
lassort -h

---------------

if you find bugs let me (martin.isenburg@rapidlasso.com) know.
