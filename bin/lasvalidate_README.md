# lasvalidate

A simple tool to determine if LAS files conform to the ASPRS LAS 1.0 to 1.5 specifications.

lasvalidate validates LAS files for compliance with the respective LAS specification.
The tool analyzes header information, structural metadata, CRS definitions, and selected 
point attributes, and generates a structured validation report.

By default, the output file of the lasvalidate report is written in JSON format as 
'result.json'. To write the report to a specific file, you can explicitly define the output 
name and format (JSON, XML, or TXT) using '-o output.json', '-o output.xml', or '-o output.txt'. 
Alternatively, you can use the implicit format arguments '-ojs', '-oxml', or '-otxt', which 
automatically generate an output file based on the input file name. For example, if the 
input file is 'lidar.laz', the report will be written to 'lidar.json', 'lidar.xml', or 'lidar.txt'.
It is also possible to create a simplified report in CSV format using '-o lidar.csv' or -ocsv.
The output can be suppressed entirely using '-quiet'. These options are especially useful in
batch mode such as:

lasvalidate64 -i *.laz -ojson -odir ..\reports -odix _report -cores 3

The tool is used for quality assurance of point cloud data before further processing, 
archiving, or transfer to third parties.
The validation includes the following:

**Header validation:**
1. The **Global Encoding** field is verified for correctly set bits according to the LAS version and point data format.

2. The **LAS version number**, **System Identifier**, **Generating Software**, and **File Creation Date** are validated.

3. The **header size** is checked against the minimum required size according to the LAS version specification, and the **offset to point data** is validated.

4. The **point data format** is verified to ensure it is permitted for the specified LAS version.

5. The **point data record length** is checked against the minimum required length for the given point data format.

6. The **extra bytes** in the point data record are checked against an **Extra Bytes VLR or EVLR**.

7. It is verified that **at least one point** record exists in the file.

8. It is checked that the **number of points by return** is less than or equal to the total **number of point records**.

9. For LAS 1.4 and higher, integrity between the **legacy number of point records** and the **extended number of point records** is verified.

10. For LAS 1.4 and higher, integrity between the **legacy number of points by return** and the **extended number of points by return** is verified.

11. If a tile size is specified, it is checked whether the **header bounding box** agrees with the given tile size.

12. The **number of point records** stored in the header is compared against the counted inventory.

13. The **extended number of points by return** stored in the header is compared against the counted inventory.

14. The **scale factors (X, Y, Z)** and **offset values (X, Y, Z)** are validated.

15. The **Global Encoding** field and **Waveform Data Packet** information are verified.

16. Coordinates are checked for **excessive resolution** or **rounding artifacts** ("resolution fluff").

17. The inventory is checked for **invalid return numbers**.

18. The inventory is checked for **invalid numbers of returns per pulse**.

19. The inventory is checked for **abnormal intensity values**.

20. The inventory is checked for **abnormal scan angle values**.

21. The inventory is checked for **zero Point Source IDs**.

22. Consistency between the **File Source ID** and **Point Source IDs** is verified.

23. For point data formats 1, 3, 4, and higher, it is checked whether **all GPS time stamps are identical**.

24. For point data formats 2, 3, 7, 8, and 10, it is checked whether **all RGB values are identical**.

25. **Wave packet indices** are validated.

26. **GPS time** values are checked for validity.



**Point validation:**
1. **Point coordinates** are verified to ensure they lie within the **header-defined bounding box**.

2. The **return number** and **number of returns**, as well as the **extended return number** and **extended number of returns**, are checked for consistency.

3. The **scan direction** and **edge of flight line** are correctly set to 0 or 1

4. check that the **point classification** value is valid according to LAS version 

5. **GPS time values** are validated against the **header-defined time range**.


**CRS-Validierung:**
1. It is checked whether a **GeoTIFF CRS** or an **OGC WKT CRS** is present.

2. The **CRS specification** (GeoTIFF or OGC WKT) is verified according to the LAS minor version.

3. The **GeoTIFF CRS** or **OGC WKT CRS** definition is validated for correctness.


## Examples

    lasvalidate64 -i lidar.laz

reports all information to the console.


    lasvalidate64 -i lidar.laz -o result.json

reports all information to the result.json file.


    lasvalidate64 -i lidar1.laz lidar2.laz -o result.json

reports validation for all input LAS file lidar1.laz and lidar2.laz to the result.json ouput file.


    lasvalidate64 -i lidar.laz -odir ../results -otxt
	
a simplified report for the input LAS file lidar1.laz to the result.csv ouput file.


    lasvalidate64 -i *.laz -odir ../results -otxt
	
reports validation results for lidar.laz into the specified directory as a TXT output file.


    lasvalidate64 -i *.laz -odir ../results -oxml -report_per_file
	
reports validation results for all LAZ files in the directory to separate XML output files.


    lasvalidate64 *.laz -o result.json -no_CRS_fail

reports validation information for all input files into a single output file and writes 
CRS (Coordinate Reference System) validation errors as warnings.



Further examples

    lasvalidate64 -i lidar.laz  
    lasvalidate64 -i *.las  
    lasvalidate64 -i *.laz -ojs  
    lasvalidate64 -i *.laz -oxml  
    lasvalidate64 -i *.laz -otxt  


## lasvalidate specific arguments

-ocsv                               : Simplified output as csv file  
-oxml                               : output as xml file  
-otxt                               : output as text file  
-ojs                                : output as json file  
-csv                                : simplified output in csv format (otherwise default json)  
-txt                                : output in txt format (otherwise default json)  
-xml                                : output in xml format (otherwise default json)  
-report_per_file                    : creates a separate validation report file for each input file  
-tile_size [n]                      : checks if header bounding box exceeds the tile size [n]  
-no_CRS_fail                        : all CRS (Coordinate Reference System) validation errors are issued as warnings  
-halt_on_errors                     : halt on errors, even if more then 1 input files checked  


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

### Input
-i [fnp]        : input file or input file mask [fnp] (e.g. *.laz;fo?.la?;esri.shp,...)  
-io_ibuffer [n] : use read-input-buffer of size [n] bytes  
-iparse [xyz]   : define fields [xyz] for text input parser  
-ipts           : input as PTS (plain text lidar source), store header in VLR  
-iptx           : input as PTX (plain text extended lidar data), store header in VLR  
-iptx_transform : use PTX file header to transform point data  
-itxt           : expect input as text file  
-lof [fnf]      : use input out of a list of files [fnf]  
-subdir         : enables recursive search in subdirectories. (Linux: enclose wildcard patterns like "*.laz" in quotes)  
-unique         : remove duplicate files in a -lof list  
-stdin          : pipe from stdin  

### Output
-nil             : pipe output to NULL (suppress output)  
-o [n]           : use [n] as output file  
-ocut [n]        : cut the last [n] characters from name  
-odir [n]        : set output directory to [n]  
-odix [n]        : set output file name suffix to [n]  
-oforce          : force output creation also on errors or warnings  
-pipe_on         : write output to command pipe, see also -std_in  
-stdout          : pipe to stdout  
-temp_files [n]  : set base file name [n] for temp files (example: E:\tmp)  


### parse
The '-parse [xyz]' flag specifies how to interpret each line of the ASCII file.
For example, 'tsxyzssa' means that the first number is the gpstime, the next
number should be skipped, the next three numbers are the x, y, and z coordinate,
the next two should be skipped, and the next number is the scan angle.

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
  c : [c]lassification. If extended classes are used: Use o,l or I to force 1.4 format.  
  u : [u]ser data  
  p : [p]oint source ID  
  e : [e]dge of flight line flag  
  d : [d]irection of scan flag  
  0-9 : additional attributes described as extra bytes (0 through 9)  
  (13) : additional attributes described as extra bytes (10 and up)  
  H : a hexadecimal string encoding the RGB color  
  J : a hexadecimal string encoding the intensity  


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
