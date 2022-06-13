# shp2las

Converts from points from ESRI's Shapefile to LAS/LAZ/ASCII
format given the input contains Points or MultiPoints (that
is any of the shape types 1,11,21,8,18,28).

Allows adding a VLR to the header with projection information.


## Examples

    shp2las lidar.shp

converts the ESRI's Shapefile 'lidar.shp' to the LAS file 'lidar.las'


    shp2las -compress lidar.shp

converts 'lidar.shp' to the compressed LAZ file 'lidar.laz'


    shp2las -set_scale 0.001 0.001 0.001 -i lidar.shp -o lidar.las

converts 'lidar.shp' to the LAS file 'lidar.las' with the specified scale


    shp2las -i lidar.shp -o lidar.laz -set_offset 500000 4000000

converts 'lidar.shp' to the compressed LAZ file 'lidar.laz' with the specified offset


    shp2las -i lidar.shp -o lidar.las -v

converts 'lidar.shp' to the LAS file 'lidar.las' and outputs some of
the header information found in the SHP file


## shp2las specific arguments

-cores [n]                      : process multiple inputs on [n] cores in parallel  
-idbf                           : use DBF file of shapefile as additional input  
-set_class [n]                  : set classification to [n]  
-set_classification [n]         : set classification to [n]  
-set_creation_date [day] [year] : set creation date to [day] [year]  
-set_file_creation [day] [year] : set creation date to [day] [year]  
-set_offset [x] [y]             : set offset to [x] [y]  
-set_scale [x] [y] [z]          : quantize ASCII points with [x] [y] [z] (unit meters)  
-set_version [m] [n]            : set version to major.minor = [m].[n]  

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
