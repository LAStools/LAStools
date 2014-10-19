::
:: an example batch script for ground classifying points in a single
:: huge LAS/LAZ file
::

echo off

set PATH=%PATH%;C:\lastools\bin;

:: create temporary tile directory

rmdir temp_tiles /s /q
mkdir temp_tiles

:: create a temporary and reversible tiling with tile size 500 and buffer 50

lastile -i %1 ^
        -reversible -tile_size 500 -buffer 50 ^
        -o temp_tiles\tile.laz -olaz

:: create another temporary tile directory

rmdir temp_tiles_ground /s /q
mkdir temp_tiles_ground

:: run lasground on all temporary tiles on 3 cores

lasground -i temp_tiles\tile*.laz ^
          -town -extra_fine ^
          -odir temp_tiles_ground -olaz ^
          -cores 3

:: delete temporary tile directory

rmdir temp_tiles /s /q

:: recreate ground classified huge LAS / LAZ file

lastile -i temp_tiles_ground\tile*.laz ^
        -reverse_tiling ^
        -o %1 -odix _ground -olaz 

:: delete other temporary tile directory

rmdir temp_tiles_ground /s /q
