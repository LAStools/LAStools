::
:: an example batch script for processing a huge LAS/LAZ file into a 
:: file containing only ground points using a tile-based multi-core
:: processing pipeline by tiling, removing noise, classifying ground,
:: and finally merging the resulting ground points into one file. the
:: settings below are for a dense-survey from a low-flying helicopter.
::

::echo off

:: set relevant variables

set LAStools=E:\LAStools\bin\
set TILE_SIZE=250
set BUFFER=30
set STEP=1
set CORES=4

:: print-out which LAStools version are we running

%LAStools%^
lastile -version

:: create temporary tile directory

rmdir temp_tiles /s /q
mkdir temp_tiles

:: create a temporary tiling with tile size 250 and buffer 30

%LAStools%^
lastile -i %1 ^
        -set_classification 0 ^
        -tile_size %TILE_SIZE% -buffer %BUFFER% -flag_as_withheld ^
        -o temp_tiles\tile.laz -olaz

:: create another temporary tile directory

rmdir temp_tiles_denoised /s /q
mkdir temp_tiles_denoised

:: run lasnoise on all temporary tiles on multiple cores while
:: aggressively filtering noise points in an attempt to get as
:: many erroneous points below the ground as possible. wrong
:: classification of points above the grounds as noise does not
:: matter here.

%LAStools%^
lasnoise -i temp_tiles\tile*.laz ^
         -step_xy 1 -step_z 0.25 ^
         -classify_as 7 ^
         -odir temp_tiles_denoised -olaz ^
         -cores %CORES%

:: delete temporary tile directory

rmdir temp_tiles /s /q

:: create another temporary tile directory

rmdir temp_tiles_thinned /s /q
mkdir temp_tiles_thinned

:: run lasthin on all temporary tiles on multiple cores to mark
:: the point closest to the fifth (5th) lowest percentile in each
:: 1 meter by 1 meter cell with temporary classification code 6

%LAStools%^
lasthin -i temp_tiles_denoised\tile*.laz ^
        -ignore_class 7 ^
        -step 1 -percentile 0.05 ^
        -classify_as 6 ^
        -odir temp_tiles_thinned -olaz ^
        -cores %CORES%

:: delete temporary tile directory

rmdir temp_tiles_denoised /s /q

:: create another temporary tile directory

rmdir temp_tiles_ground /s /q
mkdir temp_tiles_ground

:: run lasground_new on all temporary tiles on multiple cores
:: but only on that one point marked as classification 6 per
:: 1 meter by 1 meter (by ignoring the other two classification
:: codes 7 and 0) with options '-city' and '-ultra_fine'.

%LAStools%^
lasground_new -i temp_tiles_thinned\tile*.laz ^
              -ignore_class 7 0 ^
              -city -ultra_fine ^
              -odir temp_tiles_ground -olaz ^
              -cores %CORES%

:: delete temporary tile directory

rmdir temp_tiles_thinned /s /q

:: merge the ground points of all tiles (all points that now
:: have classification 2) except the buffer points that were
:: flagged as 'withheld' during the initial tiling by lasthin  

%LAStools%^
lasmerge -i temp_tiles_ground\tile*.laz ^
         -drop_withheld -keep_class 2 ^
         -o %2 -olaz

:: delete other temporary tile directory

rmdir temp_tiles_ground /s /q
