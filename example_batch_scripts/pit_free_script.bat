::
:: Best practice to generate a pit-free raster Canopy Height Model (CHM) with LAStools
::
:: Read more at
::
:: http://rapidlasso.com/2014/11/04/rasterizing-perfect-canopy-height-models-from-lidar/
:: 

set PATH=%PATH%;..;
set INPUT=sample.laz
set STEP=0.5
set HALF_STEP=0.25
set KILL=1.5

:: ground classify LiDAR

lasground -i %INPUT% -wilderness -o ground.laz

:: normalize the LiDAR data

lasheight -i ground.laz -replace_z -o normalized.laz

:: create CHM with highest point per grid cell with lasgrid

lasgrid -i normalized.laz -step %STEP% -highest -false -set_min_max 0 65 -o chm_grd.png

:: create CHM with highest point per grid cell with lasgrid and 10 cm circles

lasgrid -i normalized.laz -subcircle 0.05 -step %STEP% -highest -false -set_min_max 0 65 -o chm_grd_d10.png

:: create CHM with highest point per grid cell with lasgrid and 5 cm circles

lasgrid -i normalized.laz -subcircle 0.025 -step %STEP% -highest -false -set_min_max 0 65 -o chm_grd_d05.png

:: create CHM with modified pit-free algorithm using 10 cm circles

rmdir temp_dir /s /q
mkdir temp_dir
las2dem -i normalized.laz -drop_z_above 0.1 -step %STEP% -o temp_dir/chm_ground.bil
lasthin -i normalized.laz -subcircle 0.05 -step %HALF_STEP% -highest -o temp.laz
las2dem -i temp.laz -step %STEP% -kill %KILL% -o temp_dir/chm_00.bil
las2dem -i temp.laz -drop_z_below  2 -step %STEP% -kill %KILL% -o temp_dir/chm_02.bil
las2dem -i temp.laz -drop_z_below  5 -step %STEP% -kill %KILL% -o temp_dir/chm_05.bil
las2dem -i temp.laz -drop_z_below 10 -step %STEP% -kill %KILL% -o temp_dir/chm_10.bil
las2dem -i temp.laz -drop_z_below 15 -step %STEP% -kill %KILL% -o temp_dir/chm_15.bil
las2dem -i temp.laz -drop_z_below 20 -step %STEP% -kill %KILL% -o temp_dir/chm_20.bil
las2dem -i temp.laz -drop_z_below 25 -step %STEP% -kill %KILL% -o temp_dir/chm_25.bil
las2dem -i temp.laz -drop_z_below 30 -step %STEP% -kill %KILL% -o temp_dir/chm_30.bil
las2dem -i temp.laz -drop_z_below 35 -step %STEP% -kill %KILL% -o temp_dir/chm_35.bil
las2dem -i temp.laz -drop_z_below 40 -step %STEP% -kill %KILL% -o temp_dir/chm_40.bil
las2dem -i temp.laz -drop_z_below 45 -step %STEP% -kill %KILL% -o temp_dir/chm_45.bil
las2dem -i temp.laz -drop_z_below 50 -step %STEP% -kill %KILL% -o temp_dir/chm_50.bil
las2dem -i temp.laz -drop_z_below 55 -step %STEP% -kill %KILL% -o temp_dir/chm_55.bil
las2dem -i temp.laz -drop_z_below 60 -step %STEP% -kill %KILL% -o temp_dir/chm_60.bil
las2dem -i temp.laz -drop_z_below 65 -step %STEP% -kill %KILL% -o temp_dir/chm_65.bil
lasgrid -i temp_dir/chm*.bil -merged -step %STEP% -highest -false -set_min_max 0 65 -o chm_pit_free_d10.png
rmdir temp_dir /s /q

:: same as above but even more conservative by using 5 cm instead of 10 cm circles

rmdir temp_dir /s /q
mkdir temp_dir
las2dem -i normalized.laz -drop_z_above 0.1 -step %STEP% -o temp_dir/chm_ground.bil
lasthin -i normalized.laz -subcircle 0.025 -step %HALF_STEP% -highest -o temp.laz
las2dem -i temp.laz -step %STEP% -kill %KILL% -o temp_dir/chm_00.bil
las2dem -i temp.laz -drop_z_below  2 -step %STEP% -kill %KILL% -o temp_dir/chm_02.bil
las2dem -i temp.laz -drop_z_below  5 -step %STEP% -kill %KILL% -o temp_dir/chm_05.bil
las2dem -i temp.laz -drop_z_below 10 -step %STEP% -kill %KILL% -o temp_dir/chm_10.bil
las2dem -i temp.laz -drop_z_below 15 -step %STEP% -kill %KILL% -o temp_dir/chm_15.bil
las2dem -i temp.laz -drop_z_below 20 -step %STEP% -kill %KILL% -o temp_dir/chm_20.bil
las2dem -i temp.laz -drop_z_below 25 -step %STEP% -kill %KILL% -o temp_dir/chm_25.bil
las2dem -i temp.laz -drop_z_below 30 -step %STEP% -kill %KILL% -o temp_dir/chm_30.bil
las2dem -i temp.laz -drop_z_below 35 -step %STEP% -kill %KILL% -o temp_dir/chm_35.bil
las2dem -i temp.laz -drop_z_below 40 -step %STEP% -kill %KILL% -o temp_dir/chm_40.bil
las2dem -i temp.laz -drop_z_below 45 -step %STEP% -kill %KILL% -o temp_dir/chm_45.bil
las2dem -i temp.laz -drop_z_below 50 -step %STEP% -kill %KILL% -o temp_dir/chm_50.bil
las2dem -i temp.laz -drop_z_below 55 -step %STEP% -kill %KILL% -o temp_dir/chm_55.bil
las2dem -i temp.laz -drop_z_below 60 -step %STEP% -kill %KILL% -o temp_dir/chm_60.bil
las2dem -i temp.laz -drop_z_below 65 -step %STEP% -kill %KILL% -o temp_dir/chm_65.bil
lasgrid -i temp_dir/chm*.bil -merged -step %STEP% -highest -false -set_min_max 0 65 -o chm_pit_free_d05.png
rmdir temp_dir /s /q

