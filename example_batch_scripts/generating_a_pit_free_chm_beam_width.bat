::
:: a batch script for generating a pit-free CHM as outlined
:: in the Silvilaser 2013 poster by A. Khosravipour et al.
:: with an additional step that incorporated the beam width
:: of the laser to "airbrush" a fuller canopy
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

:: specify the sub-resolution for the beam file (about half the step or less)
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

blast2dem -i %BEAM_LIDAR% ^
          -step %STEP% ^
          -odir %TEMP_CHM_DIR% -odix _00 -obil

blast2dem -i %BEAM_LIDAR% ^
          -drop_z_below 2 ^
          -step %STEP% -kill %KILL% ^
          -odir %TEMP_CHM_DIR% -odix _02 -obil

blast2dem -i %BEAM_LIDAR% ^
          -drop_z_below 5 ^
          -step %STEP% -kill %KILL% ^
          -odir %TEMP_CHM_DIR% -odix _05 -obil

blast2dem -i %BEAM_LIDAR% ^
          -drop_z_below 10 ^
          -step %STEP% -kill %KILL% ^
          -odir %TEMP_CHM_DIR% -odix _10 -obil

blast2dem -i %BEAM_LIDAR% ^
          -drop_z_below 15 ^
          -step %STEP% -kill %KILL% ^
          -odir %TEMP_CHM_DIR% -odix _15 -obil

:: this would be sufficient for forests with tree heights
:: of up to about 20 meter. maybe delete higher up steps

blast2dem -i %BEAM_LIDAR% ^
          -drop_z_below 20 ^
          -step %STEP% -kill %KILL% ^
          -odir %TEMP_CHM_DIR% -odix _20 -obil

blast2dem -i %BEAM_LIDAR% ^
          -drop_z_below 25 ^
          -step %STEP% -kill %KILL% ^
          -odir %TEMP_CHM_DIR% -odix _25 -obil

blast2dem -i %BEAM_LIDAR% ^
          -drop_z_below 30 ^
          -step %STEP% -kill %KILL% ^
          -odir %TEMP_CHM_DIR% -odix _30 -obil

:: merge partial CHMs together into pit-free CHM

lasgrid -i %TEMP_CHM_DIR%\*.bil -merged ^
        -step %STEP% -highest ^
        -o %PIT_FREE_CHM%

:: remove the temporary files and directory

rmdir %TEMP_CHM_DIR% /s /q

echo "bye bye"
