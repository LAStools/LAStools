****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

  lastool

  One tool to rule them all - a simple GUI for LAStools.
  
  This tool is obsolete now:
  Right now we have a GUI within most tool items.
  Soon we will provide a new GUI wrapper for all tools.
  

  Please license from info@rapidlasso.de to use lastool 
  commercially.

  For updates check the website or join the LAStools mailing list.

  http://lastools.org/
  http://groups.google.com/group/lastools/

  Jochen @lastools
 
  Acknowledgment:
  The GUI of lastool is realized with Paul Rademacher's GLUI - a
  GLUT-based C++ user interface library which provides controls
  such as buttons, checkboxes, radio buttons, and spinners to
  OpenGL applications. see: http://www.cs.unc.edu/~rademach/glui/

****************************************************************

example usage:

>> lastool
>> lastool -i terrain.las
>> lastool -i flight1.las flight2.las flight3.las
>> lastool -i *.las
>> lastool -i *.laz
>> lastool -i C:\data\lake_superior\*.las

loads some preview information of the listed files into the
bounding box view of the lastool GUI.

for more info:

C:\lastools\bin> lastool -h
Please note that LAStools is not "free" (see http://lastools.org/LICENSE.txt)
contact 'info@rapidlasso.de' to clarify licensing terms if needed.
LAStools (by info@rapidlasso.de) version 140615
usage:
lastool
lastool -i *.las
lastool -i c:\data\lake_superior\*.laz
lastool -i flight1*.laz flight2*.laz
lastool -i lidar1.las lidar2.las lidar3.las lidar4.las
lastool -i *.txt -iparse xyziat
lastool -h

---------------

if you find bugs let us (info@rapidlasso.de) know
