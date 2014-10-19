:: 
:: lasplanes script for (raw) TLS data in ECEF coordinates
::

::::::::::::::::::::::::
:: define parameters
::::::::::::::::::::::::

set NUM_CORES=8
set UTM_ZONE=34N

:: folder containing input files and format
set INPUT_FOLDER=D:\lastools\bin\tls_test_input
set INPUT_FORMAT=las

:: folder and names for output file and planes
set OUTPUT_FOLDER=D:\lastools\bin\tls_test_output
set OUTPUT_NAME=BP006_111_01
set POLYGON_NAME=T_006_111_01
set OUTPUT_FORMAT=pef

:: only use points with height from BELOW to ABOVE
set CUTOFF_BELOW=0.5
set CUTOFF_ABOVE=10

:: lasplanes parameters
set CELL_SIZE=1.0
set CELL_POINTS=100
set EIGEN_RATIO_SMALLEST=0.0001
set EIGEN_RATIO_LARGEST=0.9
set PLANE_THICKNESS=0.01
set PLANE_EXCLUSION=5
set PLANE_POINTS=100
set POLYGON_AREA=0.5
set POLYGON_POINTS=8
set POLYGON_STDDEV=0.01

:: convert file(s) from ECEF to UTM, preserve high precision, zero classifications

las2las -i %INPUT_FOLDER%\*.%INPUT_FORMAT% ^
        -set_classification 0 ^
        -ecef ^
        -target_utm %UTM_ZONE% ^
        -target_precision 0.00025 ^
        -odir %OUTPUT_FOLDER% -odix _utm -olaz ^
        -cores %NUM_CORES%

:: merge resulting UTM files and keep only lowest point per 0.10 by 0.10 meter area

lasthin -i %OUTPUT_FOLDER%\*_utm.laz -merged ^
        -step 0.1 -lowest ^
        -odir %OUTPUT_FOLDER% -o %OUTPUT_NAME%_thinned.laz

:: ground-classify the thinned point set

lasground -i %OUTPUT_FOLDER%\%OUTPUT_NAME%_thinned.laz ^
          -step 5 -no_stddev -spike 0.25 ^
          -odir %OUTPUT_FOLDER% -o %OUTPUT_NAME%_classified.laz

:: extract the ground points into a new file

las2las -i %OUTPUT_FOLDER%\%OUTPUT_NAME%_classified.laz ^
        -keep_class 2 ^
        -odir %OUTPUT_FOLDER% -o %OUTPUT_NAME%_ground.laz

:: keep only points that are between 0.5 and 10 meters above the thinned ground

lasheight -i %OUTPUT_FOLDER%\*_utm.laz ^
          -ground_points %OUTPUT_FOLDER%\%OUTPUT_NAME%_ground.laz ^
          -drop_below %CUTOFF_BELOW% -drop_above %CUTOFF_ABOVE% ^
          -odir %OUTPUT_FOLDER% -odix _cut -olaz ^
          -cores %NUM_CORES%

:: convert file from UTM34 back to ECEF

las2las -i %OUTPUT_FOLDER%\*_utm_cut.laz ^
        -target_ecef ^
        -target_precision 0.00025 ^
        -odir %OUTPUT_FOLDER% -odix _ecef -olaz ^
        -cores %NUM_CORES%

:: find planes 

lasplanes -i %OUTPUT_FOLDER%\*_utm_cut_ecef.laz -merged ^
          -cell_size %CELL_SIZE% ^
          -cell_points %CELL_POINTS% ^
          -eigen_ratio_smallest %EIGEN_RATIO_SMALLEST% ^
          -eigen_ratio_largest %EIGEN_RATIO_LARGEST% ^
          -plane_thickness %PLANE_THICKNESS% ^
          -plane_exclusion %PLANE_EXCLUSION% ^
          -plane_points %PLANE_POINTS% ^
          -polygon_area %POLYGON_AREA% ^
          -polygon_points %POLYGON_POINTS% ^
          -polygon_stddev %POLYGON_STDDEV% ^
          -polygon_name %POLYGON_NAME% ^
          -odir %OUTPUT_FOLDER% -o %OUTPUT_NAME%.%OUTPUT_FORMAT% -o%OUTPUT_FORMAT%

:: clean-up temp files

del %OUTPUT_FOLDER%\*_utm.laz
del %OUTPUT_FOLDER%\%OUTPUT_NAME%_thinned.laz
del %OUTPUT_FOLDER%\%OUTPUT_NAME%_classified.laz
del %OUTPUT_FOLDER%\%OUTPUT_NAME%_ground.laz
del %OUTPUT_FOLDER%\*_utm_cut.laz
del %OUTPUT_FOLDER%\*_utm_cut_ecef.laz

GOTO:EOF
