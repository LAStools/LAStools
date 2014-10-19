::
:: an example batch script for converting raw flight
:: lines into a number of products with a tile-based
:: multi-core batch pipeline
::

set PATH=%PATH%;C:\lastools\bin;

:: create point density grid for entire project

lasgrid -i flightlines\*.laz -merged ^
        -density -step 10 -false -set_min_max 0 200 ^
        -o density.png

:: create buffered tiling

mkdir tiles_raw
lastile -i flightlines\*.laz -files_are_flightlines ^
        -tile_size 1000 -buffer 50 ^
        -o tiles_raw\tile -olaz

:: ground classify all tiles

mkdir tiles_ground
lasground -i tiles_raw\tile*.laz ^
          -extra_fine ^
          -odir tiles_ground -olaz ^
          -cores 7

:: create raster DTM hillshades (2 meters)

mkdir tiles_dtm
las2dem -i tiles_ground\*.laz ^
        -keep_class 2 -extra_pass -step 2 -use_tile_bb -hillshade ^
        -odir tiles_dtm -opng ^
        -cores 7

:: classify vegetation and remove high and low points

mkdir tiles_classified
lasheight -i tiles_ground\*.laz ^
          -drop_below -5 -drop_above 100 ^
          -classify_between 0.5 2 3 -classify_between 2 5 4 -classify_between 5 100 5 ^
          -odir tiles_classified -olaz ^
          -cores 7

:: create raster DSM hillshades (2 meters)

mkdir tiles_dsm
las2dem -i tiles_classified\*.laz ^
        -first_only -step 2 -use_tile_bb -hillshade ^
        -odir tiles_dsm -opng ^
        -cores 7

:: remove buffer and make clean LAZ tiles as product

mkdir tiles_final
lastile -i tiles_classified\*.laz ^
        -remove_buffer -set_user_data 0 ^
        -odir tiles_final -olaz

:: create contours for final tiles

mkdir tiles_contours
las2iso -i tiles_final\*.laz ^
        -keep_class 2 -extra_pass -iso_every 2 -smooth 2 -simplify 1 -clean 5 ^
        -odir tiles_contours -oshp ^
        -cores 7

:: create single raster DTM hillshade (2 meters)

blast2dem -i tiles_final\*.laz -merged ^
          -keep_class 2 -step 2 -hillshade ^
          -o dtm.png

:: create single raster DSM hillshade (2 meters)

blast2dem -i tiles_final\*.laz -merged ^
          -first_only -step 2 -hillshade ^
          -o dsm.png
