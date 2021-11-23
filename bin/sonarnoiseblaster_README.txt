****************************************************************

  sonarnoiseblaster:

  NOTE: This tool is *NOT* part of LAStools

  Removes noise from multi-beam echo-sounder (MBES) data using a
  streaming connected component algorithm. The two key parameters
  to change are the '-cut_threshold 0.8' that specifies at which
  vertical elevation difference the edges between the points are
  cut and the '-connected_echoes 20000' value that determines the
  size that a connected point cluster must minimally have to be
  part of the output. All smaller connected point clusters will
  be deleted. The default values are 0.5 and 10000.

  The points are "Across=Delaunay-Connected" which means they do
  not only have edges to their Delaunay triangulation neighbors
  that is computed using only the points' x and y coordinates but
  also to the corresponding fourth vertex that lies in the across
  triangles of all triangles that are part of their Delaunay star.

  Below an example where a vertex has 6 Delaunay-connected vertices
  and 6 across-connected vertices. All of the Delaunay-connected 
  vertices but only 2 of the across-connected vertices are shown:


                       D-------D
                      / \     / \          * - considered vertex 
                     /   \   /   \         D - Delaunay-connected
                    /     \ /     \        X - across-connected
                   D-------*-------D
                  / \     / \     /                       
                     \   /   \   /
                /     \ /     \ /          
               X- - - -D-------D
                        \     /
                             
                          \ /
                           X

  https://rapidlasso.de/
  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @lastools

****************************************************************

example usage:

>> sonarnoiseblaster -i reef.laz -o reef_cleaned.laz

Cleans the file 'reef.laz' with the default cut_threshold of 0.5
and keep all connected components 10000 and bigger.

>> sonarnoiseblaster -i reef.laz -o reef_cleaned.laz -large 

Same as above but prints some info about the 7 largest components
to the console.

>> sonarnoiseblaster -i reef.laz -o reef_cleaned.laz -histo

Same as above but prints a histogram over all component sizes to
the console.

>> sonarnoiseblaster -i reef.laz -o reef_cleaned.laz -cut_threshold 0.4 -connected_echoes 5000

Same as above but cutting edges already when the vertical different
of the end-points is 40 cm but keeping points once the size of their
connected component reaches 5000 or higher.


For more info:

D:\lastools\bin>sonarnoiseblaster.exe -h
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
SONAR clean (by info@rapidlasso.de) version 140614 (unlicensed)
usage:
sonarnoiseblaster -i sonar.las -o cleaned_sonar.las
sonarnoiseblaster -i *.laz -merged -o cleaned_sonar.laz
sonarnoiseblaster -i *.laz -merged -o cleaned_sonar.laz -large
sonarnoiseblaster -i *.laz -merged -o cleaned_sonar.laz -histo
sonarnoiseblaster -i *.laz -merged -o cleaned_sonar.laz -cut_threshold 0.7
sonarnoiseblaster -i *.laz -merged -o cleaned_sonar.laz -connected_echoes 20000
sonarnoiseblaster -i *.laz -merged -o cleaned_sonar.laz -cut_threshold 1.0 -connected_echoes 30000
sonarnoiseblaster -h

---------------

if you find bugs let me (info@rapidlasso.de) know.