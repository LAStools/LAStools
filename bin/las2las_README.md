# las2las

reads and writes LIDAR data in LAS/LAZ/ASCII format to filter,
transform, project, thin, or otherwise modify its contents.

Sometimes it is not neccessary to use las2las prior other
lastools, because most arguments can be used by the other
tool as well.

Examples are keeping only those points that are within a
rextangle '-keep_xy 10 10 20 20' or points that are between
a certain height '-keep_z 10 100', or dropping points that
are a certain return '-drop_return 2', that have a scan angle
above some threshold '-drop_scan_angle_above 5', or below some
intensity '-drop_intensity_below 15'. Sometimes points are far
outside the bounding box (corrupted files) and it is handy to
remove them with '-clip_to_bounding_box'.

It is also possible to add missing projection information to
the LAS/LAZ file or to reproject (using the same ellipsoid)
for example from latitude/longitude to UTM or the stateplane
of Ohio_North or to Earth-centered Earth-fixed (ECEF). You can
also use common EPSG codes with '-epsg 32754'. For LAS 1.4 it
is important to '-set_ogc_wkt' which translates the GeoTIFF
keys into an CRS string in the OGC WKT format and adds them as
the payload of the corresponding VLR. For LAS 1.4 files you
can use '-set_ogc_wkt_in_evlr' to put that string into the
EVLR instead of the VLR (but we don't recommend that).

Another typical use is extract only first (-first_only) or only
last (-last_only) returns. Extracting the first return is the
same as dropping all others (e.g. -drop_return 2 3 4 5).

Or one can extract a subsequence of 1000 points (-subseq 540 1000)
which will start at point 540.

Finally one can also only keep or drop certain classifications.
The option -keep_class 2 3 will keep only those points that are
of classification 2 or 3 and the option -drop_class 2 3 will drop
only those points. For all options run 'las2las -h'.

 
## Examples

    las2las64 -i s1885565.laz -o out.las -sp83 OH_S -feet -elevation_feet

Adding the projection information to the file 's1885565.laz'. This
will not modify the points but merely change the projection VLR in the
header to contain these four geokeys:

  GeoKeyDirectoryTag version 1.1.0 number of keys 4  
  - key 1024 value_offset 1 - GTModelTypeGeoKey: ModelTypeProjected  
  - key 3072 value_offset 32123 - ProjectedCSTypeGeoKey: PCS_NAD83_Ohio_South  
  - key 3076 value_offset 9002 - ProjLinearUnitsGeoKey: Linear_Foot  
  - key 4099 value_offset 9002 - VerticalUnitsGeoKey: Linear_Foot  


    las2las64 -i s1885565.laz -o out.las -sp83 OH_S -feet -elevation_feet -target_utm auto

Reprojects the points from the Ohio_South NAD83 state plane with all units
in feet to NAD83 UTM coordinates with all units in meter and sets these four
geokeys as the projection information:

  GeoKeyDirectoryTag version 1.1.0 number of keys 4  
  - key 1024 value_offset 1 - GTModelTypeGeoKey: ModelTypeProjected  
  - key 3072 value_offset 26917 - ProjectedCSTypeGeoKey: PCS_NAD83_UTM_zone_17N  
  - key 3076 value_offset 9001 - ProjLinearUnitsGeoKey: Linear_Meter  
  - key 4099 value_offset 9001 - VerticalUnitsGeoKey: Linear_Meter  


    las2las64 -i s1885565.laz -o out.las -sp83 OH_S -feet -elevation_feet -target_longlat

Reprojects the points from the Ohio_South NAD83 state plane with all units
in feet to geographic coordinates with x being longitude and y latitude and
sets these three geokeys as the projection information:

  GeoKeyDirectoryTag version 1.1.0 number of keys 3  
  - key 1024 value_offset 2 - GTModelTypeGeoKey: ModelTypeGeographic  
  - key 2048 value_offset 4269 - GeographicTypeGeoKey: GCS_NAD83  
  - key 4099 value_offset 9001 - VerticalUnitsGeoKey: Linear_Meter  


    las2las64 -i s1885565.laz -o out.las -sp83 OH_S -feet -elevation_feet -target_sp83 OH_N -target_survey_feet -target_elevation_survey_feet 
    las2las64 -i TO_core_last_zoom.laz -o out.laz -utm 17T
    las2las64 -i TO_core_last_zoom.laz -o out.laz -utm 17T -target_latlong

other variations of adding / changing projection information.


    las2las64 -i *.las -last_only

processes all LAS files that match *.las and stores only the last returns
to a corresponding LAS file called *_1.las (an added '_1' in the name).


    las2las64 -i *.las -olaz -keep_tile 630000 4830000 10000

keeps a 10000 by 10000 tile with a lower left coordinate of x=630000
and y=4830000 out of all LAS files that match *.las and stores each as a
compressed LAZ file *_1.laz (an added '_1' in the name).


    las2las64 -i *.txt -iparse xyztiarn -keep_scan_angle -15 15

processes all ASCII files that match *.txt, parses them with "xyztiarn",
keeps all points whose scan angle is between -15 and 15, and stores them
to a corresponding LAS file called *_1.las (an added '_1' in the name).


    las2las64 -i in.las -o out.las -keep_xy 630250 4834500 630500 4834750

keeps only points of in.las whose double-precision coordinates fall inside
the rectangle (630250,4834500) to (630500,4834750) and stores these points 
to out.las.


    las2las64 -lof file_list.txt -merged -o out.laz -keep_circle 630000 4850000 100

keeps only those points from all files listed in the list of files file_list.txt
whose double-precision coordinates fall into the circle centered at 630000 4850000
with radius 100 and stores these points compressed to out.laz.


    las2las64 -i in.las -o out.las -keep_z 10 100

keeps points of in.las whose double-precision elevations falls inside the
range 10 to 100 and stores these points to out.las.


    las2las64 -i in.las -o out.laz -drop_return 1

drops all points of in.las that are designated first returns by
the value in their return_number field and stores surviving points
compressed to out.laz.


    las2las64 -i in.laz -o out.las -drop_scan_angle_above 15

drops all points of compressed in.laz whose scan angle is above 15 or
below -15 and stores surviving points compressed to out.laz.


    las2las64 -i in.las -o out.las -drop_intensity_below 1000 -remove_padding

drops all points of in.las whose intensity is below 1000 and stores
surviving points to out.las. in addition any additional user data after
the LAS header or after the VLR block are stripped from the file.


    las2las64 -i in.laz -o out.laz -last_only

extracts all last return points from compressed in.laz and stores them
compressed to out.laz.


    las2las64 -i in.las -o out.las -scale_rgb_up

multiplies all rgb values in the file with 256. this is used to scale
the rgb values from standard unsigned char range (0 ... 255) to the
unsigned short range (0 ... 65535) used in the LAS format.


    las2las64 -i in.laz -o out.laz -scale_rgb_down

does the opposite with compressed input and output files


    las2las64 -i in.las -o out.las -subseq 1000 2000

extracts a subsequence of points by skipping the first 1000 points and
then collecting points until 2000 points were read.


    las2las64 -i in.las -o out.las -keep_class 2 -keep_class 3

extracts all points classfied as 2 or 3 from in.las and stores
them to out.las.


    las2las64 -i in.las -o out.las -keep_XY 63025000 483450000 63050000 483475000

similar to '-keep_xy' but uses the integer values point.X and point.Y
that the points are stored with for the checks (and not the double
precision floating point coordinates they represent). drops all the
points of in.las that have point.X<63025000 or point.Y<483450000 or
point.X>63050000 or point.Y>483475000 and stores surviving points to
out.las (use lasinfo.exe to see the range of point.Z and point.Y).


    las2las64 -i in.las -o out.las -keep_Z 1000 4000

similar to '-keep_z' but uses the integer values point.Z that the
points are stored with for the checks (and not the double-precision
floating point coordinates they represent). drops all the points
of in.las that have point.Z<1000 or point.Z>4000 and stores all
surviving points to out.las (use lasinfo.exe to see the range of
point.Z).


    las2las64 -h
    las2las64 -i *.las -utm 13N
    las2las64 -i *.laz -first_only -olaz
    las2las64 -i *.las -drop_return 4 5 -olaz
    las2las64 -latlong -target_utm 12T -i in.las -o out.las
    las2las64 -i in.laz -target_epsg 2972 -o out.laz
    las2las64 -set_point_type 0 -lof file_list.txt -merged -o out.las
    las2las64 -remove_vlr 2 -scale_rgb_up -i in.las -o out.las
    las2las64 -i in.las -keep_xy 630000 4834500 630500 4835000 -keep_z 10 100 -o out.las
    las2las64 -i in.txt -iparse xyzit -keep_circle 630200 4834750 100 -oparse xyzit -o out.txt
    las2las64 -i in.las -remove_padding -keep_scan_angle -15 15 -o out.las
    las2las64 -i in.las -rescale 0.01 0.01 0.01 -reoffset 0 300000 0 -o out.las
    las2las64 -i in.las -set_version 1.2 -keep_gpstime 46.5 47.5 -o out.las
    las2las64 -i in.las -drop_intensity_below 10 -olaz -stdout > out.laz
    las2las64 -i in.las -last_only -drop_gpstime_below 46.75 -otxt -oparse xyzt -stdout > out.txt
    las2las64 -i in.las -remove_all_vlrs -keep_class 2 3 4 -olas -stdout > out.las


## las2las specific arguments

-add_attribute [m] [n] [o] [p] [q] [t]: adds a new "extra_byte" attribute of data_type [m] name [n] description [o]; optional: scale[p] offset [q] no_data_value [t]  
-add_empty_vlr [m] [n] [o]          : add an empty VLR with user-id [m], record-id [n] and description [o]  
-adjusted_to_week                   : converts time stamps from Adjusted Standard GPS to GPS week  
-cores [n]                          : process multiple inputs on [n] cores in parallel  
-crop_to_bb                         : removes points that falls outsize the bouding box specified in the LAS header  
-crop_to_bounding_box               : removes points that falls outsize the bouding box specified in the LAS header  
-dont_remove_empty_files            : do not remove files that have zero points remaining from disk  
-elevation_feet                     : use feet for elevation  
-feet                               : use feet  
-force                              : force a GPS week conversion even if conversion is suspect.  
-load_vlrs                          : loads all VLRs from a file called vlrs.vlr and adds them to each processed file  
-load_ogc_wkt [f]                   : loads the first single-string from file [f] and puts it into the place of the OGC WKT  
-move_evlrs_to_vlrs                 : move all EVLRs with small enough payload to VLR section  
-remove_all_evlrs                   : remove all EVLRs  
-remove_all_vlrs                    : remove all VLRs  
-remove_evlr [n]                    : remove EVLR with index [n]{0=first}  
-remove_evlrs_from_to [m] [n]       : remove EVLRs with index [m] to [n]{0=first}  
-remove_original_vlr                : removes VLR containing original header information created by on-the-fly buffering  
-remove_padding                     : remove user-defined bytes before and after the header  
-remove_tiling_vlr                  : removes VLR containing tiling information created by lastile  
-remove_vlr [n]                     : remove VLR with index [n]{0=first}  
-remove_vlrs_from_to [m] [n]        : remove VLRs with index [m] to [n]{0=first}  
-reoffset [x] [y] [z]               : puts a new offset [x] [y] [z] into the header and translates the points accordingly  
-rescale [x] [y] [z]                : puts a new scale [x] [y] [z] into the header and rescales the points accordingly  
-save_vlrs                          : saves all VLRs to a file called vlrs.vlr so they can be loaded into another file  
-set_attribute_offset [m] [n]       : set offset of the attribute [m]{0-based} in the extra bytes to [n]  
-set_attribute_scale [m] [n]        : set scale of the attribute [m]{0-based} in the extra bytes to [n]  
-set_classification [n]             : set classification to [n]  
-set_global_encoding_gps_bit [n]    : sets bit in global encoding field specifying Adjusted GPS Standard time stamps  
-set_lastiling_buffer_flag [0/1]    : sets buffer flag in LAStiling VLR (if it exists) to [0/1]  
-set_ogc_wkt [n]                    : translate GeoTIFF keys [n] into CRS string in OGC WKT format and add it as VLR  
-set_ogc_wkt_in_evlr [n]            : same as "set_ogc_wkt" but adds [n] as LAS 1.4 EVLR instead. really not recommended!!!  
-set_point_data_format [n]          : force point type to be [n]{1-10}  
-set_point_data_record_length [n]   : CAREFUL! sets the point data record length field of the LAS header to size [n] without checking whether this will corrupt the file  
-set_point_size [n]                 : force point size to be [n]  
-set_point_type [n]                 : force point type to be [n]{1-10}  
-set_version 1.2                    : set LAS version number to 1.2  
-set_version_major 1                : set LAS major version number to 1  
-set_version_minor 2                : set LAS minor version number to 2  
-start_at_point [n]                 : skips all points until point number [n]  
-stop_at_point [n]                  : omits all points after point number [n]  
-subseq [m] [n]                     : extract a subsequence, start from [m] using [n] points  
-switch_G_B                         : switch green and blue value  
-unset_attribute_offset [n]         : unsets the offset of attribute [n]{0=first} in the extra bytes  
-unset_attribute_scale [n]          : unsets the scale of attribute [n]{0=first} in the extra bytes  
-week_to_adjusted [n]               : converts time stamps from GPS week [n] to Adjusted Standard GPS  

### Basics
-cpu64        : start 64 bit executable (instead of default 32 bit executable)  
-fail         : fail if license expired or invalid  
-gui          : start with files loaded into GUI  
-h            : print help output  
-help         : print help output  
-v            : verbose output (print extra information)  
-verbose      : verbose output (print extra information)  
-version      : reports this tool's version number  
-very_verbose : very verbose output (print even more information)  
-vv           : very verbose output (print even more information)  

## Module arguments

### General
-buffered [n]      : define read or write buffer of size [n]{default=262144}  
-chunk_size [n]    : set chunk size [n] in number of bytes  
-comma_not_point   : use comma instead of point as decimal separator  
-neighbors [n]     : set neighbors filename or wildcard [n]  
-neighbors_lof [n] : set neighbors list of files [fnf]  
-stored            : use in memory reader  
-unique            : remove duplicate points  

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

### CRS
-aeac [m] [n] [meter/survey_feet/feet] [o] [p] [q] [r]: Albers Equal Area Conic Projection: False Easting [m] False Northing[n] [meter/survey_feet/feet] Central Meridian [o] Standard Parallel 1 [p] Standard Parallel 2 [q] Latitude of origin [r]  
-ecef                               : input is geocentric (Earth-centered Earth-fixed)  
-elevation_meter                    : use meter for elevation  
-elevation_survey_feet              : set vertical units from meters to US survey feet  
-elevation_surveyfeet               : use survey feet for elevation  
-ellipsoid [n]                      : use the WGS-84 ellipsoid [n]{do -ellipsoid -1 for a list of ellipsoids}  
-epsg [n]                           : set datum to EPSG [n]  
-etrs89                             : use datum ETRS89  
-gda2020                            : use datum GDA2020  
-gda94                              : use datum GDA94  
-grs80                              : use datum GRS1980  
-latlong                            : geometric coordinates in latitude/longitude order  
-lcc 609601.22 0.0 meter 33.75 -79 34.33333 36.16666: specifies a lambertian conic confomal projection  
-longlat                            : geometric coordinates in longitude/latitude order  
-meter                              : use meter  
-nad27                              : use the NAD27 ellipsoid  
-nad83                              : use the NAD83 ellipsoid  
-nad83_2011                         : use datum NAD83_2011  
-nad83_csrs                         : use datum NAD83_CSRS  
-nad83_harn                         : use datum NAD83_HARN  
-nad83_pa11                         : set horizontal datum to NAD83 PA11  
-osgb1936                           : use datum OSGB 1936  
-sp27 SC_N                          : use the NAD27 South Carolina North state plane  
-sp83 CO_S                          : use the NAD83 Colorado South state plane for georeferencing  
-survey_feet                        : use survey feet  
-surveyfeet                         : use survey feet as unit of measurement  
-target_aeac [m] [n] [meter/survey_feet/feet] [o] [p] [q] [r]: Albers Equal Area Conic Projection for target: False Easting [m] False Northing[n] [meter/survey_feet/feet] Central Meridian [o] Standard Parallel 1 [p] Standard Parallel 2 [q] Latitude of origin [r]  
-target_ecef                        : output is geocentric (Earth-centered Earth-fixed)  
-target_elevation_feet              : output uses feet for elevation  
-target_elevation_meter             : output uses meter for elevation  
-target_elevation_precision [n]     : output uses [n] (meter/feet) resolution for z  
-target_elevation_survey_feet       : use elevation survey feet as target unit  
-target_elevation_surveyfeet        : output uses survey feet for elevation  
-target_epsg [n]                    : output is EPSG code [n] (e.g. 2193=NZGD2000)  
-target_feet                        : output uses feet  
-target_latlong                     : output is geometric coordinates in latitude/longitude  
-target_lcc 609601.22 0.0 meter 33.75 -79 34.33333 36.16666: specifies a lambertian conic confomal projection at target  
-target_longlat                     : output is geometric coordinates in longitude/latitude  
-target_meter                       : output uses meter  
-target_precision [n]               : output uses [n] (meter/feet) resolution for x and y  
-target_sp27 SC_N                   : output is state plane NAD27 South Carolina North  
-target_sp83 CO_S                   : output is state plane NAD83 Colorado South  
-target_survey_feet                 : output uses survey feet  
-target_surveyfeet                  : use survey feet as target unit  
-target_tm                          : use transverse mercator projection for target  
-target_utm 12T                     : output is UTM zone 12T  
-tm 609601.22 0.0 meter 33.75 -79 0.99996: specifies a transverse mercator projection  
-transverse_mercator                : use transverse mercator projection  
-utm 12T                            : use UTM zone 12T  
-vertical_cgvd2013                  : set vertical datum to CGVD2013  
-vertical_cgvd28                    : set vertical datum to CGVD28  
-vertical_dhhn2016                  : set vertical datum to DHHN2016  
-vertical_dhhn92                    : set vertical datum to DHHN92  
-vertical_dvr90                     : set vertical datum to DVR90  
-vertical_epsg [n]                  : set vertical datum to EPSG [n]  
-vertical_evrf2007                  : set vertical datum to EVRF2007  
-vertical_navd29                    : set vertical datum to NAVD29  
-vertical_navd88                    : set vertical datum to NAVD88  
-vertical_ngvd29                    : set vertical datum to NGVD29  
-vertical_nn2000                    : set vertical datum to NN2000  
-vertical_nn54                      : set vertical datum to NN54  
-vertical_nzvd2016                  : set vertical datum to NZVD2016  
-vertical_wgs84                     : set vertical datum to WGS84  
-wgs72                              : use the WGS-72 ellipsoid  
-wgs84                              : use the WGS-84 ellipsoid  

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
-otxt            : output as textfile  
-owrl            : output as VRLM (Virtual Reality Modeling Language) text  
-pipe_on         : write output to command pipe, see also -std_in  
-populate        : populate header on output  
-stdout          : pipe to stdout  
-target_ecef     : output is geocentric (Earth-centered Earth-fixed)  
-temp_files [n]  : set base file name [n] for temp files (example: E:\tmp)  

### add_attribute
The '-add_attribute' argument allow as first parameter the datatype
out of this values:
  0 : undocumented - extra bytes specify value in options field
  1 : unsigned char (1 byte)
  2 : char (1 byte)
  3 : unsigned short (2 bytes)
  4 : short (2 bytes)
  5 : unsigned long (4 bytes)
  6 : long (4 bytes)
  7 : unsigned long long (8 bytes)
  8 : long long (8 bytes)
  9 : float (4 bytes)
  10 : double (8 bytes)
  11-30 : deprecated
  31-255 : reserved

### parse
The '-parse [xyz]' flag specifies how to set the 
columns in a ASCII output file. 
For example, 'tsxyzssa' means that the first number 
is the gpstime, the next number should be skipped, 
the next three numbers are the x, y, and z coordinate, 
the next two should be skipped and the next number
is the scan angle.

The other supported entries are:  
  x : [x] coordinate  
  y : [y] coordinate  
  z : [z] coordinate  
  X : unscaled raw [X] value 
  Y : unscaled raw [Y] value
  Z : unscaled raw [Z] value
  t : gps [t]ime  
  R : RGB [R]ed channel  
  G : RGB [G]reen channel  
  B : RGB [B]lue channel  
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
  m : point index, starting at 0
  M : point index, starting at 1
  W : all wavepacket attributes
  w : [w]avepacket descriptor index
  c : [c]lassification  
  u : [u]ser data  
  p : [p]oint source ID  
  e : [e]dge of flight line flag  
  d : [d]irection of scan flag  
  0-9 : additional attributes described as extra bytes (0 through 9)  

### output separator
The '-osep [sep]' argument specifies the output format of a text(xyz) output.
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

