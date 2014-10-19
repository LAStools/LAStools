::
:: a batch script for converting raw flight lines (not tiled) into
:: a number of products with a tile-based multi-core batch pipeline
::

::
:: specify parameters
::

:: allows you to run the script from other folders
set PATH=%PATH%;C:\Users\Public\lastools\bin;

:: here we specify the directory (e.g. the folder) in which the
:: original raw flight lines files are stored
set RAW_FLIGHT_LINES=E:\Raw_strips

:: here we specify in which format the original raw flight lines 
:: files are stored in the RAW_FLIGHT_LINES folder
set RAW_FORMAT=las

:: here we specify the directory (e.g. the folder) in which the
:: temporary files are stored
set TEMP_FILES=C:\lastools_temp

:: here we specify the directory (e.g. the folder) in which the
:: resulting output files are stored
set OUTPUT_FILES=C:\lastools_output

:: here we specify where the GEOID transformation grid is stored
set GEOID=C:\Users\Public\Documents\WGS84_to_EGM2008_GRID_Adjustment\wgs84_2_egm08.asc

:: here we specify the number of cores we want to run on
set NUM_CORES=7

:: here we specify the target tile size of the tiling
set TILE_SIZE=1000

:: here we specify the target buffer size for each tile
set TILE_BUFFER=50

rmdir %TEMP_FILES% /s /q

::
:: start processing
::

:: transform the strips to GEOID EGM2008 orthometric heights

mkdir %TEMP_FILES%\orthometric_strips
lasheight -i %RAW_FLIGHT_LINES%\*.%RAW_FORMAT% ^
          -rescale 0.01 0.01 0.01 -auto_reoffset ^
          -ground_points %GEOID% ^
          -replace_z ^
          -odir %TEMP_FILES%\orthometric_strips -olaz ^
          -cores %NUM_CORES%

:: create buffered tiling from orthometric strips

mkdir %TEMP_FILES%\tiles_raw
lastile -i %TEMP_FILES%\orthometric_strips\*.laz -files_are_flightlines ^
        -tile_size %TILE_SIZE% -buffer %TILE_BUFFER% ^
        -o %TEMP_FILES%\tiles_raw\tile -olaz

:: ground classify all orthometric tiles

mkdir %TEMP_FILES%\tiles_ground_town
lasground -i %TEMP_FILES%\tiles_raw\tile*.laz ^
          -town -extra_fine ^
          -odir %TEMP_FILES%\tiles_ground_town -olaz ^
          -cores %NUM_CORES%

:: remove low and high outliers

mkdir %TEMP_FILES%\height_denoised
lasheight -i %TEMP_FILES%\tiles_ground_town\*.laz ^
          -drop_above 40 -drop_below -3 ^
          -odir %TEMP_FILES%\height_denoised -olaz ^
          -cores %NUM_CORES%

:: building & veggy classify all ground and denoised tiles

mkdir %OUTPUT_FILES%\classified_buffered_tiles
lasclassify -i %TEMP_FILES%\height_denoised\*.laz ^
            -odir %OUTPUT_FILES%\classified_buffered_tiles -olaz ^
            -cores %NUM_CORES%

echo "bye bye"

goto the_end

:the_end
