****************************************************************

  lasground:

  This is a tool for bare-earth extraction: it classifies LIDAR
  points into ground points (class = 2) and non-ground points
  (class = 1). This tools works very well in natural environments
  such as mountains, forests, fields, hills, or other terrain
  with few man-made objects. Note that lasground was designed for
  airborne LiDAR. When using lasground with terrestrial or with
  mobile data play with the '-not_airborne' option (which may
  work better) or try the '-no_stddev' or '-no_bulge' flags.

  You can use other classifications than the ASRPS standard with
  '-ground_class 10' or '-non_ground_class 25'. It is possible to
  leave the classification of points that are not added to the
  ground class intact with option '-non_ground_unchanged'.

  The tool also produces excellent results for town or cities
  but buildings larger than the step size can be problematic.
  The default step size is 5 meters, which is good for forest
  or mountains. For towns or flat terrains '-town' the step
  size is increased to 10 meters. For cities or warehouses
  '-city' the step size is increased to 25 meters. For very
  large cities use '-metro' and the step size is increased
  to 50 meters You can also set it directly with '-step 35'.

  You can classify the points '-by_flightline' assuming that
  the point source ID of each point properly specifies from
  which flightline a point is.

  It is important to tell the tool whether the horizontal and
  vertical units are meters (which is assumed by default) or
  '-feet' or '-elevation_feet'. Should the LAS file contain
  projection information then there is no need to specify this
  explicitly. If the input coordinates are in an earth-centered
  or a longlat representation, the file needs converted to, for
  example, a UTM projection first. That said, some folks have
  successfully ground-classified longlat represtations using a
  very small '-step 0.000005' or so.

  By default the tool only considers the last return. Earlier
  returns are considered non-ground. You can turn this off by
  requesting '-all_returns'. If you want to leave out certain
  classifications from the bare-earth calculation you can do
  so with '-ignore_class 7'.

  For very steep hills you can intensify the search for initial
  ground points with '-fine' or '-extra_fine' and similarly for
  flat terrains you can simplify the search with '-coarse' or
  '-extra_coarse' but try the default setting first. 

  The experienced user can fine-tune the algorithm by specifing
  the threshold in meters at which spikes get removed. setting
  '-spike 0.5' will remove up-spikes above 50 centimeter and
  down-spikes below 5 meters in the coarsest TIN.

  Another parameter of interest is the '-bulge 1.0' parameter
  that specifies how much the TIN is allowed to bulge up when
  including points as it is getting refined. The default bulge
  is one tenth of the step for step sizes larger than 5 meters
  and one fifth of the step otherwise. Especially for ground-
  classification of non-LiDAR points such as dense-matching or
  photogrammetry output by Agisoft of Pix4D the fine-tuning of
  this parameter can produce great results.

  The maximal standard deviation for planar patches in centimeter
  can be set with '-stddev 10'. The maximal offset in meters up
  to which points above the current ground estimate get included
  can be set with '-offset 0.1'.

  Finally you can ask lasground to compute the height above the
  ground for each point (so you can use lasclassify next without
  needing to run lasheight first) with '-compute_height' or even
  ask to have the computed height replace the elevation value with
  option '-replace_z'. Then you directly get a height normalized
  LAS/LAZ file that can be used, for example, with lascanopy or
  lasgrid for canopy height model (CHM) computations.

  Should lasground miss-behave try turning off some optimizations
  using the '-no_stddev' or the '-no_bulge' flags.

  If there are too few points to do reliable ground classification
  then the files are simply copied (and in case of '-replace_z' all
  elevations are zeroed). Alternatively these files can be skipped
  with the command-line option '-skip_files'.

  Please license from martin@rapidlasso.com to use lasground
  commercially. Please note that the unlicensed version will set
  intensity, gps_time, user data, and point source ID to zero,
  slightly change the LAS point order, and randomly add a tiny
  bit of white noise to the points coordinates.

  For updates check the website or join the LAStools mailing list.

  http://rapidlasso.com/
  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @lastools

****************************************************************

example usage:

>> lasground -i terrain.las -o classified_terrain.las

classifies a terrain with the default settings.

>> lasground -i terrain.las -o classified_terrain.las -feet -elevation_feet

classifies a terrain where both horizontal and vertical units are
in feet instead of in meters (which is assumed by default unless
there is projection information in the LAS file saying otherwise).

>> lasground -i terrain.las -o classified_terrain.las -all_returns

classifies a terrain considering all points - not just the last
returns (as is the default behavior).

>> lasground -i *.las

classifies all LAS files with the default settings.

>> lasground -i *.las -town

the same as above but uses wider spacing to allow for small
buildings and other man-made structures.

>> lasground -i *.las -city

the same as above but uses even wider spacing to allow for
very large buildings.

****************************************************************

overview of all tool-specific switches:

-v                                   : more info reported in console
-vv                                  : even more info reported in console
-quiet                               : nothing reported in console
-wait                                : wait for <ENTER> in the console at end of process
-version                             : reports this tool's version number
-fail                                : fail if license expired or invalid
-gui                                 : start with files loaded into GUI
-cores 4                             : process multiple inputs on 4 cores in parallel
-ignore_class 1 3 4 5 6 7 9          : ignores points with specified classification codes
-ignore_extended_class 42 43 45 67   : ignores points with specified extended classification codes
-ignore_single                       : ignores single returns
-ignore_first                        : ignores first returns
-ignore_last                         : ignores last returns
-ignore_first_of_many                : ignores first returns (but only those of multi-returns)
-ignore_intermediate                 : ignores intermediate returns
-ignore_last_of_many                 : ignores last returns (but only those of multi-returns)
-ignore_withheld                     : ignores points flagged withheld
-ignore_overlap                      : ignores points flagged overlap (new LAS 1.4 point types only)
-step 8.0                            : set resolution of grid used for initial ground to 8.0 [default is 5.0]
-sub 5                               :
-offset 0.02                         : offset that points can be above bulged ground TIN and still become ground [default is 0.05]
-bulge 1.8                           : amount that TIN triangles with sloped neighbourhoods are bulging up or down [default is 
-no_bulge                            : sets bulge to zero 
-spike 0.5                           :
-no_clean                            : do not perform cleaning steps that remove spikes from the current ground TIN estimate
-stddev 0.1                          :
-no_stddev                           : sets stddev to zero 
-not_airborne                        : same as setting these three: '-no_bulge', '-all_returns', and '-stddev 0' 
-pertube                             :
-by_flightline                       : ground classfies each flightline separately (needs populated point source IDs)
-iterate 1                           :
-no_iterate                          : sets max number of refinement loops to zero 
-refine 3                            :
-no_refine                           : sets max number of refinement loops to zero 
-extra_coarse                        :
-coarse                              :
-fine                                :
-extra_fine                          :
-ultra_fine                          :
-hyper_fine                          :
-archeology                          :
-wilderness                          :
-nature                              :
-town                                :
-city                                :
-metro                               :
-ground_class 8                      :
-non_ground_class 20                 :
-non_ground_unchanged                :
-all_returns                         :
-extra_pass                          :
-cutoff_z_above 180.0                : points above this elevation are completely excluded from ground search
-compute_height                      : 
-replace_z                           :
-store_in_user_data                  :
-skip_files                          : skip (instead of the default copy) files that have an insufficient number of ground points
-remain_buffered                     : write buffer points to output when using '-buffered 25' on-the-fly buffering  
-ilay                                : apply all LASlayers found in corresponding *.lay file on read
-ilay 3                              : apply first three LASlayers found in corresponding *.lay file on read
-ilaydir E:\my_layers                : look for corresponding *.lay file in directory E:\my_layers
-olay                                : write or append classification changes to a LASlayers *.lay file
-olaydir E:\my_layers                : write the output *.lay file in directory E:\my_layers

****************************************************************

for more info:

C:\lastools\bin>lasground -h
Filter points based on their coordinates.
  -keep_tile 631000 4834000 1000 (ll_x ll_y size)
  -keep_circle 630250.00 4834750.00 100 (x y radius)
  -keep_xy 630000 4834000 631000 4836000 (min_x min_y max_x max_y)
  -drop_xy 630000 4834000 631000 4836000 (min_x min_y max_x max_y)
  -keep_x 631500.50 631501.00 (min_x max_x)
  -drop_x 631500.50 631501.00 (min_x max_x)
  -drop_x_below 630000.50 (min_x)
  -drop_x_above 630500.50 (max_x)
  -keep_y 4834500.25 4834550.25 (min_y max_y)
  -drop_y 4834500.25 4834550.25 (min_y max_y)
  -drop_y_below 4834500.25 (min_y)
  -drop_y_above 4836000.75 (max_y)
  -keep_z 11.125 130.725 (min_z max_z)
  -drop_z 11.125 130.725 (min_z max_z)
  -drop_z_below 11.125 (min_z)
  -drop_z_above 130.725 (max_z)
  -keep_xyz 620000 4830000 100 621000 4831000 200 (min_x min_y min_z max_x max_y max_z)
  -drop_xyz 620000 4830000 100 621000 4831000 200 (min_x min_y min_z max_x max_y max_z)
Filter points based on their return number.
  -first_only -keep_first -drop_first
  -last_only -keep_last -drop_last
  -keep_middle -drop_middle
  -keep_return 1 2 3
  -drop_return 3 4
  -keep_single -drop_single
  -keep_double -drop_double
  -keep_triple -drop_triple
  -keep_quadruple -drop_quadruple
  -keep_quintuple -drop_quintuple
Filter points based on the scanline flags.
  -drop_scan_direction 0
  -scan_direction_change_only
  -edge_of_flight_line_only
Filter points based on their intensity.
  -keep_intensity 20 380
  -drop_intensity_below 20
  -drop_intensity_above 380
  -drop_intensity_between 4000 5000
Filter points based on their classification.
  -keep_class 1 3 7
  -drop_class 4 2
  -drop_synthetic -keep_synthetic
  -drop_keypoint -keep_keypoint
  -drop_withheld -keep_withheld
Filter points based on their user data.
  -keep_user_data 1
  -drop_user_data 255
  -keep_user_data_between 10 20
  -drop_user_data_below 1
  -drop_user_data_above 100
  -drop_user_data_between 10 40
Filter points based on their point source ID.
  -keep_point_source 3
  -keep_point_source_between 2 6
  -drop_point_source 27
  -drop_point_source_below 6
  -drop_point_source_above 15
  -drop_point_source_between 17 21
Filter points based on their scan angle.
  -keep_scan_angle -15 15
  -drop_abs_scan_angle_above 15
  -drop_scan_angle_below -15
  -drop_scan_angle_above 15
  -drop_scan_angle_between -25 -23
Filter points based on their gps time.
  -keep_gps_time 11.125 130.725
  -drop_gps_time_below 11.125
  -drop_gps_time_above 130.725
  -drop_gps_time_between 22.0 48.0
Filter points based on their wavepacket.
  -keep_wavepacket 0
  -drop_wavepacket 3
Filter points with simple thinning.
  -keep_every_nth 2
  -keep_random_fraction 0.1
  -thin_with_grid 1.0
Transform coordinates.
  -translate_x -2.5
  -scale_z 0.3048
  -rotate_xy 15.0 620000 4100000 (angle + origin)
  -translate_xyz 0.5 0.5 0
  -translate_then_scale_y -0.5 1.001
  -switch_x_y -switch_x_z -switch_y_z
  -clamp_z_below 70.5
  -clamp_z 70.5 72.5
Transform raw xyz integers.
  -translate_raw_z 20
  -translate_raw_xyz 1 1 0
  -clamp_raw_z 500 800
Transform intensity.
  -scale_intensity 2.5
  -translate_intensity 50
  -translate_then_scale_intensity 0.5 3.1
  -clamp_intensity 0 255
  -clamp_intensity_above 255
Transform scan_angle.
  -scale_scan_angle 1.944445
  -translate_scan_angle -5
  -translate_then_scale_scan_angle -0.5 2.1
Change the return number or return count of points.
  -repair_zero_returns
  -set_return_number 1
  -change_return_number_from_to 2 1
  -set_number_of_returns 2
  -change_number_of_returns_from_to 0 2
Modify the classification.
  -set_classification 2
  -change_classification_from_to 2 4
  -classify_z_below_as -5.0 7
  -classify_z_above_as 70.0 7
  -classify_z_between_as 2.0 5.0 4
  -classify_intensity_above_as 200 9
  -classify_intensity_below_as 30 11
Modify the user data.
  -set_user_data 0
  -change_user_data_from_to 23 26
Modify the point source ID.
  -set_point_source 500
  -change_point_source_from_to 1023 1024
  -quantize_Z_into_point_source 200
Transform gps_time.
  -translate_gps_time 40.50
  -adjusted_to_week
  -week_to_adjusted 1671
Transform RGB colors.
  -scale_rgb_down (by 256)
  -scale_rgb_up (by 256)
Supported LAS Inputs
  -i lidar.las
  -i lidar.laz
  -i lidar1.las lidar2.las lidar3.las -merged
  -i *.las - merged
  -i flight0??.laz flight1??.laz
  -i terrasolid.bin
  -i esri.shp
  -i nasa.qi
  -i lidar.txt -iparse xyzti -iskip 2 (on-the-fly from ASCII)
  -i lidar.txt -iparse xyzi -itranslate_intensity 1024
  -lof file_list.txt
  -stdin (pipe from stdin)
  -rescale 0.01 0.01 0.001
  -rescale_xy 0.01 0.01
  -rescale_z 0.01
  -reoffset 600000 4000000 0
Supported LAS Outputs
  -o lidar.las
  -o lidar.laz
  -o xyzta.txt -oparse xyzta (on-the-fly to ASCII)
  -o terrasolid.bin
  -o nasa.qi
  -odir C:\data\ground (specify output directory)
  -odix _classified (specify file name appendix)
  -ocut 2 (cut the last two characters from name)
  -olas -olaz -otxt -obin -oqfit (specify format)
  -stdout (pipe to stdout)
  -nil    (pipe to NULL)
LAStools (by martin@rapidlasso.com) version 140614 (unlicensed)
usage:
lasground -i in.las -o out.las
lasground -i in.las -o out.las -feet -elevation_feet
lasground -i in.las -o out.las -town
lasground -i in.las -o out.las -city -ignore_class 7
lasground -i in.las -o out.las -metro
lasground -i in.las -o out.las -v -step 10 -spike 2 -offset 0.1
lasground -i in.laz -o out.laz -no_stddev -no_bulge
lasground -i *.las -v -odix _g -olaz
lasground -i *.laz -v -odir ground_classified  -olaz
lasground -h

---------------

if you find bugs let me (martin.isenburg@rapidlasso.com) know.
