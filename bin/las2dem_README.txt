****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

  las2dem:

  This tool reads LIDAR points from the LAS/LAZ format (or some
  ASCII format), triangulates them temporarily into a TIN, and
  then rasters the TIN onto a DEM. The tool can either rather
  the '-elevation', the '-slope', the '-intensity', the '-rgb'
  values, or a '-hillshade' or '-gray' or '-false' coloring. The
  output is either in BIL, ASC, IMG, FLT, XYZ, DTM, TIF, PNG or
  JPG format. Additional attributes that some LAS/LAZ files can
  store as "Extra Bytes" can be rasterized with '-attribute 0'
  or '-attribute 1' or '-attribute 2' ...

  For BIL, ASC, IMG, DTM, and XYZ output one typically stores
  the actual '-elevation' or '-intensity' values whereas the
  TIF, PNG, and JPG formats are usually used for a '-hillshade',
  '-gray', or '-false' coloring, or for the '-rgb' raster. The
  particular range of values to be color mapped can be clamped
  using '-set_min_max 10 100' or can be set '-compute_min_max'.
  The color ramps can be inverted with '-invert_ramp'.

  An interesting new option rasterizes the length of the longest
  or shortest edge around every vertex, which is useful for point
  spacing analysis across the surveyed area. Use '-edge_longest'
  or '-edge_shortest' to enable these options.

  If you use filters such as '-last_only' or '-keep_class 2' you
  may use the '-extra_pass' option to first determine how many
  points get triangulated. This saves memory.

  Closed breaklines can be supplied for hydro-enforcment of
  lakes, for example ('-lakes lakes.shp', '-lakes hydro.txt')
  but they must form proper closed polygons and have elevations.

  Hard breaklines can be integrated for improving the TIN before
  it is sampled with ('-creeks roads.shp', '-creeks creeks.txt')
  and while they can be open they must also have elevations.

  By default the generated raster is sized based on the extend
  of the bounding box. If the LAS/LAZ file was generated using
  lastile, its extend can be set to that of the tile using the
  '-use_tile_bb' option. Any "border buffer" that the tile may
  have had is then not rastered. This allows to avoid boundary
  artifacts and yet create matching tiles in parallel. The exact
  raster extend can also be defined by setting '-ll min_x min_y'
  together with '-ncols 512' and '-nrows 512'.    

  By default triangles whose edges are longer than 100 meters are
  not rasterized. This value can be changed with '-kill 200'. The
  value is always assumed to be meters and will be multipled with
  3.28 for LAS/LAZ files where x and y are known to be in feet.

  Automatically a KML file is generated to allow the resulting
  DEM to be displayed inside Google Earth (for TIF/PNG/JPG). In
  case the LAS/LAZ file contains projection information (i.e. a
  VLR with geokeys) this is used for georeferencing the KML file.
  It is also possible to provide the georeferencing information
  in the command-line.

  Please license from info@rapidlasso.de to use las2dem
  commercially.

  For updates check the website or join the LAStools mailing list.

  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @lastools
 
****************************************************************

example with LAZ file from the .\lastools\data folder:

>> las2dem -i ..\data\TO_core_last_zoom.laz -o dem.asc -v
>> las2dem -i ..\data\TO_core_last_zoom.laz -o dem.bil -v -step 0.5
>> las2dem -i ..\data\TO_core_last_zoom.laz -o dem.bil -v -nbits 16
>> las2dem -i ..\data\TO_core_last_zoom.laz -o dem.asc -v -intensity
>> las2dem -i ..\data\TO_core_last_zoom.laz -o dem.bil -v -intensity -step 2.0

>> las2dem -i ..\data\TO_core_last_zoom.laz -o dem.png -utm 17T -v -hillshade
>> las2dem -i ..\data\TO_core_last_zoom.laz -o dem.png -utm 17T -v -hillshade -light 0 0 1
>> las2dem -i ..\data\TO_core_last_zoom.laz -o dem.png -utm 17T -v -gray
>> las2dem -i ..\data\TO_core_last_zoom.laz -o dem.png -utm 17T -v -false
>> las2dem -i ..\data\TO_core_last_zoom.laz -o dem.png -utm 17T -v -set_min_max 46.83 90 -gray 
>> las2dem -i ..\data\TO_core_last_zoom.laz -o dem.png -utm 17T -v -set_min_max 46.83 90 -false 
>> las2dem -i ..\data\TO_core_last_zoom.laz -o dem.png -utm 17T -intensity -set_min_max 0 1000 -gray 
>> las2dem -i ..\data\TO_core_last_zoom.laz -o dem.png -utm 17T -intensity -set_min_max 0 1000 -false

>> las2dem -i ..\data\TO_core_last_zoom.laz -o dem.png -v -hillshade -nrows 200 -ncols 100
>> las2dem -i ..\data\TO_core_last_zoom.laz -o dem.png -v -hillshade -nrows 200 -ncols 100 -ll 630300 4834550

example usage:

>> las2dem -i *.las -oasc

rasters the elevations of all LAS files *.las with step size 1 and
stores the resulting DEM in ASC format.

>> las2dem -i *.laz -opng -utm 17T -step 2.5 -hillshade

rasters the hillside-shaded elevations of all LAZ files *.laz with
step size 2.5 and stores the resulting DEM in PNG format and with
it a KML file that geo-references each PNG in GE with UTM zone 17T.

>> las2dem -i *.txt -iparse xyzi -obil -step 2 -intensity

rasters the intensities of all ASCII files *.txt with step size 2
and stores the resulting DEM in BIL format with 16 bit precision.

>> las2dem -i lidar.las -o dem.asc -step 2

creates a temporary TIN from all points in the LAS file 'lidar.las',
rasters the elevation values of each TIN facet onto a grid with step
size 2, and stores the resulting DEM in ASC format.

>> las2dem -i lidar.txt -iparse ssxysi-o dem.bil -step 0.5 -intensity

creates a temporary TIN from all points in the ASCII file 'lidar.txt'
using the 3rd and 4th line entry as the x and y coordinate and the 5th
as the intensity, rasters the intensity values of each TIN facet onto
a grid with step size 0.5, and stores the resulting DEM in BIL format
with 16 bit precision.

>> las2dem -lof lidar_files.txt -merged -o dem.bil -last_only

creates a temporary TIN from all last returns of all files listed in
the text file 'lidar_files.txt', rasters the elevation values of
each TIN facet onto a grid with step size 1 and stores the resulting
DEM in BIL format with 32 bit floating-point precision.

>> las2dem -i lidar.las lidar2.las lidar3.las -merged -hillshade -o dem.png -step 5 -keep_class 2

creates a temporary TIN from the merged ground points (i.e. points
with classification 2) of the 3 LAS files 'lidar1.las', 'lidar2.las',
and 'lidar3.las', rasters hillside-shaded TIN facets onto a grid with
step size 5, and stores the resulting grid in PNG format with 8 bit
per pixel.

>> las2dem -i lidar1.txt -i lidar2.txt -iparse xyz -o dem.jpg -hillshade -last_only

creates a temporary TIN from the last returns of the two ASCII files
'lidar1.txt' and 'lidar2.txt' using the 1st, 2nd, and 3rd, entry on
each line as the x, y, and z coordinate, rasters hillside-shaded TIN
facets onto a grid with step size 5, and stores the resulting grid
in JPG format with 8 bit per pixel.

>> las2dem -i lidar.las -o dem.tif -first_only -gray -step 2

creates a temporary TIN from all first returns in the LAS file
'lidar.las', rasters the elevation values of each TIN facet with
gray-scale elevation coloring onto a grid with step size 2, and
stores the resulting grid in TIF format with 8 bit per pixel.

>> las2dem -i lidar.las -o dem.png -first_only -false -step 2 -utm 14T

same as above but with false elevation coloring and output of a KML
file that georeferences the PNG file in Google Earth

try the following commands for generating some interesting georeferenced
DEMs that you can look at in Google Earth by double clicking the automatically
generated KML file

>> las2dem -i ..\data\test.las -false -intensity -o test.png
>> las2dem -i ..\data\TO_core_last_zoom.las -hillshade -o toronto.png -utm 17T
>> las2dem -i ..\data\SerpentMound.las -hillshade -o SerpentMound.png

other commandline arguments are

-kill 50              : do not raster triangles with edges longer than 50 meters
-step 2               : raster with stepsize 2 (the default is 1)
-nrows 512            : raster at most 512 rows
-ncols 512            : raster at most 512 columns
-ll 300000 600000     : start rastering at these lower left x and y coordinates
-nodata -9999         : use -9999 as the nodata value in the BIL/ASC format
-hillshade            : color the image with hillside shading
-intensity            : use intensity values
-rgb                  : use rgb values if available (only used with PNG/TIF/JPG)
-gray                 : gray-scale based on elevation/intensity (used with PNG/TIF/JPG)
-false                : false-color based on elevation/intensity (used with PNG/TIF/JPG)
-set_min_max          : sets min & max range for -gray and -false
-compute_min_max      : computes the range for -gray and -false
-scale 2.0            : multiply all elevation/intensity values by 2.0 before rastering
-nbits 32             : use 32 bits to represent the elevation (mainly used with BIL format)
-light 1 1 3          : change the direction of the light vector for hillside shading
-utm 12T              : use UTM zone 12T to spatially georeference the raster
-sp83 CO_S            : use the NAD83 Colorado South state plane for georeferencing
-sp27 SC_N            : use the NAD27 South Carolina North state plane
-longlat              : geometric coordinates in longitude/latitude order 
-latlong              : geometric coordinates in latitude/longitude order 
-wgs84                : use the WGS-84 ellipsoid
-wgs72                : use the WGS-72 ellipsoid
-nad83                : use the NAD83 ellipsoid
-nad27                : use the NAD27 ellipsoid
-survey_feet          : use survey feet
-feet                 : use feet
-meter                : use meter
-elevation_surveyfeet : use survey feet for elevation
-elevation_feet       : use feet for elevation
-elevation_meter      : use meter for elevation
-tiling_ns crater 500 : create a tiling of DEMs named crater with tiles of size 500 
-tm 609601.22 0.0 meter 33.75 -79 0.99996
-transverse_mercator 1804461.942257 0.0 feet 0.8203047 -2.1089395 0.99996
-lcc 609601.22 0.0 meter 33.75 -79 34.33333 36.16666
-lambert_conic_conformal 1640416.666667 0.0 surveyfeet 47.000000 -120.833333 47.50 48.733333
-ellipsoid 23         : use the WGS-84 ellipsoid (do -ellipsoid -1 for a list of ellipsoids)

for more info:

C:\lastools\bin>las2dem -h
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
Supported Raster Outputs
  -o dtm.asc
  -o dsm.bil
  -o canopy.flt
  -o dtm.dtm
  -o density.xyz
  -o intensity.img
  -o hillshade.png
  -o slope.tif
  -o false_color.jpg
  -oasc -obil -oflt -oimg -opng -odtm -otif -ojpg -oxyz -nil
  -odir C:\data\hillshade (specify output directory)
  -odix _small (specify file name appendix)
  -ocut 2 (cut the last two characters from name)
LAStools (by info@rapidlasso.de) version 140301 (unlicensed)
usage:
las2dem -i lidar.las -o lidar.asc
las2dem -i lidar.las -o lidar.bil -intensity -kill 50
las2dem -i *.las -step 2.0 -opng -hillshade
las2dem -i lidar.las -o lidar.png -utm 11S -false
las2dem -i lidar.las -o lidar.png -sp83 TX_N -intensity -false -set_min_max 10 50
las2dem -i lidar.las -ll 640000 4320000 -ncols 400 -nrows 400 -o lidar.jpg -gray
las2dem -i lidar.las -keep_class 2 -o dem.png -sp27 PA_N -last_only -gray
las2dem -i lidar.las -keep_class 8 3 -o dem.tif -step 2.0 -intensity
las2dem -h

---------------

if you find bugs let me (info@rapidlasso.de) know

---------------

Please note that this software does not work in streaming mode and is therefore
not suited for large LAS files beyond 20 million points. Use the BLAST extension
(aka blast2dem) which work efficiently out-of-core and can process up to 2 billion
points of LAS/LAZ data into a seamless DEM (optionally tiled on output).
