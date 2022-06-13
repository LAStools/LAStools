****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

  shp2las:

  Converts from points from ESRI's Shapefile to LAS/LAZ/ASCII
  format given the input contains Points or MultiPoints (that
  is any of the shape types 1,11,21,8,18,28).
 
  Allows adding a VLR to the header with projection information.

  Please license from info@rapidlasso.de to use shp2las
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

>> shp2las lidar.shp

converts the ESRI's Shapefile 'lidar.shp' to the LAS file 'lidar.las'

>> shp2las -compress lidar.shp

converts 'lidar.shp' to the compressed LAZ file 'lidar.laz'

>> shp2las -set_scale 0.001 0.001 0.001 -i lidar.shp -o lidar.las

converts 'lidar.shp' to the LAS file 'lidar.las' with the specified scale

>> shp2las -i lidar.shp -o lidar.laz -set_offset 500000 4000000

converts 'lidar.shp' to the compressed LAZ file 'lidar.laz' with the specified offsey

>> shp2las -i lidar.shp -o lidar.las -v

converts 'lidar.shp' to the LAS file 'lidar.las' and outputs some of
the header information found in the SHP file

C:\lastools\bin>shp2las -h
Supported LAS Outputs
  -o lidar.las
  -o lidar.laz
  -o xyzta.txt -oparse xyzta (on-the-fly to ASCII)
  -olas -olaz -otxt (specify format)
  -stdout (pipe to stdout)
  -nil    (pipe to NULL)
usage:
shp2las -i lidar.shp
shp2las -i lidar.shp -compress
shp2las -i lidar.shp -o lidar.las -utm 12S
shp2las -set_offset 500000 4000000 -ishp -o lidar.laz < lidar.shp
shp2las -set_scale 0.001 0.001 0.001 0 lidar.shp lidar.las
shp2las -h
---------------------------------------------
Other parameters are
'-set_scale 0.05 0.05 0.001'
'-set_offset 500000 2000000 0'
'-set_file_creation 67 2011'
'-set_system_identifier "Riegl 500,000 Hz"'
'-set_generating_software "LAStools"'
'-utm 14T'
'-sp83 CA_I -feet -elevation_survey_feet'
'-longlat -elevation_feet'

---------------

if you find bugs let me (info@rapidlasso.de) know.
