::
:: an example batch script for sorting points by their GPS time for
:: a single huge LAS/LAZ file
::

echo off

set PATH=%PATH%;..\bin;

:: create temporary tiles directory

rmdir temp_tiles /s /q
mkdir temp_tiles

:: create a temporary tiling of the file into 400 by 400 meter tiles

lastile -i %1 ^
        -tile_size 400 ^
        -o temp_tiles\tiles.laz -olaz

:: create another temporary tile directory

rmdir temp_tiles_sorted /s /q
mkdir temp_tiles_sorted

:: run lassort on all temporary tiled files on 3 cores

lassort -i temp_tiles\split*.laz ^
        -odir temp_tiles_sorted -olaz ^
        -cores 3

:: delete temporary split directory

rmdir temp_tiles /s /q

:: merge the sorted tiles files back into one huge LAS / LAZ file

lasmerge -i temp_tiles_sorted\split*.laz ^
         -o %1 -odix _sorted -olaz 

:: delete other temporary tile directory

rmdir temp_tiles_sorted /s /q
