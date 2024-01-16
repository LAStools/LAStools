# lasoverlap

This tool reads LIDAR points from LAS/LAZ/ASCII/BIN/SHP and checks the 
flight line overlap and / or the vertical and horizontal alignment. 
Do not add  '-files_are_flightlines' or '-faf' to the command-line if the
 input is tiles. Only use these command-line switches when files *are* 
flight lines.   
The most important parameter '-step n' specifies the n x n area that of 
LiDAR points that are gridded on one raster (or pixel) that are then used
 for doing the overlap and/or difference calculations. The output is 
either in BIL, ASC, IMG, TIF, PNG, JPG, XYZ,or DTM format. When computing
 the difference in the flight line overlaps, by default the tool will 
check '-elevation_lowest', but '-intensity_highest', or '-counter' are 
possible too.

By default the tools create visualization images rather than alignment 
difference rasters or overlap counter grids. There are three ways to 
specify how to visualize the differences:

1. Use '-min_diff 0.1' and '-max_diff 0.2' to specify that elevation 
   differences of up to +/- 10 cm are okay (get mapped to white) but 
   differences of over +/- 20 cm are critical (get mapped to red or blue). 
   Differences between min and max increase in saturation shades of red or 
   blue.

2. Use '-bands 0.1 0.2 0.3 0.4' to map differences from 0 up to +/- 10 cm
   to white, up to +/- 20 cm to blue, up to +/- 20 cm to green, up to +/- 
   30 cm to yellow, and above +/- 40 cm to red. 

3. Use with '-color_bands 0.15 0x00FF00 0.4 0xFFA500' to map differences 
   from 0 up to +/- 15 cm to white, up to +/- 40 cm to green, and above +/- 
   40 cm to orange. 

When the input are already tiled flightlines that do not have flight line 
information in the point source ID then the option '-recover_flightlines' 
may be useful that tries to use the GPS time stamp of each point to resolve 
which points are from the same flightline.

Optionally, a KML file is generated that allows the resulting raster  to 
be immediately displayed inside a geospatial context provided by Google 
Earth (for TIF/PNG/JPG images). In case the LAS/LAZ file contains 
projection information (i.e. geo keys as variable length records) this 
metadata is used to correctly geo-reference the KML file. It is also 
possible to provide the proper geo-referencing information in the 
command-line.

Use '-subsample n' with 1 < n < 5 to anti-alias the gridding of LiDAR 
points by their x and y coordinate into disjunct rasters.
The option '-subsample 3' adds each LiDAR point 9 times to the raster at 
locations (x/y), (x+0.33*step/y), (x+0.66*step/y),
(x/y+0.33*step) (x+0.33*step/y+0.33*step) (x+0.66*step/y+0.33*step), 
(x/y+0.66*step) (x+0.33*step/y+0.66*step) (x+0.66*step/y+0.66*step) and 
thereby "washes out" hard boundaries.


## Examples

    lasoverlap64 -i tile.las -step 2 -o overlap.png

creates an overlap raster as well as a difference raster with a step of 2 
units. For this, it is necessary that the LiDAR points in the LAS file 
have their point source ID populated with the flight line ID. The overlap 
raster uses the default color ramp which maps overlap counts from 0 to 5 
to a different colors. The difference raster uses the default color ramp 
that maps blue to -2.5, white to 0, and red to 2.5. The default output 
is PNG.


    lasoverlap64 -i tile.las -step 2 -values -oasc

same as above but output the actual values into an ASC raster (instead of 
the color-coded values into a PNG).


    lasoverlap64 -i tile.las -step 2 -values -oasc -recover_flightlines

same as above for the case the tile does not have proper information in 
the point source ID field but has valid GPS time stamps. The GPS time 
stamps are then used to recover the flight line information by looking 
for continous segments (or rather interruptions) in the GPS time stamps.


    lasoverlap64 -i LDR*.las -files_are_flightlines -step 3 ^
	           -min_diff 0.1 -max_diff 0.4 -o overlap.png

merges all the files "LDR*.las" while assigning the points of each file 
a unique point source ID (aka flight line number), and then creates an 
overlap raster as well as a difference raster with a step of 3 units. 
The overlap raster uses the default range of 5 overlaps lines for the 
color ramp. The difference raster uses '-min_diff 0.1' and '-max_diff 
0.4' which maps the range (-0.4 ... -0.1) to (blue ... white) and the 
range (0.1 ... 0.4) to (white ... red). The range (-0.1 ... 0.1) is 
mapped to white.  


    lasoverlap64 -i LDR*.las -files_are_flightlines -step 3 ^
	           -min_diff 1.0 -max_diff 2.0 -o overlap.png

same as above. But here the difference raster uses '-min_diff 1' and 
'-max_diff 2' to map range (-2 ... -1) to color ramp (blue ... white), 
range (-1 ... +1) to white and (+1 ... +2) to (white ... red).


    lasoverlap64 -i LDR*.las -files_are_flightlines -step 2 ^
	           -max_diff 0.5 -no_over

same as above but does not output an overlap raster ('-no_over') and 
operates with different '-step 2' and '-max_diff 0.5' settings.


    lasoverlap64 -i LDR*.las -files_are_flightlines -step 5 -no_diff

same as above but does not output a difference raster ('-no_diff') but 
only a coarse (-step 5) overlap raster.


    lasoverlap64 -i tiles*.laz -step 2 -no_over -utm 15N

operates tile by tile (e.g. creates one difference raster per file) and 
generates a Google Earth KML file for each tile using UTM 15N. It is 
necessary that the LiDAR points in the LAZ file have their point source 
ID populated.


    lasoverlap64 -i tiles*.laz -step 2 -no_over -utm 15N ^
	           -intensity -highest

same as above but computes intensity differences. For intensities the 
difference raster maps range -255 ... 0 ... 255 (e.g. '-max_diff 255').


    lasoverlap64 -i tiles*.laz -keep_last -step 2 -no_over ^
	           -utm 15N -counter -highest

same as above but computes last return count differences. For counts the 
difference raster maps range -100 ... 0 ... 100 (e.g. '-max_diff 100').


## lasoverlap specific arguments

-average                          : compute per cell averages before checking difference  
-avg                              : compute per cell averages before checking difference  
-bands [w] [b] [g]                : map absolute differences up to [w] to white,  [b] to blue, [g] to green, and above to red  
-color_bands [m] [n]              : see tool description  
-cores [n]                        : process multiple inputs on [n] cores in parallel  
-counter                          : check difference in point counts per overlapping flightline per cell with an 8 bit counter  
-counter_16bit                    : check difference in point counts per overlapping flightline per cell with a 16 bit counter  
-counter_32bit                    : check difference in point counts per overlapping flightline per cell with a 32 bit counter  
-density                          : check difference in point density per overlapping flightline per cell with an 8 bit counter  
-density_16bit                    : check difference in point density per overlapping flightline per cell with a 16 bit counter  
-density_32bit                    : check difference in point density per overlapping flightline per cell with a 32 bit counter  
-elevation                        : check difference in elevation values per overlapping flightline per cell  
-elevation_feet                   : use feet for elevation  
-false                            : map values to colors  
-feet                             : use feet  
-fill [n]                         : fills voids in the grid with a square search radius of [n]  
-gray                             : map values to gray values instead of colors  
-grey                             : map values to gray values instead of colors  
-high                             : use highest value per cell to checking difference  
-highest                          : use highest value per cell to checking difference  
-ilay [n]                         : apply [n] or all LASlayers found in corresponding *.lay file on read  
-ilaydir [n]                      : look for corresponding *.lay file in directory [n]  
-intensity                        : check difference in intensity values per overlapping flightline per cell  
-low                              : use lowest value per cell to checking difference  
-lowest                           : use lowest value per cell to checking difference  
-max                              : use highest value per cell to checking difference  
-max_band [w] [b] [g]             : map absolute differences up to [w] to white,  [b] to blue, [g] to green, and above to red  
-max_diff [n]                     : map differences below -[n] or above +[n] to saturated blue or red  
-max_over [n]                     : use a color ramp with up to [n] different colors  
-mean                             : compute per cell averages before checking difference  
-mem [n]                          : use [n] MB of main memory (500-2000; default=1500)  
-min                              : use lowest value per cell to checking difference  
-min_diff [n]                     : map differences between -[n] and +[n] to white  
-nbits [n]                        : use [n] bits to represent the elevation (mainly used with BIL format)  
-no_diff                          : do not generate difference rasters  
-no_over                          : do not generate overlap rasters  
-nodata [n]                       : use [n] as the nodata value in the BIL/ASC format  
-number_returns                   : check difference in number of returns per overlapping flightline per cell  
-recover_flightlines              : try to recover missing flightline information (for tiles!!!) from GPS time stamps  
-recover_flightlines_interval [n] : look for minimum [n] second gaps in GPS time stamp to recover flightlines  
-scan_angle_abs                   : check difference in absolute scan angle per overlapping flightline per cell  
-step [n]                         : raster with stepsize [n]{default=1}  
-subsample [n]                    : see long explanation above [n]  
-switch_G_B                       : switch green and blue value  
-temp_files [n]                   : set base file name [n] for temp files (example: E:\tmp)  
-use_bb                           : raster full extend of bounding box  
-use_orig_bb                      : only raster extend of original bounding box (for tiles generated with '-buffered 30')  
-use_tile_bb                      : only raster extend of tile bounding box (for tiles generated with lastile)  
-values                           : do not map values to gray or to color tones but output actual difference and overlap values  
-week_to_adjusted [n]             : converts time stamps from GPS week [n] to Adjusted Standard GPS  

### Basics
-cpu64   : start 64 bit executable (instead of default 32 bit executable)  
-fail    : fail if license expired or invalid  
-gui     : start with files loaded into GUI  
-h       : print help output  
-help    : print help output  
-license : show license information  
-v       : verbose output (print extra information)  
-verbose : verbose output (print extra information)  
-version : reports this tool's version number  

## Module arguments

### General
-buffered [n]      : define read or write buffer of size [n]{default=262144}  
-comma_not_point   : use comma instead of point as decimal separator  
-neighbors [n]     : set neighbors filename or wildcard [n]  
-neighbors_lof [n] : set neighbors list of files [fnf]  
-no_data [n]       : use [n] as the nodata value in the BIL / ASC format  
-no_kml            : avoids auto-creation of KML wrapper  
-no_world_file     : avoid world-file for PNG, JPG, TIF and BIL output  
-stored            : use in memory reader  
-unique            : remove duplicate points  

### Color
-clamp_RGB_to_8bit                  : limit RGB values to 8 bit (otherwise: 16 bit)  
-copy_B_into_NIR                    : copy blue color value into NearInfraRed value  
-copy_B_into_intensity              : copy blue color value to intensity  
-copy_B_into_register [n]           : copy blue color value into register [n]  
-copy_G_into_NIR                    : copy green color value into NearInfraRed value  
-copy_G_into_intensity              : copy green color value to intensity  
-copy_G_into_register [n]           : copy green color value into register [n]  
-copy_NIR_into_intensity            : copy NIR into intensity  
-copy_NIR_into_register [n]         : copy NearInfraRed value into register [n]  
-copy_RGB_into_intensity            : copy weighted RGB value to intensity  
-copy_R_into_NIR                    : copy red color value into NearInfraRed value  
-copy_R_into_intensity              : copy red color value to intensity  
-copy_R_into_register [n]           : copy red color value into register [n]  
-copy_attribute_into_B [n]          : copy attribute [n] value into blue  
-copy_attribute_into_G [n]          : copy attribute [n] value into green  
-copy_attribute_into_NIR [n]        : copy attribute [n] value into NIR (NearInfraRed)  
-copy_attribute_into_R [n]          : copy attribute [n] value into red  
-copy_intensity_into_NIR            : copy intensity into NIR (NearInfraRed) value  
-copy_register_into_B [n]           : copy register [n] into blue color value  
-copy_register_into_G [n]           : copy register [n] into green color value  
-copy_register_into_I [n]           : copy register [n] into NearInfraRed value  
-copy_register_into_NIR [n]         : copy register [n] into NearInfraRed value  
-copy_register_into_R [n]           : copy register [n] into red color value  
-drop_RGB_green [min] [max]         : drop points with green color value between [min] and [max]  
-drop_RGB_red [min] [max]           : drop points with red color value between [min] and [max]  
-force_RGB                          : force the use of the RGB value even if the point format does not support RGB  
-keep_NDVI_from_CIR [min] [max]     : keep NDVI (Normalized Difference Vegetation Index) from CIR between [min] [max]  
-keep_NDVI_green_is_NIR [min] [max] : keep NDVI (Normalized Difference Vegetation Index) where green is NIR between [min] [max]  
-keep_NDVI_intensity_is_NIR [min] [max]: keep NDVI (Normalized Difference Vegetation Index) where intensity is NIR between [min] [max]  
-keep_RGB_blue [m] [n]              : keep points with RGB blue color values between [min] [max]  
-keep_RGB_green [min] [max]         : keep points with green color value between [min] and [max]  
-keep_RGB_greenness [m] [n]         : keep points with RGB greenness values between [min] [max]  
-keep_RGB_nir [m] [n]               : keep points with RGB NIR values between [min] [max]  
-keep_RGB_red [min] [max]           : keep points with red color value between [min] and [max]  
-map_attribute_into_RGB [a] [fnm]   : map attribute [a] by table in file [fnm] to RGB values  
-scale_NIR [n]                      : scale NearInfraRed value by factor [n]  
-scale_NIR_down                     : scale NearInfraRed value down by 256  
-scale_NIR_to_16bit                 : scale 8 bit NearInfraRed value to 16 bit  
-scale_NIR_to_8bit                  : scale 16 bit NearInfraRed value downto 8 bit  
-scale_NIR_up                       : scale NearInfraRed value up by 256  
-scale_RGB [r] [g] [b]              : scale RGB values by factors in [r][g][b]  
-scale_RGB_down                     : scale RGB color values down by 256  
-scale_RGB_to_16bit                 : scale 8 bit color values to 16 bit  
-scale_RGB_to_8bit                  : scale 16 bit color values downto 8 bit  
-scale_RGB_up                       : scale RGB values from 8 bit up to 16 bit (multiply with 256)  
-scale_rgb_down                     : divides all RGB values by 256 (to go from 16 bit to 8 bit numbers)  
-scale_rgb_up                       : multiplies all RGB values by 256 (to go from 8 bit to 16 bit numbers)  
-set_NIR [n]                        : set NearInfraRed value to [n]  
-set_RGB [r] [g] [b]                : set color to [r] [g] [b]  
-set_RGB_of_class [c] [r] [g] [b]   : set RGB values of class [c] to [r][g][b] (8 or 16 bit)  
-switch_RGBI_into_CIR               : set R to NIR; G to R; B to G  
-switch_RGB_intensity_into_CIR      : set R to intensity; G to R; B to G  
-switch_R_B                         : switch red and blue color value  
-switch_R_G                         : switch red and green color value  

### Coordinates
-add_attribute_to_z [n]             : add value of attribute [n] to z value  
-add_scaled_attribute_to_z [m] [n]  : scale attribute [m] value by [n] and add to z value  
-auto_reoffset                      : puts a reasonable offset in the header and translates the points accordingly  
-bin_Z_into_point_source [n]        : set point source to z/[n]  
-clamp_raw_z [min] [max]            : limit raw z values to [min] and [max]  
-clamp_z [min] [max]                : limit z values to [min] and [max]  
-clamp_z_above [n]                  : limit z values to maximal [n]  
-clamp_z_below [n]                  : limit z values to minimal [n]  
-classify_z_above_as [m] [n]        : for z value above [m] set class to [n]  
-classify_z_below_as [m] [n]        : for z value below [m] set class to [n]  
-classify_z_between_as [m] [n] [o]  : for z value between [m] and [n] set class to [o]  
-copy_attribute_into_x [n]          : copy attribute [n] value into x  
-copy_attribute_into_y [n]          : copy attribute [n] value into y  
-copy_attribute_into_z [n]          : copy attribute [n] value into z  
-copy_intensity_into_z              : copy intensity to z value  
-copy_register_into_x [n]           : copy register [n] to x value  
-copy_register_into_y [n]           : copy register [n] to y value  
-copy_register_into_z [n]           : copy register [n] to z value  
-copy_user_data_into_z              : copy user data into z  
-copy_z_into_attribute [n]          : copy z value into attribute [n] value  
-drop_x [m] [n]                     : drop points with x value between [m] and [n]  
-drop_x_above [n]                   : drop points with x value above [n]  
-drop_x_below [n]                   : drop points with x value below [n]  
-drop_xy [x1] [y1] [x2] [y2]        : drop points within the [x1] [y1] [x2] [y2] rectangle  
-drop_xyz [x1] [y1] [z1] [x2] [y2] [z2]: drop points within the given cube dimensions  
-drop_y [m] [n]                     : drop points with y value between [m] and [n]  
-drop_y_above [n]                   : drop points with y value above [n]  
-drop_y_below [n]                   : drop points with y value below [n]  
-drop_z [m] [n]                     : drop points with z value between [m] and [n]  
-drop_z_above [n]                   : drop points with z value above [n]  
-drop_z_below [n]                   : drop points with z value below [n]  
-inside [x1] [y1] [x2] [y2]         : use only points within the [x1] [y1] [x2] [y2] rectangle  
-inside_circle [x] [y] [r]          : keep circle at pos [x] [y] with radius [r]  
-inside_rectangle [x1] [y1] [x2] [y2]: use only points within the [x1] [y1] [x2] [y2] rectangle  
-inside_tile [m] [n] [o]            : use only points inside tile at lower-left [x] [y] with size [s]  
-keep_circle [x] [y] [r]            : keep circle at pos [x] [y] with radius [r]  
-keep_profile [x1] [y1] [x2] [y2] [w]: keep profile with [x1] [y1] [x2] [y2] [w]  
-keep_tile [x] [y] [size]           : keep tile at lower-left [x] [y] with size [s]  
-keep_x [m] [n]                     : keep points with x value between [m] and [n]  
-keep_xy [x1] [y1] [x2] [y2]        : keep points within the [x1] [y1] [x2] [y2] rectangle  
-keep_xyz [x1] [y1] [z1] [x2] [y2] [z2]: keep points within the given cube dimensions  
-keep_y [m] [n]                     : keep points with y value between [m] and [n]  
-keep_z [m] [n]                     : keep points with z value between [m] and [n]  
-keep_z_above [n]                   : keep points with z value above [n]  
-keep_z_below [n]                   : keep points with z value below [n]  
-reoffset [x] [y] [z]               : puts a new offset [x] [y] [z] into the header and translates the points accordingly  
-rescale [x] [y] [z]                : puts a new scale [x] [y] [z] into the header and rescales the points accordingly  
-rescale_xy [x] [y]                 : rescale x y by [x] [y]  
-rescale_z [z]                      : rescale z by [z]  
-rotate_xy [a] [x] [y]              : rotate points by [a] degrees, center at [x] [y]  
-rotate_xz [a] [x] [z]              : rotate points by [a] degrees, center at [x] [z]  
-rotate_yz [a] [y] [z]              : rotate points by [a] degrees, center at [y] [z]  
-scale_x [n]                        : scale x value by [n]  
-scale_xyz [m] [n] [o]              : scale xyz values by [m] [n] [o]  
-scale_y [n]                        : scale y value by [n]  
-scale_z [n]                        : scale z value by [n]  
-switch_x_y                         : exchange x and y value  
-switch_x_z                         : exchange x and z value  
-switch_y_z                         : exchange z and x value  
-transform_affine [a],[b],[c],[d]   : transform input using affine transformation with [a],[b],[c],[d]  
-transform_helmert [m] [n] [o]      : do a helmert transformation with 3 or 7 comma separated parameters [n] ...  
-transform_matrix [r11,r12,r13] [r21,r22,r23] [r31,r32,r33] [tr1,tr2,tr3]: transform input using matrix [r11,r12,r13] [r21,r22,r23] [r31,r32,r33] [tr1,tr2,tr3]  
-translate_raw_x [n]                : translate raw x value by [n]  
-translate_raw_xy_at_random [x] [y] : translate raw xy values by random and max offset of [x] [y]  
-translate_raw_xyz [x] [y] [z]      : translate raw coordinates by [x] [y] [z]  
-translate_raw_y [n]                : translate raw y value by [n]  
-translate_raw_z [n]                : translate raw z value by [n]  
-translate_then_scale_x [m] [n]     : translate x value by [m] and scale by [n]  
-translate_then_scale_y [m] [n]     : translate y value by [m] and scale by [n]  
-translate_then_scale_z [m] [n]     : translate z value by [m] and scale by [n]  
-translate_x [n]                    : translate y value by [n]  
-translate_xyz [x] [y] [z]          : translate point coordinates by [x] [y] [z]  
-translate_y [n]                    : translate y value by [n]  
-translate_z [n]                    : translate z value by [n]  

### Simple thinning
-drop_every_nth [n]           : drop every [n]th point  
-keep_every_nth [n]           : keep every [n]th point  
-keep_random_fraction [m] [n] : keep points by random fraction [m]{0-1}, optional seed [n]  
-thin_points_with_time [n]    : thin points with time, [n] = timespacing  
-thin_pulses_with_time [n]    : thin pulses with time, [n] = timespacing  
-thin_with_grid [n]           : thin points by min grid size of [n]  
-thin_with_time [n]           : thin pulses with time, [n] = timespacing  

### Return number
-change_extended_number_of_returns_from_to [m] [n]: change extended number of returns from [m] to [n]  
-change_extended_return_number_from_to [m] [n]: change extended return number from [m] to [n]  
-change_number_of_returns_from_to [m] [n]: change number of returns from [m] to [n]  
-change_return_number_from_to [m] [n]: change return number from [m] to [n]  
-drop_double                        : drop double returns  
-drop_first                         : drop first return  
-drop_first_of_many                 : drop first of many returns  
-drop_last                          : drop last return  
-drop_last_of_many                  : drop last of many returns  
-drop_middle                        : drop middle returns  
-drop_number_of_returns [n]         : drop points with [n] number of returns  
-drop_quadruple                     : drop quadruple returns  
-drop_quintuple                     : drop quintuple returns  
-drop_return [m] [n]...             : drop points with return [m] [n]...  
-drop_return_mask [n]               : drop points with return mask [n]  
-drop_second_last                   : drop points with second last return  
-drop_single                        : drop points with single return  
-drop_triple                        : drop points with triple return  
-first_only                         : use first return only  
-keep_double                        : keep double returns  
-keep_first                         : keep first return  
-keep_first_of_many                 : keep first of many returns  
-keep_last                          : keep last return  
-keep_last_of_many                  : keep last of many returns  
-keep_middle                        : keep mittle returns  
-keep_number_of_returns [n]         : keep points with [n] number of returns  
-keep_quadruple                     : keep quadruple returns  
-keep_quintuple                     : keep quintuple returns  
-keep_return [m] [n]...             : keep points with return [m] [n]...  
-keep_return_mask [n]               : keep points with return mask [n]  
-keep_second_last                   : keep points with second last return  
-keep_single                        : keep points with single return  
-keep_triple                        : keep points with triple return  
-last_only                          : use last return only  
-repair_zero_returns                : sets return counts and number of returns that are zero to one  
-set_extended_number_of_returns [n] : set extended number of returns to [n]  
-set_extended_return_number [n]     : set extended return number to [n]  
-set_number_of_returns [n]          : set number of returns to [n]  
-set_return_number [n]              : set return number to [n]  

### Scanline
-drop_scan_direction [n]       : drop points with scan direction [n]  
-faf                           : input files are flightlines. do ***NOT*** use this for tiled input  
-faf_index [n]                 : set files are flightlines index [n]  
-files_are_flightlines         : input files are flightlines. do ***NOT*** use this for tiled input  
-keep_edge_of_flight_line      : keep points with "Edge of Flight Line" flag set  
-keep_scan_direction_change    : keep points with changed scan direction flag  
-set_edge_of_flight_line [0/1] : set "Edge of Flight Line" flag to [0/1]  
-set_scan_direction_flag [0/1] : set scan direction flag to [0/1]  

### Scanner channel
-copy_scanner_channel_into_point_source: copy scanner channel into point_source  
-copy_scanner_channel_into_user_data: copy scanner channel into user data  
-copy_user_data_into_scanner_channel: copy user data into scanner channel  
-drop_scanner_channel [n]           : drop points with scanner channel [n]  
-keep_scanner_channel [n]           : keep points with scanner channel [n]  
-merge_scanner_channel_into_point_source: merge scanner channel to point source  
-set_extended_scanner_channel [n]   : set extended scanner channel to [n]  
-set_scanner_channel [n]            : set scanner channel to [n]  
-split_scanner_channel_from_point_source: split scanner channel from point source and save as extended scanner channel  

### Source ID
-apply_file_source_ID               : copy file source ID to target  
-bin_Z_into_point_source [n]        : set point source to z/[n]  
-bin_abs_scan_angle_into_point_source [n]: set point source to scan_angle/[n]  
-bin_gps_time_into_point_source [n] : set point source to gps/[n]  
-change_point_source_from_to [m] [n]: change point source from [m] to [n]  
-copy_attribute_into_point_source [n]: copy attribute [n] value into point source  
-copy_classification_into_point_source: copy classification to point source  
-copy_point_source_into_register [n]: copy point source into register [n]  
-copy_register_into_point_source [n]: copy register [n] to point source  
-copy_scanner_channel_into_point_source: copy scanner channel into point_source  
-copy_user_data_into_point_source   : copy user data into point source  
-drop_point_source [n]              : drop points with point source [n]  
-drop_point_source_above [n]        : drop points with with point source above [n]  
-drop_point_source_below [n]        : drop points with with point source below [n]  
-drop_point_source_between [m] [n]  : drop points with with point source between [n] and [m]  
-keep_point_source [n]              : keep points with point source [n]  
-keep_point_source_between [m] [n]  : keep points with with point source between [n] and [m]  
-map_point_source [fnm]             : set the point source by map in file [fnm]  
-merge_scanner_channel_into_point_source: merge scanner channel to point source  
-set_point_source [n]               : set point source to [n]  
-split_scanner_channel_from_point_source: split scanner channel from point source and save as extended scanner channel  

### User data
-add_scaled_attribute_to_user_data [m] [n]: scale attribute [m] value by [n] and add to user data  
-change_user_data_from_to [m] [n]   : change user data from [m] to [n]  
-copy_attribute_into_user_data [n]  : copy attribute [n] value into user data field  
-copy_classification_into_user_data : copy classification to user data  
-copy_register_into_user_data [n]   : copy register [n] to user data  
-copy_scanner_channel_into_user_data: copy scanner channel into user data  
-copy_user_data_into_attribute [n]  : copy user data into attribute [n] value  
-copy_user_data_into_classification : copy user data into classification  
-copy_user_data_into_point_source   : copy user data into point source  
-copy_user_data_into_register [n]   : copy user data to register [n]  
-copy_user_data_into_scanner_channel: copy user data into scanner channel  
-copy_user_data_into_z              : copy user data into z  
-drop_user_data [n]                 : drop points with user data value of [n]  
-drop_user_data_above [n]           : drop points with user data value above [n]  
-drop_user_data_below [n]           : drop points with user data value below [n]  
-drop_user_data_between [m] [n]     : drop points with user data between [m] and [n]  
-keep_user_data [n]                 : keep points with user data value of [n]  
-keep_user_data_above [n]           : keep points with user data value above [n]  
-keep_user_data_below [n]           : keep points with user data value below [n]  
-keep_user_data_between [m] [n]     : keep points with user data between [m] and [n]  
-map_user_data [fnm]                : set the user data by map in file [fnm]  
-scale_user_data [n]                : scale user data by [n]  
-set_user_data [n]                  : sets all user_data fields to [n]  

### Classification
-change_class_from_to [m] [n]       : change classification from [m] to [n]  
-change_classification_from_to [m] [n]: change classification from [m] to [n]  
-change_extended_class_from_to [m] [n]: change extended class from [m] to [n]  
-change_extended_classification_from_to [m] [n]: change extended class from [m] to [n]  
-classify_attribute_above_as [m] [n] [o]: for attribute [m] with value above [n] set class to [o]  
-classify_attribute_below_as [m] [n] [o]: for attribute [m] with value below [n] set class to [o]  
-classify_attribute_between_as [m] [n] [o] [p]: for attribute [m] with value between [n] and [o] set class to [p]  
-classify_intensity_above_as [m] [n]: for intensity value above [m] set class to [n]  
-classify_intensity_below_as [m] [n]: for intensity value below [m] set class to [n]  
-classify_intensity_between_as [m] [n] [o]: for intensity value between [m] and [n] set class to [o]  
-classify_z_above_as [m] [n]        : for z value above [m] set class to [n]  
-classify_z_below_as [m] [n]        : for z value below [m] set class to [n]  
-classify_z_between_as [m] [n] [o]  : for z value between [m] and [n] set class to [o]  
-copy_classification_into_point_source: copy classification to point source  
-copy_classification_into_user_data : copy classification to user data  
-copy_intensity_into_classification : copy intensity to classification  
-copy_user_data_into_classification : copy user data into classification  
-drop_class [m] [n] [o]...          : drop points with class in [m][n][o]...  
-drop_classification [m] [n] [o]... : drop points with class in [m][n][o]...  
-drop_classification_mask [n]       : drop points with classification mask matches [n]  
-drop_extended_class [m] [n]...     : drop extended class [m] [n]...  
-drop_extended_classification [n]   : drop points with extended classification [n]  
-drop_extended_classification_mask [a] [b] [c] [d] [e] [f] [g] [h]: drop points with extended classification mask matches [a] [b] [c] [d] [e] [f] [g] [h]  
-keep_class [m] [n] [o]...          : keep points with class in [m][n][o]...  
-keep_classification [m] [n] [o]... : keep points with class in [m][n][o]...  
-keep_classification_mask [n]       : keep points with classification mask matches [n]  
-keep_extended_class [m] [n]...     : keep extended class [m] [n]...  
-keep_extended_classification [n]   : keep points with extended class [n]  
-move_ancient_to_extended_classification: move old data to extended classification  
-set_RGB_of_class [c] [r] [g] [b]   : set RGB values of class [c] to [r][g][b] (8 or 16 bit)  
-set_classification [n]             : set classification to [n]  
-set_extended_classification [n]    : set extended classification to [n]  

### Extra byte
-add_attribute_to_z [n]             : add value of attribute [n] to z value  
-add_scaled_attribute_to_user_data [m] [n]: scale attribute [m] value by [n] and add to user data  
-add_scaled_attribute_to_z [m] [n]  : scale attribute [m] value by [n] and add to z value  
-classify_attribute_above_as [m] [n] [o]: for attribute [m] with value above [n] set class to [o]  
-classify_attribute_below_as [m] [n] [o]: for attribute [m] with value below [n] set class to [o]  
-classify_attribute_between_as [m] [n] [o] [p]: for attribute [m] with value between [n] and [o] set class to [p]  
-copy_attribute_into_B [n]          : copy attribute [n] value into blue  
-copy_attribute_into_G [n]          : copy attribute [n] value into green  
-copy_attribute_into_I [n]          : copy attribute [n] value into intensity  
-copy_attribute_into_NIR [n]        : copy attribute [n] value into NIR (NearInfraRed)  
-copy_attribute_into_R [n]          : copy attribute [n] value into red  
-copy_attribute_into_intensity [n]  : copy attribute [n] value into intensity  
-copy_attribute_into_point_source [n]: copy attribute [n] value into point source  
-copy_attribute_into_register [m] [n]: copy attribute [m] value into register [m]  
-copy_attribute_into_user_data [n]  : copy attribute [n] value into user data field  
-copy_attribute_into_x [n]          : copy attribute [n] value into x  
-copy_attribute_into_y [n]          : copy attribute [n] value into y  
-copy_attribute_into_z [n]          : copy attribute [n] value into z  
-copy_intensity_into_attribute [n]  : copy intensity to attribute [n] value  
-copy_register_into_attribute [m] [n]: copy register [m] to attribute [n] value  
-copy_user_data_into_attribute [n]  : copy user data into attribute [n] value  
-copy_z_into_attribute [n]          : copy z value into attribute [n] value  
-drop_attribute_above [m] [n]       : drop points with attribute [m] value > [n]  
-drop_attribute_below [m] [n]       : drop points with attribute [m] value < [n]  
-drop_attribute_between [m] [n] [o] : drop points with attribute [m] in range [n]...[o]  
-iadd_attribute [m] [n] [o] [p] [q] [r] [s] [t]: adds a new "extra_byte" attribute of data_type [m] name [n] description [o]; optional: scale[p] offset [q] pre_scale [r] pre_offset [s] no_data_value [t]  
-iadd_extra [m] [n] [o] [p] [q] [r] [s] [t]: adds a new "extra_byte" attribute of data_type [m] name [n] description [o]; optional: scale[p] offset [q] pre_scale [r] pre_offset [s] no_data_value [t]  
-keep_attribute_above [m] [n]       : keep points with attribute [m] value > [n]  
-keep_attribute_below [m] [n]       : keep points with attribute [m] value < [n]  
-keep_attribute_between [m] [n] [o] : keep points with attribute [m] in range [n]...[o]  
-load_attribute_from_text [m] [fnt] : load attribute [m] from file [fnt]  
-map_attribute_into_RGB [a] [fnm]   : map attribute [a] by table in file [fnm] to RGB values  
-scale_attribute [m] [n]            : scale attribute [m] by [n]  
-set_attribute [m] [n]              : set attribute [m] with value [n]  
-translate_attribute [m] [n]        : translate attribute [n] by [n]  

### Flags
-drop_keypoint                   : drop points flaged as keypoint  
-drop_overlap                    : drop points flaged as overlap  
-drop_scan_direction [n]         : drop points with scan direction [n]  
-drop_synthetic                  : drop points flaged as synthetic  
-drop_withheld                   : drop points flaged as withheld  
-keep_edge_of_flight_line        : keep points with "Edge of Flight Line" flag set  
-keep_keypoint                   : keep points flaged as keypoint  
-keep_overlap                    : keep points flaged as overlap  
-keep_scan_direction_change      : keep points with changed scan direction flag  
-keep_synthetic                  : keep points flaged as synthetic  
-keep_withheld                   : keep points flaged as withheld  
-set_edge_of_flight_line [0/1]   : set "Edge of Flight Line" flag to [0/1]  
-set_extended_overlap_flag [0/1] : set extended overlap flag to [0/1]  
-set_keypoint_flag [0/1]         : set keypoint flag to [0/1]  
-set_overlap_flag [0/1]          : set overlap flag to [0/1]  
-set_scan_direction_flag [0/1]   : set scan direction flag to [0/1]  
-set_synthetic_flag [0/1]        : set synthetic flag to [0/1]  
-set_withheld_flag [0/1]         : set withheld flag to [0/1]  

### GPS time
-adjusted_to_week                   : converts time stamps from Adjusted Standard GPS to GPS week  
-bin_gps_time_into_intensity [n]    : set intensity time to gps/[n]  
-bin_gps_time_into_point_source [n] : set point source to gps/[n]  
-drop_gps_time_above [n]            : drop points with GPS time above [n]  
-drop_gps_time_below [n]            : drop points with GPS time below [n]  
-drop_gps_time_between [m] [n]      : drop points with GPS time between [m] and [n]  
-drop_gpstime_above [n]             : drop points with GPS time above [n]  
-drop_gpstime_below [n]             : drop points with GPS time below [n]  
-drop_gpstime_between [m] [n]       : drop points with GPS time between [m] and [n]  
-keep_gps_time [m] [n]              : keep points with GPS time between [m] and [n]  
-keep_gps_time_above [n]            : keep points with GPS time above [n]  
-keep_gps_time_below [n]            : keep points with GPS time below [n]  
-keep_gps_time_between [m] [n]      : keep points with GPS time between [m] and [n]  
-keep_gpstime [m] [n]               : keep points with GPS time between [m] and [n]  
-keep_gpstime_above [n]             : keep points with GPS time above [n]  
-keep_gpstime_below [n]             : keep points with GPS time below [n]  
-keep_gpstime_between [m] [n]       : keep points with GPS time between [m] and [n]  
-set_gps_time [n]                   : set gps time to [n]  
-translate_gps_time [n]             : translate GPS time by [n]  

### Intensity
-bin_gps_time_into_intensity [n]    : set intensity time to gps/[n]  
-clamp_intensity [min] [max]        : limit intensity values to [min] and [max]  
-clamp_intensity_above [max]        : limit intensity values to maximal [max]  
-clamp_intensity_below [max]        : limit intensity values to minimal [min]  
-classify_intensity_above_as [m] [n]: for intensity value above [m] set class to [n]  
-classify_intensity_below_as [m] [n]: for intensity value below [m] set class to [n]  
-classify_intensity_between_as [m] [n] [o]: for intensity value between [m] and [n] set class to [o]  
-copy_B_into_intensity              : copy blue color value to intensity  
-copy_G_into_intensity              : copy green color value to intensity  
-copy_NIR_into_intensity            : copy NIR into intensity  
-copy_RGB_into_intensity            : copy weighted RGB value to intensity  
-copy_R_into_intensity              : copy red color value to intensity  
-copy_attribute_into_I [n]          : copy attribute [n] value into intensity  
-copy_attribute_into_intensity [n]  : copy attribute [n] value into intensity  
-copy_intensity_into_NIR            : copy intensity into NIR (NearInfraRed) value  
-copy_intensity_into_attribute [n]  : copy intensity to attribute [n] value  
-copy_intensity_into_classification : copy intensity to classification  
-copy_intensity_into_register [n]   : copy color intensitiy value into register [n]  
-copy_intensity_into_z              : copy intensity to z value  
-copy_register_into_intensity [n]   : copy register [n] into point intensitiy value  
-drop_intensity_above [n]           : drop points with intensity value above [n]  
-drop_intensity_below [n]           : drop points with intensity value below [n]  
-drop_intensity_between [m] [n]     : drop points with intensity value between [m] and [n]  
-iscale_intensity [n]               : scale intensity value by [n]  
-itranslate_intensity [n]           : translate input intensity by [n]  
-keep_NDVI_intensity_is_NIR [min] [max]: keep NDVI (Normalized Difference Vegetation Index) where intensity is NIR between [min] [max]  
-keep_intensity [m] [n]             : keep points with intensity between [m] and [n]  
-keep_intensity_above [n]           : keep points with intensity value above [n]  
-keep_intensity_below [n]           : keep points with intensity value below [n]  
-map_intensity [fnm]                : set the intensity by map in file [fnm]  
-scale_intensity [n]                : multiply intensity by [n]  
-set_intensity [n]                  : set intensity to [n]  
-switch_RGB_intensity_into_CIR      : set R to intensity; G to R; B to G  
-translate_intensity [n]            : translate intensity by [n]  
-translate_then_scale_intensity [m] [n]: translate intensity by [m] and scale by [n]  

### Raw point values
-clamp_raw_z [min] [max]            : limit raw z values to [min] and [max]  
-translate_raw_x [n]                : translate raw x value by [n]  
-translate_raw_xy_at_random [x] [y] : translate raw xy values by random and max offset of [x] [y]  
-translate_raw_xyz [x] [y] [z]      : translate raw coordinates by [x] [y] [z]  
-translate_raw_y [n]                : translate raw y value by [n]  
-translate_raw_z [n]                : translate raw z value by [n]  

### Registers
-add_registers [m] [n] [o]          : add register [m] and [n] and store result in register [o]  
-copy_B_into_register [n]           : copy blue color value into register [n]  
-copy_G_into_register [n]           : copy green color value into register [n]  
-copy_NIR_into_register [n]         : copy NearInfraRed value into register [n]  
-copy_R_into_register [n]           : copy red color value into register [n]  
-copy_attribute_into_register [m] [n]: copy attribute [m] value into register [m]  
-copy_intensity_into_register [n]   : copy color intensitiy value into register [n]  
-copy_point_source_into_register [n]: copy point source into register [n]  
-copy_register_into_B [n]           : copy register [n] into blue color value  
-copy_register_into_G [n]           : copy register [n] into green color value  
-copy_register_into_I [n]           : copy register [n] into NearInfraRed value  
-copy_register_into_NIR [n]         : copy register [n] into NearInfraRed value  
-copy_register_into_R [n]           : copy register [n] into red color value  
-copy_register_into_attribute [m] [n]: copy register [m] to attribute [n] value  
-copy_register_into_intensity [n]   : copy register [n] into point intensitiy value  
-copy_register_into_point_source [n]: copy register [n] to point source  
-copy_register_into_user_data [n]   : copy register [n] to user data  
-copy_register_into_x [n]           : copy register [n] to x value  
-copy_register_into_y [n]           : copy register [n] to y value  
-copy_register_into_z [n]           : copy register [n] to z value  
-copy_user_data_into_register [n]   : copy user data to register [n]  
-divide_registers [m] [n] [o]       : divide register [m] by register [n] and store result in register [o]  
-multiply_registers [m] [n] [o]     : Multiply register [m] with [n] and store result in register [o]  
-scale_register [m] [n]             : scale register index [m] with factor [n]  
-set_register [m] [n]               : set register [m] with value [n]  
-subtract_registers [m] [n] [o]     : subtract register [m] by register [n] and store result in register [o]  
-translate_register [m] [n]         : translate register index [m] value by [n]  

### Scan angle
-bin_abs_scan_angle_into_point_source [n]: set point source to scan_angle/[n]  
-drop_abs_scan_angle_above [max]    : drop points with absolute scan angle above [max]  
-drop_abs_scan_angle_below [min]    : drop points with absolute scan angle below [min]  
-drop_scan_angle_above [n]          : drop points with scan angle above [n]  
-drop_scan_angle_below [n]          : drop points with scan angle below [n]  
-drop_scan_angle_between [m] [n]    : drop points with scan angle between [m] and [n]  
-iscale_scan_angle [n]              : scale scan angle by [n]  
-itranslate_scan_angle [n]          : translate input scan angle by [n]  
-keep_scan_angle [m] [n]            : keep points with scan angle between [m] and [n]  
-keep_scan_angle_between [m] [n]    : keep points with scan angle between [m] and [n]  
-scale_scan_angle [n]               : scale scan angle by [n]  
-set_scan_angle [n]                 : set scan angle to [n]  
-translate_scan_angle [n]           : translate scan angle by [n]  
-translate_then_scale_scan_angle [m] [n]: translate scan angle by [m] and scale by [n]  

### Tiles
-keep_tile [x] [y] [size] : keep tile at lower-left [x] [y] with size [s]  
-tiles_ns [m] [n]         : create a tiling of DEMs with name [m] with tiles of size [n]  
-tiling_ns crater 500     : create a tiling of DEMs named 'crater' with tiles of size 500  

### Waveform packet
-drop_wavepacket [n]     : drop points with wavepacket value of [n]  
-flip_waveform_direction : flip the waveform direction in the waveform VLR  
-keep_wavepacket [n]     : keep points with wavepacket value of [n]  

### CRS
-aeac [m] [n] [meter/survey_feet/feet] [o] [p] [q] [r]: Albers Equal Area Conic Projection: False Easting [m] False Northing[n] [meter/survey_feet/feet] Central Meridian [o] Standard Parallel 1 [p] Standard Parallel 2 [q] Latitude of origin [r]  
-ecef                               : input is geocentric (Earth-centered Earth-fixed)  
-elevation_meter                    : use meter for elevation  
-elevation_survey_feet              : set vertical units from meters to US survey feet  
-elevation_surveyfeet               : use survey feet for elevation  
-ellipsoid [n]                      : use the WGS-84 ellipsoid [n]{do -ellipsoid -1 for a list of ellipsoids}  
-epsg [n]                           : set datum to EPSG [n]  
-etrs89                             : use datum ETRS89  
-gda2020                            : use datum GDA2020  
-gda94                              : use datum GDA94  
-grs80                              : use datum GRS1980  
-latlong                            : geometric coordinates in latitude/longitude order  
-lcc 609601.22 0.0 meter 33.75 -79 34.33333 36.16666: specifies a lambertian conic confomal projection  
-longlat                            : geometric coordinates in longitude/latitude order  
-meter                              : use meter  
-nad27                              : use the NAD27 ellipsoid  
-nad83                              : use the NAD83 ellipsoid  
-nad83_2011                         : use datum NAD83_2011  
-nad83_csrs                         : use datum NAD83_CSRS  
-nad83_harn                         : use datum NAD83_HARN  
-nad83_pa11                         : set horizontal datum to NAD83 PA11  
-osgb1936                           : use datum OSGB 1936  
-sp27 SC_N                          : use the NAD27 South Carolina North state plane  
-sp83 CO_S                          : use the NAD83 Colorado South state plane for georeferencing  
-survey_feet                        : use survey feet  
-surveyfeet                         : use survey feet as unit of measurement  
-target_aeac [m] [n] [meter/survey_feet/feet] [o] [p] [q] [r]: Albers Equal Area Conic Projection for target: False Easting [m] False Northing[n] [meter/survey_feet/feet] Central Meridian [o] Standard Parallel 1 [p] Standard Parallel 2 [q] Latitude of origin [r]  
-target_ecef                        : output is geocentric (Earth-centered Earth-fixed)  
-target_elevation_feet              : output uses feet for elevation  
-target_elevation_meter             : output uses meter for elevation  
-target_elevation_precision [n]     : output uses [n] (meter/feet) resolution for z  
-target_elevation_survey_feet       : use elevation survey feet as target unit  
-target_elevation_surveyfeet        : output uses survey feet for elevation  
-target_epsg [n]                    : output is EPSG code [n] (e.g. 2193=NZGD2000)  
-target_feet                        : output uses feet  
-target_latlong                     : output is geometric coordinates in latitude/longitude  
-target_lcc 609601.22 0.0 meter 33.75 -79 34.33333 36.16666: specifies a lambertian conic confomal projection at target  
-target_longlat                     : output is geometric coordinates in longitude/latitude  
-target_meter                       : output uses meter  
-target_precision [n]               : output uses [n] (meter/feet) resolution for x and y  
-target_sp27 SC_N                   : output is state plane NAD27 South Carolina North  
-target_sp83 CO_S                   : output is state plane NAD83 Colorado South  
-target_survey_feet                 : output uses survey feet  
-target_surveyfeet                  : use survey feet as target unit  
-target_tm                          : use transverse mercator projection for target  
-target_utm 12T                     : output is UTM zone 12T  
-tm 609601.22 0.0 meter 33.75 -79 0.99996: specifies a transverse mercator projection  
-transverse_mercator                : use transverse mercator projection  
-utm 12T                            : use UTM zone 12T  
-vertical_cgvd2013                  : set vertical datum to CGVD2013  
-vertical_cgvd28                    : set vertical datum to CGVD28  
-vertical_dhhn2016                  : set vertical datum to DHHN2016  
-vertical_dhhn92                    : set vertical datum to DHHN92  
-vertical_dvr90                     : set vertical datum to DVR90  
-vertical_epsg [n]                  : set vertical datum to EPSG [n]  
-vertical_evrf2007                  : set vertical datum to EVRF2007  
-vertical_navd29                    : set vertical datum to NAVD29  
-vertical_navd88                    : set vertical datum to NAVD88  
-vertical_ngvd29                    : set vertical datum to NGVD29  
-vertical_nn2000                    : set vertical datum to NN2000  
-vertical_nn54                      : set vertical datum to NN54  
-vertical_nzvd2016                  : set vertical datum to NZVD2016  
-vertical_wgs84                     : set vertical datum to WGS84  
-wgs72                              : use the WGS-72 ellipsoid  
-wgs84                              : use the WGS-84 ellipsoid  

### Logical
-filter_and         : boolean AND combination of last 2 filters  
-filter_or          : boolean OR combination of last 2 filters  
-filtered_transform : do the transformation only on points of the current filter  

### Input
-i [fnp]        : input file or input file mask [fnp] (e.g. *.laz;fo?.la?;esri.shp,...)  
-io_ibuffer [n] : use read-input-buffer of size [n] bytes  
-iparse [xyz]   : define fields [xyz] for text input parser  
-ipts           : input as PTS (plain text lidar source), store header in VLR  
-iptx           : input as PTX (plain text extended lidar data), store header in VLR  
-iptx_transform : use PTX file header to transform point data  
-iskip [n]      : skip [n] lines at the beginning of the text input  
-itxt           : expect input as text file  
-lof [fnf]      : use input out of a list of files [fnf]  
-merged         : merge input files  
-stdin          : pipe from stdin  

### Output
-compression_quality [n] : set compression quality to [n] for compressable image formats  
-do_not_populate         : do not populate header on output  
-nil                     : pipe output to NULL (suppress output)  
-o [n]                   : use [n] as output file  
-oasc                    : output as ascii file  
-obil                    : output as bil (Band Interleaved by Line)  
-ocsv                    : output as CSV (comma separated value)  
-ocut [n]                : cut the last [n] characters from name  
-odir [n]                : set output directory to [n]  
-odix [n]                : set output file name suffix to [n]  
-odtm                    : output as dtm (Digital Terrain Models)  
-oflt                    : output as flt (Float file format)  
-oforce                  : force output creation also on errors or warnings  
-oimg                    : output as img (Image file)  
-ojpg                    : output as jpg (JPG image)  
-olaz                    : output as LAZ (compressed LAS)  
-opng                    : output as png (PNG image)  
-osep [n]                : set text output separator as [sep] (see below, only xyz)  
-otif                    : output as GeoTIFF image  
-oxyz                    : output as xyz textfile  
-pipe_on                 : write output to command pipe, see also -std_in  
-populate                : populate header on output  
-target_ecef             : output is geocentric (Earth-centered Earth-fixed)

### parse
The '-parse [xyz]' flag specifies how to interpret
each line of the ASCII file. For example, 'tsxyzssa'
means that the first number is the gpstime, the next
number should be skipped, the next three numbers are
the x, y, and z coordinate, the next two should be
skipped, and the next number is the scan angle.

The other supported entries are:  
  x : [x] coordinate  
  y : [y] coordinate  
  z : [z] coordinate  
  t : gps [t]ime  
  R : RGB [R]ed channel  
  G : RGB [G]reen channel  
  B : RGB [B]lue channel  
  I : N[I]R channel of LAS 1.4 point type 8  
  s : [s]kip a string or a number that we don't care about  
  i : [i]ntensity  
  a : scan [a]ngle  
  n : [n]umber of returns of that given pulse  
  r : number of [r]eturn  
  h : with[h]eld flag  
  k : [k]eypoint flag  
  g : synthetic fla[g]  
  o : [o]verlap flag of LAS 1.4 point types 6, 7, 8  
  l : scanner channe[l] of LAS 1.4 point types 6, 7, 8  
  E : terrasolid [E]hco Encoding  
  c : [c]lassification  
  u : [u]ser data  
  p : [p]oint source ID  
  e : [e]dge of flight line flag  
  d : [d]irection of scan flag  
  0-9 : additional attributes described as extra bytes (0 through 9)  
  (13) : additional attributes described as extra bytes (10 and up)  
  H : a hexadecimal string encoding the RGB color  
  J : a hexadecimal string encoding the intensity  

### output separator
The '-osep [sep]' argument specifies the output format of a text(xyz) output.
Supported [sep] values:

  comma
  tab
  dot
  colon
  semicolon
  hyphen
  space

## License

Please license from info@rapidlasso.de to use the tool
commercially. 
You may use the tool to do tests with up to 3 mio points.
Please note that the unlicensed version may will adjust
some data and add a bit of white noise to the coordinates.

## Support

To get more information about a tool just goto the
[LAStools Google Group](http://groups.google.com/group/lastools/)
and enter the tool name in the search function.
You will get plenty of samples to this tool.

To get further support see our
[rapidlasso service page](https://rapidlasso.de/service/)

Check for latest updates at
https://rapidlasso.de/category/blog/releases/

If you have any suggestions please let us (info@rapidlasso.de) know.

