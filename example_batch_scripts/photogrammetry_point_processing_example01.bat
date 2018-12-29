::
:: a batch script for converting a photogrammetry points into a
:: number of products with a tile-based multi-core batch pipeline
::

:: add LAStools\bin directory to PATH to run script from anywhere

set PATH=%PATH%;C:\software\LAStools\bin

:: specify the number of cores to use

set NUM_CORES=4

:: create a lasinfo report and a 0.5 m RGB raster forinput LAZ file

rmdir .\1_quality /s /q
mkdir .\1_quality

lasinfo -i 0_photogrammetry\points.laz ^
        -cd ^
        -o 1_quality\fillongley.txt

lasgrid -i 0_photogrammetry\points.laz ^
        -step 0.5 ^
        -rgb ^
        -fill 1 ^
        -o 1_quality\fillongley.png

:: use lastile to create a buffered tiling from the original
:: photogrammetry points of Fillongley. we use '-tile_size 200'
:: to specify the tile size and request a buffer of 30 meters
:: around every tile with '-buffer 30' and '-flag_as_withheld'
:: all the buffer points so they can easily be dropped later.
:: the '-olaz' flag requests LASzip compressed output tiles to
:: lower the I/O bottleneck.

rmdir .\2_tiles_raw /s /q
mkdir .\2_tiles_raw

lastile -i 0_photogrammetry\points.laz ^
        -tile_size 200 -buffer 25 -flag_as_withheld ^
        -o 2_tiles_raw\fillongley.laz -olaz

rmdir .\3_tiles_temp1 /s /q
mkdir .\3_tiles_temp1

:: give the point closest to the 20th elevation percentile per
:: 90 cm by 90 cm cell the classification code 8 (but only do
:: this for cells containing 20 or more points) using lasthin

lasthin -i 2_tiles_raw\fillongley_*.laz ^
        -step 0.9 ^
        -percentile 20 20 ^
        -classify_as 8 ^
        -odir 3_tiles_temp1 -olaz ^
        -cores %NUM_CORES%

:: considering only points with classification code 8 (ignoring
:: those with classification code 0) change to code from 8 to 12
:: for all "overly isolated" points using lasnoise. the check
:: for isolation uses cells of size 200 cm by 200 cm by 50 cm 
:: and marks points in cells whose neighbourhood of 27 cells has
:: only 3 or fewer points in total (see lasnoise_README.txt)

rmdir .\3_tiles_temp2 /s /q
mkdir .\3_tiles_temp2

lasnoise -i 3_tiles_temp1\fillongley_*.laz ^
         -ignore_class 0 ^
         -step_xy 2 -step_z 0.5 -isolated 3 ^
         -classify_as 12 ^
         -odir 3_tiles_temp2 -olaz ^
         -cores %NUM_CORES%

:: considering only the surviving points with classification
:: code 8 (ignoring those with classification code 0 or 12)
:: change their classification code from 8 either to ground (2)
:: or to non-ground (1) using lasground. the temporary ground
:: surface defined by the resulting ground points will be used
:: to classify points below it as noise in the next step.

rmdir .\3_tiles_temp3 /s /q
mkdir .\3_tiles_temp3

lasground -i 3_tiles_temp2\fillongley_*.laz ^
          -ignore_class 0 12 ^
          -town -ultra_fine ^
          -odir 3_tiles_temp3 -olaz ^
          -cores %NUM_CORES%

:: classify all points that are 20 cm or more below the surface
:: that results from Delaunay triangulating the temporary ground
:: points as noise (7) and all others as unclassified (1)

rmdir .\4_tiles_denoised /s /q
mkdir .\4_tiles_denoised

lasheight -i 3_tiles_temp3\fillongley_*.laz ^
          -classify_below -0.2 7 ^
          -classify_above -0.2 1 ^
          -odir 4_tiles_denoised -olaz ^
          -cores %NUM_CORES%

:: classify the lowest points per 25 cm by 25 cm cell that is *not*
:: noise (i.e. classification other than 7) as 8 using lasthin 

rmdir .\5_tiles_thinned_lowest /s /q
mkdir .\5_tiles_thinned_lowest

lasthin -i 4_tiles_denoised\fillongley_*.laz ^
        -ignore_class 7 ^
        -step 0.25 ^
        -lowest ^
        -classify_as 8 ^
        -odir 5_tiles_thinned_lowest -olaz ^
        -cores %NUM_CORES%

:: classify points considering only the points with classification code 8 
:: (i.e. ignore classification 1 and 7) into ground (2) and non-ground (1) 
:: points using lasground with options '-town -extra_fine -bulge 0.1' 

rmdir .\6_tiles_ground /s /q
mkdir .\6_tiles_ground

lasground -i 5_tiles_thinned_lowest\fillongley_*.laz ^
          -ignore_class 1 7 ^
          -town -extra_fine -bulge 0.1 ^
          -odir 6_tiles_ground -olaz ^
          -cores %NUM_CORES%

:: interpolate points classified as 2 into a TIN and raster a 25 cm DTM
:: but cutting out only the center 200 meter by 200 meter tile but not
:: rasterizing the buffers. the DTM raster is stored as gridded LAZ for
:: maximal compression

rmdir .\7_tiles_dtm /s /q
mkdir .\7_tiles_dtm

las2dem -i 6_tiles_ground\fillongley_*.laz ^
        -keep_class 2 ^
        -step 0.25 ^
        -use_tile_bb ^
        -odir 7_tiles_dtm -olaz ^
        -cores %NUM_CORES%

:: we merge the gridded LAZ files for the DTM into one input and create
:: a 25cm hillshaded DTM raster in PNG format

blast2dem -i 7_tiles_dtm\fillongley_*.laz -merged ^
          -hillshade ^
          -step 0.25 ^
          -o dtm_hillshaded.png

:: the highest points per 25 cm by 25 cm cell that is *not* a noise point
:: (i.e. classification other than 7) is classified as 8 with lasthin 

rmdir .\8_tiles_thinned_highest /s /q
mkdir .\8_tiles_thinned_highest

lasthin -i 4_tiles_denoised\fillongley_*.laz ^
        -ignore_class 7 ^
        -step 0.25 ^
        -highest ^
        -classify_as 8 ^
        -odir 8_tiles_thinned_highest -olaz ^
        -cores %NUM_CORES%

:: interpolate points classified as 8 into a TIN and raster a 25 cm DSM
:: but cutting out only the center 200 meter by 200 meter tile but not
:: rasterizing the buffers. the DSM raster is stored as gridded LAZ for
:: maximal compression

rmdir .\9_tiles_dsm /s /q
mkdir .\9_tiles_dsm

las2dem -i 8_tiles_thinned_highest\fillongley_*.laz ^
        -keep_class 8 ^
        -step 0.25 ^
        -use_tile_bb ^
        -odir 9_tiles_dsm -olaz ^
        -cores %NUM_CORES%

:: we merge the gridded LAZ files for the DSM into one input and create
:: a 25cm hillshaded DSM raster in PNG format

blast2dem -i 9_tiles_dsm\fillongley_*.laz -merged ^
          -hillshade ^
          -step 0.25 ^
          -o dsm_hillshaded.png
