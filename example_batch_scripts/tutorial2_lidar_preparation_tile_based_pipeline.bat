::
:: a batch script for converting raw flight lines (not tiled) into
:: a number of products with a tile-based multi-core batch pipeline
::

:: include LAStools in PATH to allow running script from anywhere

set PATH=%PATH%;..;

:: specify the number of cores to use

set NUM_CORES=1

:: create clean folder for the raw tiles with buffer

rmdir .\tiles_raw /s /q
mkdir .\tiles_raw

:: use lastile to create a buffered tiling from the original
:: flight strips. the flag '-files_are_flightlines' assures
:: that points from different flight lines will get a unique
:: flight lines ID stored in the 'point source ID' attribute
:: that makes it possible to later identify from which points
:: belong to the same flight strip. we use '-tile_size 1000'
:: to specify the tile size and request a buffer of 50 meters
:: around every tile with '-buffer 50'. this buffer helps to
:: reduce edge artifacts at tile boundaries in a tile-based
:: processing pipeline. we shift the coordinate plane tiling
:: by 920 in x and 320 in y so that the flight strips fit in
:: exactly 4 tiles (experimentally discovered). the '-olaz'
:: flag requests compressed output tiles to overcome the I/O
:: bottleneck.
:: NOTE: usually you won't include option '-tile_ll 920 320'

lastile -i strips_raw\*.laz -files_are_flightlines ^
        -tile_size 1000 -buffer 50 -tile_ll 920 320 ^
        -o tiles_raw\tile.laz -olaz

:: create clean folder for the ground-classified tiles

rmdir .\tiles_ground /s /q
mkdir .\tiles_ground

:: use lasground to find the bare-earth points in all tiles
:: with the '-metro' setting (which uses a step of 50m)
:: and '-extra_fine' setting for the initial ground estimate
:: (see: lasground_README.txt). the '-odir tiles_ground -olaz'
:: parameters specify to store the ground-classified tiles
:: compressed and with the same name to the 'tiles_ground'
:: folder. if we have multiple tiles then this process runs
:: on as many cores as specified by the %NUM_CORES% set above.

lasground -i tiles_raw\*.laz ^
          -metro -extra_fine ^
          -odir tiles_ground -olaz ^
          -cores %NUM_CORES%

::
:: NOTE: if the only objective is to create bare-earth DTM rasters
:: (as it may well be the case in archeological applications) we
:: can skip from here straight to the last step where the buffers
:: are stripped off each tile
::

:: create clean folder for the denoised tiles

rmdir .\tiles_denoised /s /q
mkdir .\tiles_denoised

:: use lasheight to remove low and high outliers that are often
:: just noise (e.g. clouds or birds). by default lasheight uses
:: the points classified as ground to construct a TIN and then
:: calculates the height of all other points in respect to this
:: ground surface TIN. with '-drop_above 40 -drop_below -3' all
:: points that are 40 meters above the ground or 3 meters below
:: the ground are removed from the output LAZ tiles that are to
:: be stored in the 'tiles_denoised' folder. if we have multiple
:: input files this process runs on %NUM_CORES% many cores.

lasheight -i tiles_ground\*.laz ^
          -drop_above 40 -drop_below -3 ^
          -odir tiles_denoised -olaz ^
          -cores %NUM_CORES%

:: create clean folder for the classified tiles

rmdir .\tiles_classified /s /q
mkdir .\tiles_classified

:: use lasclassify to identify buildings and trees in all denoised
:: tiles. your milage may vary on this step because automatic LiDAR
:: classification is a hard problem. all the default settings are
:: used (see: lasclassify_README.txt).

lasclassify -i tiles_denoised\*.laz ^
            -odir tiles_classified -olaz ^
            -cores %NUM_CORES%

:: create clean folder for the final tiles (stripped of the buffer)

rmdir .\tiles_final /s /q
mkdir .\tiles_final

:: use lastile to remove the buffer from the classified tiles which
:: is requested with the option '-remove_buffer'.

lastile -i tiles_classified\*.laz ^
        -remove_buffer ^
        -odir tiles_final -olaz
