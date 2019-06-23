****************************************************************

  lasclip:

  takes as input a LAS/LAZ/TXT file and a SHP/TXT file with one
  or many polygons (e.g. building footprints), clips away all the
  points that fall outside all polygons (or inside some polygons),
  and stores the surviving points to the output LAS/LAZ/TXT file.

  Instead of clipping the points they can also be reclassified with
  the '-classify 6' option or flagged with '-flag_as_withheld'.

  The input SHP/TXT file *must* contain clean polygons or polylines
  that are free of self-intersections, duplicate points, and/or
  overlaps and they must all form closed loops (e.g. last point
  and first point are identical).

  Sometimes polygons describe donut-shaped objects such as lakes
  with an island. Here the winding order of the lake with be CW
  and that of the island will be CCW. In order to correcly clip
  or classify only the lake (but now the island) the command line
  option '-donuts' needs to be added.

  There is an option called '-split' that splits the input LiDAR
  (be it one LAS/LAZ file or several on-the-fly '-merged' LAS/LAZ
  files) into one output file per polygon. If your SHP file has an
  associated DBF file then you can use '-split 0' to give output
  files the name or number stored in attribute with index 0. Or
  you can use '-split IDX' with IDX being the attribute's name.

  You can exclude certain point classes from the clipping or the
  reclassifying with option '-ignore_class 2' or '-ignore_class
  3 4 5 6' and they will all be written back to the clipped or
  the reclassified file. Then the clipping or the reclassifying
  will be applied exclusively to the other points.

  You can also ignore points based on their return counts with
  '-ignore_first_of_many', '-ignore_intermediate', '-ignore_last',
  '-ignore_last_of_many', '-ignore_first', '-ignore_single'.

  There is an example SHP file called "TO_city_hall.shp" that
  can be used together with the TO_core_last_zoom.las or the
  TO_core_last.las data set to clip away the Toronto city hall.

  Please license from martin.isenburg@rapidlasso.com to use lasclip
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

>> lasclip -i *.las -poly polygon.shp -v

clips all the LAS files matching "*.las" against the polygon(s) in 
"polygon.shp" and stores each result to a LAS file called "*_1.las".

>> lasclip -i *.txt -iparse xyzt -poly polygon.shp -otxt -oparse xyzt

same but for ASCII input/output that gets parsed with "xyzt".

>> lasclip -i TO_core_last_zoom.laz -poly TO_city_hall.shp -o output.laz -interior -v

clips the points falling *inside* the polygon that describes the building
footprint of the toronto city hall from the file TO_core_last_zoom.laz
and stores the result to output.laz.

>> lasclip -i TO_core_last_zoom.laz -poly TO_city_hall.shp -o output.laz -v

same as above but now it clips points falling *outside* of the polygon.

>> lasclip -i TO_core_last_zoom.laz -poly TO_city_hall.shp -o output.laz -classify 6 -interior -v

classifies the points falling *inside* the polygon as "Building".

>> lasclip -i TO_core_last_zoom.laz -poly TO_city_hall.shp -o output.laz -flag_as_withheld -interior

flags the points falling *inside* the polygon as 'withheld'.

>> lasclip -i city.las -poly buildings.txt -interior -o city_without_buildings.las

clips the points from the inside of the buildings footprints specified
in 'buildings.txt' out of the LAS file 'city.las' and stores the other
points to 'city_without_buildings.las'. The text file should have the
following format:

757600 3.6927e+006
757432 3.69264e+006
757400 3.69271e+006
757541 3.69272e+006
757600 3.6927e+006
#
757800 3.6917e+006
757632 3.69164e+006
757600 3.69171e+006
757741 3.69172e+006
757800 3.6917e+006
[...]

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
-ignore_class 0 1 3 5 6 7 9          : ignores points with specified classification codes
-ignore_extended_class 42 43 45 67   : ignores points with specified extended classification codes
-ignore_single                       : ignores single returns
-ignore_first                        : ignores first returns
-ignore_last                         : ignores last returns
-ignore_first_of_many                : ignores first returns (but only those of multi-returns)
-ignore_intermediate                 : ignores intermediate returns
-ignore_last_of_many                 : ignores last returns (but only those of multi-returns)



... more to come ...

-dont_remove_empty_files             : do not remove files that have zero points remaining from disk
-ilay                                : apply all LASlayers found in corresponding *.lay file on read
-ilay 3                              : apply first three LASlayers found in corresponding *.lay file on read
-ilaydir E:\my_layers                : look for corresponding *.lay file in directory E:\my_layers
-olay                                : write or append classification changes to a LASlayers *.lay file
-olaydir E:\my_layers                : write the output *.lay file in directory E:\my_layers

****************************************************************


for more info:

D:\LAStools\bin>lasclip -h
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
LAStools (by martin@rapidlasso.com) version 150526 (commercial)
usage:
lasclip -i *.las -poly polygon.shp -v
lasclip -i *.txt -iparse xyzt -poly polygon.shp -otxt -oparse xyzt
lasclip -i lidar.las -poly footprint.shp -o lidar_clipped.laz -v
lasclip -i lidar.laz -poly buildings.shp -o lidar_clipped.laz -interior
lasclip -i lidar.laz -poly swath.shp -o lidar_overlap.laz -classify_as 12
lasclip -i lidar.laz -poly hydro.shp -o clean.laz -ignore_class 2
lasclip -i lidar.laz -poly noise.shp -o lidar_clean.laz -interior -flag_as_withheld
lasclip -i forest\*.laz -merged -poly plots.shp -split -o plots.laz

---------------

if you find bugs let me (martin.isenburg@rapidlasso.com) know.
