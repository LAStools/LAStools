# demzip

Compresses and uncompresses raster data from ASC, BIL, TIF, IMG
format to the compressed RasterLAZ format. The expected inputs
are rasters containing elevation data such as DTM, DSM, or CHM
rasters, or GEOID difference grids, or forestry metrics. These
values will be compressed into the z coordinate of RasterLAZ.

## Examples

see
https://groups.google.com/d/topic/lastools/39hR_4BvvIA/discussion  
https://groups.google.com/d/topic/lastools/nMPU75zpqPw/discussion


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
