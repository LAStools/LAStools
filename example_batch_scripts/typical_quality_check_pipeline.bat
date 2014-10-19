::
:: a batch script for quality checking raw flight lines
::

::
:: specify parameters
::

:: allows you to run the script from other folders
set PATH=%PATH%;C:\Users\Public\lastools\bin;

:: here we specify the directory (e.g. the folder) in which the
:: original raw flight lines files are stored
set RAW_FLIGHT_LINES=C:\Users\Public\Documents\pam8a_outputlas

:: here we specify in which format the original raw flight lines 
:: files are stored in the RAW_FLIGHT_LINES folder
set RAW_FORMAT=las

:: here we specify the directory (e.g. the folder) in which the
:: temporary files are stored
set TEMP_FILES=C:\lastools_temp

:: here we specify the directory (e.g. the folder) in which the
:: resulting quality reports are stored
set OUTPUT_FILES=C:\lastools_quality

:: here we specify the number of cores we want to run on
set NUM_CORES=7

rmdir %TEMP_FILES% /s /q

::
:: start processing
::

:: run lasinfo on all the input flight lines

mkdir %TEMP_FILES%\lasinfo_output
lasinfo -i %RAW_FLIGHT_LINES%\*.%RAW_FORMAT% ^
        -odir %TEMP_FILES%\lasinfo_output -otxt ^
        -cores %NUM_CORES%

find /N "WARNING" %TEMP_FILES%\lasinfo_output\*.txt

:: create visualization overlap and alignment of flight lines

mkdir %OUTPUT_FILES%
lasoverlap -i %RAW_FLIGHT_LINES%\*.%RAW_FORMAT% -files_are_flightlines ^
           -max_diff 1.0 -max_over 5 -step 2 ^
           -o %OUTPUT_FILES%\lasoverlap.png

:: create density grid of flight lines

lasgrid -i %RAW_FLIGHT_LINES%\*.%RAW_FORMAT% -merged -last_only ^
        -counter -step 2 -false -set_min_max 0 8 ^
        -o %OUTPUT_FILES%\density_2_pts.png

lasgrid -i %RAW_FLIGHT_LINES%\*.%RAW_FORMAT% -merged -last_only ^
        -counter -step 2 -false -set_min_max 0 16 ^
        -o %OUTPUT_FILES%\density_4_pts.png

:: compute a boundary around the merged set of flight lines (also look for holes)

lasboundary -i %RAW_FLIGHT_LINES%\*.%RAW_FORMAT% -merged ^
            -last_only -thin_with_grid 2 ^
            -concavity 10 -disjoint -holes ^
            -o %OUTPUT_FILES%\boundary.kml

echo "bye bye"

goto the_end

:the_end
