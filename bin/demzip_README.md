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
-cores [n]          : process multiple inputs on [n] cores in parallel  
-nodata_max [n]     : raster values [n] or above considered nodata  
-nodata_min [n]     : raster values [n] or below considered nodata  
-nodata_value [n]   : raster value [n] considered nodata  
-scale [n]          : set vertical resolution to [n] (meter/feet)  
-sigmaxy [n]        : horizontal accuracy expected at [n] meters (inactive)  

### Basics
-cpu64   : start 64 bit executable (instead of default 32 bit executable)  
-fail    : fail if license expired or invalid  
-gui     : start with files loaded into GUI  
-h       : print help output  
-help    : print help output  
-quiet   : nothing reported in console  
-v       : verbose output (print extra information)  
-verbose : verbose output (print extra information)  
-version : reports this tool's version number  
-vv      : very verbose output (print even more information)  


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

