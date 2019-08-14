****************************************************************
 
  lasoverlap:

  This tool reads LIDAR points from LAS/LAZ/ASCII/BIN/SHP and
  checks the flight line overlap and / or the vertical and
  horizontal alignment. Do not add  '-files_are_flightlines'
  or '-faf' to the command-line the input is tiles. Only use
  these command-line switches when files *are* flight lines. 
  
  The most important parameter '-step n' specifies the n x n
  area that of LiDAR points that are gridded on one raster
  (or pixel) that are then used for doing the overlap and/or
  difference calculations. The output is either in BIL, ASC,
  IMG, TIF, PNG, JPG, XYZ,or DTM format. When computing the
  difference in the flight line overlaps, by default the tool
  will check '-elevation_lowest', but '-intensity_highest', or
  '-counter' are possible too.
  
  By default the tools create visualization images rather than
  alignment difference rasters or overlap counter grids. There
  are three ways to specify how to visualize the differences:
  (1) Use '-min_diff 0.1' and '-max_diff 0.2' to specify that 
      elevation differences of up to +/- 10 cm are okay (get 
      mapped to white) but differences of over +/- 20 cm are 
      critical (get mapped to red or blue). Differences between
      min and max increase in saturation shades of red or blue.
  (2) Use '-bands 0.1 0.2 0.3 0.4' to map differences from 0 up
      to +/- 10 cm to white, up to +/- 20 cm to blue, up to +/-
      20 cm to green, up to +/- 30 cm to yellow, and above +/- 40
      cm to red. 
  (3) Use with '-color_bands 0.15 0x00FF00 0.4 0xFFA500' to map
      differences from 0 up to +/- 15 cm to white, up to +/- 40 cm
      to green, and above +/- 40 cm to orange. 

  When the input are already tiled flightlines that do not have
  flight line information in the point source ID then the option
  '-recover_flightlines' may be useful that tries to use the GPS
  time stamp of each point to resolve which points are from the
  same flightline.

  Optionally, a KML file is generated that allows the resulting
  raster  to be immediately displayed inside a geospatial context
  provided by Google Earth (for TIF/PNG/JPG images). In case the
  LAS/LAZ file contains projection information (i.e. geo keys
  as variable length records) this metadata is used to correctly
  geo-reference the KML file. It is also possible to provide the
  proper geo-referencing information in the command-line.

  Use '-subsample n' with 1 < n < 5 to anti-alias the gridding of
  LiDAR points by their x and y coordinate into disjunct rasters.
  The option '-subsample 3' adds each LiDAR point 9 times to the
  raster at locations (x/y), (x+0.33*step/y), (x+0.66*step/y),
  (x/y+0.33*step) (x+0.33*step/y+0.33*step) (x+0.66*step/y+0.33*step),
  (x/y+0.66*step) (x+0.33*step/y+0.66*step) (x+0.66*step/y+0.66*step)
  and thereby "washes out" hard boundaries.

  Please license from martin.isenburg@rapidlasso.com to use lasoverlap
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

>> lasoverlap -i tile.las -step 2 -o overlap.png

creates an overlap raster as well as a difference raster with a
step of 2 units. For this, it is necessary that the LiDAR points
in the LAS file have their point source ID populated with the
flight line ID. The overlap raster uses the default color ramp
which maps overlap counts from 0 to 5 to a different colors. The
difference raster uses the default color ramp that maps blue to
-2.5, white to 0, and red to 2.5. The default output is PNG.

>> lasoverlap -i tile.las -step 2 -values -oasc

same as above but output the actual values into an ASC raster
(instead of the color-coded values into a PNG).

>> lasoverlap -i tile.las -step 2 -values -oasc -recover_flightlines

same as above for the case the tile does not have proper information 
in the point source ID field but has valid GPS time stamps. The GPS
time stamps are then used to recover the flight line information by
looking for continous segments (or rather interruptions) in the GPS
time stamps.

>> lasoverlap -i LDR*.las -files_are_flightlines -step 3 -min_diff 0.1 -max_diff 0.4 -o overlap.png

merges all the files "LDR*.las" while assigning the points of each
file a unique point source ID (aka flight line number), and then
creates an overlap raster as well as a difference raster with a step
of 3 units. The overlap raster uses the default range of 5 overlaps
lines for the color ramp. The difference raster uses '-min_diff 0.1' and
'-max_diff 0.4' which maps the range (-0.4 ... -0.1) to (blue ... white)
and the range (0.1 ... 0.4) to (white ... red). The range (-0.1 ... 0.1)
is mapped to white.  

>> lasoverlap -i LDR*.las -files_are_flightlines -step 3 -min_diff 1.0 -max_diff 2.0 -o overlap.png

same as above. But here the difference raster uses '-min_diff 1' and
'-max_diff 2' to map range (-2 ... -1) to color ramp (blue ... white),
range (-1 ... +1) to white and (+1 ... +2) to (white ... red).

>> lasoverlap -i LDR*.las -files_are_flightlines -step 2 -max_diff 0.5 -no_over

same as above but does not output an overlap raster ('-no_over') and
operates with different '-step 2' and '-max_diff 0.5' settings.

>> lasoverlap -i LDR*.las -files_are_flightlines -step 5 -no_diff

same as above but does not output a difference raster ('-no_diff') but
only a coarse (-step 5) overlap raster.

>> lasoverlap -i tiles*.laz -step 2 -no_over -utm 15N

operates tile by tile (e.g. creates one difference raster per file)
and generates a Google Earth KML file for each tile using UTM 15N. It
is necessary that the LiDAR points in the LAZ file have their point
source ID populated.

>> lasoverlap -i tiles*.laz -step 2 -no_over -utm 15N -intensity -highest

same as above but computes intensity differences. For intensities the
difference raster maps range -255 ... 0 ... 255 (e.g. '-max_diff 255').

>> lasoverlap -i tiles*.laz -keep_last -step 2 -no_over -utm 15N -counter -highest

same as above but computes last return count differences. For counts the
difference raster maps range -100 ... 0 ... 100 (e.g. '-max_diff 100').

overview of all tool-specific switches:

-v                                   : more info reported in console
-vv                                  : even more info reported in console
-quiet                               : nothing reported in console
-wait                                : wait for <ENTER> in the console at end of process
-version                             : reports this tool's version number
-fail                                : fail if license expired or invalid
-gui                                 : start with files loaded into GUI
-cores 4                             : process multiple inputs on 4 cores in parallel
-faf                   : input files are flightlines. do ***NOT*** use this for tiled input
-mem                   : amount of main memory to use in MB (500 - 2000) [default: 1500]
-temp_files            : base file name for temp files (example: E:\tmp)
-step 2                : raster with stepsize 2 [default: 1]
-fill 1                : fills voids in the grid with a square search radius of 1 
-use_bb                : raster full extend of bounding box
-use_tile_bb           : only raster extend of tile bounding box (for tiles generated with lastile)
-use_orig_bb           : only raster extend of original bounding box (for tiles generated with '-buffered 30')
-subsample 2           : see long explanation above
-nbits 16              : use 16 bits to represent the elevation (mainly used with BIL format)
-no_over               : do not generate overlap rasters
-no_diff               : do not generate difference rasters
-max_over 8            : use a color ramp with up to 8 different colors
-min_diff 0.1          : map differences between -0.1 and +0.1 to white
-max_diff 0.2          : map differences below -0.2 or above +0.2 to saturated blue or red
-bands 0.1 0.2 0.4     : map absolute differences up to 0.1 to white, 0.2 to blue, 0.4 to green, and above to red 
-color_bands 0.1 0x00..: see long explanation above
-gray                  : map values to gray values instead of colors
-false                 : map values to colors
-values                : do not map values to gray or to color tones but output actual difference and overlap values
-elevation             : check difference in elevation values per overlapping flightline per cell
-intensity             : check difference in intensity values per overlapping flightline per cell
-number_returns        : check difference in number of returns per overlapping flightline per cell
-scan_angle_abs        : check difference in absolute scan angle per overlapping flightline per cell
-density               : check difference in point density per overlapping flightline per cell with an 8 bit counter
-density_16bit         : check difference in point density per overlapping flightline per cell with a 16 bit counter
-density_32bit         : check difference in point density per overlapping flightline per cell with a 32 bit counter
-counter               : check difference in point counts per overlapping flightline per cell with an 8 bit counter
-counter_16bit         : check difference in point counts per overlapping flightline per cell with a 16 bit counter
-counter_32bit         : check difference in point counts per overlapping flightline per cell with a 32 bit counter
-lowest -low -min      : use lowest value per cell to checking difference
-highest -high -max    : use highest value per cell to checking difference
-average -avg -mean    : compute per cell averages before checking difference
-recover_flightlines   : try to recover missing flightline information (for tiles!!!) from GPS time stamps
-recover_flightlines_interval 20 : look for minimum 20 second gaps in GPS time stamp to recover flightlines
-nodata 9999           : use 9999 as the nodata value in the BIL / ASC format
-utm 12T               : use UTM zone 12T to spatially georeference the raster
-sp83 CO_S             : use the NAD83 Colorado South state plane for georeferencing
-sp27 SC_N             : use the NAD27 South Carolina North state plane
-longlat               : geometric coordinates in longitude/latitude order 
-latlong               : geometric coordinates in latitude/longitude order 
-wgs84                 : use the WGS-84 ellipsoid
-wgs72                 : use the WGS-72 ellipsoid
-nad83                 : use the NAD83 ellipsoid
-nad27                 : use the NAD27 ellipsoid
-survey_feet           : use survey feet
-feet                  : use feet
-meter                 : use meter
-elevation_surveyfeet  : use survey feet for elevation
-elevation_feet        : use feet for elevation
-elevation_meter       : use meter for elevation
-tm 609601.22 0.0 meter 33.75 -79 0.99996
-transverse_mercator 1804461.942257 0.0 feet 0.8203047 -2.1089395 0.99996
-lcc 609601.22 0.0 meter 33.75 -79 34.33333 36.16666
-lambert_conic_conformal 1640416.666667 0.0 surveyfeet 47.000000 -120.833333 47.50 48.733333
-ellipsoid 23          : use the WGS-84 ellipsoid (do -ellipsoid -1 for a list)
-ilay                                : apply all LASlayers found in corresponding *.lay file on read
-ilay 3                              : apply first three LASlayers found in corresponding *.lay file on read
-ilaydir E:\my_layers                : look for corresponding *.lay file in directory E:\my_layers

for more info:

C:\software\LAStools\bin>lasoverlap -h
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
  -drop_RGB_red 5000 20000
  -keep_RGB_green 30 100
  -drop_RGB_green 2000 10000
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
  -transform_helmert -199.87,74.79,246.62
  -transform_helmert 598.1,73.7,418.2,0.202,0.045,-2.455,6.7
  -transform_affine 0.9999652,0.903571,171.67,736.26
  -switch_x_y -switch_x_z -switch_y_z
  -clamp_z_below 70.5
  -clamp_z 70.5 72.5
  -copy_attribute_into_z 0
  -add_attribute_to_z 1
  -add_scaled_attribute_to_z 1 -1.2
  -copy_intensity_into_z
  -copy_user_data_into_z
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
  -map_intensity map_file.txt
  -copy_RGB_into_intensity
  -copy_NIR_into_intensity
  -copy_attribute_into_intensity 0
  -bin_gps_time_into_intensity 0.5
Transform scan_angle.
  -set_scan_angle 0.0
  -scale_scan_angle 1.944445
  -translate_scan_angle -5
  -translate_then_scale_scan_angle -0.5 2.1
Change the return number or return count of points.
  -repair_zero_returns
  -set_return_number 1
  -set_extended_return_number 10
  -change_return_number_from_to 2 1
  -change_extended_return_number_from_to 2 8
  -set_number_of_returns 2
  -set_extended_number_of_returns 15
  -change_number_of_returns_from_to 0 2
  -change_extended_number_of_returns_from_to 8 10
Modify the classification.
  -set_classification 2
  -set_extended_classification 41
  -change_classification_from_to 2 4
  -classify_z_below_as -5.0 7
  -classify_z_above_as 70.0 7
  -classify_z_between_as 2.0 5.0 4
  -classify_intensity_above_as 200 9
  -classify_intensity_below_as 30 11
  -classify_intensity_between_as 500 900 15
  -classify_attribute_below_as 0 -5.0 7
  -classify_attribute_above_as 1 70.0 7
  -classify_attribute_between_as 1 2.0 5.0 4
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
  -map_user_data map_file.txt
  -copy_attribute_into_user_data 1
  -add_scaled_attribute_to_user_data 0 10.0
Modify the point source ID.
  -set_point_source 500
  -change_point_source_from_to 1023 1024
  -map_point_source map_file.txt
  -copy_user_data_into_point_source
  -copy_scanner_channel_into_point_source
  -merge_scanner_channel_into_point_source
  -split_scanner_channel_from_point_source
  -bin_Z_into_point_source 200
  -bin_abs_scan_angle_into_point_source 2
  -bin_gps_time_into_point_source 5.0
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
  -switch_RGBI_into_CIR
  -switch_RGB_intensity_into_CIR
Transform attributes in "Extra Bytes".
  -scale_attribute 0 1.5
  -translate_attribute 1 0.2
  -copy_user_data_into_attribute 0
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
Supported Raster Outputs
  -o dtm.asc
  -o dsm.bil
  -o dem.laz
  -o canopy.flt
  -o dtm.dtm
  -o density.xyz
  -o spreadsheet.csv
  -o hillshade.png
  -o slope.tif
  -o false_color.jpg
  -o intensity.img
  -oasc -obil -oflt -oimg -opng -odtm -otif -ojpg -oxyz -olaz -nil
  -odir C:\data\hillshade (specify output directory)
  -odix _small (specify file name appendix)
  -ocut 2 (cut the last two characters from name)
LAStools (by martin@rapidlasso.com) version 190812 (academic)
usage:
lasoverlap -i tile.laz -step 1 -max_diff 0.25
lasoverlap -i tile.las -step 3 -values
lasoverlap -i *.laz -step 4 -max_diff 0.75
lasoverlap -i tiles*.las -step 2 -max_over 7
lasoverlap -i tile1.las tile2.las tile3.las tile4.las -merged -step 2
lasoverlap -i swath1.las swath2.las swath3.las -files_are_flightlines -step 2
lasoverlap -i tiles*.las -merged -counter
lasoverlap -i tiles*.las -files_are_flightlines -intensity -highest
lasoverlap -h

---------------

if you find bugs let me (martin.isenburg@rapidlasso.com) know
