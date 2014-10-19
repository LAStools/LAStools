::
:: a batch script for converting classified, buffered, LiDAR tiles
:: into a number of raster products with a tile-based multi-core
:: batch pipeline
::

::
:: specify parameters
::

:: allows you to run the script from other folders
set PATH=%PATH%;C:\Users\Public\lastools\bin;

:: here we specify the directory (e.g. the folder) in which the
:: classified, buffered, LiDAR tiles are stored
set INPUT_TILES=C:\lastools_output\classified_buffered_tiles

:: here we specify in which format the original raw flight lines 
:: files are stored in the RAW_FLIGHT_LINES folder
set RAW_FORMAT=las

:: here we specify the directory (e.g. the folder) in which the
:: temporary files are stored
set TEMP_FILES=C:\lastools_temp

:: here we specify the directory (e.g. the folder) in which the
:: resulting output files are stored
set OUTPUT_FILES=C:\lastools_output

:: here we specify the resolution of the output DEMs
set STEP=1.0

:: here we specify the number of cores we want to run on
set NUM_CORES=7

rmdir %TEMP_FILES% /s /q

::
:: start processing
::

:: create raster DTM hillshades (with resolution of STEP meters)

mkdir %OUTPUT_FILES%\tiles_dtm
las2dem -i %INPUT_TILES%\*.laz ^
        -keep_class 2 -extra_pass -step %STEP% -use_tile_bb -hillshade ^
        -odir %OUTPUT_FILES%\tiles_dtm -opng ^
        -cores %NUM_CORES%

:: create raster DSM hillshades (with resolution of STEP meters)

mkdir %OUTPUT_FILES%\tiles_dsm
las2dem -i %INPUT_TILES%\*.laz ^
        -first_only -step %STEP% -use_tile_bb -hillshade ^
        -odir  %OUTPUT_FILES%\tiles_dsm -opng ^
        -cores %NUM_CORES%

:: create height normalized tiles (stores "height above ground" as z coordinate)

mkdir %TEMP_FILES%\height_normalized
lasheight -i %INPUT_TILES%\*.laz ^
          -replace_z ^
          -odir %TEMP_FILES%\height_normalized -olaz ^
          -cores %NUM_CORES%

:: create canopy height raster (with resolution of STEP meters)

lasgrid -i %TEMP_FILES%\height_normalized\*.laz -merged ^
        -highest -step %STEP% -false -set_min_max 0 10 -fill 2 ^
        -o %OUTPUT_FILES%\canopy.png

echo "bye bye"

goto the_end

:the_end
