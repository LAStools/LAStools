****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

  lasoverage:

  This tool reads LIDAR point in the LAS/LAZ/ASCII/BIN format
  of an airborne collect and finds the "overage" points that
  get covered by more than a single flightline. It either marks
  these overage points or removes them from the output files.
  The tool requires that the files either have the flightline
  information stored for each point in the point source ID field
  (e.g. for tiles containing overlapping flightlines) or that
  there are multiple files where each corresponds to a flight
  line ('-files_are_flightlines'). It is also required that the
  scan angle field of each point is populated.

  If the point source ID field of a LAS tile is not properly
  populated (but there are GPS time stamps) and each point has
  a scan angle, then you can use the '-recover_flightlines'
  flag that reconstructs the missing flightline information
  from gaps in the GPS time.

  The most important parameter is '-step n' that specifies the
  granularity with which the overage points are computed. It
  should be set to approximately 2 times the point spacing in
  meters.  If unknown, you can compute the point spacing with
  the lasinfo tool via 'lasinfo -i lidar.las -cd'. 

  If the x and y coordinates of the input files are in feet the
  '-feet' flag needs to be set (unless the LAS/LAZ file has a
  projection with this information).

  If the input are multiple files corresponding to individual
  flight lines the '-files_are_flightlines' parameter must be
  set.

  By default the tool will set the classification of the overage
  points to 12. However, instead you can also choose to use the
  '-flag_as_withheld' or '-flag_as_overlap' (new LAS 1.4 point
  types only) flags, '-remove_overage' points from the output,
  or '-classify_as 18' to a different classification.
  
  Rather than classifying/flagging/deleting the overage you can
  also determine the '-entire_overlap' in the file and classify,
  flag, or delete it with the options described above.

  Below an explaination based on what Karl Heidemann of the USGS
  once told me regarding "overlap" versus "overage" points:

  "Overlap" applies to any point in one flightline or swath that
  is anywhere within the boundary of another flightline. Whether
  or not a point is overlap is determined, basically, at the time
  of flight. It is a fundamentally immutable characteristic.

  "Overage" applies to any point not in the "tenderloin" of the
  flightline or swath, that is, the central core of the flightline
  that - when combined with the others - creates a non-overlapped
  and gapless coverage of the surface.  Whether or not a point is
  overage is determined by the operator (or software) that defines
  the single-coverage tenderloins of the flightlines. Hence, it is
  a subjective characteristic -- it depends solely on where somebody
  (here: lasoverage.exe) chooses to draw the boundary.

  Unfortunately the LAS specification uses the word "overlap" where
  it should be using the word "overage".

  Please license from info@rapidlasso.de before using lasoverage
  commercially.

  For updates check the website or join the LAStools mailing list.

  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @lastools
 
****************************************************************

example usage:

>> lasoverage -i tile.las -step 2 -o tile_overage.laz

finds the overlap points and classifies them as 12 for a LAS tile
with a point spacing of around 1.0 meters. For this to work, the
LiDAR points in the LAS file have their point source ID populated
with the flight line number. The output is also compressed.

>> lasoverage -i tiles\tile_*.laz -step 2 -flag_as_withheld -olaz

same as above but for an entire folder of LAZ tiles and with the
overage points being marked as "withheld".

>> lasoverage -i tiles\tile_*.laz -step 2 -recover_flightlines -flag_as_withheld -cores 4 -odix _flagged -olaz

same as above but with an initial pass over each tile where the
flightline information is reconstructed in an initial pass over
the points by looking for continuous intervals of GPS time stamps
and by operating on 4 cores.

>> lasoverage -i tiles\tile_*.las -step 2 -flag_as_overlap -odix _flagged -olas

same as above but for an entire folder of LAS 1.4 tiles and with 
the overage points being marked as "overlap". this can only be used
with the new point types 6 and higher ...

>> lasoverage -i flight\lines*.laz -files_are_flightlines -odir flight_flagged -olaz

here all files are considered to be part of the same flight and
the overlap in flightstrips is computed across all files. all points
of a file are considered to be from the same flightline and their
overlap against all other files is computed. The LiDAR points of
each strip should have a point spacing around 0.5 meters since the
default step of 1 is used.

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
-faf                                 : only makes sense for multiple overlapping input files then treated as individual flightlines
-step 2.0                            : set resolution of grid (granularity) used for overage points computation [default is 1.0]
-flag_as_withheld                    : set 'withheld flag' for overage points instead of setting classification to 12
-flag_as_overlap                     : set 'overlap flag' for overage points instead of setting classification to 12
-remove_overage                      : remove overage points from output instead of setting their classification to 12
-recover_flightlines                 : try to recover flightline information by finding 10 seconds gaps in GPS time
-recover_flightlines_interval 5      : try to recover flightline information by finding 5 seconds gaps in GPS time
-ilay                                : apply all LASlayers found in corresponding *.lay file on read
-ilay 3                              : apply first three LASlayers found in corresponding *.lay file on read
-ilaydir E:\my_layers                : look for corresponding *.lay file in directory E:\my_layers
-olay                                : write or append classification changes to a LASlayers *.lay file
-olaydir E:\my_layers                : write the output *.lay file in directory E:\my_layers

****************************************************************

for more info:

---------------

if you find bugs let me (info@rapidlasso.de) know
