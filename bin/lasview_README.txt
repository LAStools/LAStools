****************************************************************

  lasview:

  A simple OpenGL-based viewer for LIDAR in LAS/LAZ/ASCII format
  that can also edit or delete points as well as compute/display
  a TIN computed from (a selection of) the points.

  EDITING & DELETING POINTS

  Press <e> to start r<e>classifying point as noise (7). Use the
  pop-up menu (right click) to select other target classes. Draw
  a polygon enclosing some points and press <r> to register the
  operation. With <ESC> you can unset the last polygon point and
  with <CTRL+u> you can undo the operation or with <CTRL+o> redo
  it again.

  Press <d> to start <d>eleting points. Otherwise same procedure
  as above.

  When finished press <CTRL+s> to save the edits as LASlayers in
  to a LAY file. Warning ... if you did not start lasview with
  the '-ilay' option you will not append to an existing LAY file
  but overwrite it. There will be a WARNING though. Later, you
  can check your layers of edits with 'laslayers.exe' or view
  and continue editing with the '-ilay' option when starting
  lasview again.

  If you want to apply your edits and creating a new file press
  <CTRL-a> and an entirely new file (currently with "_1" at the
  end of the input file name) is created.

  Do not use lasview with on-the-fly filters or transforms when
  you plan to do editing operations that you are going to save.

  VIEWING AND TRAVERSING CROSS SECTIONS 

  For quality checking and detail views you can press <x> and
  select a cross section or smaller area of interest. By default
  you will draw a profile rectangle. Holding down <SHIFT> draws
  a profile line. Its width can be chosen in the pop-up menu. Move
  either around with the arrow key. Holding down <SHIFT> or <ALT>
  is taking larger or smaller steps. Press <x> again and you see
  only the selected area. Again, traverse the LiDAR my moving
  around with the arrow keys. You can do your edits in this
  cross section view. Jumping back between cross section view
  and the full extend with <x> help maintaining orientation.

  With <CTRL-x> you can end the cross section view.

  Please contact martin.isenburg@rapidlasso.com if you want to
  use lasview commercially.

  For updates check the website or join the LAStools mailing list.

  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @rapidlasso

****************************************************************

example usage:

>> lasview lidar.las
>> lasview -i lidar.las

reads around 1000000 subsampled lidar points and displays in 50 steps

>> lasview *.las

merges all LAS files into one and displays a subsampling of it

>> lasview -i lidar.txt -iparse xyzc

converts an ASCII file on-the-fly with parse string 'xyzc' and displays it

>> lasview -i lidar.las -win 1600 1200

same as above but with a larger display window

>> lasview -i lidar.las -steps 10 -points 10000000

reads around 10 million subsampled lidar points and displays in 11 steps

interactive options:

<t>     compute a TIN from the displayed returns
<T>     remove the TIN <SHIFT>+<t>
<h>     change shading mode for TIN (hill-shade, elevation, wire-frame)
<G>     show rasterization "needles" for grid set by '-grid n' value <SHIFT>+<g>
<R>     raster TIN with grid cell size set by '-grid n' value <SHIFT>+<r>

<y>     take one step of spike-free algorithm (press many times to see algorithm in action)
<Y>     complete spike-free algorithm to end <SHIFT>+<y>

<a>     display all returns
<l>     display last returns only
<f>     display first returns only
<g>     display returns classified as ground
<b>     display returns classified as building
<B>     display returns classified as building + ground <SHIFT>+<b>
<v>     display returns classified as vegetation
<V>     display returns classified as vegetation + ground <SHIFT>+<v>
<j>     display returns classified as building + vegetation
<J>     display returns classified as building + vegetation + ground <SHIFT>+<j>
<m>     display returns classified as keypoints / mass points
<n>     display returns classified as noise
<o>     display returns classified as overlap
<w>     display returns classified as water
<u>     display returns that are unclassified
<0>     display ((point source IDs % 8) == 0)
<1>     display ((point source IDs % 8) == 1)
<2>     display ((point source IDs % 8) == 2)
<3>     display ((point source IDs % 8) == 3)
<4>     display ((point source IDs % 8) == 4)
<5>     display ((point source IDs % 8) == 5)
<6>     display ((point source IDs % 8) == 6)
<7>     display ((point source IDs % 8) == 7)

<e>      start editing points
<d>      start deleting points
<r>      register edit / delete operation

<CTRL-u> undo last changes
<CTRL-o> redo last undo

<CTRL-s> save changes as LASlayers
<CTRL-f> force LAY file overwrite
<CTRL-a> apply LASlayers to create new LAS/LAZ file

<i>       pick a point
<I>       pick a point and draw line to last picked point <SHIFT>+<i>
<x>       turn on and toggle between overview and rectangle / line cross-section 
<SHIFT>   hold down to pick line instead of rectangle cross-section when in overview 
<CTRL-x>  overview and rectangle / line cross-section view on / off

<SHIFT> translate mode
<CTRL>  zoom mode
<ALT>   pan mode
<space> switch between pan/translate/zoom/tilt
<-/=>   render points smaller/bigger
<[/]>   scale elevation
<{/}>   scale xy plane
<c>     change color mode
<X>     hide/show bounding box <SHIFT>+<x>
<s/S>   step forward/backward
<z/Z>   tiny step forward/backward

<A>     toggle adaptive Z-scaling on/off <SHIFT>+<a>
<Q>     show spatial index structure (if LAX file available) <SHIFT>+<q>
<q>     pick spatial index cell
<E>     render structural elements around all points <SHIFT>+<e>
<:>     decrease structural element radius
<">     increase structural element radius
<W>     show LAS 1.3 waveforms +/- 25 points around picked point <SHIFT>+<w>

****************************************************************

overview of all tool-specific switches:

-v                                   : more info reported in console
-vv                                  : even more info reported in console
-quiet                               : nothing reported in console
-version                             : reports this tool's version number
-fail                                : fail if license expired or invalid
-gui                                 : start with files loaded into GUI
-win 1200 900                        : start with render window size of 1200 by 900 pixels
-subseq 1000000 2000000              : only load subsequence from 1 millionth to 2 millionth point
-start_at_point 1500000              : start loading from point at position 1500000 in the file
-stop_at_point 5000000               : stop loading points once the 5 millionth point was read
-light 1 0 1                         : set light direction vector to (1/0/1) for hillshaded TIN
-points 10000000                     : load maximally 10 million points from file (with regular sub sampling)
                                       default is 5000000
-steps 30                            : visualize incremental loading of points from file in 30 steps
                                       default is 50
-every 1000000                       : visualize incremental loading of points every 1000000 points
-concavity 100                       : remove large exterior triangles from TIN who have an edge longer 100
                                       default is 50
-holes 10  (or -kill 10)             : remove any triangle from TIN that has an edge length bigger than 10
-grid 5                              : set raster spacing for visualization of rasterization with <R>
                                       default is 1
-load_gps_time or -load_gps_second   : also loads GPS time stamps from file and displays them when pressing <i>
-circles 0.1                         : define structural elements as circles of radius 0.1
-cones 0.2 0.5                       : define structural elements as cones of radius 0.2 and height 0.5
-flats 0.1 0.3 0.2                   : define structural elements as flats of small radius 0.1, height 0.3, and large radius 0.2
-mark_point 152617                   : surround point at position 152617 in file with a pink sphere of 0.5 meter radius
-set_min_max 47.0 63.0               : clamp color ramp used for elevation colorings to range [47.0 63.0] instead of [min max]
-steep 1.5                           : remove all "steep" triangles from the TIN with z values spanning more than 1.5 meters
-spike_free 0.7 0.3 0.75             : run spike-free algorithm with freeze = 0.7, interval = 0.3, buffer = 0.75
-spike_free                          : run spike-free algorithm with defaults freeze = 1.5, interval = 0.25, buffer = 0.5
-reversed                            : run spike-free in reverse (from bottom to top)
-kamera 0 -59 -24 0 -0.0572 0.385    : move virtual camera to specifies parameters (print parameters with <K> or <SHIFT>+<k>)
-only_first                          : render only first returns
-only_last                           : render only last returns
-only_multi                          : render only multiple returns
-only_single                         : render only single returns
-ground                              : render only points classified as ground
-buildings                           : render only points classified as building
-vegetation                          : render only points classified as vegetation
-object                              : render only points classified as vegetation or building
-ground_buildings                    : render only points classified as ground or building
-ground_vegetation                   : render only points classified as ground or vegetation
-ground_object                       : render only points classified as ground, vegetation, or building
-scale_rgb                           : down-scale 16 bit RGB values for storage to 8 bit RGB values (usually auto-detected).
-cp control.txt                      : load control points with parse string xyz from file 'control.txt' and visualize each as red sphere with radius 1 meter
-cp_parse ssxyz                      : use parse string ssxyz to parse control point file
-subcircle 0.2                       : prior to creating TIN with <t> replace each point with 8 segment circle of radius 0.2 
-color_by_elevation1                 : render points by elevation color ramp (black->red->yellow->white)
-color_by_elevation2                 : render points by elevation color ramp (blue->yellow->red)
-color_by_rgb                        : render points by RGB color
-color_by_intensity                  : render points by intensity
-color_by_classification             : render points by classification color
-color_by_return                     : render points by return colors (single = yellow, first of many = red, last of many = blue, intermediate = green)
-color_by_flightline                 : render points by randomly assigning 8 different colors based on the flightline ID
-color_by_user_data                  : render points by mapping the 8-bit user data field to a color ramp (blue -> green -> red)
-suppress_z                          : don't decompress z coordinates for native-compressed LAS 1.4 point types 6 or higher
-suppress_classification             : don't decompress classification for native-compressed LAS 1.4 point types 6 or higher
-suppress_intensity                  : don't decompress RGB for native-compressed LAS 1.4 point types 6 or higher
-suppress_user_data                  : don't decompress RGB for native-compressed LAS 1.4 point types 6 or higher
-suppress_point_source               : don't decompress RGB for native-compressed LAS 1.4 point types 6 or higher
-suppress_RGB                        : don't decompress RGB for native-compressed LAS 1.4 point types 6 or higher
-background_black                    : start viewer with a black background
-background_white                    : start viewer with a white background
-background_grey                     : start viewer with a grey background
-background_red                      : start viewer with a red background
-background_green                    : start viewer with a green background
-background_blue                     : start viewer with a blue background
-no_bounding_box                     : do not render the green bounding box
-ilay                                : apply all LASlayers found in corresponding *.lay file on read
-ilay 3                              : apply first three LASlayers found in corresponding *.lay file on read
-ilaydir E:\my_layers                : look for corresponding *.lay file in directory E:\my_layers
-olaydir E:\my_layers                : write the output *.lay file (from editing) in directory E:\my_layers

****************************************************************

for more info:

E:\LAStools\bin>lasview -h
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
LAStools (by martin.isenburg@rapidlasso.com) version 180209
usage:
lasview -i terrain.las -verbose
lasview -i flight1*.laz flight2*.laz
lasview -i lidar1.las lidar2.las lidar3.las
lasview -i *.txt -iparse xyz
lasview -i lidar.laz -win 1600 1200 -steps 10 -points 200000
lasview -i lidar.laz -subseq 10000 20000
lasview -i lidar.laz -color_by_elevation1
lasview -i lidar.laz -color_by_elevation2
lasview -i lidar.laz -color_by_rgb
lasview -i lidar.laz -color_by_intensity
lasview -i lidar.laz -color_by_classification
lasview -i lidar.laz -color_by_return
lasview -i lidar.laz -color_by_flightline
lasview -i lidar.laz -color_by_user_data
lasview -h

----

if you find bugs let me (martin.isenburg@rapidlasso.com) know
