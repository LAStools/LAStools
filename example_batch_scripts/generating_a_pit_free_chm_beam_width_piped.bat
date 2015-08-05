::
:: a batch script for generating a pit-free CHM as outlined
:: in the Silvilaser 2013 poster by A. Khosravipour et al.
:: with an additional step that incorporated the beam width
:: of the laser to "airbrush" a fuller canopy.
::
:: this version uses a (multi-core) pipeline of las2dem calls
::

::
:: specify parameters
::

:: allows you to run the script from other folders
set PATH=%PATH%;D:\lastools\bin;

:: here we specify the input (already height-normalized)
set NORMALIZED_LIDAR=D:\data\normalized\bois_noir.laz

:: specify the desired resolution 
set STEP=0.5

:: specify the radius of the laser beam
set BEAM_RADIUS=0.1

:: specify the sub-resolution for the beam file (about half the step)
set SUB_STEP=0.25

:: specify the desired triangle kill (about 3 times the step)
set KILL=1.5

:: specify a temporary directory for the beam file and the partial CHMs
set TEMP_CHM_DIR=temp_chms

:: specify the temporary beam file
set BEAM_LIDAR=%TEMP_CHM_DIR%\beam.laz

:: specify the final output CHM file name and format
set PIT_FREE_CHM=D:\data\result\chm.tif

rmdir %TEMP_CHM_DIR% /s /q
mkdir %TEMP_CHM_DIR%

::
:: do the actual processing
::

lasthin -i %NORMALIZED_LIDAR% ^
        -highest ^
        -step %SUB_STEP% ^
        -subcircle %BEAM_RADIUS% ^
        -o %BEAM_LIDAR%

las2dem -i %BEAM_LIDAR% ^
        -pipe_on ^
        -step %STEP% ^
        -o %TEMP_CHM_DIR%\chm_00.bil | ^
las2dem -stdin ^
        -drop_z_below 2 ^
        -pipe_on ^
        -step %STEP% -kill %KILL% ^
        -o %TEMP_CHM_DIR%\chm_02.bil | ^
las2dem -stdin ^
        -drop_z_below 5 ^
        -pipe_on ^
        -step %STEP% -kill %KILL% ^
        -o %TEMP_CHM_DIR%\chm_05.bil | ^
las2dem -stdin ^
        -drop_z_below 10 ^
        -pipe_on ^
        -step %STEP% -kill %KILL% ^
        -o %TEMP_CHM_DIR%\chm_10.bil | ^
las2dem -stdin ^
        -drop_z_below 15 ^
        -pipe_on ^
        -step %STEP% -kill %KILL% ^
        -o %TEMP_CHM_DIR%\chm_15.bil | ^
las2dem -stdin ^
        -drop_z_below 20 ^
        -pipe_on ^
        -step %STEP% -kill %KILL% ^
        -o %TEMP_CHM_DIR%\chm_20.bil | ^
las2dem -stdin ^
        -drop_z_below 25 ^
        -pipe_on ^
        -step %STEP% -kill %KILL% ^
        -o %TEMP_CHM_DIR%\chm_25.bil | ^
las2dem -stdin ^
        -drop_z_below 30 ^
        -pipe_on ^
        -step %STEP% -kill %KILL% ^
        -o %TEMP_CHM_DIR%\chm_30.bil

:: merge partial CHMs together into pit-free CHM

lasgrid -i %TEMP_CHM_DIR%\chm_*.bil -merged ^
        -step %STEP% -highest ^
        -o %PIT_FREE_CHM%

:: remove the temporary files and directory

rmdir %TEMP_CHM_DIR% /s /q

echo "bye bye"
