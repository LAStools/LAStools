# lasinfo

Reports the contents of the header and a short summary of the
points. Warns when there is a difference between the header
information and the point content. When run with option '-cd'
or '-compute_density', lasinfo will compute the point density.

All differences can be repaired with the '-repair' option. The
option '-repair_bb' will only repair (or tighten) the bounding
box while '-repair_counters' will only repair wrong reported
number of points. 

By default the output of lasinfo goes to stderr. The output can
be suppressed with '-quiet' or changed to '-stdout'. To write
to a particular file use '-o output.txt' or '-otxt' which stores
the lasinfo report to "lidar.txt" assuming the file has the name
"lidar.las". With the option '-odix _info' you change the name
to "lidar_info.txt" and with '-odir E:\reports' you choose a
particular directory. These options are especially useful in
batch mode such as:

lasinfo64 -i *.laz -otxt -odir ..\reports -odix _info -cores 3

The tool can also be used to modify various other entries in
the header as described below. This needs to be done with care
as such changes can potentially corrupt the file.

In case you just want to report or modify the header entries
you can skip the parsing of the points with '-no_check'. Some
LAS files have excessive amounts of VLRs. You can supress them
being output with '-no_vlrs'. By default lasinfo reports the 
min and the max of every point attribute after parsing all the
points. You can supress this with '-no_min_max'. By default the
tool also counts the points falling outside the header bounding
box. This can be disabled with '-no_check_outside'. If instead
you want to report all the individual points that fall outside
use '-report_outside'. Another interesting option is to report
the GPS time min and max as '-gps_week' in case it is stored as
adjusted Standard GPS time. 

There are a lot of ways that lasinfo can be used to modify the
contents of the LAS header *in place* without copying the file
and the GUI will teach you these command line options. Some not
listed in the GUI are: 

-set_GUID F794F8A4-A23E-421E-A134-ACF7754E1C54
-set_system_identifier "RIEGL Optech Leica"
-set_generating_software "awesome LAStools"
-set_file_source_ID 27
-set_file_creation 8 2007
-auto_date
-set_global_encoding 1

## Examples

    lasinfo64 lidar.las

reports all information to stderr.


    lasinfo64 -i lidar1.las lidar2.las -merged

reports all information for a merged LAS file that contains the
merged contents of lidar1.las and lidar2.las to stderr.


    lasinfo64 -i lidar1.las lidar2.las

reports the information of the two files individually to stderr.


    lasinfo64 *.las -merged -stdout

reports information for all files merged to stdout.


    lasinfo64 *.las -stdout

reports information for all files individually to stdout.


    lasinfo64 *.las -odix _info

reports information for all files individually to *_info.txt


    lasinfo64 -i lidar.las -o lidar_info.txt 

reports the information to a text file called lidar_info.txt.


    lasinfo64 -i lidar.las -no_header

supresses the reporting of the header information (short: '-nh').


    lasinfo64 -i lidar.las -no_vlrs

supresses the reporting of variable length records (short: '-nv').


    lasinfo64 -i lidar.las -no_min_max

supresses the reporting of minimum/maximum value for each attribute
of the point records (short: '-nmm').


    lasinfo64 -i lidar.las -no_check_outside

supresses checking whether points fall outside of the bounding box that
is reported in the header (short: '-nco').


    lasinfo64 -i lidar.las -report_outside

reports the index and the coordinates of each point that falls outside
of the bounding box that is reported in the header (short: '-ro').


    lasinfo64 -i lidar.las -nh -nv -progress 1000000

supresses reporting of the header information and the vlrs and reports
progress each time 1,000,000 points are parsed.


    lasinfo64 -i lidar.las -no_check

only reports header information (short: '-nc'). does not parse the points.


    lasinfo64 -i lidar.las -compute_density

computes and reports a good estimate of the point density (short: '-cd').


    lasinfo64 -i lidar.las -repair_bb

corrects missing or wrong bounding box info in the header.


    lasinfo64 -i lidar.las -repair_counters

corrects missing or wrong point number info in the header.


    lasinfo64 -i lidar.las -set_file_source_ID 27

sets the file source ID in the LAS header to 27.


    lasinfo64 -i lidar.las -auto_date

sets the file creation day/year in the header to the creation date of the file.


    lasinfo64 -i lidar.las -set_file_creation 8 2007

sets the file creation day/year in the header to 8/2007


    lasinfo64 -i lidar.las -set_system_identifier "hello world!"

copies the first 31 characters of the string into the system_identifier field of the header 


    lasinfo64 -i lidar.las -set_generating_software "this is a test (-:"

copies the first 31 characters of the string into the generating_software field of the header 


    lasinfo64 -i lidar.las -set_bounding_box 4100000 1800000 150 4110000 1810000 400

sets the bounding box in the header to min_x=4100000, max_x=4110000, min_y=1800000, ...


    lasinfo64 -i lidar.las -set_offset 3000000 1000000 0

CAREFUL! sets the offset in the LAS header to 3000000 1000000 0 without changing
the point coordinates. hence this will, in general, translate the point cloud.


    lasinfo64 -i lidar.las -set_scale 0.001 0.001 0.001

CAREFUL! sets the scale in the LAS header to 0.001 0.001 0.001 without changing
the point coordinates. hence this will, in general, scale the point cloud.


    lasinfo64 -i lidar.las -set_global_encoding 1

CAREFUL! sets the global encoding field of the LAS header to 1 without checking
whether this will corrupt the file.


    lasinfo64 -i lidar.las -set_version 1.1

CAREFUL! sets the version field of the LAS header to 1.1 without checking whether
this will corrupt the file.


    lasinfo64 -i lidar.las -set_header_size 235

CAREFUL! sets the header size field of the LAS header to 235 without checking
whether this will corrupt the file.


    lasinfo64 -i lidar.las -set_offset_to_point_data 460

CAREFUL! sets the offset to point data field of the LAS header to 460 without
checking whether this will corrupt the file.


    lasinfo64 -i lidar.las -set_point_data_format 1

CAREFUL! sets the point data format field of the LAS header to point type 1
without checking whether this will corrupt the file.


    lasinfo64 -i lidar.las -set_point_data_record_length 32

CAREFUL! sets the point data record length field of the LAS header to size 32
without checking whether this will corrupt the file.


    lasinfo64 -i lidar.las -set_start_of_waveform_data_packet_record 0

CAREFUL! sets the start of waveform data packet record field of the LAS header
to 0 without checking whether this will corrupt the file.


lasinfo64 -i lidar.las  
lasinfo64 -i lidar.las -compute_density -o lidar_info.txt  
lasinfo64 -i *.las  
lasinfo64 -i *.las -single -otxt  
lasinfo64 -no_header -no_vlrs -i lidar.laz  
lasinfo64 -nv -nc -stdout -i lidar.las  
lasinfo64 -nv -nc -stdout -i *.laz -single | grep version  
lasinfo64 -i *.laz -subseq 100000 100100 -histo user_data 8  
lasinfo64 -i *.las -repair  
lasinfo64 -i *.laz -repair_bb -set_file_creation 8 2007  
lasinfo64 -i *.las -repair_counters -set_version 1.2  
lasinfo64 -i *.laz -set_system_identifier "hello world!" -set_generating_software "this is a test (-:"


## lasinfo specific arguments

-auto_creation                      : set (in place) creation year / day of year in header automatically based on file creation date  
-auto_creation_date                 : set (in place) creation year / day of year in header automatically based on file creation date  
-auto_date                          : set (in place) creation year / day of year in header automatically based on file creation date  
-cd                                 : compute rough approximation for covered area, density, and spacing  
-compute_density                    : compute rough approximation for covered area, density, and spacing  
-cores [n]                          : process multiple inputs on [n] cores in parallel  
-delete_empty                       : delete LAS files with zero points  
-gps_week                           : compute the GPS week (if data is Adjusted Standard GPS time)  
-gw                                 : compute the GPS week (if data is Adjusted Standard GPS time)  
-nc                                 : don't parse points (only check header and VLRs)  
-nco                                : don't check whether points fall outside of LAS header bounding box  
-nh                                 : don't output LAS header information  
-nmm                                : don't output point minimum / maximum entry information  
-no_check                           : don't parse points (only check header and VLRs)  
-no_check_outside                   : don't check whether points fall outside of LAS header bounding box  
-no_header                          : don't output LAS header information  
-no_min_max                         : don't output point minimum / maximum entry information  
-no_returns                         : don't output return information  
-no_vlrs                            : don't output VLR information  
-no_warnings                        : don't output WARNINGs  
-nr                                 : don't output return information  
-nv                                 : don't output VLR information  
-nw                                 : don't output WARNINGs  
-otxt                               : output as textfile  
-progress [n]                       : report progress every [n] points  
-rename [n]                         : renames input file 'fusa.laz' to '[n]_277750_6122250.laz'  
-repair                             : repair both bounding box and counters  
-repair_bb                          : repair bounding box  
-repair_counters                    : set (in place) the counters for point number and (extended) return histograms in header  
-report_outside                     : report attributes of each point that falls outside of LAS header bounding box  
-ro                                 : report attributes of each point that falls outside of LAS header bounding box  
-set_bb [x1] [y1] [z1] [x2] [y2] [z2]: set bounding box to [x1] [y1] [z1] [x2] [y2] [z2]  
-set_bounding_box [x1] [y1] [z1] [x2] [y2] [z2]: set bounding box to [x1] [y1] [z1] [x2] [y2] [z2]  
-set_creation_date [day] [year]     : set creation date to [day] [year]  
-set_file_source_ID [n]             : set (in place) file source ID in header to [n]  
-set_file_source_ID_from_point_source_ID: set (in place) file source ID in header to value that *all* points have in point source ID  
-set_generating_software [n]        : set generating software header entry to [n] (max. 31 char string)  
-set_geotiff_epsg [n]               : adds EPSG code [n] in-place when other GeoTIFF tags present  
-set_global_encoding [0/1]          : CAREFUL! sets the global encoding field of the LAS header to [0/1] without checking whether this will corrupt the file  
-set_GUID [n]                       : sets the GUID [n] in header (assuming it is given in example hexadecimal format shown)  
-set_header_size [n]                : CAREFUL! sets the header size field of the LAS header to [n] without checking whether this will corrupt the file  
-set_number_of_point_records [n]    : set number of point records to [n]  
-set_number_of_points_by_return  [m] [n] [o] [p] [q]: set "number of points by return" header info [m] [n] [o] [p] [q]  
-set_number_of_variable_length_records [n]: set number of variable length records to [n]  
-set_offset [x] [y] [z]             : CAREFUL! sets the offset in the LAS header to [x] [y] [z] without changing the point coordinates  
-set_offset_to_point_data [n]       : CAREFUL! sets the offset to point data field of the LAS header to [n] without checking whether this will corrupt the file  
-set_point_data_format [n]          : CAREFUL! sets the point data format field of the LAS header to point type [n]{1-10} without checking whether this will corrupt the file  
-set_point_data_record_length [n]   : CAREFUL! sets the point data record length field of the LAS header to size [n] without checking whether this will corrupt the file  
-set_scale [x] [y] [z]              : quantize ASCII points with [x] [y] [z] (unit meters)  
-set_start_of_waveform_data_packet_record [n]: CAREFUL! sets the start of waveform data packet record field of the LAS header to [n] without checking whether this will corrupt the file  
-set_system_identifier [n]          : set the system identifier header entry to [n] (max 31 characters)  
-set_version [n]                    : CAREFUL! sets the version field of the LAS header to [n]{e.g. 1.2}  without checking whether this will corrupt the file  
-set_vlr_description [m] [n]        : set description of vlr [m] to [n]  
-set_vlr_record_id [m] [n]          : set record id of vlr [m] to [n]  
-set_vlr_user_id [m] [n]            : set user id of vlr [m] to [n]  
-start_at_point [n]                 : start loading from point position [n]  
-stop_at_point [n]                  : stop loading after [n] points  
-subseq [m] [n]                     : only load subsequence from point [m] to [n]  
-suppress_classification            : do not decompress classification for native-compressed LAS 1.4 point types 6 or higher  
-suppress_extra_bytes               : do not decompress "extra bytes" for native-compressed LAS 1.4 point types 6 or higher  
-suppress_flags                     : do not decompress flags for native-compressed LAS 1.4 point types 6 or higher  
-suppress_intensity                 : do not decompress intensity for native-compressed LAS 1.4 point types 6 or higher  
-suppress_point_source              : do not decompress point source ID for native-compressed LAS 1.4 point types 6 or higher  
-suppress_RGB                       : do not decompress RGB for native-compressed LAS 1.4 point types 6 or higher  
-suppress_scan_angle                : do not decompress scan angle for native-compressed LAS 1.4 point types 6 or higher  
-suppress_user_data                 : do not decompress user data field for native-compressed LAS 1.4 point types 6 or higher  
-suppress_z                         : do not decompress z coordinates for native-compressed LAS 1.4 point types 6 or higher  
-week_to_adjusted [n]               : converts time stamps from GPS week [n] to Adjusted Standard GPS  

### Basics
-cpu64   : start 64 bit executable (instead of default 32 bit executable)  
-fail    : fail if license expired or invalid  
-gui     : start with files loaded into GUI  
-h       : print help output  
-quiet   : nothing reported in console  
-v       : verbose output (print extra information)  
-version : reports this tool's version number  
-wait    : wait for <ENTER> in the console at end of process  

## Module arguments

### General
-buffered [n]          : define read or write buffer of size [n]{default=262144}  
-chunk_size [n]        : set chunk size [n] in number of bytes  
-comma_not_point       : use comma instead of point as decimal separator  
-histo_avg [m] [n] [o] : histogram output about [m] with step width [n] and average [o]  
-neighbors [n]         : set neighbors filename or wildcard [n]  
-neighbors_lof [n]     : set neighbors list of files [fnf]  
-stored                : use in memory reader  
-unique                : remove duplicate points  

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
-oscale_rgb [n]                     : scale output RGB by [n]  
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
-switch_G_B                         : switch green and blue value  
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

### Waveform packet
-drop_wavepacket [n]     : drop points with wavepacket value of [n]  
-flip_waveform_direction : flip the waveform direction in the waveform VLR  
-keep_wavepacket [n]     : keep points with wavepacket value of [n]  

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
-compatible      : write LAS/LAZ output in compatibility mode  
-do_not_populate : do not populate header on output  
-io_obuffer [n]  : use write-out-buffer of size [n] bytes  
-native          : write LAS/LAZ output in native/actual mode  
-nil             : pipe output to NULL (suppress output)  
-o [n]           : use [n] as output file  
-obin            : output as BIN (terrasolid binary)  
-ocut [n]        : cut the last [n] characters from name  
-odir [n]        : set output directory to [n]  
-odix [n]        : set output file name suffix to [n]  
-oforce          : force output creation also on errors or warnings  
-olas            : output as LAS file  
-olaz            : output as LAZ (compressed LAS)  
-oparse [xyz]    : parse on-the-fly to ASCII using fields [xyz]  
-opts            : output as PTS (plain text lidar data)  
-optx            : output as PTX (plain text with header)  
-oqi             : output in QFIT format (.qi)(ATM project, NASA)  
-oscale_rgb [n]  : scale output RGB by [n]  
-osep [sep]      : set text output separator as [sep](see table below)  
-owrl            : output as VRLM (Virtual Reality Modeling Language) text  
-pipe_on         : write output to command pipe, see also -std_in  
-populate        : populate header on output  
-stdout          : pipe to stdout  
-temp_files [n]  : set base file name [n] for temp files (example: E:\tmp)  

### Basics
-help : print help output

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
The '-osep [sep]' argument specifies the output format of a text(xyz or csv) output.
Supported [sep] values:

  comma
  tab
  dot
  colon
  semicolon
  hyphen
  space

## License

This tool is free to use.

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

