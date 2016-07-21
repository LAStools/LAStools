****************************************************************

  blast2iso:

  This tool can reads billion of LIDAR points from the LAS/LAZ
  format, triangulates them a seamless (!) TIN, and extracts 
  contours at the specified elevations from the TIN. Optionally
  the TIN can first be smoothed, the contours be simplified or
  cleaned. and the output be tiled. Supported output formats are
  SHP, TXT, WKT, or KML. The latter is only possible if there
  is geo-referencing information in the input file or if it is
  specified explicitely in the command line. 

  Note: to guarantee non-crossing iso-contours the '-simplify'
  (or '-simplify_length) and the '-simplify_area' parameters
  should not be used (or set to zero). This is because those
  two options directly try to simplify individual contours after
  they were extracted, which can lead to crossing. However, the
  '-smooth' option operates on the TIN before extracting the
  contours, and hence is safe. The '-clean' option simply removes
  entire contours that are too short. You can get less wiggly
  contours by first thinning the points with the '-contours' 
  option available in lasthin.

  Isolines crossing triangles whose edge length is larger than a
  threshold can be eliminated with the '-kill 250' option. The
  default is a kill of 50 meter. Use '-kill 1000000' to disable
  this. The value is always assumed to be meters and will be
  multipled with 3.28 for LAS/LAZ files where x and y are known
  to be in feet.

  This is part of the BLAST extension pack of LAStools that is
  built on streaming TINs via spatial finalization & streaming
  Delaunay. Please license from martin@rapidlasso.com before
  you use blast2iso commercially.

  For updates check the website or join the LAStools mailing list.

  http://rapidlasso.com/
  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @lastools
 
****************************************************************

example with LAZ file from the .\lastools\data folder:

>> blast2iso -i ..\data\TO_core_last_zoom.laz -o contours.shp -iso_every 10 
>> blast2iso -i ..\data\TO_core_last_zoom.laz -o contours.shp -iso_every 20 

>> blast2iso -i ..\data\TO_core_last_zoom.laz -o contours.kml -iso_every 10 -utm 17T
>> blast2iso -i ..\data\TO_core_last_zoom.laz -o contours.kml -iso_every 20 -utm 17T

example usage:

>> blast2iso -i *.laz -merged -keep_class 2 8 -o contours.shp -iso_every 2

combines the ground points of all LAZ files that match *.las, creates
seamlessly contours every two elevation units (feet or meters), and
stores the resulting contours in SHP format.

>> blast2iso -i *.las -oshp -iso_every 10

contours all points of each LAS files *.las individually with a
spacing of 10 units between contours and stores each resulting
set of coutours in SHP format.

>> blast2iso -i *.laz -merged -utm 17T -iso_every 5 -o merged_contours.kml

contours the merged points all the LAZ files *.laz with a contour
spacing of 5 units and stores the result in KML format under the
assumption that all points are in UTM zone 17T.

for more info:

C:\lastools\bin>blast2iso.exe -h

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
LAStools BLAST (by martin.isenburg@rapidlasso.com) version 140301 (unlicensed)
usage:
blast2iso -i gigantic.laz -o contours.shp -iso_range 1510 1540 1
blast2iso -i huge_lidar.las -o contours.shp -iso_number 20
blast2iso -i huge_lidar.laz -keep_class 2 -iso 1200 -iso 1210 -iso 1220 -oshp
blast2iso -i *.laz -iso_every 2 -oshp -kill 250
blast2iso -i *.laz -merged -iso_every 2 -o merged.shp
blast2iso -i huge_lidar.las -keep_class 2 6 8 -o contours.shp -iso_every 0.5
blast2iso -h

---------------

if you find bugs let me (martin.isenburg@rapidlasso.com) know
