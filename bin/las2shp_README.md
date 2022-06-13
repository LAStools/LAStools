# las2shp

converts LIDAR from LAS/LAZ/ASCII to ESRI's Shapefile format by
grouping consecutive points into MultiPointZ records. The default
size is 1024. It can be changed with '-record 2048'. 
If you want to use PointZ records instead you need to add 
'-single_points' to the command line.

 
## Examples

    las2shp -i *.las

converts all LAS files matching '*.las' to ESRI's Shapefile format
'*.shp' using MultiPointZ records containing 1024 points each.

    las2shp -i lidar.laz -o shapefile.shp -record 2048

converts the LAZ file 'lidar.las' to ESRI's Shapefile 'shapefile.shp'
using MultiPointZ records containing 2048 points each.

    las2shp -lof file_list.txt -merged -o lidar.shp -record 2048

converts the contents of all LAS files listed in 'file_list.txt' to ESRI's
Shapefile format 'lidar.shp' using MultiPointZ records containing 2048
points each.


las2shp -h  
las2shp -i *.las  
las2shp -i *.txt -iparse xyza -keep_scan_angle -10 10  
las2shp -i lidar.las -o shapefile.shp -record_size 2048  
las2shp -i lidar.laz -last_only -record_size 10000  
las2shp -i lidar.laz -keep_class 2 8  
las2shp -i *.laz -drop_return 1


## las2shp specific arguments

-cores [n]            : process multiple inputs on [n] cores in parallel  
-parse [xyz]          : use parse string [xyz] to access point values  
-record [n]           : set shp output record size to [n](default=1024)  
-record_size [n]      : set shp output record size to [n](default=1024)  
-single_points        : force use of SHP point type 11 instead of point type 18  
-week_to_adjusted [n] : converts time stamps from GPS week [n] to Adjusted Standard GPS  

### Basics
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
-chunk_size [n] : set chunk size [n] in number of bytes  

### Color
-oscale_rgb [n] : scale output RGB by [n]  

### Output
-compatible     : write LAS/LAZ output in compatibility mode  
-io_obuffer [n] : use write-out-buffer of size [n] bytes  
-native         : write LAS/LAZ output in native/actual mode  
-nil            : pipe output to NULL (suppress output)  
-o [n]          : use [n] as output file  
-obin           : output as BIN (terrasolid binary)  
-ocut [n]       : cut the last [n] characters from name  
-odir [n]       : set output directory to [n]  
-odix [n]       : set output file name suffix to [n]  
-oforce         : force output creation also on errors or warnings  
-olas           : output as LAS file  
-olaz           : output as LAZ (compressed LAS)  
-oparse [xyz]   : parse on-the-fly to ASCII using fields [xyz]  
-opts           : output as PTS (plain text lidar data)  
-optx           : output as PTX (plain text with header)  
-oqi            : output in QFIT format (.qi)(ATM project, NASA)  
-oscale_rgb [n] : scale output RGB by [n]  
-osep [n]       : set text output separator as char [n]  
-otxt           : output as textfile  
-owrl           : output as VRLM (Virtual Reality Modeling Language) text  
-stdout         : pipe to stdout

### parse
The '-parse [xyz]' flag specifies how to interpret
each line of the ASCII file. For example, 'tsxyzssa'
means that the first number is the gpstime, the next
number should be skipped, the next three numbers are
the x, y, and z coordinate, the next two should be
skipped, and the next number is the scan angle.

The other supported entries are:
  x : <x> coordinate
  y : <y> coordinate
  z : <z> coordinate
  t : gps <t>ime
  R : RGB <R>ed channel
  G : RGB <G>reen channel
  B : RGB <B>lue channel
  I : N<I>R channel of LAS 1.4 point type 8
  s : <s>kip a string or a number that we don't care about
  i : <i>ntensity
  a : scan <a>ngle
  n : <n>umber of returns of that given pulse
  r : number of <r>eturn
  h : with<h>eld flag
  k : <k>eypoint flag
  g : synthetic fla<g>
  o : <o>verlap flag of LAS 1.4 point types 6, 7, 8
  l : scanner channe<l> of LAS 1.4 point types 6, 7, 8
  E : terrasolid <E>hco Encoding
  c : <c>lassification
  u : <u>ser data
  p : <p>oint source ID
  e : <e>dge of flight line flag
  d : <d>irection of scan flag
  0-9 : additional attributes described as extra bytes (0 through 9)
  (13) : additional attributes described as extra bytes (10 and up)
  H : a hexadecimal string encoding the RGB color
  J : a hexadecimal string encoding the intensity


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
