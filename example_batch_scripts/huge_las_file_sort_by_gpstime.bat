::
:: an example batch script for sorting points by their GPS time for
:: a single huge LAS/LAZ file
::

echo off

set PATH=%PATH%;..\bin;

:: create temporary split directory

rmdir temp_split /s /q
mkdir temp_split

:: create a temporary splitting of the file into 100 second splits

lassplit -i %1 ^
         -by_gps_time_interval 100 -digits 6  ^
         -o temp_split\split.laz -olaz

:: create another temporary tile directory

rmdir temp_split_sorted /s /q
mkdir temp_split_sorted

:: run lassort on all temporary split files on 3 cores

lassort -i temp_split\split*.laz ^
        -gps_time ^
        -odir temp_split_sorted -olaz ^
        -cores 3

:: delete temporary split directory

rmdir temp_split /s /q

:: merge the sorted split files into one huge LAS / LAZ file

lasmerge -i temp_split_sorted\split*.laz ^
        -o %1 -odix _sorted -olaz 

:: delete other temporary tile directory

rmdir temp_split_sorted /s /q
