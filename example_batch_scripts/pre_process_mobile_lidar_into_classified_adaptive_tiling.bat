::
:: an example batch script for pre-processing mobile LiDAR trajetories
:: into an adaptive tiling pre-classified based on trajectory data and
:: using a pre-thinning step for the ground classification
::

echo off

set PATH=%PATH%;C:\LAStools\bin;

:: here we specify the input folder 

set WORK_DIRECTORY=inspectation

:: here we set the base name of the produced tiles

set TILE_BASE_NAME=oosteinde

:: number of cores to run on

set NUM_CORES=4

:: convert trajectory file from GPS-week time to Adjusted Standard GPS time

las2las -i %WORK_DIRECTORY%\Trajectory.txt ^
        -iparse txyz -week_to_adjusted 1957 ^
        -o %WORK_DIRECTORY%\trajectory.laz

:: manually set missing file source IDs in original files (last digit specifies scanner channel)

lasinfo -i "%WORK_DIRECTORY%\strips_raw\Boom - Scanner 1 - 170713_073558_Scanner_1 - originalpoints.laz" ^
        -nc ^
        -set_file_source_ID 11

lasinfo -i "%WORK_DIRECTORY%\strips_raw\Boom - Scanner 2 - 170713_073558_Scanner_2 - originalpoints.laz" ^
        -nc ^
        -set_file_source_ID 12

lasinfo -i "%WORK_DIRECTORY%\strips_raw\Boom - Scanner 1 - 170713_073738_Scanner_1 - originalpoints.laz" ^
        -nc ^
        -set_file_source_ID 21

lasinfo -i "%WORK_DIRECTORY%\strips_raw\Boom - Scanner 2 - 170713_073738_Scanner_2 - originalpoints.laz" ^
        -nc ^
        -set_file_source_ID 22
        
lasinfo -i "%WORK_DIRECTORY%\strips_raw\Boom - Scanner 1 - 170713_073943_Scanner_1 - originalpoints.laz" ^
        -nc ^
        -set_file_source_ID 31

lasinfo -i "%WORK_DIRECTORY%\strips_raw\Boom - Scanner 2 - 170713_073943_Scanner_2 - originalpoints.laz" ^
        -nc ^
        -set_file_source_ID 32

:: rescale, reoffset, fix projection

rmdir %WORK_DIRECTORY%\strips_fixed /s /q
mkdir %WORK_DIRECTORY%\strips_fixed
las2las -i %WORK_DIRECTORY%\strips_raw\*.laz ^
        -rescale 0.01 0.01 0.01 ^
        -auto_reoffset ^
        -epsg 28992 -set_ogc_wkt ^
        -odir %WORK_DIRECTORY%\strips_fixed -olaz ^
        -cores %NUM_CORES%

:: create adaptive tiling (create initial 128 by 128 meter tiles)

rmdir %WORK_DIRECTORY%\tiles_buffered /s /q
mkdir %WORK_DIRECTORY%\tiles_buffered
lastile -i %WORK_DIRECTORY%\strips_fixed\*.laz ^
        -apply_file_source_ID ^
        -tile_size 128 ^
        -buffer 1 -flag_as_withheld ^
        -refine_tiling 15000000 ^
        -odir %WORK_DIRECTORY%\tiles_buffered -o %TILE_BASE_NAME%.laz

:: refine adaptive tiling (refine too large 128 by 128 meter into 64 by 64 tiles)

lastile -i %WORK_DIRECTORY%\tiles_buffered\%TILE_BASE_NAME%*_128.laz ^
        -refine_tiles 15000000 ^
        -flag_as_withheld ^
        -olaz ^
        -cores %NUM_CORES%
        
:: refine adaptive tiling (refine too large 64 by 64 meter into 32 by 32 tiles)

lastile -i %WORK_DIRECTORY%\tiles_buffered\%TILE_BASE_NAME%*_64.laz ^
        -refine_tiles 15000000 ^
        -flag_as_withheld ^
        -olaz ^
        -cores %NUM_CORES%

:: refine adaptive tiling (refine too large 32 by 32 meter into 16 by 16 tiles)

lastile -i %WORK_DIRECTORY%\tiles_buffered\%TILE_BASE_NAME%*_32.laz ^
        -refine_tiles 15000000 ^
        -flag_as_withheld ^
        -olaz ^
        -cores %NUM_CORES%

:: refine adaptive tiling (refine too large 16 by 16 meter into 8 by 8 tiles)

lastile -i %WORK_DIRECTORY%\tiles_buffered\%TILE_BASE_NAME%*_16.laz ^
        -refine_tiles 15000000 ^
        -flag_as_withheld ^
        -olaz ^
        -cores %NUM_CORES%

:: pre-classification based on height above trajectory

rmdir %WORK_DIRECTORY%\tiles_trajectory_coded /s /q
mkdir %WORK_DIRECTORY%\tiles_trajectory_coded
lastrack -i %WORK_DIRECTORY%\tiles_buffered\%TILE_BASE_NAME%*.laz ^
         -track %WORK_DIRECTORY%\trajectory.laz ^
         -offset -2 ^
         -classify_below -0.3 7 ^
         -classify_between -0.3 0.3 8 ^
         -classify_between 0.3 1.3 3 ^
         -classify_between 1.3 2 9 ^
         -classify_between 2 50 5 ^
         -classify_above 50 7 ^
         -odir %WORK_DIRECTORY%\tiles_trajectory_coded -olaz ^
         -cores %NUM_CORES%

:: mark most central point from class 8 per 2 by 2 cm cell as class 20 

rmdir %WORK_DIRECTORY%\tiles_thinned_preground /s /q
mkdir %WORK_DIRECTORY%\tiles_thinned_preground
lasthin -i %WORK_DIRECTORY%\tiles_trajectory_coded\%TILE_BASE_NAME%*.laz ^
        -ignore_class 3 5 7 9 ^
        -classify_as 20 -step 0.02 -central ^
        -odir %WORK_DIRECTORY%\tiles_thinned_preground -olaz ^
        -cores %NUM_CORES%

:: remove isolated noise points from class 20 and store result in a tiny LAY file

lasnoise -i %WORK_DIRECTORY%\tiles_thinned_preground\%TILE_BASE_NAME%*.laz ^
        -ignore_class 3 5 7 9 ^
        -step_xy 0.5 -step_z 0.1 -isolated 10 ^
        -olay ^
        -cores %NUM_CORES%

:: classify only points of class 20 into (2) ground and (1) non-ground (also use LAY files from last step)

rmdir %WORK_DIRECTORY%\tiles_ground /s /q
mkdir %WORK_DIRECTORY%\tiles_ground
lasground -i %WORK_DIRECTORY%\tiles_thinned_preground\%TILE_BASE_NAME%*.laz -ilay ^
          -ignore_class 3 5 7 8 9 ^
          -step 1 -bulge 0.1 -spike 0.2 -offset 0.02 -fine -all_returns -compute_height ^
          -odir %WORK_DIRECTORY%\tiles_ground -olaz ^
          -cores %NUM_CORES%

:: create output DTM

rmdir %WORK_DIRECTORY%\tiles_dtm /s /q
mkdir %WORK_DIRECTORY%\tiles_dtm
las2dem -i %WORK_DIRECTORY%\tiles_ground\%TILE_BASE_NAME%*.laz ^
        -keep_class 2 ^
        -step 0.05 -use_tile_bb ^
        -odir %WORK_DIRECTORY%\tiles_dtm -obil ^
        -cores %NUM_CORES%

:: remove buffer points prior to publishing point cloud on server

rmdir %WORK_DIRECTORY%\tiles_final /s /q
mkdir %WORK_DIRECTORY%\tiles_final
lastile -i %WORK_DIRECTORY%\tiles_ground\%TILE_BASE_NAME%*.laz ^
        -set_user_data 0 ^
        -remove_buffer ^
        -odir %WORK_DIRECTORY%\tiles_final -olaz ^
        -cores %NUM_CORES%

::optimize for compression and indexing

rmdir %WORK_DIRECTORY%\tiles_optimized /s /q
mkdir %WORK_DIRECTORY%\tiles_optimized
lasoptimize -i %WORK_DIRECTORY%\tiles_final\%TILE_BASE_NAME%*.laz ^
            -odir %WORK_DIRECTORY%\tiles_optimized -olaz ^
            -cores %NUM_CORES%

::publish with Potree in 3D on the Web

rmdir %WORK_DIRECTORY%\portal /s /q
mkdir %WORK_DIRECTORY%\portal
laspublish -i %WORK_DIRECTORY%\tiles_optimized\%TILE_BASE_NAME%*.laz ^
           -only_3D -elevation -overwrite ^
           -title "My Cool Portal" ^
           -description "This is so cool!" ^
           -odir %WORK_DIRECTORY%\portal -o portal.html -olaz

::stop here

GOTO:EOF

::other unused stuff

lasview -i "f:\Rail_noise_2\Noise_at_new_rec - Scanner 1 - 170512_131454.laz" ^
        -drop_intensity_above 0 -keep_first_of_many -filtered_transform -set_classification 7 
        
laslayers -i "f:\Rail_noise_2\Noise_at_new_rec - Scanner 1 - 170512_131454.laz" ^
        -drop_intensity_above 0 -keep_first_of_many -filtered_transform -set_classification 7 -olay
