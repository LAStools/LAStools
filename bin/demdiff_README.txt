****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

  demdiff:

  Compares rasters in ASC, BIL, TIF, IMG and RasterLAZ format and
  reports differences.

  For updates check the website or join the LAStools mailing list.

  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @rapidlasso

****************************************************************

for example usage see:

https://groups.google.com/d/topic/lastools/39hR_4BvvIA/discussion
https://groups.google.com/d/topic/lastools/nMPU75zpqPw/discussion

overview of all tool-specific switches:

-h                                   : show help output
-v                                   : more info reported in console
-vv                                  : even more info reported in console
-quiet                               : nothing reported in console
-version                             : reports this tool's version number

for more info:

D:\software\LAStools\bin>demdiff -h
usage:
demdiff -i dem.tif -i dem.laz
demdiff -i dem.img -i dem.laz
demdiff -i dem.asc -i dem.laz
demdiff -i dem.bil -i dem.laz
demdiff -i dem1.laz -i dem2.laz
demdiff -i *.tif
demdiff -i *.img
demdiff -i *.asc
demdiff -i *.bil
demdiff -h

---------------

if you find bugs let me (info@rapidlasso.de) know.