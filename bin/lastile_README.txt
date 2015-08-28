****************************************************************

  lastile:

  tiles a potentially very large amount of LAS points from one
  or many files into square non-overlapping tiles of a specified
  size and save them into LAS or LAZ format. 

  The square tiling used by lastile is chosen for two reasons:

  (a) it is by far the most common way that LAS files are tiled
      for archival or distribution

  (b) it will (eventually) be exploited by our "streaming TIN"
      generation code to seamlessly Delaunay triangulate large
      amounts of tiles in a highly memory-efficient fashion. For
      that purpose, lastile adds a small VLR to the header of
      each generated LAS/LAZ tile that stored its index or its
      "finalization tag" in the square quad tree.

  The tool can either operate in one or in two reading passes
  via a commandline switch (-extra_pass). The additional reading 
  pass is used to collect information about how many points fall 
  into each cell. This allows us to deallocate LASwriters for tiles
  that have seen all their points. This is *only* really needed 
  when writing LASzip compressed output of very large tilings
  to avoid having the LASwriters using LASzip compression for
  all tiles in memory at the same time.

  Optionally the tool can also create a small '-buffer 10' 
  around every tile where the parameter 10 specifies the number
  of units each tile is (temporarily) grown in each direction. It
  is possible to remove the buffer from a tile by running lastile
  across all tiles again but with the '-remove_buffer' option. You
  can also '-flag_as_withheld' or '-flag_as_synthetic' all of the
  buffer points to drop them more easily with the standard filters.

  Optionally the tool can also create an '-reversible' tiling
  that will allow to recreate the original file from all the
  individual tiles. This is useful to, for example, break a
  large LAS file into many tiles with buffers, classify each
  tile individually with lasclassify.exe or compute the height
  of each point with lasheight.exe, and then put the original
  large LAS file back together with '-reverse_tiling'. Note
  that for the unlicensed version the gps_time is set to zero
  and the point are permutated a tiny bit.

  In order to prevent the bounding box in the LAS header from
  being shrunk to the actual extent of the points and set it
  to the full extent of the corresponding tile use '-full_bb'.
  This will pad the tiles to tile size plus buffer when run
  in '-buffer 20' mode. Used together with '-remove_buffer'
  this option results all tiles being set to the full extent
  of each tile after the offset was removed.

  It is also possible to create adaptive tilings. Start with
  the largest desired tile size and add '-refine 10000000' as
  an additional option to the command line. Next call lastile
  again using all the generated tiles from the first call as
  input and instruct lastile to '-refine_tiles 10000000'. You
  may repeat if greater adaptivity is needed.

  To shift the tiling off its standard modulo tile_size tiling
  you can use the '-tile_ll 25 75' option.

  If you run lastile in parallel using '-cores 4' or so it is
  really important that your input data is spatially indexed
  or things will slow down a lot (as each tile requires reading
  the entire input). Make sure you run lasindex to create a LAX
  file for each input file before lastiling on mutiple cores.

  Please license from martin.isenburg@rapidlasso.com to use LAStools
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

>> lastile -i *.las -o tile.las

tiles all points from all files using the default tile size of 1000.

>> lasindex -i *.laz -cores 8
>> lastile -i *.laz -files_are_flightlines -buffer 25 -o tiles\tile.laz -cores 4

spatially indexes all compressed LAZ files and then tiles them on 4
cores using the default tile size of 1000 and a buffer of 25 while
setting the point source ID of each point to the file number it is
from.

>> lastile -i *.las -full_bb -o tile.laz

same but sets the bounding box in the header to the full extend of
all tiles (rather than to the actual extent of its points) and also
compresses the while writing them tiles

>> mkdir tiles
>> mkdir tiles_no_buffer
>> lastile -i *.las -buffer 10 -o tiles\tile.las
>> lastile -i tiles\tile_*.las -remove_buffer -odir tiles_no_buffer -olaz

each tile gets buffer points for 10 units in all directions. also puts
the tiles into directory 'tiles'. the second command removes all buffer
points and writes the tiles compressed to the 'tiles_no_buffer' folder

>> lastile -i large.laz -tile_size 500 -buffer 10 -reversible -o tile.laz 
>> lastile -i tile_*.laz -reverse_tiling -o large_reversed.laz

tiles file 'large.laz' with tile size 500 and buffer 10 in reversible
mode. the second command removes all buffer points, reconstructs the
original point order, and stored the result as 'large_reversed.laz'.

>> mkdir toronto
>> lastile -i *.txt -iparse xyzti -odir toronto -o tile.laz 

same but with on-the-fly converted ASCII input

>> lastile -i in1.las in2.las in3.las -o sydney.laz -tile_size 500

tiles the points from the three LAS files with a tile size of 500. 

>> mkdir outer_banks 
>> lastile -lof obx_files.txt -keep_class 2 3 -tile_size 100 -odir outer_banks -o tile.laz 

tiles all LAS/LAZ files listed in the text file with a tile size
of 100 keeping only points with classification 2 or 3

>> lastile -lof file_list.txt -o tile.laz -extra_pass

tiles all LAS/LAZ files listed in the text file into a LASzip
compressed tiling using the default tile size of 1000 and uses
an extra read pass in an attempt to use less memory.

>> mkdir toronto
>> lastile -i huge.laz -last_only -odir toronto -o tile.laz

tiles the last returns from huge.laz into compressed tiling.

for more info:

C:\lastools\bin>lastile -h
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
LAStools (by martin.isenburg@rapidlasso.com) version 140301 (unlicensed)
usage:
lastile -i *.las -o tile.las
lastile -i *.las -o tiles\tile.laz
lastile -i toronto.las -tile_size 500 -odir tiles -o toronto.las
lastile -v -lof tahoe_files.txt -extra_pass -o tahoe.laz
lastile -v -i file1.laz file2.laz file3.laz -o airport\tile.laz
lastile -lof lidar_files.txt -last_only -tile_size 100 -o tile.las
lastile -i flight1*.laz flight2*.laz -tile_size 250 -buffer 25 -o obx.laz
lastile -i huge.laz -tile_size 250 -buffer 25 -reversible -o tiles\huge.laz
lastile -h

---------------

if you find bugs let me (martin.isenburg@rapidlasso.com) know.
