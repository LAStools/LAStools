****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************


  lastile:

  tiles a potentially very large amount of LAS points from one
  or many files into square non-overlapping tiles of a specified
  size and save them into LAS or LAZ format. 

  The square tiling used by lastile is chosen for two reasons:

  (a) it is by far the most common way that LAS files are tiled
      for archival or distribution

  (b) it can (potentially) be exploited by our "streaming TIN"
      code for seamless & memory-efficient Delaunay triangulation
      of large amounts of tiles.

  A small VLR added to the header of each generated LAS/LAZ tile
  stores the tile index in the square quad-tree from which its
  min/max extend can be computed. The VLR also tells LAStools
  whether a tile has buffers. Why are buffers important? See:

  http://rapidlasso.com/2015/08/07/use-buffers-when-processing-lidar-in-tiles/

  The tool can either operate in one or in two reading passes
  via a commandline switch ('-extra_pass'). The additional reading 
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
  and the point are permutated a tiny bit. Do *not* use this
  option unless you are *really* sure you need it. 

  In order to prevent the bounding box in the LAS header from
  being shrunk to the actual extent of the points and set it
  to the full extent of the corresponding tile use '-full_bb'.
  This will pad the tiles to tile size plus buffer when run
  in '-buffer 20' mode. Used together with '-remove_buffer'
  this option results all tiles being set to the full extent
  of each tile after the offset was removed.

  It is also possible to create adaptive tilings. Start with
  the largest desired tile size and add '-refine 10000000' or
  '-refine_tiling 10000000' as an option to the command line.
  Next call lastile again using all the just generated tiles as
  input and instruct lastile to '-refine_tiles 10000000'. You
  may repeat if greater adaptivity is needed. This is especially
  useful for surveys with great density variation, like mobile,
  terrestrial, and UAV scans. Here a small example:
  
  lastile -i mobile_scan/strip0*.laz ^
          -tile_size 1024 -buffer 5 ^
		  -refine_tiling 10000000 ^
		  -odir tiles_raw -o singapore.laz		  

  lastile -i tiles_raw/singapore*_1024.laz ^
		  -refine_tiling 10000000 ^
		  -olaz ^
		  -cores 4

  lastile -i tiles_raw/singapore*_512.laz ^
		  -refine_tiling 10000000 ^
		  -olaz ^
		  -cores 4

  lastile -i tiles_raw/singapore*_256.laz ^
		  -refine_tiling 10000000 ^
		  -olaz ^
		  -cores 4		  

  lastile -i tiles_raw/singapore*_128.laz ^
		  -refine_tiling 10000000 ^
		  -olaz ^
		  -cores 4

  By default a tile gets deleted after it was refined into four
  smaller tiles. Add '-dont_delete_refined' to the command line
  to keep the original tiles around.

  To shift the tiling off its standard modulo tile_size tiling
  you can use the '-tile_ll 25 75' option.

  If you run lastile in parallel using '-cores 4' or so it is
  *REALLY* important that your input data is spatially indexed
  or things will slow down a lot (as each tile requires reading
  the entire input). Make sure you run lasindex to create a LAX
  file for each input file before lastiling on mutiple cores.

  For those who have user-defined tilings to deliver there is also
  the option '-external_tiling tiles_utm_600m.shp TNAME' that expects
  a SHP files with rectangular tiles with a corresponding string
  attribute called TNAME in the DBF file. 

  Please license from info@rapidlasso.de to use LAStools
  commercially.

  For updates check the website or join the LAStools google group.
  
  https://rapidlasso.de/
  http://groups.google.com/group/lastools/


see also:
  lassplit - Merge or split lidar data files by number of points


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


-extra_pass : do extra read pass to count points (only makes sense when filtering)
-overview : unused
-flag_as_withheld : flag buffer points as withheld
-flag_as_synthetic : flag buffer points as synthetic
-refine : refine a former tiling
-refine_tiling : refine a former tiling
-refine_tiles : refine a former tiling
-unindexed : force processing even if input is not indexed
-kdtree : use tree structure for fast overlap checks
-external_tiling tiles.shp [n]: use external tile info out of the given shape with DBF attribute [n]
-reversible : allow reverse tiling after processing the tiles
-reverse_tiling : recreate the original file by reverse tiling
-full_bb : prevent the bounding box from being shrunk to the actual extent of the points
-remove_buffer : set size to the full extent of each tile after removing offset
-single_tile [n] : generate just tile with index [n]
-single_tile_bb [n] min_x min_y max_x max_y : generate tile with index [n] and the given bounding box
-external_tile : generate one external defined tile with the given bounding box
-dont_delete_refined : keep original tiles around tile refinement to 4 smaller tiles
-tile_ll [m] [n] : shift tiling off its standard modulo tile_size tiling

  
