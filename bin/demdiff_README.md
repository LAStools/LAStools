# demdiff

Compares rasters in ASC, BIL, TIF, IMG and RasterLAZ format and
reports differences.

If the input parameters are correct, the comparison result is reported as text output.

For identical files the output looks like:

rasters are identical. both have [n] value and [o] nodata cells.
done with '[p]'. total time [q] sec.
done with all files. total time for [r] files [s] sec.

For non-identical files the output looks like:

rasters are not identical. rasters have [m] different and [n] identical value cells. but both have [o] nodata cells
done with '[p]'. total time [q] sec.
done with all files. total time for [r] files [s] sec.

The text output can be parsed to get further information for automated processes.


## Warnings and Errors
If the input files or the arguments are not valid, an error output will be displayed:

    ERROR: cannot open raster1 with file name '...'.
The given filename was not found.

    ERROR: raster '...' has ... bands. not implemented.
Only files with 1 layers/bands are supported.

    ERROR: raster '...' has .. bits. not implemented.
Only files with 8,16 or 32 bits per pixel are supported.

    ERROR: raster '...' has no columns. skipping ...
    ERROR: raster '...' has no rows. skipping ... 
    ERROR: raster has no lower left x info
    ERROR: raster has no lower left y info
    ERROR: raster has no upper right x info
    ERROR: raster has no upper right y info
    ERROR: raster has no step x info
    ERROR: raster has no step y info
    ERROR: stepx_y of ... not supported
    ERROR: stepy_x of ... not supported
The input file should contain less then 1 data row or column.
Most likely the file is empty.

    ERROR: if more than two input rasters, they must all be in TIF, IMG, ASC, or BIL, not in LAZ format
When processing more than 2 input files they all have to be in the same format, and not LAZ files.
To compare LAZ files only an input of exact 2 files is supported.
    
    ERROR: raster '...' has ... bands. not implemented.
Only files with 1 color band (layers) are supported.
    
    ERROR: rasters have different ncols of ... and .... difference too big ...
    ERROR: rasters have different nrows of ... and .... difference too big ...
    WARNING: rasters have different nbits of ... and ...
The number of columns, rows and color bands of all files to compare has to be identical.


## Examples

    demdiff -i lake1.tif lake2.tif
    demdiff -i lake1.tif -i lake2.tif
Both commands compare the raster difference between file 'lake1.tif' and 'lake2.tif'.


## demdiff specific arguments

-i         : one or many input files to proceed  
-cores [n] : process multiple inputs on [n] cores in parallel  

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
