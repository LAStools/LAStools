****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

  lasboundary:

  reads LIDAR from LAS/LAZ/ASCII format and computes a boundary
  polygon for the points. By default this is a *concave hull*
  of the points which is - by default - always a single polygon
  where "islands of points" are connected by edges that are
  traversed in each direction once.

  Optionally a *disjoint hull* is computed with the '-disjoint'
  flag. This can lead to multiple hulls in case of islands. Note
  that tiny islands of the size of one or two LIDAR points that
  are too small to form a triangle and are "lost".

  The tool can also compute *interior* holes in the data via
  the '-holes' flag. It not only finds holes but also islands in
  the holes.

  The controlling value is '-concavity 100' that can be specified
  in the command line. The default is 50, meaning that voids with
  distances of more than 50 meters are considered the exterior (or
  part of an interior hole). For files in feet the concavity value
  that is always assumed to be meters will be multipled with 3.28.

  lasboundary can directly output in KML format for easy viewing
  in GE. In case there is no projection information in the LAS
  file it can be specified in the command line with '-utm 15T' or
  '-sp83 OH_N' or similar. If you request '-labels' then there will
  be one label per file (!!!) in the center of the bounding box.

  For both KML and SHP output you can put many outputs into one
  file with the '-overview' option. To have additional into about
  bounding box and number of points in each file add '-labels' in
  addition to the command line.

  Finally, the tool can also compute a standard *convex hull* with
  the '-convex' flag.

  There is also the option to use only the rectangular bounding
  box of the LAZ/LAS file as a ultra-coarse proxy for its shape
  with '-use_bb'.

  There is also the option to produce a coarse approximation of
  the boundary polygon in case the input file already has associated
  spatial indexing information generated with lasindex. If a LAS or
  LAZ file has a corresponding LAX file (possibly appended) then
  you can use option '-use_lax' to approximate the boundary from
  the contents of this spatial-indexing LAX file.

  The algorithm has recently been redesigned to make very efficient
  use of main memory. It now scales to much much larger LAS/LAZ/ASCII
  inputs than it was previously possible. For comparison, you can
  still run the older version of the algorithm that was limited to
  30 million points with the '-use_old' flag.
                 
  Please license from info@rapidlasso.de to use lasboundary
  in production.

  For updates check the website or join the LAStools mailing list.

  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @lastools

****************************************************************

example usage:

>> lasboundary -i *.las -oshp

computes the boundaries of all LAS file '*.las' individually and stores 
the result to ESRI's Shapefiles '*.shp'.

>> lasboundary -i *.las -merged -o merged.shp

computes the boundaries of the merged points from all LAS file '*.las'
and stores the result to the ESRI Shapefile 'merged.shp'.

>> lasboundary -i lidar1.las lidar2.las -merged -o lidar_boundary.shp

computes the boundary of the LAS file created from merging 'lidar1.las'
and 'lidar2.las' and stores the result to 'lidar_boundary.shp'.

>> lasboundary -i lidar1.las lidar2.las -otxt

the same but without merging and storing the results to ASCII files.

>> lasboundary -i lidar1.las lidar2.las -oshp -concavity 100

the same but with creating less detailed concavities. the default
value for concavities is 50 (meaning edges along the convex hull
that are shorter than 50 units get "pushed" inwards)

>> lasboundary -i lidar.las -o lidar_boundary.kml -utm 10T -disjoint

computes a disjoint hull instead of a concave hull and uses a utm
projeciton 10T to store the boundary in geo-referenced KML format

>> lasboundary -i lidar.las -o lidar_holes.kml -disjoint -holes

same as before but assumes geo-referencing is in the KML file. it
also computes holes in the interior of the boundary.

****************************************************************

overview of all tool-specific switches:

-v                                   : more info reported in console
-vv                                  : even more info reported in console
-quiet                               : nothing reported in console
-wait                                : wait for <ENTER> in the console at end of process
-version                             : reports this tool's version number
-fail                                : fail if license expired or invalid
-gui                                 : start with files loaded into GUI
-cores 4                             : process multiple inputs on 4 cores in parallel
-extra_pass                          : do extra read pass to count points (only makes sense when filtering)
-use_old                             : use an older (unoptimized) version of the algorithm
-concavity 20                        : granularity for grow concavities inwards (minimum: twice the pulse spacing)
                                       default: 50
-convex or -convex_hull              : compute a convex hull (same as setting concavity to infinite)
-disjoint or -disjoint_hull          : allow polygon to fragment for point clusters farther than concavity apart
-holes                               : find internal holes and output hole polygoons
-overview                            : create single overview file for multiple inputs
-labels                              : label outputs with file name, bounding box, number of points, etc ...
-base_url http://lidar.me/tiles      : construct valid download URLs by combining the base URL with file name
-use_bb                              : use header bounding box instead of points to contruct an approximate polygon
-use_tile_bb                         : use tile bounding box instead of points to contruct an approximate polygon
-use_lax                             : parse the LAX file instead of points to contruct an approximate polygon
-largest_only                        : when using the LAZ file ('-use_lax') only output the largest polygon found
-ilay                                : apply all LASlayers found in corresponding *.lay file on read
-ilay 3                              : apply first three LASlayers found in corresponding *.lay file on read
-ilaydir E:\my_layers                : look for corresponding *.lay file in directory E:\my_layers

****************************************************************

for more info:


E:\LAStools\bin>lasboundary -h
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
LAStools (by info@rapidlasso.de) version 180209 (academic)
usage:
lasboundary -i *.las -merged -o merged.shp
lasboundary -i *.laz -owkt -concavity 100 (default is 50)
lasboundary -i flight???.las -feet -oshp -disjoint
lasboundary -i tiles\*.laz -use_bb -oshp
lasboundary -i tiles\*.laz -use_lax -odir outlines -okml -utm 32N
lasboundary -i Serpent.las -disjoint -concavity 10 -holes -o outline.kml
lasboundary -i *.laz -merged -o merged.kml -disjoint -utm 17S
lasboundary -i lidar.las -o boundary.kml -longlat -concavity 0.00002
lasboundary -i *.txt -iparse ssxyz -otxt -first_only
lasboundary -i tiles\*.laz -merged -keep_class 4 5 -convavity 2.5 -o vegetation_layer.shp
lasboundary -i lidar.las -keep_class 6 -convavity 1.5 -o building_footprints.shp
lasboundary -h

other possible transformations for KML generation:
-sp83 IA_N
-sp27 IA_S

---------------

if you find bugs let me (info@rapidlasso.de) know.
