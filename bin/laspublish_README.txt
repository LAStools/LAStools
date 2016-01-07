****************************************************************

  laspublish:

  Allows to visualize (and eventually to download) LiDAR in any
  modern Web browser using Potree from Markus Schuetz.

  For updates check the website or join the LAStools mailing list.

  http://lastools.org/
  http://groups.google.com/group/LAStools/
  http://twitter.com/LAStools/
  http://facebook.com/LAStools/
  http://linkedin.com/groups?gid=4408378

  Martin @LAStools

****************************************************************

example usage:

>> laspublish -i lidar.laz -odir portal -o portal.html -olaz

creates a directory called "./portal" that contains an HTML file
called "portal.html" that allows exploring the LiDAR points from
the file "lidar.laz" once the directory is moved into Web space.

>> lasindex -i tiles_final/*.laz -odir portal -o portal.html -olaz

same as above for an entire folder of input files.

---------------

if you find bugs let me (martin.isenburg@rapidlasso.com) know.