::
:: a batch script for quality checking of poor tiles
::

::
:: specify parameters
::

:: allows you to run the script from other folders
set PATH=%PATH%;D:\lastools\bin;

:: here we specify the directory (e.g. the folder) in which the
:: bad tiles are stored
set BAD_TILES_FOLDER=D:\data_fiji\FMG1986

:: here we specify in which format the bad tiles are stored in
:: the BAD_TILES_FOLDER folder
set TILE_FORMAT=laz

:: here we specify the directory (e.g. the folder) in which the
:: temporary files are stored
set TEMP_FILES=D:\data_fiji\temp

:: here we specify the directory (e.g. the folder) in which the
:: resulting quality reports are stored
set OUTPUT_DIRECTORY=D:\data_fiji\quality

:: here we specify the number of cores we want to run on
set NUM_CORES=7

rmdir %TEMP_FILES% /s /q

::
:: loop over all bad tiles
::

for %%f IN (%BAD_TILES_FOLDER%\*.%TILE_FORMAT%) DO (
::FOR /R %%f IN (*.*) DO (
  ECHO processing %BAD_TILES_FOLDER%\%%~nxf to %OUTPUT_DIRECTORY%\%%~nf.png
  mkdir %TEMP_FILES%
  CALL lassplit -i %BAD_TILES_FOLDER%\%%~nxf -by_gps_time_interval 500 -olaz -odir %TEMP_FILES%
  CALL lasoverlap -i %TEMP_FILES%\*.laz -files_are_flightlines -o %OUTPUT_DIRECTORY%\%%~nf.png -utm 60K
  rmdir %TEMP_FILES% /s /q
)

echo "bye bye"
