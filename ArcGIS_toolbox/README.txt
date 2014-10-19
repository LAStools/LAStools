****************************************************************

  LAStools LiDAR processing toolboxes for ArcGIS 9.3 ---> 10.2

   (c) 2012-2014 by rapidlasso - fast tools to catch reality

  legal details: http://lastools.org/LICENSE.txt

****************************************************************

(1) Do not move or copy the "LAStools.tbx" from its original
location .\lastools\ArcGIS_toolbox\LAStools.tbx where it is
after unzipping the lastools.zip distribution. Instead, import
the "LAStools.tbx" at its original location using the "add
Toolbox" mechanism. To get to this location you may have to
make a "folder connection" in ArcGIS to the place where you
unzipped lastools.zip to.

(2) Do not move the .\lastools\ArcGIS_toolbox\scripts or the
.\lastools\bin or the folder. The relative path between the
ArcGIS toolbox and the LAStools binaries needs to be preserved.

(3) It's VERY VERY important that there is no space (e.g. " ")
or other funny characters such as "(2)" or else in the name of
your LAStools folder path. The limitation is inherent to Python.
Try to always avoid spaces or unusual characters in a directory
name or in a file name. It's bad practice. Instead you can use
underscores to seperate words.

This is okay:
C:\lastools
C:\LiDAR\lastools
C:\kickass_tools\lastools
C:\super_fast_LiDAR_tools\lastools
  
This is *NOT* okay:
C:\Documents and Settings\isenburg\lastools
C:\Program Files\lastools
C:\Program Files\lastools(2)
C:\LiDAR software\lastools
C:\i am totally loving it\lastools
