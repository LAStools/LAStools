# lasplanes

Finds sufficiently planar patches of LAS/LAZ points fulfilling
a number of user-defineable criteria that are output as simple
polygons to SHP or RIEGL's PEF format. The tool was originally
designed for generating tie planes to match the point clouds of
a mobile scan that suffer from errors in the GPS trajectory to
accurate terrestrial scans using clean planar patches that are
"seen" without obstruction by both scanners.

The algorithm divides the input into cells that are n by n by n
units big. It then performs an eigen value decomposition of the
covariance matrix of the points in each cell that has more than
the minimal number of points. The three eigenvalues have to pass
the small_eigen_max, middle_eigen_min, eigen_ratio_smallest, and
eigen_ratio_largest criteria. And the plane of points must be
sufficiently thin and formed by sufficiently many points after
removing no more than a certain percentage of them. Then a polygon
with a maximal number of points enclosing a subset of the points
is formed that is checked for having a minimal size and a maximal
off-planar standard deviation. The surviving planes are output
(optionally only if they are sufficiently far from another).
  
  
## Examples

    lasplanes -i terrestrial_scan.laz -o planes.pef

finds all planar patches in the file 'terrestrial_scan.laz' and
stores the result in RIEGL's PEF format


    lasplanes -i terrestrial_scan.laz -o planes.pef

same as above but outputting the SHP format


lasplanes -h  
lasplanes -i in.las -o planes.shp  
lasplanes -i in.las -oshp  
lasplanes -i in.las -o planes.pef  
lasplanes -i in.las -opef


## lasplanes specific arguments

-cell_points [n]           : skip if less than [n] points (default=100)  
-cell_size [s]             : set cell size to [s]*[s]*[s] (default=1)  
-cell_size_xyz [x] [y] [z] : set cell size to [x]*[y]*[z]  
-cores [n]                 : process multiple inputs on [n] cores in parallel  
-eigen_ratio_largest [n]   : skip if largest eigenvalue over sum of all three eigenvalues > [n](default=0.90000)  
-eigen_ratio_smallest [n]  : skip if smallest eigenvalue over sum of all three eigenvalues > [n](default=0.00010)  
-ilay [n]                  : apply [n] or all LASlayers found in corresponding *.lay file on read  
-max_number                : maximal number of check points to output  
-middle_eigen_min [n]      : skip if middle eigenvalue < [n](default=0(not used))  
-olay                      : write or append classification changes to a LASlayers *.lay file  
-oneighbors [n]            : neighborhood base filename [n]  
-only_2d                   : use only 2 dimensions (xy)  
-opef                      : output as RIEGL pef file  
-opoly [n]                 : overlap polygon output filename [n]  
-osamp [n]                 : sample point output filename [n]  
-output_marked_point_cloud : default <FALSE>  
-output_number [n]         : maximum number [n] of output items  
-output_polygon_points     : default <FALSE>  
-output_start [n]          : start output with sample [n]  
-plane_exclusion [n]       : skip if more than [n]% of points had to be excluded to make plane slim (default=5.00)  
-plane_points [n]          : skip if plane is formed by less than [n] points (default=100)  
-plane_thickness [n]       : skip if points form plane thicker than [n](default=0.01)  
-polygon_area [n]          : skip if area formed by points is less than [n](default=0.50)  
-polygon_digits [n]        : set number of digits to [n] to enumerate polygon names (default=5)  
-polygon_distance [n]      : skip if polygon distance to others is less than [n]  
-polygon_name [n]          : set polygon base name to [n](default='patch')  
-polygon_points [n]        : simplify polygon to have maximal [n] points (default=0(not used))  
-polygon_stddev [n]        : skip if standard deviation of polygon points from plane > stddev [n](default=0(not used))  
-small_eigen_max [n]       : skip if smallest eigenvalue > [n](default=0(not used))  
-week_to_adjusted [n]      : converts time stamps from GPS week [n] to Adjusted Standard GPS  

### Basics
-fail    : fail if license expired or invalid  
-gui     : start with files loaded into GUI  
-h       : print help output  
-license : show license information  
-v       : verbose output (print extra information)  
-version : reports this tool's version number  
-vv      : very verbose output (print even more information)  
-wait    : wait for <ENTER> in the console at end of process  

## Module arguments

### General
-kml_absolute             : set kml elevation to absolute values  
-kml_elevation_offset [n] : add an elevation offset of [n]  

### Output
-2d       : only output xy  
-o [n]    : use [n] as output file  
-ocut [n] : cut the last [n] characters from name  
-odbf     : output as DBF (dBase IV database)  
-odir [n] : set output directory to [n]  
-odix [n] : set output file name suffix to [n]  
-okml     : output as kml (Keyhole Markup Language XML)  
-oshp     : output as SHP *.shp file  
-otxt     : output as textfile  
-owkt     : output as wkt (well-known-text)  
-stdout   : pipe to stdout  

### Basics
-help : print help output  


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

If you have any suggestions please let us (support@rapidlasso.de) know.
Jochen @rapidlasso
