****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

  lasgrid:

  This tool reads LIDAR from LAS/LAZ/ASCII and grids them onto
  a raster. The most important parameter '-step n' specifies the
  n x n area that of LiDAR points that are gridded on one raster
  (or pixel). The output is either in BIL, ASC, IMG, TIF, PNG,
  JPG, XYZ, CSV, or DTM format. The tool can raster '-elevation'
  or '-intensity' of each point and can compute the '-lowest' or
  the '-highest', the '-average', or the standard deviation
  '-stddev', as well as the '-range'.
 
  Other gridding options are '-scan_angle_abs', '-scan_angle',
  '-point_density', '-point_density_16bit', '-point_density_32bit', 
  '-counter', '-counter_16bit', '-counter_32bit', '-user_data',
  '-point_source', '-rgb', '-number_returns' and more. See the
  end for a complete list. Additional attributes that some LAS
  or LAZ files sometimes store as "Extra Bytes" can be gridded
  with '-attribute 0' or '-attribute 1' or '-attribute 2' ...

  Sometimes vendors will cut off clouds or haze in the data in
  airborne surveys before delivering the data. Often the return
  below the clouds will usually be of lower quality. Areas with
  missing first returns can be found by gridding '-return_type'
  with option '-highest'. Areas with missing last returns can be
  found by gridding '-return_type' with option '-lowest'. Using
  a '-false' coloring makes it easy to spot afected areas. 

  This tool can read BILLIONS of points very efficiently. By
  default it uses only 1000MB of main memory. You can increase
  this with the '-mem 2000' option to up to 2 GB. The tool
  pages larger rasters out to disk. If you have a second hard
  drive it is beneficial to use this instead. You can specify
  the temporary file location with '-temp_files E:\temp\temp'.

  For BIL, ASC, IMG, DTM, and XYZ output one typically stores
  the actual (elevation, intensity, ...) values whereas for TIF,
  PNG, and JPG one usually chooses to express the variation with
  '-gray' or with '-false' colors for simple visualizion. Here
  the variation can be limited with '-set_min_max 10 100' to a
  particular range or it can be set to '-compute_min_max'. The
  color scheme can also be inverted with '-invert_ramp'

  Optionally, a KML file is generated that allows the resulting
  raster  to be immediately displayed inside a geospatial context
  provided by Google Earth (for TIF/PNG/JPG images). In case the
  LAS/LAZ file contains projection information (i.e. geo keys
  as variable length records) this metadata is used to correctly
  geo-reference the KML file. It is also possible to provide the
  proper geo-referencing information in the command-line.

  By default the generated raster spans the extend of all LiDAR
  points. It is possible to specify this to be identical to the
  bounding box with '-use_bb' or the bounding box of the tile
  with '-use_tile_bb' (the latter only if the LAS/LAZ file was
  generated using lastile). The extend can also be defined by
  setting '-ll min_x min_y' plus '-ncols 512' and '-nrows 512'.

  Use '-subsample n' with n > 1 to anti-alias "hard" gridding of
  LiDAR points by their x and y coordinate into disjunct rasters.
  The option '-subsample 3' adds each LiDAR point 9 times to the
  raster at locations (x/y), (x+0.33*step/y), (x+0.66*step/y),
  (x/y+0.33*step) (x+0.33*step/y+0.33*step) (x+0.66*step/y+0.33*step),
  (x/y+0.66*step) (x+0.33*step/y+0.66*step) (x+0.66*step/y+0.66*step)
  and thereby "washes out" hard boundaries. Obviously, this will
  lead to wrongful increase in the '-counter' counters, but the
  '-averages', '-highest', '-lowest', and '-stddev' will have less
  aliasing

  It is also possible to "thicken" your points as you thin them
  to simulate a diameter for the laser beam. The '-subcircle 0.1'
  option will replicate each point 8 times in a discrete circle
  with radius 0.1 around every original input point. This makes
  sense in combination with '-highest' in order to create a nice
  set of points for subsequent CHM or DSM construction. By adding
  a second value '-subcircle 0.2 -0.05' you can lower of raise the
  z value of the 8 points on the discrete circle by the specified
  amount, here they would be 0.05 units lower than the original,
  which might be useful for subsequent tree top detection.

  Please license from info@rapidlasso.de to use lasgrid
  commercially.

  For updates check the website or join the LAStools mailing list.

  https://rapidlasso.de/LAStools
  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/LAStools
  http://facebook.com/LAStools
  http://linkedin.com/groups?gid=4408378

  Martin @rapidlasso
 
****************************************************************

example command lines with this LAZ file:
http://www.cs.unc.edu/~isenburg/lastools/download/test/s1885565.laz

lasgrid -v -sp83 OH_S -feet -i s1885565.laz -step 10 -gray -o elev_low.png
lasgrid -v -sp83 OH_S -feet -i s1885565.laz -step 10 -gray -o elev_high.png -highest
lasgrid -v -sp83 OH_S -feet -i s1885565.laz -step 10 -gray -o elev_std.png -stddev
lasgrid -v -sp83 OH_S -feet -i s1885565.laz -step 10 -gray -keep_class 2 -o elev_grnd_low.png
lasgrid -v -sp83 OH_S -feet -i s1885565.laz -step 10 -gray -keep_class 2 -o elev_grnd_low_fill.png -fill 5
lasgrid -v -sp83 OH_S -feet -i s1885565.laz -step 10 -gray -keep_class 2 -o elev_grnd_std_fill.png -stddev -fill 5

lasgrid -v -sp83 OH_S -feet -i s1885565.laz -step 10 -false -o elev_f_low.png
lasgrid -v -sp83 OH_S -feet -i s1885565.laz -step 10 -false -o elev_f_high.png -highest
lasgrid -v -sp83 OH_S -feet -i s1885565.laz -step 10 -false -o elev_f_std.png -stddev
lasgrid -v -sp83 OH_S -feet -i s1885565.laz -step 10 -false -keep_class 2 -o elev_f_grnd_low.png
lasgrid -v -sp83 OH_S -feet -i s1885565.laz -step 10 -false -keep_class 2 -o elev_f_grnd_low_fill.png -fill 5
lasgrid -v -sp83 OH_S -feet -i s1885565.laz -step 10 -false -keep_class 2 -o elev_f_grnd_std_fill.png -std -fill 5

lasgrid -v -sp83 OH_S -feet -i s1885565.laz -step 10 -gray -intensity -o int_low.png
lasgrid -v -sp83 OH_S -feet -i s1885565.laz -step 10 -gray -intensity -o int_high.png -highest
lasgrid -v -sp83 OH_S -feet -i s1885565.laz -step 10 -gray -intensity -o int_avg.png -average
lasgrid -v -sp83 OH_S -feet -i s1885565.laz -step 10 -gray -intensity -o int_std.png -stddev

lasgrid -v -sp83 OH_S -feet -i s1885565.laz -step 10 -false -intensity -o int_f_low.png
lasgrid -v -sp83 OH_S -feet -i s1885565.laz -step 10 -false -intensity -o int_f_high.png -highest
lasgrid -v -sp83 OH_S -feet -i s1885565.laz -step 10 -false -intensity -o int_f_avg.png -average
lasgrid -v -sp83 OH_S -feet -i s1885565.laz -step 10 -false -intensity -o int_f_std.png -stddev

example command lines with data from http://liblas.org/samples

lasgrid -v -o result.png -false -i line_27007_dd.las -lonlat -step 0.00002 -stddev
lasgrid -v -o result.png -false -i IowaDNR-CloudPeakSoft-1.0-UTM15N.las 
lasgrid -v -o result.png -false -i LAS12_Sample_withIntensity_Quick_Terrain_Modeler.las -step 2 -stddev
lasgrid -v -o result.png -false -i LAS12_Sample_withRGB_Quick_Terrain_Modeler.las -high
lasgrid -v -o result.png -false -i Lincoln.las -utm 14T -step 5
lasgrid -v -o result.png -false -i S1C1_strip021.las -set_min_max 1630 1690 -step 2 -high
lasgrid -v -o result.png -false -i "Serpent Mound Model LAS Data.las" -intensity -set_min_max 0 400
lasgrid -v -o result.png -false -i USACE_Merrick_lots_of_VLRs.las -step 10 -intensity

example usage:

>> lasgrid -i *.las -opng -step 5 -false -sp83 OH_N

rasters for each *.las files the lowest elevation of all points
that fall into cells of size 5 by 5, stores the resulting grid
in PNG format using false coloring, and creates a KML file that
maps the PNG to state plane NAD83 of Northern Ohio.

>> lasgrid -i *.txt -iparse xyz -oasc -step 2 -highest

rasters for each *.txt files the highest elevation of all points
that fall into cells of size 2 by 2 and stores the resulting grids
in ASC format.

>> lasgrid -i lidar1.las lidar2.las lidar3.las -merged -o dem.bil -step 4 -highest -intensity

merges the points of lidar1.las lidar2.las lidar3.las and rasters
the highest intensity of all points that fall into cells of size 4
by 4 and stores the resulting grid in BIL format.

>> lasgrid -v -i lidar.las -o dem.png -step 5 -false -stddev -utm 14T

rasters the standard deviations of the elevation of all points that
fall into cells of size 5 by 5 and stores the resulting grid in PNG
format using false coloring and creates a KML file that maps the file
to UTM zone 14

>> lasgrid -v -i lidar.las -o dem.jpg -last_only -false -highest -step 2

rasters the highest elevation from all points that fall into cells of
size 2 by 2 units and are classfied as last returns and stores the
resulting grid in JPG format using false elevation coloring

>> lasgrid -v -i lidar.las -o dem.tif -keep_class 2 -keep_class 3 -gray

rasters the lowest elevation from all points that fall into cells of
size 1 by 1 unit and are classfied as 2 or 3 and stores the resulting
grid in TIF format using gray-scale elevation coloring

>> lasgrid -v -i lidar.las -o dem.asc -step 2 -average

rasters the average elevations from all points that fall into cells of
size 2 by 2 units and stores the resulting grid in ASC format.

>> lasgrid -v -lof lidar_files.txt -merged -o merged.bil -step 10

rasters the lowest elevation from all points of all files listed in
lidar_files.txt that fall into cells of size 10 by 10 units and stores
the resulting grid in BIL format with 32 bits floats.

>> lasgrid -v -lof lidar_files.txt -obil -step 10

rasters the lowest elevation for each file listed in lidar_files.txt
individually that fall into cells of size 10 by 10 units and stores
each resulting grid in BIL format with 32 bits floats.

the following commands generate some interesting georeferenced grids that
you can look at in Google Earth by double clicking the generated KML file

>> lasgrid -i ..\data\test.las -false -o test.png
>> lasgrid -i ..\data\TO_core_last_zoom.las -gray -o toronto.png -utm 17T
>> lasgrid -i ..\data\SerpentMound.las -false -o SerpentMound.png

overview of all tool-specific switches:

-v                                   : more info reported in console
-vv                                  : even more info reported in console
-quiet                               : nothing reported in console
-wait                                : wait for <ENTER> in the console at end of process
-version                             : reports this tool's version number
-fail                                : fail if license expired or invalid
-gui                                 : start with files loaded into GUI
-cores 4                             : process multiple inputs on 4 cores in parallel
-mem                   : amount of main memory to use in MB (500 - 2000) [default: 1500]
-temp_files            : base file name for temp files (example: E:\tmp)
-step 2                : raster with stepsize 2 [default: 1]
-fill 5                : fills voids in the grid with a square search radius of 5 
-subcircle 0.2         : each point is "splatted" with a circle of extra 8 points at radius 0.2
-subcircle 0.2 -0.05   : each point is "splatted" with a circle of extra 8 points at radius 0.2 but 0.05 lower
-use_bb                : raster full extend of bounding box
-use_tile_bb           : only raster extend of tile bounding box (for tiles generated with lastile)
-use_orig_bb           : only raster extend of original bounding box (for tiles generated with '-buffered 30')
-nbits 16              : use 16 bits to represent the elevation (mainly used with BIL format)
-nrows 256             : raster at most 256 rows (starting from the lower left)
-ncols 512             : raster at most 512 columns (starting from the lower left)
-ll 300000 600000      : start rastering at these lower left x and y coordinates
-nodata 9999           : use 9999 as the nodata value in the BIL / ASC format
-elevation             : use elevation values
-intensity             : use intensity values
-highest -high -max    : for each grid cell keep highest value
-lowest -low -min      : for each grid cell keep lowest value
-average -avg -mean    : for each grid cell compute average
-stddev -std           : for each grid cell compute standard deviation
-counter               : counts points per cell with an 8 bit counter
-counter_16bit         : counts points per cell with a 16 bit counter
-counter_32bit         : counts points per cell with a 32 bit counter
-point_density         : computes area-normalized point densities with an 8 bit counter
-point_density_16bit   : computes area-normalized point densities with a 16 bit counter
-point_density_32bit   : computes area-normalized point densities with a 32 bit counter
-scan_angle_lowest     : for each grid cell keep lowest scan angle value 
-scan_angle_highest    : for each grid cell keep highest scan angle value 
-scan_angle_abs_lowest : for each grid cell keep lowest absolute scan angle value 
-scan_angle_abs_highest: for each grid cell keep highest absolute scan angle value 
-user_data_lowest      : for each grid cell keep lowest user data value 
-user_data_highest     : for each grid cell keep highest user data value 
-point_source_lowest   : for each grid cell keep lowest point source value 
-point_source_highest  : for each grid cell keep highest point source value 
-gray                  : gray-scale based on min/max range (used with PNG/TIF/JPG)
-false                 : false-color based on min/max range (used with PNG/TIF/JPG)
-set_min_max           : sets min & max range for -gray and -false
-compute_min_max       : computes the range for -gray and -false
-nbits 16              : use 16 bits to represent the elevation (mainly used with BIL format)
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
-tiling_ns crater 500  : create a tiling of DEMs named crater with tiles of size 500 
-tm 609601.22 0.0 meter 33.75 -79 0.99996
-transverse_mercator 1804461.942257 0.0 feet 0.8203047 -2.1089395 0.99996
-lcc 609601.22 0.0 meter 33.75 -79 34.33333 36.16666
-lambert_conic_conformal 1640416.666667 0.0 surveyfeet 47.000000 -120.833333 47.50 48.733333
-ellipsoid 23          : use the WGS-84 ellipsoid (do -ellipsoid -1 for a list)
-ilay                                : apply all LASlayers found in corresponding *.lay file on read
-ilay 3                              : apply first three LASlayers found in corresponding *.lay file on read
-ilaydir E:\my_layers                : look for corresponding *.lay file in directory E:\my_layers

for more info:

E:\LAStools\bin>lasgrid -h
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
  -keep_first -first_only -drop_first
  -keep_last -last_only -drop_last
  -keep_first_of_many -keep_last_of_many
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
  -keep_every_nth 2
  -keep_random_fraction 0.1
  -thin_with_grid 1.0
  -thin_with_time 0.001
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
  -change_extended_classification_from_to 6 46
  -move_ancient_to_extended_classification
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
  -copy_R_into_NIR -copy_G_into_NIR -copy_B_into_NIR
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
  -o intensity.img
  -o hillshade.png
  -o slope.tif
  -o false_color.jpg
  -oasc -obil -oflt -oimg -opng -odtm -otif -ojpg -oxyz -nil
  -odir C:\data\hillshade (specify output directory)
  -odix _small (specify file name appendix)
  -ocut 2 (cut the last two characters from name)
LAStools (by info@rapidlasso.de) version 161023 (commercial)
Supported raster operations
  -elevation_lowest (default)
  -elevation_highest
  -elevation_range
  -elevation_average
  -elevation_stddev
  -intensity_lowest
  -intensity_highest
  -intensity_average
  -intensity_range
  -intensity_stddev
  -number_returns_lowest
  -number_returns_highest
  -number_returns_average
  -number_returns_stddev
  -return_type_lowest
  -return_type_highest
  -occupancy
  -counter
  -counter_16bit
  -counter_32bit
  -classification_variety
  -extended_classification_variety
  -scan_angle_lowest
  -scan_angle_highest
  -scan_angle_range
  -scan_angle_abs_lowest
  -scan_angle_abs_highest
  -scan_angle_abs_average
  -user_data_lowest
  -user_data_highest
  -user_data_range
  -point_source_lowest
  -point_source_highest
  -point_source_range
  -rgb
usage:
lasgrid -i *.las -opng -step 5 -false
lasgrid -i in.las -o dtm.asc -mem 1000
lasgrid -i in.las -o chm.png -false -subcircle 0.5 -set_min_max 0 30
lasgrid -i in.laz -o dsm.img -elevation_highest -mem 1900 -temp_files E:\tmp
lasgrid -i in.laz -o out.png -elevation_stddev -false -step 5
lasgrid -i in.las -o intensity.asc -intensity_lowest -step 2 -temp_files E:\tmp
lasgrid -i in.laz -o out.png -scan_angle_abs_lowest -gray -step 2
lasgrid -i in.las -o map.asc -occupancy -step 0.5
lasgrid -i in.laz -o ortho.png -rgb -temp_files E:\tmp
lasgrid -i in.las -o counter.tif -counter -false -step 2
lasgrid -i in.laz -o counter.bil -counter_16bit -step 5
lasgrid -i in.laz -o out.asc -classification_majority -step 1
lasgrid -h

---------------

if you find bugs let me (info@rapidlasso.de) know
