::
:: a batch script for quality checking raw flight lines
::

:: include LAStools in PATH to allow running script from here

set PATH=%PATH%;..;

:: create clean folder for the output files of quality checking

rmdir .\quality /s /q
mkdir .\quality

:: run lasinfo on the raw flight lines on 4 cores. the parameter
:: '-compute_density' instructs lasinfo to compute an estimate for
:: the density of the points. the parameters '-odir quality' and
:: '-otxt' tell lasinfo to output the result as simple ASCII files
:: with the same file name but an '*.txt' ending into the 'quality'
:: subfolder

lasinfo -i strips_raw\*.laz ^
        -compute_density ^
        -odir quality -otxt ^
        -cores 4

:: look for warnings in the lasinfo output. this is a DOS command
:: that parses all the generated ASCII files and outputs any line
:: containing the word 'WARNING' - an indication that lasinfo found
:: something in the LAZ file that is not specification conform.

find /N "WARNING" quality\*.txt

:: report points density from the lasinfo output. another DOS command
:: that parses the generated ASCII files. this time it outputs for
:: each file the line that reports the 'point density'.

find /N "point density" quality\*.txt

:: same as above for the point 'spacing'.

find /N "spacing" quality\*.txt

:: create visualizations of the overlap and the vertical alignment of
:: flight lines. a vertical difference of more than 1 meter is mapped
:: to a saturated red (or blue) color and white is perfect alignment.
:: the '-files_are_flightlines' flag tells lasoverlap that it should
:: disregard the 'point source ID' of each point (which usually stores
:: the flight line ID) and assigne a different unique ID to the points
:: of each file. the '-max_diff 1.0' parameter is only meaningful when
:: the vertical difference are mapped to colors (the default mode of
:: lasverlap). A positive (negative) difference of 1.0 or higher will
:: be mapped to saturated red (blue) while a difference of zero shows
:: up as white. The '-step 1' specifies the resolution at which the
:: overlap rasters are computed. The step should be 2-3 times larger
:: than the point spacing.

lasoverlap -i strips_raw\*.laz -files_are_flightlines ^
           -max_diff 1.0 -step 1 ^
           -o quality\lasoverlap.png

:: create density grid visualizations for 5 pts/m^2 and 10 pts/m^2.
:: the '-merged' flag tells lasgrid to merge all LAZ files on-the-fly
:: into a larger LAZ file and treat it as one. the '-last_only' flag
:: keeps only last returns to assure we count only one point per laser
:: pulse.  The '-step 1' parameter specifies the resolution at which
:: the density rasters are computed. The '-false' flag tell lasgrid
:: to map the counters to false colors. The '-set_min_max 0 5' (or
:: '-set_min_max 0 10') specify that a point counter of 0 is mapped to
:: blue and a point counter of 5 (or 10) or higher is mapped to red.

lasgrid -i strips_raw\*.laz -merged -last_only ^
        -counter -step 1 -false -set_min_max 0 5 ^
        -o quality\density_5_pts.png

lasgrid -i strips_raw\*.laz -merged -last_only ^
        -counter -step 1 -false -set_min_max 0 10 ^
        -o quality\density_10_pts.png

:: compute boundary around merged flight lines (and look for holes)

:: creates a polygonal outline of the spatial extend of the points
:: in each flight line. the '-last_only' flag means we only use last
:: returns to compute this outline and the '-thin_with_grid 2' flag
:: eases the computational burden by keeping only one point for each
:: 2 by 2 cell. the '-concavity 10' flag instructs lasboundary to 
:: report details of up to 10 meters along the outline and in the
:: presence of '-holes'. The '-disjoint' flag allows lasboundary to
:: report multiple polygonal outlines in case there are islands of
:: LiDAR points. Finally, the '-odix _boundary' and '-okml' parameter
:: tell LAStools to output each file to the KML format while adding
:: the appendix '_boundary' to the file name. Run on '-cores 4'.

lasboundary -i strips_raw\*.laz ^
            -last_only -thin_with_grid 2 ^
            -concavity 10 -disjoint -holes ^
            -odir quality -odix _boundary -okml ^
            -cores 4

:: same as above after merging all LAZ files into one

lasboundary -i strips_raw\*.laz -merged ^
            -last_only -thin_with_grid 2 ^
            -concavity 10 -disjoint -holes ^
            -o quality\boundary.kml
