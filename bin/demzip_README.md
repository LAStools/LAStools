# demzip

Compresses and uncompresses raster data from ASC, BIL, TIF, IMG
format to the compressed RasterLAZ format. The expected inputs
are rasters containing elevation data such as DTM, DSM, or CHM
rasters, or GEOID difference grids, or forestry metrics. These
values will be compressed into the z coordinate of RasterLAZ.

Advantages of storing elevation grids and rasters as RasterLAZ:
1) highest compression as far as I can tell
2) directly feed into existing LAZ processing pipelines
3) raster and points are merging anyways (photogrammetic DSMs)
4) elevation values with fixed-point resolution, not in floating-point
5) any (!!!) reasonable raster order is supported
6) spatial indexing is readily supported
7) entire range can be used. "nodata" rasters are simply omitted
8) optional fast coverage decompression (coded separately for LAS 1.4) 
9) storage of additional (LAS-like) attributes supported 
10) display rasters mixed with points via Potree and/or Entwine / Greyhound

A RasterLAZ file is a LAZ file with an extra VLR that stores the raster
relevant extra information such as ncols, nrows, stepx resolution, stepy resolution, ...

## Examples

    demzip64 -i sample.tif -o sample.laz

compress the input tif file into a RasterLAZ file.


    demzip64 -o sample.laz -o sample.tif

convert the RasterLAZ file into a TIFF file.


## demzip specific arguments

-class [n]          : use point class [n]  
-classification [n] : set classification to [n]  
-nodata_max [n]     : raster values [n] or above considered nodata  
-nodata_min [n]     : raster values [n] or below considered nodata  
-nodata_value [n]   : raster value [n] considered nodata  
-scale [n]          : set vertical resolution to [n] (meter/feet)  
-sigmaxy [n]        : horizontal accuracy expected at [n] meters (inactive)  

### Basics
-cores [n]      : process multiple inputs on [n] cores in parallel  
-h, -help       : print help output  
-v, -verbose    : verbose output (print extra information)  
-vv             : very verbose output (print even more information)  
-silent         : only output on errors or warnings
-quiet          : no output at all
-force          : continue, even if serious warnings occur  
-errors_ignore  : continue, even if errors occur (if possible). Use with caution!
-print_log_stats: print additional log statistics  
-cpu64          : force 32bit version to start 64 bit in multi core (obsolete)
-gui            : start with files loaded into GUI  
-version        : reports this tool's version number  

## Licensing

This tool is free to use.

## Support

1. We invite you to join our LAStools Google Group (http://groups.google.com/group/lastools/).
   If you are looking for information about a specific tool, enter the tool name in the search 
   function and you'll find all discussions related to the respective tool. 
2. Customer Support Page: https://rapidlasso.de/customer-support/.  
3. Download LAStools: https://rapidlasso.de/downloads/.  
4. Changelog: https://rapidlasso.de/changelog/.  


If you want to send us feedback or have questions that are not answered in the resources above, 
please email to info@rapidlasso.de.
