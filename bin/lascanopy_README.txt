****************************************************************

  lascanopy:

  This tool reads LiDAR from LAS/LAZ/BIN/SHP/QFIT/ASCII, computes
  popular forestry metrics, and grids them onto a raster. A very
  important parameter is '-step n' that specifies the n x n area
  of LiDAR points that are gridded on one raster (or pixel). The
  default of step is 20. The output can be either in BIL, ASC, IMG,
  TIF, XYZ, FLT, or DTM format. New is raster output in CSV format
  where you can request the '-centroids' to be added. In order to
  shift the raster grid that the points are binned into away from
  the default alignment of (0/0) to (5/15) use '-grid_ll 5 15'.

  If your input files are plots you can use the '-files_are_plots'
  option. Here you can ask for the file '-names' to be added to
  the output CSV file. You can also query a list of circular plots
  from a text file with each line listing: "center_x center_y radius"
  with a command like:

  lascanopy -i forest\*.laz -loc circles.txt -cov -p 50 95 -o plots.csv

  And if you want to override the radius then you can do this by
  adding a fix radius with '-loc_radius 2' or '-loc_radius 7.5'
  to the command above. It is also possible to use text files
  where each line consists of "name center_x center_y radius" as
  the list of circular plots. Then add '-names' before '-loc' to
  handle this correctly.

  You can also query a list of rectangular plots from a text
  file with each line listing: "min_x min_y max_x max_y" with a
  command like:

  lascanopy -i forest\*.laz -lor rectangles.txt -dns -gap -b 50 75 -o stands.csv

  It is also possible to use text files where each line consists
  of "name min_x min_y max_x max_y" as the list of rectangular
  plots. Then add '-names' before '-lor' to handle this correctly.

  You can also load more general polygonal plots from a shapefile with

  lascanopy -i forest\*.laz -lop polygons.shp -int_p 25 50 75 -centroids -o results.csv
  
  If the SHP file with plots has a DBF file with  that contain either an
  integer number or a string attributes for each plot you can add the name
  or the index of the field to the argument as shown below:
  
  -lop polygons.shp plot_ID
  -lop polygons.shp 3

  The tool can concurrently compute a number height percentiles
  ('-p 5 10 25 50 75 90'), the '-min', the '-max', the average
  '-avg', and the standard deviation '-std' of all heights above
  the cutoff that by default is breast height of 1.37. It can be
  changed with the option '-height_cutoff 2.0'. Also the skewness
  with '-ske', the kurtosis with '-kur', and the average square
  height '-qav' can be computed. All these statistical metrics
  only consider the points above the height cutoff. You can compute
  the number of points that actually are above the cutoff and are
  participating in the computation with '-abv' and the total number
  of points (including those that are below the cutoff) with '-all'.

  With the command '-s_upper 95' you can limit the computation of
  these statistics -qav -avg -ske -kur -std to use only the highest
  95 percent of the points above the height cutoff.
  
  There is also the concept of height "bincentiles" where '-b 90'
  would deliver the percentage or fraction of points between the
  height cutoff (aka breast height) and the maximum height. Hence
  the results may not be what you want for '-height_cutoff -2'.
  To utilize all of the points for the bincentile calculation use
  '-height_cutoff 0' and the transform '-clamp_z_below 0.0'. There
  is also option '-b_upper 97' that specifies a certain height
  percentile to be used instead of the maximum height as "upper"
  limit for the bins. This metric is sometimes also referred to
  as "deciles". Request multiple bincentiles with '-b 50 75 90'.

  The tool can also produce the canopy cover using option '-cov'.
  The canopy cover is computed as the number of first returns
  above the cover cutoff divided by the number of all first
  returns and output as a percentage. Similarly, with the option
  '-dns' the canopy density can be produced. The canopy density
  is computed as the number of all points above the cover cutoff
  divided by the number of all returns. By default this cover 
  cutoff is identical to the height cutoff. However, using the
  option '-cover_cutoff 5.0' you can set it to a lower or a
  higher value. As the output default percentages between 0.0%
  and 100.0% are produced. Use option '-fractions' to produce
  fractions between 0.000 and 1.000 instead. 

  It is possible to compute the inverse of the canopy cover and
  canopy density with the '-gap' option which will give you 100%
  (or for fractions 1.0) minus canopy cover or canopy density.

  In addition, the tool can also concurrently produce several
  height count rasters. The option '-c 0.5 2 4 10 50', for example,
  would compute four rasters that count the points whose heights
  are falling into the intervals: [0.5, 2), [2, 4), [4, 10), and
  [10, 50). In the same manner the option '-d 0.5 2 4 10 50' will
  produce a relative height density raster in which the counts are
  divided by the total number of points and scaled to a percentage.
  
  The Vertical Complexity Index (VCI) is also implemented and can
  be computed for different vertical bin sizes with '-vc 1 2 4' or
  '-vci 2.5 5.0'.
  
  The height & intensity metric known as "Height of Median Energy"
  or "HOME" can be computed via the switch '-hom'. All points above
  the height cutoff are ordered by their elevation. Then the height
  is computed at which the sum of intensities of points below and
  the sum of intensities of points above is identical.
  
  Metrics also exist for intensities and '-int_min' and '-int_max'
  do the obvious, just like '-int_avg', '-int_qav', '-int_std',
  '-int_ske' or '-int_kur'. Similarly you can produce intensity
  percentiles with '-int_p 25 50 75' or intensity counts as well
  as densities using the corresponding '-int_c 0 128 256 1024' or
  '-int_d 0 128 256 1024'.

  By default the generated raster spans the extend of the header
  bounding box. You can use the bounding box of the tile with
  '-use_tile_bb' (which only makes sense if the LAS/LAZ file was
  generated using lastile) or the original bounding box in case
  of a buffered tile with '-use_orig_bb' (which only makes sense
  if the input has an on-the-fly buffer aka '-buffered 50'). The
  extend can also be defined by setting '-ll min_x min_y' plus
  '-ncols 512' and '-nrows 512'.

  For the height_cutoff to make sense it is important that the input
  is height normalized, meaning that the z coordinate of each point
  corresponds to the height above ground and not the elevation of
  the point. With 'lasheight -i in.laz -replace_z -o out.laz' you
  can height-normalize a ground-classified LiDAR file.
  
  Depending on the height cutoff or the input file it is possible
  that generated rasters contain only 'no data' values but not a
  single real value. Adding option '-remove_empty_rasters' to the
  command line will delete those files from disk.

  Let me know which other metrics you would like to see ...

  Please license from martin.isenburg@rapidlasso.com before you
  use lascanopy commercially.

  For updates check the website or join the LAStools mailing list.

  http://rapidlasso.com/LAStools
  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/LAStools
  http://facebook.com/LAStools
  http://linkedin.com/groups?gid=4408378

  Martin @rapidlasso
 
****************************************************************

example usage:

>> lascanopy -i *.las -min -max -avg

for each *.las files and for all height above 1.37 it computes
the minimum, maximum, and average value from all points that fall
into cells of size 20 by 20 and stores the resulting grid in ASC
format using the endings '_min.asc', '_max.asc', '_avg.asc'.

>> lascanopy -i lidar*.laz -merged -p 20 40 60 80 -step 10 -o dem.bil

merges the points of all files that match the wildcard lidar*.laz
on-the-fly into one file and computes for all heights above 1.37
the 20th, 40th, 60th, and 80th percentile for 10 by 10 grid cells
and stores the resulting rasters in BIL format using the endings
'_p20.bil', '_p40.bil', '_p60.bil', and '_p80.bil'.

other commandline arguments are

-loc
-lor
-files_are_plots
-use_bb                : raster area specified by bounding box in LAS header
-use_tile_bb           : raster tile without buffer added by lastile
-use_orig_bb           : raster tile without buffer added by on-the-fly buffering
-step 10               : raster with stepsize 10 [default: 20]
-nrows 512             : raster at most 512 rows
-ncols 512             : raster at most 512 columns
-remove_empty_rasters  : remove raster files containing only 'nodata' values
-ll 300000 600000      : start rastering at these lower left x and y coordinates
-nodata 9999           : use 9999 as the nodata value in the BIL / ASC format
-max                   : for each grid cell keep highest value
-min                   : for each grid cell keep lowest value
-avg                   : for each grid cell compute average
-std                   : for each grid cell compute standard deviation
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

for more info:

C:\lastools\bin>lascanopy -h
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
Supported Raster Outputs
  -o dtm.asc
  -o dsm.bil
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
LAStools (by martin@rapidlasso.com) version 140709 (unlicensed)
usage:
lascanopy -i *.las -max -avg -qav -p 50 95
lascanopy -i *.laz -p 1 5 10 25 50 75 90 95 99
lascanopy -i *.laz -c 2.0 5.0 10.0 20.0 -cov -dns -otif
lascanopy -i *.laz -merged -max -avg -int_p 5 50 95 -o merged.dtm
lascanopy -i *.laz -d 2.0 10.0 20.0 40.0 -cov -height_cutoff 2.0
lascanopy -i *.laz -int_p 25 50 75 -dns -gap -fractions -otif
lascanopy -i *.las -files_are_plots -int_avg -int_std -cov -gap
lascanopy -i *.laz -p 25 50 75 95 -loc list_of_circles.txt
lascanopy -i *.laz -d 2.0 4.0 6.0 8.0 -fractions -lor list_of_rectangles.txt
lascanopy -i *.laz -p 95 -int_p 95 -cov -fractions -lop list_of_polygons.shp
lascanopy -i 2014_07.laz -ll 470000 5550000 -step 10 -ncols 500 -nrows 200 -cov -p 50 95
lascanopy -h

---------------

if you find bugs let me (martin@rapidlasso.com) know
