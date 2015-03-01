::
:: Ways to generate raster Canopy Height Model (CHM) with LAStools ... (-:
::
:: Read more at
::
:: http://rapidlasso.com/2014/11/04/rasterizing-perfect-canopy-height-models-from-lidar/
:: 

set PATH=%PATH%;D:\LAStools\bin;

:: global definitions

set MUNICIPALITY=tab
set BLOCK=vsu
set STEP=0.5
set SUBGRIDSTEP=0.25
set KILL=1.5
set MIN_MAX=0 15
set CORES=6

:: below are all the used command lines used in this article

:: tile the LiDAR

:: ground classify LiDAR

:: normalize the LiDAR data

lasheight -i %BLOCK%\tiles_ground\%MUNICIPALITY%*.laz ^
          -replace_z ^
          -classify_below -2 7 ^
          -classify_above 50 7 ^
          -odir %BLOCK%\tiles_normalized -olaz ^
          -cores %CORES%

:: create CHM simply with highest point per grid cell with lasgrid

lasgrid -i %BLOCK%\tiles_normalized\%MUNICIPALITY%*.laz ^
        -step %STEP% -highest ^
        -false -set_min_max %MIN_MAX% ^
        -use_tile_bb ^
        -odir %BLOCK%\tiles_chm -odix "_grd_d00" -opng ^
        -cores %CORES%

:: same as above but "splat" points into small circles with diameter 20 cm

:GRID_D20

lasgrid -i %BLOCK%\tiles_normalized\%MUNICIPALITY%*.laz ^
        -subcircle 0.1 ^
        -step %STEP% -highest ^
        -false -set_min_max %MIN_MAX% ^
        -use_tile_bb ^
        -odir %BLOCK%\tiles_chm -odix "_grd_d20" -opng ^
        -cores %CORES% 

:: create CHM by rasterizing TIN of all first returns with las2dem

:TIN_D00

las2dem -i %BLOCK%\tiles_normalized\%MUNICIPALITY%*.laz ^
        -first_only ^
        -step %STEP% ^
        -false -set_min_max %MIN_MAX% ^
        -use_tile_bb ^
        -odir %BLOCK%\tiles_chm -odix "_tin_d00" -opng ^
        -cores %CORES%

:: create CHM by rasterizing TIN of highest point per finer (sub-)grid cell after "splatting" points into small frisbees of 20 cm diameter

:TIN_D20

rmdir %BLOCK%\tmp_dir /s /q
mkdir %BLOCK%\tmp_dir 

lasthin -i %BLOCK%\tiles_normalized\%MUNICIPALITY%*.laz ^
        -subcircle 0.1 ^
        -step %SUBGRIDSTEP% -highest ^
        -odir %BLOCK%\tmp_dir -olaz ^
        -cores %CORES%

las2dem -i %BLOCK%\tmp_dir\%MUNICIPALITY%*.laz ^
        -step %STEP% ^
        -false -set_min_max %MIN_MAX% ^
        -use_tile_bb ^
        -odir %BLOCK%\tiles_chm -odix "_tin_d20" -opng ^
        -cores %CORES%

rmdir %BLOCK%\tmp_dir /s /q

:: create CHM with modified pit-free algorithm

:PIT_FREE

rmdir %BLOCK%\tmp_dir /s /q
mkdir %BLOCK%\tmp_dir

lasthin -i %BLOCK%\tiles_normalized\%MUNICIPALITY%*.laz ^
        -subcircle 0.1 ^
        -step %SUBGRIDSTEP% -highest ^
        -odir %BLOCK%\tmp_dir -olaz ^
        -cores %CORES%

las2dem -i %BLOCK%\tmp_dir\%MUNICIPALITY%*.laz ^
        -drop_z_above 0.1 ^
        -step %STEP% ^
        -use_tile_bb ^
        -odir %BLOCK%\tmp_dir -odix "_chm_GP" -obil ^
        -cores %CORES%
  
las2dem -i %BLOCK%\tmp_dir\%MUNICIPALITY%*.laz ^
        -step %STEP% ^
        -kill %KILL% ^
        -use_tile_bb ^
        -odir %BLOCK%\tmp_dir -odix "_chm_00" -obil ^
        -cores %CORES%

las2dem -i %BLOCK%\tmp_dir\%MUNICIPALITY%*.laz ^
        -drop_z_below  2 ^
        -step %STEP% ^
        -kill %KILL% ^
        -use_tile_bb ^
        -odir %BLOCK%\tmp_dir -odix "_chm_02" -obil ^
        -cores %CORES%

las2dem -i %BLOCK%\tmp_dir\%MUNICIPALITY%*.laz ^
        -drop_z_below  5 ^
        -step %STEP% ^
        -kill %KILL% ^
        -use_tile_bb ^
        -odir %BLOCK%\tmp_dir -odix "_chm_05" -obil ^
        -cores %CORES%

las2dem -i %BLOCK%\tmp_dir\%MUNICIPALITY%*.laz ^
        -drop_z_below 8 ^
        -step %STEP% ^
        -kill %KILL% ^
        -use_tile_bb ^
        -odir %BLOCK%\tmp_dir -odix "_chm_08" -obil ^
        -cores %CORES%

las2dem -i %BLOCK%\tmp_dir\%MUNICIPALITY%*.laz ^
        -drop_z_below 11 ^
        -step %STEP% ^
        -kill %KILL% ^
        -use_tile_bb ^
        -odir %BLOCK%\tmp_dir -odix "_chm_11" -obil ^
        -cores %CORES%

las2dem -i %BLOCK%\tmp_dir\%MUNICIPALITY%*.laz ^
        -drop_z_below 14 ^
        -step %STEP% ^
        -kill %KILL% ^
        -use_tile_bb ^
        -odir %BLOCK%\tmp_dir -odix "_chm_14" -obil ^
        -cores %CORES%

lasgrid -i %BLOCK%\tmp_dir\%MUNICIPALITY%*.bil -merged ^
        -step %STEP% ^
        -highest ^
        -false -set_min_max %MIN_MAX% ^
        -use_bb ^
        -o %BLOCK%\chm_pit_free.png

rmdir %BLOCK%\tmp_dir /s /q




