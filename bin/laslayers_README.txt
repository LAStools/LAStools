****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

laslayers:

WARNING: do not expect LAY files to work across different versions
         of LAStools because the development of LASlayers is still
         not completed. this is just an awesome prototype as proof
         of concept. we continue to gather community input...

laslayers is explained with 3 exercises: 
(1) a first look at LASlayers with 'lasview', 'las2dem', and 'laslayers'
(2) work with LASlayers using various LAStools for more I/O and 
    storage efficiency, 
(3) create LASlayers as difference files between an original and a modified
    LAS/LAZ file using 'laslayers'. 

====================================================
== (1) ==        first look at LASlayers          ++
====================================================

Have a look at the ll.laz file with 'lasview':

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

laslayers -i ll.laz

Now apply all LASlayers to create a new 'll_lay4.laz' file:

laslayers -i ll.laz -ilay -o ll_lay4.laz
lasview -i ll_lay4.laz

Now apply only the first two LASlayers:

laslayers -i ll.laz -ilay 2 -o ll_lay2.laz
lasview -i ll_lay2.laz

====================================================
== (2) ==   work with LASlayers using LAStools    ++
====================================================

Imagine we obtained raw LAZ tiles and orthos from a customer and are now producing the desired products with LASlayers.

Classify isolated points as noise (7) with 'lasnoise':

lasnoise -i ll.laz -olay
laslayers -i ll.laz
lasview -i ll.laz -ilay

Or - alternatively - delete isolated points:

lasnoise -i ll.laz -olay -remove_noise
laslayers -i ll.laz
lasview -i ll.laz -ilay

Create a noise-free DSM with 'lasgrid':

las2dem -i ll.laz -ilay -first_only -step 0.5 -hillshade -odix _dsm -opng

Delete xyz-duplicate points with 'lasduplicate':

lasduplicate -i ll.laz -ilay -unique_xyz -olay
laslayers -i ll.laz
lasview -i ll.laz -ilay

Ground classify the noise-free points with 'lasground':

lasground -i ll.laz -ilay -fine -olay 
laslayers -i ll.laz
lasview -i ll.laz -ilay

Create a DTM with 'las2dem':

las2dem -i ll.laz -ilay -keep_class 2 -step 0.5 -hillshade -odix _dtm -opng

Compute the height of points above the ground and store an approximation to user_data with 'lasheight':

lasheight -i ll.laz -ilay -olay 
laslayers -i ll.laz
lasview -i ll.laz -ilay -color_by_user_data

Classify building and forest points with 'lasclassify':

lasclassify -i ll.laz -ilay -olay 
laslayers -i ll.laz
lasview -i ll.laz -ilay 

We do not like the result and want to adapt some parameters. Therefore we delete the last layers with 'laslayers':

laslayers -i ll.laz -del 5
laslayers -i ll.laz
lasview -i ll.laz -ilay 

Classify again using different parameters with 'lasclassify':

lasclassify -i ll.laz -ilay -olay -rugged 2 -step 0.5
laslayers -i ll.laz
lasview -i ll.laz -ilay 

Create raster splatted points only where vegetation is high with 'lasgrid':

lasgrid -i ll.laz -ilay -keep_class 5 -subcircle 0.1 -step 0.5 -highest -false -use_bb -odix _veg -opng

Create polygons around high vegetation areas with 'lasboundary':

lasboundary -i ll.laz -ilay -keep_class 5 -concavity 1.0 -disjoint -odix _veg -oshp

Normalize the elevations to height above ground with 'lasheight':

lasheight -i ll.laz -ilay -replace_z -olay 
laslayers -i ll.laz
lasview -i ll.laz -ilay -color_by_elevation2

Create forestry metrics canopy cover and 50 / 75 / 95 percentile with 'lasheight':

lascanopy -i ll.laz -ilay -cov -p 50 75 95 -step 5 -oasc 
lasview -i ll_p50.asc ll_p75.asc ll_p95.asc -files_are_flightlines -color_by_flightline

Color the points using an ortho TIF image with 'lascolor':

lascolor -i ll.laz -ilay -image ortho.tif -olay 
laslayers -i ll.laz
lasview -i ll.laz -ilay

Look at the command sequence that got us here with 'laslayers':

laslayers -i ll.laz -short

Delete layer 4 (no longer needed) to make the LASlayers file smaller with 'laslayers'.

laslayers -i ll.laz -del 4
laslayers -i ll.laz
lasview -i ll.laz -ilay 

Send the resulting small LAY file instead of the larger LAZ file back to the customer.

====================================================
== (3) ==    create LASlayers difference files    ++
====================================================

We have a raw and a modified LAS file and want to create LASlayers that express the difference between them, storing only those parts that have actually changed which is much more compact.

Look at the modified LAZ file with 'lasview':

lasview -i ll_modified.laz

Create LASlayers that express the difference to the raw file with 'laslayers':

laslayers -i ll.laz -i ll_modified.laz -olay
laslayers -i ll.laz
lasview -i ll.laz -ilay
lasview -i ll.laz -ilay 1
lasview -i ll.laz -ilay 2
lasview -i ll.laz -ilay 3

Apply LASlayers to the raw file to produce the modified one with 'laslayers'.

laslayers.exe -i ll.laz -ilay -o ll_applied.laz
lasview -i ll_applied.laz

Make sure the two are identical with 'lasdiff':

lasdiff -i ll_modified.laz -i ll_applied.laz

====================================================

overview of all tool-specific switches:

-detach      : detach layers
-diff        : produce diff LAY file
-add         : create or add to LAY files with LASlayers storing filters,    
               transforms or file modifications
-short       : output layer information only
-del_last n  : delete last [n] layers
-del n       : delete layer with index [n(1..32)]


if you find bugs let me (info@rapidlasso.de) know.

https://rapidlasso.de - fast tools to catch reality
