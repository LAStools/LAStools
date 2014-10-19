::
:: a batch script for converting classified LiDAR tiles into a
:: number of raster products with a tile-based multi-core batch
:: pipeline
::

:: include LAStools in PATH to allow running script from anywhere

set PATH=%PATH%;..;

:: specify the number of cores to use

set NUM_CORES=1

:: create clean folder for the raster DTMs in ESRI ASCII format

rmdir .\tiles_dtms /s /q
mkdir .\tiles_dtms

:: run las2dem on the ground-classified tiles to create raster DTMs
:: in ESRI ASCII format (*.asc) for each individual tile. important
:: is the '-keep_class 2' flag that activates a filter letting only
:: the points classified as 'ground' through. in addition we use the
:: '-thin_with_grid 0.5' filter to have only one ground point per
:: 0.5m by 0.5m area. this assures that we construct and sample a
:: TIN appropriate for the output resolution of 1.0m by 1.0m that is
:: set by '-step 1.0'. very important is the '-use_tile_bb' parameter
:: that limits rasterizating the TIN to the tile area *without* the
:: buffer added by lastile in the 'tile_based_lidar_preparation.bat'
:: batch script thereby avoiding any potential edge artifacts along 
:: the tile boundaries. the '-odir  tiles_dtms -oasc' flags specify
:: to output the resulting DTMs in ESRI's ASCII format in the folder
:: 'tiles_dtms' using the same name as the LAZ file but with a *.asc
:: extension. if we have multiple tiles then this process will run
:: on as many cores as specified by the %NUM_CORES% set above.

las2dem -i tiles_ground\*.laz ^
        -keep_class 2 -thin_with_grid 0.5 -step 1.0 -use_tile_bb ^
        -odir tiles_dtms -oasc ^
        -cores %NUM_CORES%

:: create clean folder for the hillshaded DTM rasters

rmdir .\tiles_hillshaded_dtms /s /q
mkdir .\tiles_hillshaded_dtms

:: use las2dem to create individual hillshaded DTM rasters from the
:: LiDAR points that were classified as ground in each tile. important
:: again is the '-keep_class 2' flag to only use ground points. with
:: '-thin_with_grid 0.5' we again use only one ground point per 0.5m
:: by 0.5m area. this makes the size of the TIN triangles (that are
:: '-hillshade'd using smooth vertex normals) more appropriate for
:: the output resolution of 1.0m by 1.0m that is set by '-step 1.0'.
:: again, the '-use_tile_bb' parameter omits the buffer that was added
:: by lastile from rasterization to avoid artifacts along the tile
:: boundaries. the '-odir tiles_hillshaded_dtms -opng' flags specify
:: to output the resulting hillshades in PNG format in the folder
:: 'tiles_hillshaded_dtms' using the same file name but with a *.png
:: extension. if we have multiple tiles then this process will run
:: on as many cores as specified by the %NUM_CORES% set above.

las2dem -i tiles_ground\*.laz ^
        -keep_class 2 -thin_with_grid 0.5 -step 1.0 -use_tile_bb -hillshade ^
        -odir tiles_hillshaded_dtms -opng ^
        -cores %NUM_CORES%

:: use las2dem to create a single hillshaded DTM raster from on-the
:: fly merged ASC rasters. all LAStools can read ASC (and BIL) rasters
:: that are converted into simple xyz points. the '-merged' flag lets
:: multiple files appear as a single LAS input. the output resolution
:: is specified by '-step 1.0' and '-hillshade' means that at each TIN
:: vertex a smooth normal is computed that - together with the default
:: light direction (0.5 0.5 1.0) - generates the shading.

las2dem -i tiles_dtms\*.asc -merged ^
        -step 1.0 -hillshade ^
        -o hillshaded_dtm.png

:: same as before but with a different light direction (1.0 0.0 0.5)

las2dem -i tiles_dtms\*.asc -merged ^
        -step 1.0 -hillshade -light 1.0 0.0 0.5 ^
        -o hillshaded_dtm1.png

:: same as before but with a different light direction (1.0 1.0 0.1)

las2dem -i tiles_dtms\*.asc -merged ^
        -step 1.0 -hillshade -light 1.0 1.0 0.1 ^
        -o hillshaded_dtm2.png

:: same as before but with a different light direction (0.0 1.0 0.2)

las2dem -i tiles_dtms\*.asc -merged ^
        -step 1.0 -hillshade -light 0.0 1.0 0.2 ^
        -o hillshaded_dtm3.png

:: same as before but using the actual LiDAR ground points from final
:: tiles (i.e. those without buffer) with an additional '-extra_pass'
:: flag that does an initial pass over the input to count the number
:: of points that survive the filtering for optimal memory management.

las2dem -i tiles_final\*.laz -merged ^
        -keep_class 2 -thin_with_grid 0.5 -extra_pass ^
        -step 1.0 -hillshade -light 0.0 1.0 0.2  ^
        -o hillshaded_dtm3_points.png

:: create clean folder for the false-colored slope DTM rasters

rmdir .\tiles_slope_dtms /s /q
mkdir .\tiles_slope_dtms

:: use las2dem to create individual false-colored slope DTM rasters
:: from the LiDAR points of each ground-classified tile. we use the
:: '-keep_class 2 -thin_with_grid 0.5 -step 1.0 -use_tile_bb' flags
:: for the same reasons as before. in addition we specify to compute
:: the '-slope' at when we sample the TIN and map it to a '-false'
:: color that is stored in PNG format. again we spawn the number of
:: specified processes in case there are multiple input files.

las2dem -i tiles_ground\*.laz ^
        -keep_class 2 -thin_with_grid 0.5 -step 1.0 -use_tile_bb -slope -false ^
        -odir tiles_slope_dtms -opng ^
        -cores %NUM_CORES%

:: use las2dem to create a single false-colored slope DTM raster from
:: on-the fly merged ASC rasters. all LAStools can read ASC (and BIL)
:: rasters that are converted into simple xyz points. the '-merged' flag 
:: lets multiple files appear as a single LAS input. the output resolution
:: is specified by '-step 1.0' and '-slope' means that at each TIN vertex
:: a smooth normal is computed to estimate the slope that is interpolated
:: across the sampled TIN facets. The '-false' flag indicates that the
:: slope value is mapped to a false coloring that is output in PNG format.

las2dem -i tiles_dtms\*.asc -merged ^
        -step 1.0 -slope -false ^
        -o slope_dtm.png

:: same as before but using the actual LiDAR ground points from final
:: tiles (i.e. those without buffer) with an additional '-extra_pass'
:: flag that does an initial pass over the input to count the number
:: of points that survive the filtering for optimal memory management.

las2dem -i tiles_final\*.laz -merged ^
        -keep_class 2 -thin_with_grid 0.5 -extra_pass ^
        -step 1.0 -slope -false ^
        -o slope_dtm_points.png

:: use las2dem to create a hillshaded DSM raster from the LiDAR points
:: of the '-merged' final tiles. with '-first_only' we keep only the
:: highest (aka the surface) points and with '-thin_with_grid 0.5' we 
:: subset them to an appropriate spacing of TIN vertices for our output
:: resolution of 1.0m by 1.0m.

las2dem -i tiles_final\*.laz -merged ^
        -first_only -thin_with_grid 0.5 -extra_pass ^
        -step 1.0 -hillshade ^
        -o hillshaded_dsm.png
