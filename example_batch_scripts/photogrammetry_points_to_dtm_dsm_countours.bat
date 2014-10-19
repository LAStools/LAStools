::
:: an example batch script for generating both DTM and DSM
:: rasters from a large dense-matching point cloud using a
:: tile-based processing pipeline
::

::
:: specify parameters
::

:: allows you to run the script from other folders
set PATH=%PATH%;D:\lastools\bin;

:: specify input file
set INPUT_POINTS=D:\lastools\bin\macmahanfarm.laz

:: specify desired output raster resolution
set STEP=0.5

:: specify desired contour line spacing
set EVERY=1.0
set COARSE_STEP=1.0

:: specify file name and format for DTM 
set OUTPUT_DTM=D:\lastools\bin\macmahanfarm_dtm.tif

:: specify file name and format for DSM 
set OUTPUT_DSM=D:\lastools\bin\macmahanfarm_dsm.tif

:: specify file name and format for elevation contour
set OUTPUT_ISO=D:\lastools\bin\macmahanfarm_iso.shp

:: specify temporary directory for tiles
set TEMP_DIR=D:\lastools\bin\poop

:: specify size of tile and edge-artifact-avoiding buffer
set TILE_SIZE=500
set BUFFER=25

:: specify number of cores
set CORES=4

::
:: do the actual processing
::

:: create or empty temporary directory

rmdir %TEMP_DIR% /s /q
mkdir %TEMP_DIR%

:: create buffered tiling

lastile -i %INPUT_POINTS% ^
        -rescale 0.01 0.01 0.01 ^
        -tile_size %TILE_SIZE% -buffer %BUFFER% ^
        -o %TEMP_DIR%\tile.laz -olaz

:: run lassort on all tiles

lassort -i %TEMP_DIR%\tile*.laz ^
        -odix _s -olaz ^
        -cores %CORES%

:: run lasground on all tiles

lasground -i %TEMP_DIR%\tile*_s.laz ^
          -all_returns ^
          -city -ultra_fine ^
          -odix g -olaz ^
          -cores %CORES%

:: create the DTMs for all tiles

las2dem -i %TEMP_DIR%\tile*_sg.laz ^
        -keep_class 2 ^
        -step %STEP% ^
        -use_tile_bb ^
        -ocut 3 -odix _dtm -obil ^
        -cores %CORES%

:: join into single DTM

lasgrid -i %TEMP_DIR%\tile*_dtm.bil -merged ^
        -step %STEP% ^
        -o %OUTPUT_DTM%

:: create the DSMs for all tiles

las2dem -i %TEMP_DIR%\tile*_sg.laz ^
        -step %STEP% ^
        -use_tile_bb ^
        -ocut 3 -odix _dsm -obil ^
        -cores %CORES%

:: join into single DSM

lasgrid -i %TEMP_DIR%\tile*_dsm.bil -merged ^
        -step %STEP% ^
        -o %OUTPUT_DSM%

:: create an averaged (smoother) coarse DTM 

lasgrid -i %TEMP_DIR%\tile*_dtm.bil -merged ^
        -step %COARSE_STEP% -average ^
        -o %TEMP_DIR%\dtm_coarse.bil

:: create elevation contours from DTM using BLAST

blast2iso -i %TEMP_DIR%\dtm_coarse.bil ^
          -iso_every %EVERY% ^
          -smooth 2 -simplify_length 1 -simplify_area 1 -clean 5 ^
          -o %OUTPUT_ISO%

:: remove the temporary files and directory

:: rmdir %TEMP_DIR% /s /q

echo "bye bye"
