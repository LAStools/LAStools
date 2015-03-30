::
:: Quality check across blocks (using their coarse DTMs as proxies)
:: 

set PATH=%PATH%;D:\LAStools\bin;

:: global definitions

set INPUT_PATH=.
set OUTPUT_PATH=.
set COARSE_DTM_FOLDER=coarse_dtms
set COARSE_DTM_STEP=5

:: check alignment and overlap of blocks via their coarse DTM with lasoverlap

lasoverlap  -i "%INPUT_PATH%\%COARSE_DTM_FOLDER%\*.bil" ^
            -files_are_flightlines -step %COARSE_DTM_STEP% ^
            -utm 51N ^
            -o %OUTPUT_PATH%\%COARSE_DTM_FOLDER%\block.png

             
