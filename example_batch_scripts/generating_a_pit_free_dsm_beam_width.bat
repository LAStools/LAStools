::
:: a batch script for generating a pit-free DSM by utilizing 
:: the idea described in the poster by A. Khosravipour et al.
:: in Silvilaser 2013 for pit-free CHM generation but applied
:: to the DSM case with two changes: (1) an additional step
:: is "splatting" the point into small circles to account for
:: a (conservative under-estimate of) beam width of the laser
:: and (2) using all the highest returns (from a finer grid
:: that has half the step size of the raster) for interpolation
:: instead of the first returns 
::

::
:: specify parameters
::

:: allows you to run the script from other folders
set PATH=%PATH%;D:\lastools\bin;

:: specify the input LiDAR (with height stored in dm in 'user_data' field)
set LIDAR_WITH_HEIGHTS=lidar_with_heights.laz

:: specify the desired resolution 
set STEP=0.5

:: specify the radius of the laser beam
set BEAM_RADIUS=0.1

:: specify the sub-resolution for the beam file (about half the step)
set SUB_STEP=0.25

:: specify the desired triangle kill (about 3 times the step)
set KILL=1.5

:: specify a temporary directory for the beam file and the partial DSMs
set TEMP_DSM_DIR=chicken_poop

:: specify the temporary beam file
set BEAM_LIDAR=%TEMP_DSM_DIR%\beam.laz

:: specify the final output DSM file name and format
set PIT_FREE_DSM=dsm.tif

rmdir %TEMP_DSM_DIR% /s /q
mkdir %TEMP_DSM_DIR%

::
:: do the actual processing
::

lasthin -i %LIDAR_WITH_HEIGHTS% ^
        -highest ^
        -step %SUB_STEP% ^
        -subcircle %BEAM_RADIUS% ^
        -o %BEAM_LIDAR%

blast2dem -i %BEAM_LIDAR% ^
          -step %STEP% ^
          -odir %TEMP_DSM_DIR% -odix _00 -obil

blast2dem -i %BEAM_LIDAR% ^
          -drop_user_data_below 20 ^
          -step %STEP% -kill %KILL% ^
          -odir %TEMP_DSM_DIR% -odix _02 -obil

blast2dem -i %BEAM_LIDAR% ^
          -drop_user_data_below 50 ^
          -step %STEP% -kill %KILL% ^
          -odir %TEMP_DSM_DIR% -odix _05 -obil

blast2dem -i %BEAM_LIDAR% ^
          -drop_user_data_below 100 ^
          -step %STEP% -kill %KILL% ^
          -odir %TEMP_DSM_DIR% -odix _10 -obil

blast2dem -i %BEAM_LIDAR% ^
          -drop_user_data_below 150 ^
          -step %STEP% -kill %KILL% ^
          -odir %TEMP_DSM_DIR% -odix _15 -obil

blast2dem -i %BEAM_LIDAR% ^
          -drop_user_data_below 200 ^
          -step %STEP% -kill %KILL% ^
          -odir %TEMP_DSM_DIR% -odix _20 -obil

blast2dem -i %BEAM_LIDAR% ^
          -drop_user_data_below 250 ^
          -step %STEP% -kill %KILL% ^
          -odir %TEMP_DSM_DIR% -odix _25 -obil

:: merge partial DSMs together into pit-free DSM

lasgrid -i %TEMP_DSM_DIR%\*.bil -merged ^
        -step %STEP% -highest ^
        -o %PIT_FREE_DSM%

:: remove the temporary files and directory

rmdir %TEMP_DSM_DIR% /s /q

echo "bye bye"
