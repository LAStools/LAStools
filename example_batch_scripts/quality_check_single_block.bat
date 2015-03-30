::
:: Quality check for a single block
:: 

set PATH=%PATH%;D:\LAStools\bin;

:: global definitions

set SERVER_PATH=\\Lidar02_server\raw_data_vd0\PhilLiDAR2\Classified LAS
set OUTPUT_PATH=.
set QUALITY_FOLDER=quality_checks
set COARSE_DTM_FOLDER=coarse_dtms
set BLOCK=Agno5F_20130417
set LIDAR_FOLDER=LAS_FILES
set LIDAR_FORMAT=las
set COARSE_DTM_STEP=5
set CORES=4

:: remove old new directories

rmdir %OUTPUT_PATH%\%QUALITY_FOLDER%\%BLOCK% /s /q

:: make directories

mkdir %OUTPUT_PATH%\%QUALITY_FOLDER%\%BLOCK%
mkdir %OUTPUT_PATH%\%COARSE_DTM_FOLDER%

:: check LAS file integrity with lasvalidate

lasvalidate -i "%SERVER_PATH%\%BLOCK%\%LIDAR_FOLDER%\*.%LIDAR_FORMAT%" ^
            -no_CRS_fail ^
            -o %OUTPUT_PATH%\%QUALITY_FOLDER%\%BLOCK%_validate.xml


:: check flightline overlap with lasoverlap

lasoverlap  -i "%SERVER_PATH%\%BLOCK%\%LIDAR_FOLDER%\*.%LIDAR_FORMAT%" ^
            -recover_flightlines -utm 51N ^
            -odir %OUTPUT_PATH%\%QUALITY_FOLDER%\%BLOCK% ^
            -opng ^
            -cores %CORES%
             
:: create coarse DTM for inter-block alignment check with lasgrid

lasgrid     -i "%SERVER_PATH%\%BLOCK%\%LIDAR_FOLDER%\*.%LIDAR_FORMAT%" ^
            -merged ^
            -step %COARSE_DTM_STEP% -lowest ^
            -o %OUTPUT_PATH%\%COARSE_DTM_FOLDER%\%BLOCK%_%COARSE_DTM_STEP%m.bil
