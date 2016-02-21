****************************************************************

  laspublish:

  Creates a LiDAR portal for 3D visualization (and optionally also
  for downloading) of LAS and LAZ files in any modern Web browser
  using the WebGL Potree from Markus Schuetz.

  For updates check the website or join the LAStools mailing list.

  http://rapidlasso.com/laspublish/
  http://rapidlasso.com/LAStools/
  http://groups.google.com/group/LAStools/
  http://twitter.com/LAStools/
  http://facebook.com/LAStools/
  http://linkedin.com/groups/4408378

  Martin @LAStools

****************************************************************

example usage:

>> laspublish -i lidar.laz -odir portal -o portal.html -olaz

creates a directory called "./portal" that contains an HTML file
called "portal.html" that allows exploring the LiDAR points from
the file "lidar.laz" once the directory is moved into Web space.

>> laspublish -i tiles_final/*.laz -odir portal -o portal.html -olaz

same as above for an entire folder of input files.

>> laspublish -i tiles_final/*.laz -odir portal -o portal.html -olaz

same as above for an entire folder of input files.

>> laspublish -i tiles_final/*.laz -only_2D -odir portal -o portal.html -olaz

same as above for an entire folder of input files but only the 2D map

---------------

if you find bugs let me (martin.isenburg@rapidlasso.com) know.