# laspublish

Creates a LiDAR portal for 3D visualization (and optionally also
for downloading) of LAS and LAZ files in any modern Web browser
using the [WebGL Potree](https://potree.github.io) from Markus Schuetz.


## Examples

    laspublish -i lidar.laz -odir portal_dir -o portal.html -olaz

creates a directory called "./portal_dir" containing an HTML file
called "portal.html" that allows exploring the LiDAR points from
the file "lidar.laz" once the directory is moved into Web space.


    laspublish -i tiles_final/*.laz -odir portal_dir -o portal.html -olaz

same as above for an entire folder of input files.


    laspublish -i tiles_final/*.laz -copy_source_files -odir portal_dir -o portal.html -olaz

same as above but also copies the original LAZ files into the portal
subdirectory ./portal_dir/pointclouds/portal/source where they need
to reside in order for the download map to link to them properly 


    laspublish -i tiles_final/*.laz -only_3D -odir portal_dir -o portal.html -olaz

same as above but only the 3D online viewer (without the map)


    laspublish -i tiles_final/*.laz -only_2D -odir portal_dir -o portal.html -olaz

same as above but only the 2D map


    laspublish -i tiles_final/*.laz -only_2D -copy_source_files -odir portal_dir -o portal.html -olaz

same as above but also copies the original LAZ files into the portal
subdirectory ./portal_dir/pointclouds/portal/source where they need
to reside in order for the download map to link to them properly 


Instead of using '-copy_source_files' or '-move_source_files' it is
also possible to copy or move the LAZ files manually to the correct
subfolder in the portal. For this example command line here

    laspublish -i tiles/*.laz -odir aaaaa -o bbbbb.html -olaz

this subfolder needs to be located in this exact location

./aaaaa/pointclouds/bbbbb/source

so that all links from the download map will correctly link to the
original LAZ files.


## laspublish specific arguments

-classification    : use classification value as output color parameter  
-copy_source_files : copy sourcefiles (LAS/LAZ) into portal subdirectory  
-description [n]   : use [n] as output description  
-elevation         : use elevation value as output color parameter  
-intensity         : use intensity value as output color parameter  
-move_source_files : move sourcefiles (LAS/LAZ) into portal subdirectory (need also "-really_move" argument)  
-name [n]          : use [n] as output name  
-no_edl            : do not use EDL (Eye dome lighting) at 3D output  
-no_skybox         : do not add sky at 3D output  
-only_2D           : only output xy  
-only_2d           : use only 2 dimensions (xy)  
-only_3D           : output only 3D  
-overwrite         : overwrite existing target files  
-point_source      : use "point source" value as output color parameter  
-potree14          : deprecated: potree 14 not supported anymore. will use potree 16  
-potree16          : expected protree version 16  
-potree18          : expected protree version 18  
-really_move       : confirmation to really move the files told by "-move_source_files" argument  
-return_number     : use "return number" value as output color parameter  
-rgb               : use rgb value as output color parameter  
-title [n]         : use [n] as output title  

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
