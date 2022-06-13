# lasview

A simple OpenGL-based viewer for LIDAR in LAS/LAZ/ASCII format
that can also edit or delete points as well as compute/display
a TIN computed from (a selection of) the points.

EDITING & DELETING POINTS

Press <e> to start r<e>classifying point as noise (7). Use the
pop-up menu (right click) to select other target classes. Draw
a polygon enclosing some points and press <r> to register the
operation. With <ESC> you can unset the last polygon point and
with <CTRL+u> you can undo the operation or with <CTRL+o> redo
it again.

Press <d> to start <d>eleting points. Otherwise same procedure
as above.

When finished press <CTRL+s> to save the edits as LASlayers in
to a LAY file. Warning ... if you did not start lasview with
the '-ilay' option you will not append to an existing LAY file
but overwrite it. There will be a WARNING though. Later, you
can check your layers of edits with 'laslayers.exe' or view
and continue editing with the '-ilay' option when starting
lasview again.

If you want to apply your edits and creating a new file press
<CTRL-a> and an entirely new file (currently with "_1" at the
end of the input file name) is created.

Do not use lasview with on-the-fly filters or transforms when
you plan to do editing operations that you are going to save.

VIEWING AND TRAVERSING CROSS SECTIONS 

For quality checking and detail views you can press <x> and
select a cross section or smaller area of interest. By default
you will draw a profile rectangle. Holding down <SHIFT> draws
a profile line. Its width can be chosen in the pop-up menu. Move
either around with the arrow key. Holding down <SHIFT> or <ALT>
is taking larger or smaller steps. Press <x> again and you see
only the selected area. Again, traverse the LiDAR my moving
around with the arrow keys. You can do your edits in this
cross section view. Jumping back between cross section view
and the full extend with <x> help maintaining orientation.

With <CTRL-x> you can end the cross section view.


## Examples

    lasview lidar.las
    lasview -i lidar.las

reads around 1000000 subsampled lidar points and displays in 50 steps


    lasview *.las

merges all LAS files into one and displays a subsampling of it


    lasview -i lidar.txt -iparse xyzc

converts an ASCII file on-the-fly with parse string 'xyzc' and displays it


    lasview -i lidar.las -win 1600 1200

same as above but with a larger display window


    lasview -i lidar.las -steps 10 -points 10000000

reads around 10 million subsampled lidar points and displays in 11 steps


## Interactive options

<t>     compute a TIN from the displayed returns  
<T>     remove the TIN <SHIFT>+<t>  
<h>     change shading mode for TIN (hill-shade, elevation, wire-frame)  
<G>     show rasterization "needles" for grid set by '-grid n' value <SHIFT>+<g>  
<R>     raster TIN with grid cell size set by '-grid n' value <SHIFT>+<r>  
  
<y>     take one step of spike-free algorithm (press many times to see algorithm in action)  
<Y>     complete spike-free algorithm to end <SHIFT>+<y>  
  
<a>     display all returns  
<l>     display last returns only  
<f>     display first returns only  
<g>     display returns classified as ground  
<b>     display returns classified as building  
<B>     display returns classified as building + ground <SHIFT>+<b>  
<v>     display returns classified as vegetation  
<V>     display returns classified as vegetation + ground <SHIFT>+<v>  
<j>     display returns classified as building + vegetation  
<J>     display returns classified as building + vegetation + ground <SHIFT>+<j>  
<m>     display returns classified as keypoints / mass points  
<n>     display returns classified as noise  
<o>     display returns classified as overlap  
<w>     display returns classified as water  
<u>     display returns that are unclassified  
<0>     display ((point source IDs % 8) == 0)  
<1>     display ((point source IDs % 8) == 1)  
<2>     display ((point source IDs % 8) == 2)  
<3>     display ((point source IDs % 8) == 3)  
<4>     display ((point source IDs % 8) == 4)  
<5>     display ((point source IDs % 8) == 5)  
<6>     display ((point source IDs % 8) == 6)  
<7>     display ((point source IDs % 8) == 7)  
  
<e>      start editing points  
<d>      start deleting points  
<r>      register edit / delete operation  
  
<CTRL-u> undo last changes  
<CTRL-o> redo last undo  
  
<CTRL-s> save changes as LASlayers  
<CTRL-f> force LAY file overwrite  
<CTRL-a> apply LASlayers to create new LAS/LAZ file  
  
<i>       pick a point  
<I>       pick a point and draw line to last picked point <SHIFT>+<i>  
<x>       turn on and toggle between overview and rectangle / line cross-section   
<SHIFT>   hold down to pick line instead of rectangle cross-section when in overview   
<CTRL-x>  overview and rectangle / line cross-section view on / off  
  
<SHIFT> translate mode  
<CTRL>  zoom mode  
<ALT>   pan mode  
<space> switch between pan/translate/zoom/tilt  
<-/=>   render points smaller/bigger  
<[/]>   scale elevation  
<{/}>   scale xy plane  
<c>     change color mode  
<X>     hide/show bounding box <SHIFT>+<x>  
<s/S>   step forward/backward  
<z/Z>   tiny step forward/backward  
  
<A>     toggle adaptive Z-scaling on/off <SHIFT>+<a>  
<Q>     show spatial index structure (if LAX file available) <SHIFT>+<q>  
<q>     pick spatial index cell  
<E>     render structural elements around all points <SHIFT>+<e>  
<:>     decrease structural element radius  
<">     increase structural element radius  
<W>     show LAS 1.3 waveforms +/- 25 points around picked point <SHIFT>+<w>


## lasview specific arguments

-background_black                   : start viewer with a black background  
-background_blue                    : start viewer with a blue background  
-background_green                   : start viewer with a green background  
-background_grey                    : start viewer with a grey background  
-background_red                     : start viewer with a red background  
-background_white                   : start viewer with a white background  
-buildings                          : render only points classified as building  
-bulge [n]                          : bulge sloped neighbourhoods of TIN triangles by [n](default=step/10)  
-camera 0 -59 -24 0 -0.0572 0.385   : move virtual camera to specifies parameters (print parameters with <K> or <SHIFT>+<k>)  
-circles [n]                        : define structural elements as circles of radius [n]  
-color_by_classification            : render points by classification color  
-color_by_elevation1                : render points by elevation color ramp (black->red->yellow->white)  
-color_by_elevation2                : render points by elevation color ramp (blue->yellow->red)  
-color_by_flightline                : render points by randomly assigning 8 different colors based on the flightline ID  
-color_by_intensity                 : render points by intensity  
-color_by_return                    : render points by return colors (single = yellow, first of many = red, last of many = blue, intermediate = green)  
-color_by_rgb                       : render points by RGB color  
-color_by_user_data                 : render points by mapping the 8-bit user data field to a color ramp (blue->green->red)  
-concavity [n]                      : remove large exterior triangles from TIN who have an edge longer [n](default=50)  
-cones [m] [n]                      : define structural elements as cones of radius [m] and height [n]  
-cores [n]                          : process multiple inputs on [n] cores in parallel  
-cp [fnt]                           : load control points with parse string xyz from file [fnt] and visualize each as red sphere with radius 1 meter  
-cp_parse [xyz]                     : use parse string [xyz] to parse control point file  
-every [n]                          : visualize incremental loading of points every [n] points  
-flats [m] [n] [o]                  : define structural elements as flats of small radius [m], height [n] and large radius [o]  
-grid [n]                           : set raster spacing [n] for visualization of rasterization with <R> (default=1)  
-ground                             : render only points classified as ground  
-ground_buildings                   : render only points classified as ground or building  
-ground_object                      : render only points classified as ground, vegetation, or building  
-ground_vegetation                  : render only points classified as ground or vegetation  
-holes [n]                          : remove any triangle from TIN that has an edge length bigger than [n]  
-ilay [n]                           : apply [n] or all LASlayers found in corresponding *.lay file on read  
-ilaydir [n]                        : look for corresponding *.lay file in directory [n]  
-kamera [x] [y] [z] [dx] [dy] [dz]  : move virtual camera to specified parameters (print parameters with <K> or <SHIFT>+<k>)  
-kill [n]                           : remove any triangle from TIN that has an edge length bigger than [n]  
-light [x] [y] [z]                  : set light direction vector to [x] [y] [z] for hillshaded TIN  
-load_gps_second                    : also loads GPS time stamps from file and displays them when pressing <i>  
-load_gps_time                      : also loads GPS time stamps from file and displays them when pressing <i>  
-mark_down_spike [n]                : set markers at TIN spikes larger than [n]  
-mark_down_spike2 [m] [n]           : set markers at TIN spikes larger than [m] within a distance of [n]  
-mark_point [n]                     : surround point at position [n] in file with a pink sphere of 0.5 meter radius  
-no_bounding_box                    : do not render the green bounding box  
-object                             : render only points classified as vegetation or building  
-olaydir [n]                        : write the output *.lay file (from editing) in directory [n]  
-only_first                         : render only first returns  
-only_last                          : render only last returns  
-only_multi                         : render only multiple returns  
-only_single                        : render only single returns  
-pit_free                           : run spike-free algorithm with defaults freeze = 1.5, interval = 0.25, buffer = 0.5  
-point_size [n]                     : draw points with size [n]  
-points [n]                         : load maximally [n] points from file (with regular sub sampling)(default=5 mil.)  
-points_all                         : force to load all points (otherwise: skip points on input larger than 5 mio points)  
-profile_line [x1] [y1] [x2] [y2]   : run in profile line mode with bounding box [x1] [y1] [x2] [y2]  
-profile_rectangle [x1] [y1] [x2] [y2]: run in profile rectangle mode with bounding box [x1] [y1] [x2] [y2]  
-reversed                           : run spike-free in reverse (from bottom to top)  
-scale_rgb                          : down-scale 16 bit RGB values for storage to 8 bit RGB values (usually auto-detected).  
-screenshot [n]                     : create a screenshot and save in file [n]  
-set_min_max [m] [n]                : clamp color ramp used for elevation colorings to range [m] [n] instead of (min-max)  
-spike_free                         : run spike-free algorithm with defaults freeze = 1.5, interval = 0.25, buffer = 0.5  
-spike_free [m] [n] [o]             : run spike-free algorithm with freeze = [m], interval = [n], buffer = [o]  
-start_at_point [n]                 : start loading from point position [n]  
-steep [n]                          : remove all "steep" triangles from the TIN with z values spanning more than [n] meters  
-steps [n]                          : visualize incremental loading of points from file in [n] steps (default=50)  
-stop_at_point [n]                  : stop loading after [n] points  
-subcircle [n]                      : prior to creating TIN with <t> replace each point with 8 segment circle of radius [n]  
-subseq [m] [n]                     : only load subsequence from point [m] to [n]  
-suppress_classification            : do not decompress classification for native-compressed LAS 1.4 point types 6 or higher  
-suppress_intensity                 : do not decompress intensity for native-compressed LAS 1.4 point types 6 or higher  
-suppress_point_source              : do not decompress point source ID for native-compressed LAS 1.4 point types 6 or higher  
-suppress_RGB                       : do not decompress RGB for native-compressed LAS 1.4 point types 6 or higher  
-suppress_user_data                 : do not decompress user data field for native-compressed LAS 1.4 point types 6 or higher  
-suppress_z                         : do not decompress z coordinates for native-compressed LAS 1.4 point types 6 or higher  
-switch_G_B                         : switch green and blue value  
-vegetation                         : render only points classified as vegetation  
-week_to_adjusted [n]               : converts time stamps from GPS week [n] to Adjusted Standard GPS  
-win [m] [n]                        : start with render window size of [m] by [n] pixels  

### Basics
-fail    : fail if license expired or invalid  
-gui     : start with files loaded into GUI  
-h       : print help output  
-help    : print help output  
-license : show license information  
-v       : verbose output (print extra information)  
-verbose : verbose output (print extra information)  
-version : reports this tool's version number

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
