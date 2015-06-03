****************************************************************

  lasoverage:

  This tool reads LIDAR point in the LAS/LAZ/ASCII/BIN format
  of an airborne collect and finds the "overage" points that
  get covered by more than a single flightline. It either marks
  these overage points or removes them from the output files.
  The tool requires that the files either have the flightline
  information stored for each point in the point source ID field
  (e.g. for tiles containing overlapping flightlines) or that
  there are multiple files where each corresponds to a flight
  line ('-files_are_flightlines'). It is also required that the
  scan angle field of each point is populated.

  If the point source ID field of a LAS tile is not properly
  populated (but there are GPS time stamps) and each point has
  a scan angle, then you can use the '-recover_flightlines'
  flag that reconstructs the missing flightline information
  from gaps in the GPS time.

  The most important parameter is '-step n' that specifies the
  granularity with which the overage points are computed. It
  should be set to approximately 2 times the point spacing in
  meters.  If unknown, you can compute the point spacing with
  the lasinfo tool via 'lasinfo -i lidar.las -cd'. 

  If the x and y coordinates of the input files are in feet the
  '-feet' flag needs to be set (unless the LAS/LAZ file has a
  projection with this information).

  If the input are multiple files corresponding to individual
  flight lines the '-files_are_flightlines' parameter must be
  set.

  By default the tool will set the classification of the overage
  points to 12. However, instead you can also choose to use the
  '-flag_as_withheld' or '-flag_as_overlap' (new LAS 1.4 point
  types only) flags or '-remove_overage' points from the output.
  
  Below an explaination based on what Karl Heidemann of the USGS
  once told me regarding "overlap" versus "overage" points:

  "Overlap" applies to any point in one flightline or swath that
  is anywhere within the boundary of another flightline. Whether
  or not a point is overlap is determined, basically, at the time
  of flight. It is a fundamentally immutable characteristic.

  "Overage" applies to any point not in the "tenderloin" of the
  flightline or swath, that is, the central core of the flightline
  that - when combined with the others - creates a non-overlapped
  and gapless coverage of the surface.  Whether or not a point is
  overage is determined by the operator (or software) that defines
  the single-coverage tenderloins of the flightlines. Hence, it is
  a subjective characteristic -- it depends solely on where somebody
  (here: lasoverage.exe) chooses to draw the boundary.

  Unfortunately the LAS specification uses the word "overlap" where
  it should be using the word "overage".

  Please license from martin@rapidlasso.com before using lasoverage
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

>> lasoverage -i tile.las -step 2 -o tile_overage.laz

finds the overlap points and classifies them as 12 for a LAS tile
with a point spacing of around 1.0 meters. For this to work, the
LiDAR points in the LAS file have their point source ID populated
with the flight line number. The output is also compressed.

>> lasoverage -i tiles\tile_*.laz -step 2 -flag_as_withheld -olaz

same as above but for an entire folder of LAZ tiles and with the
overage points being marked as "withheld".

>> lasoverage -i tiles\tile_*.laz -step 2 -recover_flightlines -flag_as_withheld -cores 4 -odix _flagged -olaz

same as above but with an initial pass over each tile where the
flightline information is reconstructed in an initial pass over
the points by looking for continuous intervals of GPS time stamps
and by operating on 4 cores.

>> lasoverage -i tiles\tile_*.las -step 2 -flag_as_overlap -odix _flagged -olas

same as above but for an entire folder of LAS 1.4 tiles and with 
the overage points being marked as "overlap". this can only be used
with the new point types 6 and higher ...

>> lasoverage -i flight\lines*.laz -files_are_flightlines -odir flight_flagged -olaz

here all files are considered to be part of the same flight and
the overlap in flightstrips is computed across all files. all points
of a file are considered to be from the same flightline and their
overlap against all other files is computed. The LiDAR points of
each strip should have a point spacing around 0.5 meters since the
default step of 1 is used.


for more info:

D:\LAStools\bin>lasoverage -h
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
  -olas -olaz -otxt -obin -oqfit (specify format)
  -stdout (pipe to stdout)
  -nil    (pipe to NULL)
LAStools (by martin@rapidlasso.com) version 150526 (academic)
usage:
lasoverage -i tile.las -o out.las
lasoverage -i tile.laz -o out.laz -feet
lasoverage -i tile.las -o out.laz -flag_as_withheld
lasoverage -i tile.las -o out.laz -flag_as_overlap
lasoverage -i tile.laz -o out.las -remove_overage
lasoverage -i *.las -files_are_flightlines
lasoverage -h

---------------

if you find bugs let me (martin.isenburg@rapidlasso.com) know
