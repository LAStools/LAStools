****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

  lasplanes:

  Finds sufficiently planar patches of LAS/LAZ points fulfilling
  a number of user-defineable criteria that are output as simple
  polygons to SHP or RIEGL's PEF format. The tool was originally
  designed for generating tie planes to match the point clouds of
  a mobile scan that suffer from errors in the GPS trajectory to
  accurate terrestrial scans using clean planar patches that are
  "seen" without obstruction by both scanners.

  The algorithm divides the input into cells that are n by n by n
  units big. It then performs an eigen value decomposition of the
  covariance matrix of the points in each cell that has more than
  the minimal number of points. The three eigenvalues have to pass
  the small_eigen_max, middle_eigen_min, eigen_ratio_smallest, and
  eigen_ratio_largest criteria. And the plane of points must be
  sufficiently thin and formed by sufficiently many points after
  removing no more than a certain percentage of them. Then a polygon
  with a maximal number of points enclosing a subset of the points
  is formed that is checked for having a minimal size and a maximal
  off-planar standard deviation. The surviving planes are output
  (optionally only if they are sufficiently far from another).
  
  Here what some of the parameters mean:

  -cell_size n                : sets cells to size n*n*n [default 1.00]
  -cell_size_xyx x y z        : sets cells to size x*y*z
  -cell_points min            : skip if less than min points [default 100]
  -small_eigen_max max        : skip if smallest eigenvalue > max [default=0(not used)]
  -middle_eigen_min min       : skip if middle eigenvalue < min [default=0(not used)]
  -eigen_ratio_smallest r     : skip if smallest eigenvalue over sum of all three eigenvalues > r [default 0.00010]
  -eigen_ratio_largest r      : skip if largest eigenvalue over sum of all three eigenvalues > r [default 0.90000]                  
  -plane_thickness t          : skip if points form plane thicker than t [default 0.01]
  -plane_exclusion p          : skip if more than p % of points had to be excluded to make plane slim [default 5.00]
  -plane_points min           : skip if plane is formed by less than min points [default 100]
  -polygon_distance dist      : skip if polygon not sufficiently far from others
  -polygon_area a             : skip if area formed by points is less than a [default 0.50]
  -polygon_points max         : simplify polygon to have no more than max points [default=0(not used)]
  -polygon_stddev stddev      : skip if standard deviation of polygon points from plane > stddev [default=0(not used)]
  -polygon_name T_123_23      : set polygon base name [default 'patch']
  -polygon_digits d           : set number of digits used to enumerate polygon names [default 5]
  -output_polygon_points      : default <FALSE>        
  -output_marked_point_cloud  : default <FALSE>

  Please license 'lasplanes' from info@rapidlasso.de
  before using it commercially.

  For updates check the website or join the LAStools mailing list.

  https://rapidlasso.de/
  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/LAStools
  http://facebook.com/LAStools
  http://linkedin.com/groups?gid=4408378

  Martin @rapidlasso

****************************************************************

example usage:

>> lasplanes -i terrestrial_scan.laz -o planes.pef

finds all planar patches in the file 'terrestrial_scan.laz' and
stores the result in RIEGL's PEF format

>> lasplanes -i terrestrial_scan.laz -o planes.pef

same as above but outputting the SHP format

for more info:

D:\lastools\bin>lasplanes -h
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
Change the flags.
  -set_withheld_flag 0
  -set_synthetic_flag 1
  -set_keypoint_flag 0
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
Supported Line Outputs
  -o lines.shp
  -o boundary.wkt
  -o contours.kml
  -o output.txt
  -odir C:\footprints (specify output directory)
  -odix _isolines (specify file name appendix)
  -ocut 2 (cut the last two characters from name)
  -oshp -owkt -okml -otxt
  -stdout (pipe to stdout)
Optional Settings
  -only_2d
  -kml_absolute
  -kml_elevation_offset 17.5
lasplanes (by info@rapidlasso.de) version 140916
usage:
lasplanes -i in.las -o planes.shp
lasplanes -i in.las -oshp
lasplanes -i in.las -o planes.pef
lasplanes -i in.las -opef
lasplanes -h
---------------
if you find bugs let me (info@rapidlasso.de) know.
