::
:: an example batch script for copying a point particular point attribute from
:: one file to another based on identical GPS times and return numbers using
:: lascopy for a single huge LAS/LAZ file
::

echo off

set PATH=%PATH%;..\bin;

:: create temporary split directory

rmdir temp_source/s /q
mkdir temp_source

:: create a temporary splitting of the source file into 100 second splits

lassplit -i %1 ^
         -by_gps_time_interval 100 -digits 6  ^
         -o temp_source\split.laz -olaz

:: create a temporary splitting of the target file into 100 second splits

lassplit -i %1 ^
         -by_gps_time_interval 100 -digits 6  ^
         -o temp_split\target.laz -olaz

:: create another temporary tile directory

rmdir temp_split_copied /s /q
mkdir temp_split_copied 

:: run lascopy on all temporary split files 

lassort -i temp_split\split*.laz ^
        -gps_time ^
        -odir temp_split_sorted -olaz 

:: delete temporary split directory

rmdir temp_split /s /q

:: merge the sorted split files into one huge LAS / LAZ file

lasmerge -i temp_split_sorted\split*.laz ^
        -o %1 -odix _sorted -olaz 

:: delete other temporary tile directory

rmdir temp_split_sorted /s /q
