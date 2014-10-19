::
:: an example batch script for removing xyz duplicate points in a single
:: huge LAS/LAZ file
::

echo off

set PATH=%PATH%;C:\lastools\bin;

:: create temporary tile directory

mkdir temp_tiles

:: create a temporary and reversible tiling with tile size 500

lastile -i %1 ^
        -reversible -tile_size 500 ^
        -o temp_tiles\tile.laz -olaz

:: create another temporary tile directory

mkdir temp_tiles_duplicate

:: remove unique xyz duplicates in all temporary tiles on 3 cores

lasduplicate -i temp_tiles\tile*.laz ^
             -unique_xyz ^
             -odir temp_tiles_duplicate -olaz ^
             -cores 3

:: delete temporary tile directory

rmdir temp_tiles /s /q

:: recreate huge LAS / LAZ file without duplicates

lastile -i temp_tiles_duplicate\tile*.laz ^
        -reverse_tiling ^
        -o %1 -odix _no_dup

:: delete other temporary tile directory

rmdir temp_tiles_duplicate /s /q
