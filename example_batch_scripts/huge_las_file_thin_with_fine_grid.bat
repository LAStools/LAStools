::
:: an example batch script for thinning a single huge LAS/LAZ file
:: with a very very fine grid
::

echo off

::
:: specify parameters
::

:: allows you to run the script from other folders
set PATH=%PATH%;D:\lastools\bin;

:: specify the resolution of the thinning grid
set STEP=0.05

:: specify the size of the temporary tiles. it is important that
:: the tile size can be evenly divided by the grid resolution
:: meaning that TILE_SIZE/STEP is XXXX.0 without decimal digits 
set TILE_SIZE=500

:: specify the number of cores to run on
set NUM_CORES=3

:: specify the name for temporary directory
set TEMP_DIR=temp_dir_thinning

::
:: do the actual processing
::

:: create temporary tile directory

rmdir %TEMP_DIR% /s /q
mkdir %TEMP_DIR%

:: create a temporary tiling with TILE_SIZE

lastile -i %1 ^
        -tile_size %TILE_SIZE% ^
        -o %TEMP_DIR%\tile.laz -olaz

:: thins the tiles on NUM_CORES

lasthin -i %TEMP_DIR%\tile*.laz ^
        -step %STEP% -lowest ^
        -odix _thinned -olaz ^
        -cores %NUM_CORES%

:: recreate the (less huge) thinned LAS / LAZ file

lasmerge -i %TEMP_DIR%\tile*_thinned.laz ^
         -o %1 -odix _thinned

:: delete the temporary tile directory

rmdir %TEMP_DIR% /s /q
