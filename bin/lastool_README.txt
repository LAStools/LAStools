****************************************************************

  lastool

  One tool to rule them all - a simple GUI for LAStools.

  Please license from martin.isenburg@rapidlasso.com to use lastool 
  commercially.

  For updates check the website or join the LAStools mailing list.

  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @lastools
 
  acknowledgment:
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
contact 'martin.isenburg@rapidlasso.com' to clarify licensing terms if needed.
LAStools (by martin@rapidlasso.com) version 140615
usage:
lastool
lastool -i *.las
lastool -i c:\data\lake_superior\*.laz
lastool -i flight1*.laz flight2*.laz
lastool -i lidar1.las lidar2.las lidar3.las lidar4.las
lastool -i *.txt -iparse xyziat
lastool -h

---------------

if you find bugs let me (martin.isenburg@rapidlasso.com) know
