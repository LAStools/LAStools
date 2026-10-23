# laslayers

## general

WARNING: do not expect LAY files to work across different versions
of LAStools because the development of LASlayers is still
not completed. this is just an awesome prototype as proof
of concept. we continue to gather community input...

laslayers is explained with 3 exercises:

1. a first look at LASlayers with 'lasview', 'las2dem', and 'laslayers'
2. work with LASlayers using various LAStools for more I/O and
storage efficiency,
3. create LASlayers as difference files between an original and a modified
LAS/LAZ file using 'laslayers'.

## (1) first look at LASlayers

have a look at the ll.laz file with 'lasview':

lasview -i ll.laz

Now add the LASlayers contained in the 'll.lay' file:

lasview -i ll.laz -ilay

Now put on layer by layer (or strip-off going in reverse):

lasview -i ll.laz -ilay 0  
lasview -i ll.laz -ilay 1  
lasview -i ll.laz -ilay 2  
lasview -i ll.laz -ilay 3  
lasview -i ll.laz -ilay 4

Now have a look at the LASlayers inside the 'll.lay' file:

laslayers64 -i ll.laz

Now apply all LASlayers to create a new 'll\_lay4.laz' file:

laslayers64 -i ll.laz -ilay -o ll\_lay4.laz  
lasview -i ll\_lay4.laz

Now apply only the first two LASlayers:

laslayers64 -i ll.laz -ilay 2 -o ll\_lay2.laz  
lasview -i ll\_lay2.laz



## (2) work with LASlayers

imagine we obtained raw LAZ tiles and orthos from a customer and are now producing the desired products with LASlayers.

Classify isolated points as noise (7) with 'lasnoise':

lasnoise64 -i ll.laz -olay  
laslayers64 -i ll.laz  
lasview -i ll.laz -ilay

Or - alternatively - delete isolated points:

lasnoise64 -i ll.laz -olay -remove\_noise  
laslayers64 -i ll.laz  
lasview -i ll.laz -ilay

Create a noise-free DSM with 'lasgrid':

las2dem -i ll.laz -ilay -first\_only -step 0.5 -hillshade -odix \_dsm -opng

Delete xyz-duplicate points with 'lasduplicate':

lasduplicate64 -i ll.laz -ilay -unique\_xyz -olay  
laslayers64 -i ll.laz  
lasview -i ll.laz -ilay

Ground classify the noise-free points with 'lasground':

lasground64 -i ll.laz -ilay -fine -olay  
laslayers64 -i ll.laz  
lasview -i ll.laz -ilay

Create a DTM with 'las2dem':

las2dem64 -i ll.laz -ilay -keep\_class 2 -step 0.5 -hillshade -odix \_dtm -opng

Compute the height of points above the ground and store an approximation to user\_data with 'lasheight':

lasheight64 -i ll.laz -ilay -olay  
laslayers64 -i ll.laz  
lasview -i ll.laz -ilay -color\_by\_user\_data

Classify building and forest points with 'lasclassify':

lasclassify64 -i ll.laz -ilay -olay  
laslayers64 -i ll.laz  
lasview -i ll.laz -ilay

We do not like the result and want to adapt some parameters. Therefore we delete the last layers with 'laslayers':

laslayers64 -i ll.laz -del 5  
laslayers64 -i ll.laz  
lasview -i ll.laz -ilay

Classify again using different parameters with 'lasclassify':

lasclassify64 -i ll.laz -ilay -olay -rugged 2 -step 0.5  
laslayers64 -i ll.laz  
lasview -i ll.laz -ilay

Create raster splatted points only where vegetation is high with 'lasgrid':

lasgrid64 -i ll.laz -ilay -keep\_class 5 -subcircle 0.1 -step 0.5 -highest -false -use\_bb -odix \_veg -opng

Create polygons around high vegetation areas with 'lasboundary':

lasboundary64 -i ll.laz -ilay -keep\_class 5 -concavity 1.0 -disjoint -odix \_veg -oshp

Normalize the elevations to height above ground with 'lasheight':

lasheight64 -i ll.laz -ilay -replace\_z -olay  
laslayers64 -i ll.laz  
lasview -i ll.laz -ilay -color\_by\_elevation2

Create forestry metrics canopy cover and 50 / 75 / 95 percentile with 'lasheight':

lascanopy64 -i ll.laz -ilay -cov -p 50 75 95 -step 5 -oasc  
lasview -i ll\_p50.asc ll\_p75.asc ll\_p95.asc -files\_are\_flightlines -color\_by\_flightline

Color the points using an ortho TIF image with 'lascolor':

lascolor64 -i ll.laz -ilay -image ortho.tif -olay  
laslayers64 -i ll.laz  
lasview -i ll.laz -ilay

Look at the command sequence that got us here with 'laslayers':

laslayers64 -i ll.laz -short

Delete layer 4 (no longer needed) to make the LASlayers file smaller with 'laslayers'.

laslayers64 -i ll.laz -del 4  
laslayers64 -i ll.laz  
lasview -i ll.laz -ilay

Send the resulting small LAY file instead of the larger LAZ file back to the customer.



## (3) create LASlayers difference files

we have a raw and a modified LAS file and want to create LASlayers that express the difference between them, storing only those parts that have actually changed which is much more compact.

Look at the modified LAZ file with 'lasview':

lasview -i ll\_modified.laz

Create LASlayers that express the difference to the raw file with 'laslayers':

laslayers64 -i ll.laz -i ll\_modified.laz -olay  
laslayers64 -i ll.laz  
lasview -i ll.laz -ilay  
lasview -i ll.laz -ilay 1  
lasview -i ll.laz -ilay 2  
lasview -i ll.laz -ilay 3

Apply LASlayers to the raw file to produce the modified one with 'laslayers'.

laslayers64.exe -i ll.laz -ilay -o ll\_applied.laz  
lasview -i ll\_applied.laz

Make sure the two are identical with 'lasdiff':  
lasdiff64 -i ll\_modified.laz -i ll\_applied.laz



## laslayers specific arguments

\-add           : create or add to LAY files with LASlayers storing filters, transforms or file modifications  
-del \[n]       : delete layer with index \[n(1..32)]  
-del\_last \[n]  : delete last \[n] layers  
-detach        : detach layers  
-diff          : produce diff LAY file  
-ilay \[n]      : apply \[n] or all LASlayers found in corresponding \*.lay file on read  
-ilaydir \[n]   : look for corresponding \*.lay file in directory \[n]  
-olay          : write or append classification changes to a LASlayers \*.lay file  
-olaydir \[dir] : write the output \*.lay file in directory \[dir]  
-short         : output layer information only

### basics

\-cores \[n]      : process multiple inputs on \[n] cores in parallel  
-license        : show license information  
-demo           : use LAStools in demo mode (64bit only)  
-fail           : fail if license expired or invalid (default for 64bit)  
-h, -help       : print help output  
-v, -verbose    : verbose output (print extra information)  
-vv             : very verbose output (print even more information)  
-silent         : only output on errors or warnings  
-quiet          : no output at all  
-force          : continue, even if serious warnings occur  
-errors\_ignore  : continue, even if errors occur (if possible). Use with caution!  

\-skip\_file\_on\_error: skips the current input file when an unhandled processing error occurs and continues with the next file  
-print\_log\_stats: print additional log statistics  
-cpu64          : force 32bit version to start 64 bit in multi core (obsolete)  
-gui            : start with files loaded into GUI  
-version        : reports this tool's version number



## module arguments

### general

\-chunk\_size \[n]    : set chunk size \[n] in number of bytes  
-comma\_not\_point   : use comma instead of point as decimal separator  
-neighbors \[n]     : set neighbors filename or wildcard \[n]  
-neighbors\_lof \[n] : set neighbors list of files \[fnf]  
-stored            : use in memory reader

### color

\-clamp\_RGB\_to\_8bit                  : limit RGB values to 8 bit (otherwise: 16 bit)  
-copy\_B\_into\_NIR                    : copy blue color value into NearInfraRed value  
-copy\_B\_into\_intensity              : copy blue color value to intensity  
-copy\_B\_into\_register \[n]           : copy blue color value into register \[n]  
-copy\_G\_into\_NIR                    : copy green color value into NearInfraRed value  
-copy\_G\_into\_intensity              : copy green color value to intensity  
-copy\_G\_into\_register \[n]           : copy green color value into register \[n]  
-copy\_NIR\_into\_intensity            : copy NIR into intensity  
-copy\_NIR\_into\_register \[n]         : copy NearInfraRed value into register \[n]  
-copy\_RGB\_into\_intensity            : copy weighted RGB value to intensity  
-copy\_R\_into\_NIR                    : copy red color value into NearInfraRed value  
-copy\_R\_into\_intensity              : copy red color value to intensity  
-copy\_R\_into\_register \[n]           : copy red color value into register \[n]  
-copy\_attribute\_into\_B \[n]          : copy attribute \[n] value into blue  
-copy\_attribute\_into\_G \[n]          : copy attribute \[n] value into green  
-copy\_attribute\_into\_NIR \[n]        : copy attribute \[n] value into NIR (NearInfraRed)  
-copy\_attribute\_into\_R \[n]          : copy attribute \[n] value into red  
-copy\_intensity\_into\_NIR            : copy intensity into NIR (NearInfraRed) value  
-copy\_register\_into\_B \[n]           : copy register \[n] into blue color value  
-copy\_register\_into\_G \[n]           : copy register \[n] into green color value  
-copy\_register\_into\_I \[n]           : copy register \[n] into NearInfraRed value  
-copy\_register\_into\_NIR \[n]         : copy register \[n] into NearInfraRed value  
-copy\_register\_into\_R \[n]           : copy register \[n] into red color value  
-drop\_RGB\_green \[min] \[max]         : drop points with green color value between \[min] and \[max]  
-drop\_RGB\_red \[min] \[max]           : drop points with red color value between \[min] and \[max]  
-force\_RGB                          : force the use of the RGB value even if the point format does not support RGB  
-keep\_NDVI \[min] \[max]              : keep NDVI (Normalized Difference Vegetation Index) between \[min] \[max]  
-keep\_NDVI\_from\_CIR \[min] \[max]     : keep NDVI (Normalized Difference Vegetation Index) from CIR between \[min] \[max]  
-keep\_NDVI\_green\_is\_NIR \[min] \[max] : keep NDVI (Normalized Difference Vegetation Index) where green is NIR between \[min] \[max]  
-keep\_NDVI\_blue\_is\_NIR \[min] \[max]  : keep NDVI (Normalized Difference Vegetation Index) where blue is NIR between \[min] \[max]  
-keep\_NDVI\_intensity\_is\_NIR \[min] \[max]: keep NDVI (Normalized Difference Vegetation Index) where intensity is NIR between \[min] \[max]  
-keep\_RGB\_blue \[m] \[n]              : keep points with RGB blue color values between \[min] \[max]  
-keep\_RGB\_green \[min] \[max]         : keep points with green color value between \[min] and \[max]  
-keep\_RGB\_greenness \[m] \[n]         : keep points with RGB greenness values between \[min] \[max]  
-keep\_RGB\_nir \[m] \[n]               : keep points with RGB NIR values between \[min] \[max]  
-keep\_RGB\_red \[min] \[max]           : keep points with red color value between \[min] and \[max]  
-map\_attribute\_into\_RGB \[a] \[fnm]   : map attribute \[a] by table in file \[fnm] to RGB values  
-oscale\_rgb \[n]                     : scale output RGB by \[n]  
-scale\_NIR \[n]                      : scale NearInfraRed value by factor \[n]  
-scale\_NIR\_down                     : scale NearInfraRed value down by 256  
-scale\_NIR\_to\_16bit                 : scale 8 bit NearInfraRed value to 16 bit  
-scale\_NIR\_to\_8bit                  : scale 16 bit NearInfraRed value downto 8 bit  
-scale\_NIR\_up                       : scale NearInfraRed value up by 256  
-scale\_RGB \[r] \[g] \[b]              : scale RGB values by factors in \[r]\[g]\[b]  
-scale\_RGB\_down                     : scale RGB color values down by 256  
-scale\_RGB\_to\_16bit                 : scale 8 bit color values to 16 bit  
-scale\_RGB\_to\_8bit                  : scale 16 bit color values downto 8 bit  
-scale\_RGB\_up                       : scale RGB values from 8 bit up to 16 bit (multiply with 256)  
-scale\_rgb\_down                     : divides all RGB values by 256 (to go from 16 bit to 8 bit numbers)  
-scale\_rgb\_up                       : multiplies all RGB values by 256 (to go from 8 bit to 16 bit numbers)  
-set\_NIR \[n]                        : set NearInfraRed value to \[n]  
-set\_RGB \[r] \[g] \[b]                : set color to \[r] \[g] \[b]  
-set\_RGB\_of\_class \[c] \[r] \[g] \[b]   : set RGB values of class \[c] to \[r]\[g]\[b] (8 or 16 bit)  
-switch\_G\_B                         : switch green and blue value  
-switch\_RGBI\_into\_CIR               : set R to NIR; G to R; B to G  
-switch\_RGB\_intensity\_into\_CIR      : set R to intensity; G to R; B to G  
-switch\_R\_B                         : switch red and blue color value  
-switch\_R\_G                         : switch red and green color value

### coordinates

\-add\_attribute\_to\_z \[n]             : add value of attribute \[n] to z value  
-add\_scaled\_attribute\_to\_z \[m] \[n]  : scale attribute \[m] value by \[n] and add to z value  
-auto\_reoffset                      : puts a reasonable offset in the header and translates the points accordingly. Only applicable to LAS/LAZ input files  
-bin\_Z\_into\_point\_source \[n]        : set point source to z/\[n]  
-clamp\_raw\_z \[min] \[max]            : limit raw z values to \[min] and \[max]  
-clamp\_z \[min] \[max]                : limit z values to \[min] and \[max]  
-clamp\_z\_above \[n]                  : limit z values to maximal \[n]  
-clamp\_z\_below \[n]                  : limit z values to minimal \[n]  
-classify\_z\_above\_as \[m] \[n]        : for z value above \[m] set class to \[n]  
-classify\_z\_below\_as \[m] \[n]        : for z value below \[m] set class to \[n]  
-classify\_z\_between\_as \[m] \[n] \[o]  : for z value between \[m] and \[n] set class to \[o]  
-copy\_attribute\_into\_x \[n]          : copy attribute \[n] value into x  
-copy\_attribute\_into\_y \[n]          : copy attribute \[n] value into y  
-copy\_attribute\_into\_z \[n]          : copy attribute \[n] value into z  
-copy\_intensity\_into\_z              : copy intensity to z value  
-copy\_register\_into\_x \[n]           : copy register \[n] to x value  
-copy\_register\_into\_y \[n]           : copy register \[n] to y value  
-copy\_register\_into\_z \[n]           : copy register \[n] to z value  
-copy\_user\_data\_into\_z              : copy user data into z  
-copy\_z\_into\_attribute \[n]          : copy z value into attribute \[n] value  
-drop\_x \[m] \[n]                     : drop points with x value between \[m] and \[n]  
-drop\_x\_above \[n]                   : drop points with x value above \[n]  
-drop\_x\_below \[n]                   : drop points with x value below \[n]  
-drop\_xy \[x1] \[y1] \[x2] \[y2]        : drop points within the \[x1] \[y1] \[x2] \[y2] rectangle  
-drop\_xyz \[x1] \[y1] \[z1] \[x2] \[y2] \[z2]: drop points within the given cube dimensions  
-drop\_y \[m] \[n]                     : drop points with y value between \[m] and \[n]  
-drop\_y\_above \[n]                   : drop points with y value above \[n]  
-drop\_y\_below \[n]                   : drop points with y value below \[n]  
-drop\_z \[m] \[n]                     : drop points with z value between \[m] and \[n]  
-drop\_z\_above \[n]                   : drop points with z value above \[n]  
-drop\_z\_below \[n]                   : drop points with z value below \[n]  
-inside \[x1] \[y1] \[x2] \[y2]         : use only points within the \[x1] \[y1] \[x2] \[y2] rectangle  
-inside\_circle \[x] \[y] \[r]          : keep circle at pos \[x] \[y] with radius \[r]  
-inside\_rectangle \[x1] \[y1] \[x2] \[y2]: use only points within the \[x1] \[y1] \[x2] \[y2] rectangle  
-inside\_tile \[m] \[n] \[o]            : use only points inside tile at lower-left \[x] \[y] with size \[s]  
-keep\_circle \[x] \[y] \[r]            : keep circle at pos \[x] \[y] with radius \[r]  
-keep\_profile \[x1] \[y1] \[x2] \[y2] \[w]: keep profile with \[x1] \[y1] \[x2] \[y2] \[w]  
-keep\_tile \[x] \[y] \[size]           : keep tile at lower-left \[x] \[y] with size \[s]  
-keep\_x \[m] \[n]                     : keep points with x value between \[m] and \[n]  
-keep\_xy \[x1] \[y1] \[x2] \[y2]        : keep points within the \[x1] \[y1] \[x2] \[y2] rectangle  
-keep\_xyz \[x1] \[y1] \[z1] \[x2] \[y2] \[z2]: keep points within the given cube dimensions  
-keep\_y \[m] \[n]                     : keep points with y value between \[m] and \[n]  
-keep\_z \[m] \[n]                     : keep points with z value between \[m] and \[n]  
-keep\_z\_above \[n]                   : keep points with z value above \[n]  
-keep\_z\_below \[n]                   : keep points with z value below \[n]  
-offset\_adjust                      : adjusting the offset based on the results of point operations and transformations  
-reoffset \[x] \[y] \[z]               : puts a new offset \[x] \[y] \[z] into the header and translates the points accordingly  
-rescale \[x] \[y] \[z]                : puts a new scale \[x] \[y] \[z] into the header and rescales the points accordingly  
-rescale\_xy \[x] \[y]                 : rescale x y by \[x] \[y]  
-rescale\_z \[z]                      : rescale z by \[z]  
-rotate\_xy \[a] \[x] \[y]              : rotate points by \[a] degrees, center at \[x] \[y]  
-rotate\_xz \[a] \[x] \[z]              : rotate points by \[a] degrees, center at \[x] \[z]  
-rotate\_yz \[a] \[y] \[z]              : rotate points by \[a] degrees, center at \[y] \[z]  
-scale\_x \[n]                        : scale x value by \[n]  
-scale\_xyz \[m] \[n] \[o]              : scale xyz values by \[m] \[n] \[o]  
-scale\_y \[n]                        : scale y value by \[n]  
-scale\_z \[n]                        : scale z value by \[n]  
-switch\_x\_y                         : exchange x and y value  
-switch\_x\_z                         : exchange x and z value  
-switch\_y\_z                         : exchange z and x value  
-transform\_affine \[k,w,x,y]         : transform by k*cos(w)+k*sin(w)+tx;k*cos(w)+k*sin(w)+ty - angle in second of arc  
-transform\_helmert \[m] \[n] \[o]      : do a helmert transformation with 3 or 7 comma separated parameters \[n] ...  
-transform\_matrix \[r11,r12,r13] \[r21,r22,r23] \[r31,r32,r33] \[tr1,tr2,tr3]: transform input using matrix \[r11,r12,r13] \[r21,r22,r23] \[r31,r32,r33] \[tr1,tr2,tr3]  
-translate\_raw\_x \[n]                : translate raw x value by \[n]  
-translate\_raw\_xy\_at\_random \[x] \[y] : translate raw xy values by random and max offset of \[x] \[y]  
-translate\_raw\_xyz \[x] \[y] \[z]      : translate raw coordinates by \[x] \[y] \[z]  
-translate\_raw\_y \[n]                : translate raw y value by \[n]  
-translate\_raw\_z \[n]                : translate raw z value by \[n]  
-translate\_scale\_translate\_x \[m] \[n]: calculate x value as (x–\[m])*\[n]+\[m]  
-translate\_scale\_translate\_y \[m] \[n]: calculate y value as (y–\[m])*\[n]+\[m]  
-translate\_scale\_translate\_z \[m] \[n]: calculate z value as (z–\[m])\*\[n]+\[m]  
-translate\_then\_scale\_x \[m] \[n]     : translate x value by \[m] and scale by \[n]  
-translate\_then\_scale\_y \[m] \[n]     : translate y value by \[m] and scale by \[n]  
-translate\_then\_scale\_z \[m] \[n]     : translate z value by \[m] and scale by \[n]  
-translate\_x \[n]                    : translate y value by \[n]  
-translate\_xyz \[x] \[y] \[z]          : translate point coordinates by \[x] \[y] \[z]  
-translate\_y \[n]                    : translate y value by \[n]  
-translate\_z \[n]                    : translate z value by \[n]

### simple thinning

\-drop\_every\_nth \[n]           : drop every \[n]th point  
-keep\_every\_nth \[n]           : keep every \[n]th point  
-keep\_random\_fraction \[m] \[n] : keep points by random fraction \[m]{0-1}, optional seed \[n]  
-thin\_points\_with\_time \[n]    : thin points with time, \[n] = timespacing  
-thin\_pulses\_with\_time \[n]    : thin pulses with time, \[n] = timespacing  
-thin\_with\_grid \[n]           : thin points by min grid size of \[n]  
-thin\_with\_time \[n]           : thin pulses with time, \[n] = timespacing

### return number

\-change\_extended\_number\_of\_returns\_from\_to \[m] \[n]: change extended number of returns from \[m] to \[n]  
-change\_extended\_return\_number\_from\_to \[m] \[n]: change extended return number from \[m] to \[n]  
-change\_number\_of\_returns\_from\_to \[m] \[n]: change number of returns from \[m] to \[n]  
-change\_return\_number\_from\_to \[m] \[n]: change return number from \[m] to \[n]  
-drop\_double                        : drop double returns  
-drop\_first                         : drop first return  
-drop\_first\_of\_many                 : drop first of many returns  
-drop\_last                          : drop last return  
-drop\_last\_of\_many                  : drop last of many returns  
-drop\_middle                        : drop middle returns  
-drop\_number\_of\_returns \[n]         : drop points with \[n] number of returns  
-drop\_quadruple                     : drop quadruple returns  
-drop\_quintuple                     : drop quintuple returns  
-drop\_return \[m] \[n]...             : drop points with return \[m] \[n]...  
-drop\_return\_mask \[n]               : drop points with return mask \[n]  
-drop\_second\_last                   : drop points with second last return  
-drop\_single                        : drop points with single return  
-drop\_triple                        : drop points with triple return  
-first\_only                         : use first return only  
-keep\_double                        : keep double returns  
-keep\_first                         : keep first return  
-keep\_first\_of\_many                 : keep first of many returns  
-keep\_last                          : keep last return  
-keep\_last\_of\_many                  : keep last of many returns  
-keep\_middle                        : keep mittle returns  
-keep\_number\_of\_returns \[n]         : keep points with \[n] number of returns  
-keep\_quadruple                     : keep quadruple returns  
-keep\_quintuple                     : keep quintuple returns  
-keep\_return \[m] \[n]...             : keep points with return \[m] \[n]...  
-keep\_return\_mask \[n]               : keep points with return mask \[n]  
-keep\_second\_last                   : keep points with second last return  
-keep\_single                        : keep points with single return  
-keep\_triple                        : keep points with triple return  
-last\_only                          : use last return only  
-repair\_zero\_returns                : sets return counts and number of returns that are zero to one  
-set\_extended\_number\_of\_returns \[n] : set extended number of returns to \[n]  
-set\_extended\_return\_number \[n]     : set extended return number to \[n]  
-set\_number\_of\_returns \[n]          : set number of returns to \[n]  
-set\_return\_number \[n]              : set return number to \[n]

### scanline

\-drop\_scan\_direction \[n]       : drop points with scan direction \[n]  
-faf                           : input files are flightlines. do ***NOT*** use this for tiled input  
-faf\_index \[n]                 : set files are flightlines index \[n]  
-files\_are\_flightlines         : input files are flightlines. do ***NOT*** use this for tiled input  
-drop\_edge\_of\_flight\_line      : drop points with "Edge of Flight Line" flag set  
-keep\_edge\_of\_flight\_line      : keep points with "Edge of Flight Line" flag set  
-keep\_scan\_direction\_change    : keep points with changed scan direction flag  
-set\_edge\_of\_flight\_line \[0/1] : set "Edge of Flight Line" flag to \[0/1]  
-set\_scan\_direction\_flag \[0/1] : set scan direction flag to \[0/1]

### scanner channel

\-copy\_scanner\_channel\_into\_point\_source: copy scanner channel into point\_source  
-copy\_scanner\_channel\_into\_user\_data: copy scanner channel into user data  
-copy\_user\_data\_into\_scanner\_channel: copy user data into scanner channel  
-drop\_scanner\_channel \[n]           : drop points with scanner channel \[n]  
-keep\_scanner\_channel \[n]           : keep points with scanner channel \[n]  
-merge\_scanner\_channel\_into\_point\_source: merge scanner channel to point source  
-set\_extended\_scanner\_channel \[n]   : set extended scanner channel to \[n]  
-set\_scanner\_channel \[n]            : set scanner channel to \[n]  
-split\_scanner\_channel\_from\_point\_source: split scanner channel from point source and save as extended scanner channel

### source ID

\-apply\_file\_source\_ID               : copy file source ID to target  
-bin\_Z\_into\_point\_source \[n]        : set point source to z/\[n]  
-bin\_abs\_scan\_angle\_into\_point\_source \[n]: set point source to scan\_angle/\[n]  
-bin\_gps\_time\_into\_point\_source \[n] : set point source to gps/\[n]  
-change\_point\_source\_from\_to \[m] \[n]: change point source from \[m] to \[n]  
-copy\_attribute\_into\_point\_source \[n]: copy attribute \[n] value into point source  
-copy\_classification\_into\_point\_source: copy classification to point source  
-copy\_point\_source\_into\_register \[n]: copy point source into register \[n]  
-copy\_register\_into\_point\_source \[n]: copy register \[n] to point source  
-copy\_scanner\_channel\_into\_point\_source: copy scanner channel into point\_source  
-copy\_user\_data\_into\_point\_source   : copy user data into point source  
-drop\_point\_source \[n]              : drop points with point source \[n]  
-drop\_point\_source\_above \[n]        : drop points with with point source above \[n]  
-drop\_point\_source\_below \[n]        : drop points with with point source below \[n]  
-drop\_point\_source\_between \[m] \[n]  : drop points with with point source between \[n] and \[m]  
-keep\_point\_source \[n]              : keep points with point source \[n]  
-keep\_point\_source\_between \[m] \[n]  : keep points with with point source between \[n] and \[m]  
-map\_point\_source \[fnm]             : set the point source by map in file \[fnm]  
-merge\_scanner\_channel\_into\_point\_source: merge scanner channel to point source  
-set\_point\_source \[n]               : set point source to \[n]  
-split\_scanner\_channel\_from\_point\_source: split scanner channel from point source and save as extended scanner channel

### user data

\-add\_scaled\_attribute\_to\_user\_data \[m] \[n]: scale attribute \[m] value by \[n] and add to user data  
-change\_user\_data\_from\_to \[m] \[n]   : change user data from \[m] to \[n]  
-copy\_attribute\_into\_user\_data \[n]  : copy attribute \[n] value into user data field  
-copy\_classification\_into\_user\_data : copy classification to user data  
-copy\_register\_into\_user\_data \[n]   : copy register \[n] to user data  
-copy\_scanner\_channel\_into\_user\_data: copy scanner channel into user data  
-copy\_user\_data\_into\_attribute \[n]  : copy user data into attribute \[n] value  
-copy\_user\_data\_into\_classification : copy user data into classification  
-copy\_user\_data\_into\_point\_source   : copy user data into point source  
-copy\_user\_data\_into\_register \[n]   : copy user data to register \[n]  
-copy\_user\_data\_into\_scanner\_channel: copy user data into scanner channel  
-copy\_user\_data\_into\_z              : copy user data into z  
-drop\_user\_data \[n]                 : drop points with user data value of \[n]  
-drop\_user\_data\_above \[n]           : drop points with user data value above \[n]  
-drop\_user\_data\_below \[n]           : drop points with user data value below \[n]  
-drop\_user\_data\_between \[m] \[n]     : drop points with user data between \[m] and \[n]  
-keep\_user\_data \[n]                 : keep points with user data value of \[n]  
-keep\_user\_data\_above \[n]           : keep points with user data value above \[n]  
-keep\_user\_data\_below \[n]           : keep points with user data value below \[n]  
-keep\_user\_data\_between \[m] \[n]     : keep points with user data between \[m] and \[n]  
-map\_user\_data \[fnm]                : set the user data by map in file \[fnm]  
-scale\_user\_data \[n]                : scale user data by \[n]  
-set\_user\_data \[n]                  : sets all user\_data fields to \[n]

### classification

\-change\_class\_from\_to \[m] \[n]       : change classification from \[m] to \[n]  
-change\_classification\_from\_to \[m] \[n]: change classification from \[m] to \[n]  
-change\_extended\_class\_from\_to \[m] \[n]: change extended class from \[m] to \[n]  
-change\_extended\_classification\_from\_to \[m] \[n]: change extended class from \[m] to \[n]  
-classify\_attribute\_above\_as \[m] \[n] \[o]: for attribute \[m] with value above \[n] set class to \[o]  
-classify\_attribute\_below\_as \[m] \[n] \[o]: for attribute \[m] with value below \[n] set class to \[o]  
-classify\_attribute\_between\_as \[m] \[n] \[o] \[p]: for attribute \[m] with value between \[n] and \[o] set class to \[p]  
-classify\_intensity\_above\_as \[m] \[n]: for intensity value above \[m] set class to \[n]  
-classify\_intensity\_below\_as \[m] \[n]: for intensity value below \[m] set class to \[n]  
-classify\_intensity\_between\_as \[m] \[n] \[o]: for intensity value between \[m] and \[n] set class to \[o]  
-classify\_z\_above\_as \[m] \[n]        : for z value above \[m] set class to \[n]  
-classify\_z\_below\_as \[m] \[n]        : for z value below \[m] set class to \[n]  
-classify\_z\_between\_as \[m] \[n] \[o]  : for z value between \[m] and \[n] set class to \[o]  
-copy\_classification\_into\_point\_source: copy classification to point source  
-copy\_classification\_into\_user\_data : copy classification to user data  
-copy\_intensity\_into\_classification : copy intensity to classification  
-copy\_user\_data\_into\_classification : copy user data into classification  
-drop\_class \[m] \[n] \[o]...          : drop points with class in \[m]\[n]\[o]...  
-drop\_classification \[m] \[n] \[o]... : drop points with class in \[m]\[n]\[o]...  
-drop\_classification\_mask \[n]       : drop points with classification mask matches \[n]  
-drop\_extended\_class \[m] \[n]...     : drop extended class \[m] \[n]...  
-drop\_extended\_classification \[n]   : drop points with extended classification \[n]  
-drop\_extended\_classification\_mask \[a] \[b] \[c] \[d] \[e] \[f] \[g] \[h]: drop points with extended classification mask matches \[a] \[b] \[c] \[d] \[e] \[f] \[g] \[h]  
-keep\_class \[m] \[n] \[o]...          : keep points with class in \[m]\[n]\[o]...  
-keep\_classification \[m] \[n] \[o]... : keep points with class in \[m]\[n]\[o]...  
-keep\_classification\_mask \[n]       : keep points with classification mask matches \[n]  
-keep\_extended\_class \[m] \[n]...     : keep extended class \[m] \[n]...  
-keep\_extended\_classification \[n]   : keep points with extended class \[n]  
-move\_ancient\_to\_extended\_classification: move old data to extended classification  
-set\_RGB\_of\_class \[c] \[r] \[g] \[b]   : set RGB values of class \[c] to \[r]\[g]\[b] (8 or 16 bit)  
-set\_classification \[n]             : set classification to \[n]  
-set\_extended\_classification \[n]    : set extended classification to \[n]

### extra byte

\-add\_attribute\_to\_z \[n]             : add value of attribute \[n] to z value  
-add\_scaled\_attribute\_to\_user\_data \[m] \[n]: scale attribute \[m] value by \[n] and add to user data  
-add\_scaled\_attribute\_to\_z \[m] \[n]  : scale attribute \[m] value by \[n] and add to z value  
-classify\_attribute\_above\_as \[m] \[n] \[o]: for attribute \[m] with value above \[n] set class to \[o]  
-classify\_attribute\_below\_as \[m] \[n] \[o]: for attribute \[m] with value below \[n] set class to \[o]  
-classify\_attribute\_between\_as \[m] \[n] \[o] \[p]: for attribute \[m] with value between \[n] and \[o] set class to \[p]  
-copy\_attribute\_into\_B \[n]          : copy attribute \[n] value into blue  
-copy\_attribute\_into\_G \[n]          : copy attribute \[n] value into green  
-copy\_attribute\_into\_I \[n]          : copy attribute \[n] value into intensity  
-copy\_attribute\_into\_NIR \[n]        : copy attribute \[n] value into NIR (NearInfraRed)  
-copy\_attribute\_into\_R \[n]          : copy attribute \[n] value into red  
-copy\_attribute\_into\_intensity \[n]  : copy attribute \[n] value into intensity  
-copy\_attribute\_into\_point\_source \[n]: copy attribute \[n] value into point source  
-copy\_attribute\_into\_register \[m] \[n]: copy attribute \[m] value into register \[m]  
-copy\_attribute\_into\_user\_data \[n]  : copy attribute \[n] value into user data field  
-copy\_attribute\_into\_x \[n]          : copy attribute \[n] value into x  
-copy\_attribute\_into\_y \[n]          : copy attribute \[n] value into y  
-copy\_attribute\_into\_z \[n]          : copy attribute \[n] value into z  
-copy\_intensity\_into\_attribute \[n]  : copy intensity to attribute \[n] value  
-copy\_register\_into\_attribute \[m] \[n]: copy register \[m] to attribute \[n] value  
-copy\_user\_data\_into\_attribute \[n]  : copy user data into attribute \[n] value  
-copy\_z\_into\_attribute \[n]          : copy z value into attribute \[n] value  
-drop\_attribute\_above \[m] \[n]       : drop points with attribute \[m] value > \[n]  
-drop\_attribute\_below \[m] \[n]       : drop points with attribute \[m] value < \[n]  
-drop\_attribute\_between \[m] \[n] \[o] : drop points with attribute \[m] in range \[n]...\[o]  
-iadd\_attribute \[m] \[n] \[o] \[p] \[q] \[r] \[s] \[t]: adds a new "extra\_byte" attribute of data\_type \[m] name \[n] description \[o]; optional: scale\[p] offset \[q] pre\_scale \[r] pre\_offset \[s] no\_data\_value \[t]  
-iadd\_extra \[m] \[n] \[o] \[p] \[q] \[r] \[s] \[t]: adds a new "extra\_byte" attribute of data\_type \[m] name \[n] description \[o]; optional: scale\[p] offset \[q] pre\_scale \[r] pre\_offset \[s] no\_data\_value \[t]  
-keep\_attribute\_above \[m] \[n]       : keep points with attribute \[m] value > \[n]  
-keep\_attribute\_below \[m] \[n]       : keep points with attribute \[m] value < \[n]  
-keep\_attribute\_between \[m] \[n] \[o] : keep points with attribute \[m] in range \[n]...\[o]  
-load\_attribute\_from\_text \[m] \[fnt] : load attribute \[m] from file \[fnt]  
-map\_attribute\_into\_RGB \[a] \[fnm]   : map attribute \[a] by table in file \[fnm] to RGB values  
-scale\_attribute \[m] \[n]            : scale attribute \[m] by \[n]  
-set\_attribute \[m] \[n]              : set attribute \[m] with value \[n]  
-translate\_attribute \[m] \[n]        : translate attribute \[n] by \[n]

### flags

\-drop\_keypoint                   : drop points flaged as keypoint  
-drop\_overlap                    : drop points flaged as overlap  
-drop\_scan\_direction \[n]         : drop points with scan direction \[n]  
-drop\_synthetic                  : drop points flaged as synthetic  
-drop\_withheld                   : drop points flaged as withheld  
-keep\_edge\_of\_flight\_line        : keep points with "Edge of Flight Line" flag set  
-keep\_keypoint                   : keep points flaged as keypoint  
-keep\_overlap                    : keep points flaged as overlap  
-keep\_scan\_direction\_change      : keep points with changed scan direction flag  
-keep\_synthetic                  : keep points flaged as synthetic  
-keep\_withheld                   : keep points flaged as withheld  
-set\_edge\_of\_flight\_line \[0/1]   : set "Edge of Flight Line" flag to \[0/1]  
-set\_extended\_overlap\_flag \[0/1] : set extended overlap flag to \[0/1]  
-set\_keypoint\_flag \[0/1]         : set keypoint flag to \[0/1]  
-set\_overlap\_flag \[0/1]          : set overlap flag to \[0/1]  
-set\_scan\_direction\_flag \[0/1]   : set scan direction flag to \[0/1]  
-set\_synthetic\_flag \[0/1]        : set synthetic flag to \[0/1]  
-set\_withheld\_flag \[0/1]         : set withheld flag to \[0/1]

### GPS time

\-adjusted\_to\_week                   : converts time stamps from Adjusted Standard GPS to GPS week  
-bin\_gps\_time\_into\_intensity \[n]    : set intensity time to gps/\[n]  
-bin\_gps\_time\_into\_point\_source \[n] : set point source to gps/\[n]  
-drop\_gps\_time\_above \[n]            : drop points with GPS time above \[n]  
-drop\_gps\_time\_below \[n]            : drop points with GPS time below \[n]  
-drop\_gps\_time\_between \[m] \[n]      : drop points with GPS time between \[m] and \[n]  
-drop\_gpstime\_above \[n]             : drop points with GPS time above \[n]  
-drop\_gpstime\_below \[n]             : drop points with GPS time below \[n]  
-drop\_gpstime\_between \[m] \[n]       : drop points with GPS time between \[m] and \[n]  
-keep\_gps\_time \[m] \[n]              : keep points with GPS time between \[m] and \[n]  
-keep\_gps\_time\_above \[n]            : keep points with GPS time above \[n]  
-keep\_gps\_time\_below \[n]            : keep points with GPS time below \[n]  
-keep\_gps\_time\_between \[m] \[n]      : keep points with GPS time between \[m] and \[n]  
-keep\_gpstime \[m] \[n]               : keep points with GPS time between \[m] and \[n]  
-keep\_gpstime\_above \[n]             : keep points with GPS time above \[n]  
-keep\_gpstime\_below \[n]             : keep points with GPS time below \[n]  
-keep\_gpstime\_between \[m] \[n]       : keep points with GPS time between \[m] and \[n]  
-set\_gps\_time \[n]                   : set gps time to \[n]  
-translate\_gps\_time \[n]             : translate GPS time by \[n]  
-week\_to\_adjusted \[n]               : converts time stamps from GPS week \[n] to Adjusted Standard GPS

### intensity

\-bin\_gps\_time\_into\_intensity \[n]    : set intensity time to gps/\[n]  
-clamp\_intensity \[min] \[max]        : limit intensity values to \[min] and \[max]  
-clamp\_intensity\_above \[max]        : limit intensity values to maximal \[max]  
-clamp\_intensity\_below \[max]        : limit intensity values to minimal \[min]  
-classify\_intensity\_above\_as \[m] \[n]: for intensity value above \[m] set class to \[n]  
-classify\_intensity\_below\_as \[m] \[n]: for intensity value below \[m] set class to \[n]  
-classify\_intensity\_between\_as \[m] \[n] \[o]: for intensity value between \[m] and \[n] set class to \[o]  
-copy\_B\_into\_intensity              : copy blue color value to intensity  
-copy\_G\_into\_intensity              : copy green color value to intensity  
-copy\_NIR\_into\_intensity            : copy NIR into intensity  
-copy\_RGB\_into\_intensity            : copy weighted RGB value to intensity  
-copy\_R\_into\_intensity              : copy red color value to intensity  
-copy\_attribute\_into\_I \[n]          : copy attribute \[n] value into intensity  
-copy\_attribute\_into\_intensity \[n]  : copy attribute \[n] value into intensity  
-copy\_intensity\_into\_NIR            : copy intensity into NIR (NearInfraRed) value  
-copy\_intensity\_into\_attribute \[n]  : copy intensity to attribute \[n] value  
-copy\_intensity\_into\_classification : copy intensity to classification  
-copy\_intensity\_into\_register \[n]   : copy color intensitiy value into register \[n]  
-copy\_intensity\_into\_z              : copy intensity to z value  
-copy\_register\_into\_intensity \[n]   : copy register \[n] into point intensitiy value  
-drop\_intensity\_above \[n]           : drop points with intensity value above \[n]  
-drop\_intensity\_below \[n]           : drop points with intensity value below \[n]  
-drop\_intensity\_between \[m] \[n]     : drop points with intensity value between \[m] and \[n]  
-iscale\_intensity \[n]               : scale intensity value by \[n]  
-itranslate\_intensity \[n]           : translate input intensity by \[n]  
-keep\_NDVI\_intensity\_is\_NIR \[min] \[max]: keep NDVI (Normalized Difference Vegetation Index) where intensity is NIR between \[min] \[max]  
-keep\_intensity \[m] \[n]             : keep points with intensity between \[m] and \[n]  
-keep\_intensity\_above \[n]           : keep points with intensity value above \[n]  
-keep\_intensity\_below \[n]           : keep points with intensity value below \[n]  
-map\_intensity \[fnm]                : set the intensity by map in file \[fnm]  
-scale\_intensity \[n]                : multiply intensity by \[n]  
-set\_intensity \[n]                  : set intensity to \[n]  
-switch\_RGB\_intensity\_into\_CIR      : set R to intensity; G to R; B to G  
-translate\_intensity \[n]            : translate intensity by \[n]  
-translate\_then\_scale\_intensity \[m] \[n]: translate intensity by \[m] and scale by \[n]

### raw point values

\-clamp\_raw\_z \[min] \[max]            : limit raw z values to \[min] and \[max]  
-translate\_raw\_x \[n]                : translate raw x value by \[n]  
-translate\_raw\_xy\_at\_random \[x] \[y] : translate raw xy values by random and max offset of \[x] \[y]  
-translate\_raw\_xyz \[x] \[y] \[z]      : translate raw coordinates by \[x] \[y] \[z]  
-translate\_raw\_y \[n]                : translate raw y value by \[n]  
-translate\_raw\_z \[n]                : translate raw z value by \[n]

### registers

\-add\_registers \[m] \[n] \[o]          : add register \[m] and \[n] and store result in register \[o]  
-copy\_B\_into\_register \[n]           : copy blue color value into register \[n]  
-copy\_G\_into\_register \[n]           : copy green color value into register \[n]  
-copy\_NIR\_into\_register \[n]         : copy NearInfraRed value into register \[n]  
-copy\_R\_into\_register \[n]           : copy red color value into register \[n]  
-copy\_attribute\_into\_register \[m] \[n]: copy attribute \[m] value into register \[m]  
-copy\_intensity\_into\_register \[n]   : copy color intensitiy value into register \[n]  
-copy\_point\_source\_into\_register \[n]: copy point source into register \[n]  
-copy\_register\_into\_B \[n]           : copy register \[n] into blue color value  
-copy\_register\_into\_G \[n]           : copy register \[n] into green color value  
-copy\_register\_into\_I \[n]           : copy register \[n] into NearInfraRed value  
-copy\_register\_into\_NIR \[n]         : copy register \[n] into NearInfraRed value  
-copy\_register\_into\_R \[n]           : copy register \[n] into red color value  
-copy\_register\_into\_attribute \[m] \[n]: copy register \[m] to attribute \[n] value  
-copy\_register\_into\_intensity \[n]   : copy register \[n] into point intensitiy value  
-copy\_register\_into\_point\_source \[n]: copy register \[n] to point source  
-copy\_register\_into\_user\_data \[n]   : copy register \[n] to user data  
-copy\_register\_into\_x \[n]           : copy register \[n] to x value  
-copy\_register\_into\_y \[n]           : copy register \[n] to y value  
-copy\_register\_into\_z \[n]           : copy register \[n] to z value  
-copy\_user\_data\_into\_register \[n]   : copy user data to register \[n]  
-divide\_registers \[m] \[n] \[o]       : divide register \[m] by register \[n] and store result in register \[o]  
-multiply\_registers \[m] \[n] \[o]     : Multiply register \[m] with \[n] and store result in register \[o]  
-scale\_register \[m] \[n]             : scale register index \[m] with factor \[n]  
-set\_register \[m] \[n]               : set register \[m] with value \[n]  
-subtract\_registers \[m] \[n] \[o]     : subtract register \[m] by register \[n] and store result in register \[o]  
-translate\_register \[m] \[n]         : translate register index \[m] value by \[n]

### scan angle

\-bin\_abs\_scan\_angle\_into\_point\_source \[n]: set point source to scan\_angle/\[n]  
-drop\_abs\_scan\_angle\_above \[max]    : drop points with absolute scan angle above \[max]  
-drop\_abs\_scan\_angle\_below \[min]    : drop points with absolute scan angle below \[min]  
-drop\_scan\_angle\_above \[n]          : drop points with scan angle above \[n]  
-drop\_scan\_angle\_below \[n]          : drop points with scan angle below \[n]  
-drop\_scan\_angle\_between \[m] \[n]    : drop points with scan angle between \[m] and \[n]  
-iscale\_scan\_angle \[n]              : scale scan angle by \[n]  
-itranslate\_scan\_angle \[n]          : translate input scan angle by \[n]  
-keep\_scan\_angle \[m] \[n]            : keep points with scan angle between \[m] and \[n]  
-keep\_scan\_angle\_between \[m] \[n]    : keep points with scan angle between \[m] and \[n]  
-scale\_scan\_angle \[n]               : scale scan angle by \[n]  
-set\_scan\_angle \[n]                 : set scan angle to \[n]  
-translate\_scan\_angle \[n]           : translate scan angle by \[n]  
-translate\_then\_scale\_scan\_angle \[m] \[n]: translate scan angle by \[m] and scale by \[n]

### tiles

\-keep\_tile \[x] \[y] \[size] : keep tile at lower-left \[x] \[y] with size \[s]

### waveform packet

\-drop\_wavepacket \[n]     : drop points with wavepacket value of \[n]  
-flip\_waveform\_direction : flip the waveform direction in the waveform VLR  
-keep\_wavepacket \[n]     : keep points with wavepacket value of \[n]

### CRS

\-aeac \[m] \[n] \[meter/survey\_feet/feet] \[o] \[p] \[q] \[r]: Albers Equal Area Conic Projection: False Easting \[m] False Northing\[n] \[meter/survey\_feet/feet] Central Meridian \[o] Standard Parallel 1 \[p] Standard Parallel 2 \[q] Latitude of origin \[r]  
-ecef                               : input is geocentric (Earth-centered Earth-fixed)  
-elevation\_feet                     : use feet for elevation  
-elevation\_meter                    : use meter for elevation  
-elevation\_survey\_feet              : set vertical units from meters to US survey feet  
-elevation\_surveyfeet               : use survey feet for elevation  
-ellipsoid \[n]                      : use the WGS-84 ellipsoid \[n]{do -ellipsoid -1 for a list of ellipsoids}  
-epsg \[n]                           : set datum to EPSG \[n]  
-etrs89                             : use datum ETRS89  
-feet                               : use feet  
-gda2020                            : use datum GDA2020  
-gda94                              : use datum GDA94  
-grs80                              : use datum GRS1980  
-latlong                            : geometric coordinates in latitude/longitude order  
-lcc 609601.22 0.0 meter 33.75 -79 34.33333 36.16666: specifies a lambertian conic confomal projection  
-longlat                            : geometric coordinates in longitude/latitude order  
-meter                              : use meter  
-nad27                              : use the NAD27 ellipsoid  
-nad83                              : use the NAD83 ellipsoid  
-nad83\_2011                         : use datum NAD83\_2011  
-nad83\_csrs                         : use datum NAD83\_CSRS  
-nad83\_harn                         : use datum NAD83\_HARN  
-nad83\_pa11                         : set horizontal datum to NAD83 PA11  
-osgb1936                           : use datum OSGB 1936  
-sp27 SC\_N                          : use the NAD27 South Carolina North state plane  
-sp83 CO\_S                          : use the NAD83 Colorado South state plane for georeferencing  
-survey\_feet                        : use survey feet  
-surveyfeet                         : use survey feet as unit of measurement  
-target\_aeac \[m] \[n] \[meter/survey\_feet/feet] \[o] \[p] \[q] \[r]: Albers Equal Area Conic Projection for target: False Easting \[m] False Northing\[n] \[meter/survey\_feet/feet] Central Meridian \[o] Standard Parallel 1 \[p] Standard Parallel 2 \[q] Latitude of origin \[r]  
-target\_ecef                        : output is geocentric (Earth-centered Earth-fixed)  
-target\_elevation\_feet              : output uses feet for elevation  
-target\_elevation\_meter             : output uses meter for elevation  
-target\_elevation\_precision \[n]     : output uses \[n] (meter/feet) resolution for z  
-target\_elevation\_survey\_feet       : use elevation survey feet as target unit  
-target\_elevation\_surveyfeet        : output uses survey feet for elevation  
-target\_epsg \[n]                    : output is EPSG code \[n] (e.g. 2193=NZGD2000)  
-target\_feet                        : output uses feet  
-target\_latlong                     : output is geometric coordinates in latitude/longitude  
-target\_lcc 609601.22 0.0 meter 33.75 -79 34.33333 36.16666: specifies a lambertian conic confomal projection at target  
-target\_longlat                     : output is geometric coordinates in longitude/latitude  
-target\_meter                       : output uses meter  
-target\_precision \[n]               : output uses \[n] (meter/feet) resolution for x and y  
-target\_sp27 SC\_N                   : output is state plane NAD27 South Carolina North  
-target\_sp83 CO\_S                   : output is state plane NAD83 Colorado South  
-target\_survey\_feet                 : output uses survey feet  
-target\_surveyfeet                  : use survey feet as target unit  
-target\_tm                          : use transverse mercator projection for target  
-target\_utm 12T                     : output is UTM zone 12T  
-tm 609601.22 0.0 meter 33.75 -79 0.99996: specifies a transverse mercator projection  
-transverse\_mercator                : use transverse mercator projection  
-utm 12T                            : use UTM zone 12T  
-vertical\_cgvd2013                  : set vertical datum to CGVD2013  
-vertical\_cgvd28                    : set vertical datum to CGVD28  
-vertical\_dhhn2016                  : set vertical datum to DHHN2016  
-vertical\_dhhn92                    : set vertical datum to DHHN92  
-vertical\_dvr90                     : set vertical datum to DVR90  
-vertical\_epsg \[n]                  : set vertical datum to EPSG \[n]  
-vertical\_evrf2007                  : set vertical datum to EVRF2007  
-vertical\_navd29                    : set vertical datum to NAVD29  
-vertical\_navd88                    : set vertical datum to NAVD88  
-vertical\_ngvd29                    : set vertical datum to NGVD29  
-vertical\_nn2000                    : set vertical datum to NN2000  
-vertical\_nn54                      : set vertical datum to NN54  
-vertical\_nzvd2016                  : set vertical datum to NZVD2016  
-vertical\_wgs84                     : set vertical datum to WGS84  
-wgs72                              : use the WGS-72 ellipsoid  
-wgs84                              : use the WGS-84 ellipsoid

### logical

\-filter\_and         : boolean AND combination of last 2 filters  
-filter\_or          : boolean OR combination of last 2 filters  
-filtered\_transform : do the transformation only on points of the current filter

### formula solver

\-formula \[exp]       : Allows filtering or transforming point variables using formula expressions \[exp]; LAS header variables may be used only for filtering  
-fileformula \[exp]   : Allows filtering input files or transforming LAS header variables using formula expressions \[exp]; point variables are not allowed

For details about the formula features and how to write formula expressions, see the \[formula solver documentation (formula\_solver\_README.md)]  
The formula features require a full licensed build of LAStools and are not available in the open-source version.

### input

\-i \[fnp]        : input file or input file mask \[fnp] (e.g. *.laz;fo?.la?;esri.shp,...)  
-io\_ibuffer \[n] : use read-input-buffer of size \[n] bytes  
-iparse \[xyz]   : define fields \[xyz] for text input parser  
-ipts           : input as PTS (plain text lidar source), store header in VLR  
-iptx           : input as PTX (plain text extended lidar data), store header in VLR  
-iptx\_transform : use PTX file header to transform point data  
-iskip \[n]      : skip \[n] lines at the beginning of the text input  
-itxt           : expect input as text file  
-lof \[fnf]      : use input out of a list of files \[fnf]  
-subdir         : enables recursive search in subdirectories. (Linux: enclose wildcard patterns like "*.laz" in quotes)  
-unique         : remove duplicate files in a -lof list  
-merged         : merge input files  
-buffered \[n]   : use on-the-fly buffering of size \[n] for tiles without implicit buffer  
-stdin          : pipe from stdin

### output

\-compatible      : write LAS/LAZ output in compatibility mode  
-do\_not\_populate : do not populate header on output  
-io\_obuffer \[n]  : use write-out-buffer of size \[n] bytes  
-native          : write LAS/LAZ output in native/actual mode  
-nil             : pipe output to NULL (suppress output)  
-o \[n]           : use \[n] as output file  
-obin            : output as BIN (terrasolid binary)  
-ocut \[n]        : cut the last \[n] characters from name  
-odir \[n]        : set output directory to \[n]  
-odix \[n]        : set output file name suffix to \[n]  
-oforce          : force output creation also on errors or warnings  
-olas            : output as LAS file  
-olaz            : output as LAZ (compressed LAS)  
-oparse \[xyz]    : parse on-the-fly to ASCII using fields \[xyz]  
-opts            : output as PTS (plain text lidar data)  
-optx            : output as PTX (plain text with header)  
-oqi             : output in QFIT format (.qi)(ATM project, NASA)  
-oscale\_rgb \[n]  : scale output RGB by \[n]  
-osep \[sep]      : set text output separator as \[sep](see table below)  
-otxt            : output as textfile  
-owrl            : output as VRLM (Virtual Reality Modeling Language) text  
-pipe\_on         : write output to command pipe, see also -std\_in  
-populate        : populate header on output  
-stdout          : pipe to stdout  
-target\_ecef     : output is geocentric (Earth-centered Earth-fixed)  
-temp\_files \[n]  : set base file name \[n] for temp files (example: E:\\tmp)

### parse

The '-parse \[xyz]' flag specifies how to interpret each line of the ASCII file.
For example, 'tsxyzssa' means that the first number is the gpstime, the next
number should be skipped, the next three numbers are the x, y, and z coordinate,
the next two should be skipped, and the next number is the scan angle.

The other supported entries are:  
x : \[x] coordinate  
y : \[y] coordinate  
z : \[z] coordinate  
t : gps \[t]ime  
R : RGB \[R]ed channel  
G : RGB \[G]reen channel  
B : RGB \[B]lue channel  
I : N\[I]R channel of LAS 1.4 point type 8  
s : \[s]kip a string or a number that we don't care about  
i : \[i]ntensity  
a : scan \[a]ngle  
n : \[n]umber of returns of that given pulse  
r : number of \[r]eturn  
h : with\[h]eld flag  
k : \[k]eypoint flag  
g : synthetic fla\[g]  
o : \[o]verlap flag of LAS 1.4 point types 6, 7, 8  
l : scanner channe\[l] of LAS 1.4 point types 6, 7, 8  
E : terrasolid \[E]hco Encoding  
c : \[c]lassification. If extended classes are used: Use o,l or I to force 1.4 format.  
u : \[u]ser data  
p : \[p]oint source ID  
e : \[e]dge of flight line flag  
d : \[d]irection of scan flag  
0-9 : additional attributes described as extra bytes (0 through 9)  
(13) : additional attributes described as extra bytes (10 and up)  
H : a hexadecimal string encoding the RGB color  
J : a hexadecimal string encoding the intensity

### output separator

The '-osep \[sep]' argument specifies the output format of a text(xyz or csv) output.
Supported \[sep] values:

comma  
tab  
dot  
colon  
semicolon  
hyphen  
space

## licensing

Info on licensing and pricing: https://rapidlasso.de/pricing/.
If you have any questions or need assistance, email to info@rapidlasso.de.

## evaluation and demo mode

Please use the "-demo" argument to run the tool in demo mode. For quality tests,
use small files (< 1.5 million points). If you use larger files, the output will
contain diagonal lines/output distortions due to the license protection.

## support

1. We invite you to join our LAStools Google Group (http://groups.google.com/group/lastools/).
If you are looking for information about a specific tool, enter the tool name in the search
function and you'll find all discussions related to the respective tool.
2. Customer Support Page: https://rapidlasso.de/customer-support/.
3. Download LAStools: https://rapidlasso.de/downloads/.
4. Changelog: https://rapidlasso.de/changelog/.



If you want to send us feedback or have questions that are not answered in the resources above,
please email to info@rapidlasso.de.

