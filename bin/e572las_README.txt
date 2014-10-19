****************************************************************

  e572las.exe:

  This tool reads LiDAR in the E57 format from a *.e57 file and
  converts it to either standard LAS, compressed LAZ, or simple
  ASCII TXT.

  By default all scans contained in the E57 file are merged into
  one output with all invalid points being omitted. It's possible
  to request '-split_scans' and '-include_invalid' to change the
  default behaviour.

  Another useful option is '-v' or '-v' that informs about
  the contents and the progress of processing.

  Below are some typical example command lines for the data that
  can be found here: http://www.libe57.org/data.html

  e572las -v -i pumpACartesian.e57 -o pumpACartesian.laz 
  e572las -v -i Station018.e57 -o Station018.laz -set_scale 0.002 0.002 0.002
  e572las -v -i trimble.e57 -o trimble.laz
  e572las -v -i pump.e57 -o pump.las -split_scans
  e572las -v -i trimble.e57 -o trimble.txt -oparse xyzi
  e572las -v -i pump.e57 -o pump.laz -include_invalid

  Please note that this is an alpha release that may still have
  a few bugs and also lacks some imporant features. Most notably
  the tool does not yet apply transformations and/or rotations
  that can be specified in a E57 file, which will then lead to
  wrong results when merging scans. Stay tuned ...

  Please license from martin@rapidlasso.com to use e572las.exe
  commercially. For frequent updates check the website or join
  the LAStools mailing list.

  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @lastools
 
****************************************************************

>> e572las -v -i pumpACartesian.e57 -o pumpACartesian.laz 
file 'pumpACartesian.e57' contains 1 scan
processing scan 0 ...
  contains grid of 345 by 1074 equaling 370530 points
  contains intensities (0-1)
  contains RGB colors (0-255, 0-255, 0-255)
  is written with millimeter resolution to 'pumpACartesian.laz'
  215329 invalid points were omitted
scan of 'pumpACartesian.e57' contains 215329 invalid points that were omitted


C:\released_code\e572las>e572las -v -i Station018.e57 -o Station018.laz -set_scale 0.002 0.002 0.002
file 'Station018.e57' contains 1 scan
processing scan 0 ...
  contains grid of 5026 by 1664 equaling 8363264 points
  contains intensities (0-1)
  contains RGB colors (0-255, 0-255, 0-255)
  is written with resolution 0.002 0.002 0.002 to 'Station018.laz'


C:\released_code\e572las>  e572las -v -i trimble.e57 -o trimble.laz
file 'trimble.e57' contains 13 scans. merging ...
processing scan 0 ...
  contains 237123 points
  contains intensities (0-255)
  contains RGB colors (0-255, 0-255, 0-255)
  all scans are written with millimeter resolution to 'trimble.laz'
processing scan 1 ...
  contains 4984 points
  contains intensities (0-255)
  contains RGB colors (0-255, 0-255, 0-255)
processing scan 2 ...
  contains 4018 points
  contains intensities (0-255)
  contains RGB colors (0-255, 0-255, 0-255)
processing scan 3 ...
  contains 2041 points
  contains intensities (0-255)
  contains RGB colors (0-255, 0-255, 0-255)
processing scan 4 ...
  contains 196036 points
  contains intensities (0-255)
  contains RGB colors (0-255, 0-255, 0-255)
processing scan 5 ...
  contains 3796 points
  contains intensities (0-255)
  contains RGB colors (0-255, 0-255, 0-255)
processing scan 6 ...
  contains 1125 points
  contains intensities (0-255)
  contains RGB colors (0-255, 0-255, 0-255)
processing scan 7 ...
  contains 3906 points
  contains intensities (0-255)
  contains RGB colors (0-255, 0-255, 0-255)
processing scan 8 ...
  contains 7587 points
  contains intensities (0-255)
  contains RGB colors (0-255, 0-255, 0-255)
processing scan 9 ...
  contains 1352 points
  contains intensities (0-255)
  contains RGB colors (0-255, 0-255, 0-255)
processing scan 10 ...
  contains 4769 points
  contains intensities (0-255)
  contains RGB colors (0-255, 0-255, 0-255)
processing scan 11 ...
  contains 237123 points
  contains intensities (0-255)
  contains RGB colors (0-255, 0-255, 0-255)
processing scan 12 ...
  contains 196036 points
  contains intensities (0-255)
  contains RGB colors (0-255, 0-255, 0-255)


>> e572las -v -i pump.e57 -o pump.las -split_scans
file 'pump.e57' contains 5 scans. splitting ...
processing scan 0 ...
  contains grid of 242 by 1739 equaling 420838 points
  contains intensities (0-1)
  contains RGB colors (0-255, 0-255, 0-255)
  is written with millimeter resolution to 'pump00000.las'
  246359 invalid points were omitted
processing scan 1 ...
  contains grid of 345 by 1074 equaling 370530 points
  contains intensities (0-1)
  contains RGB colors (0-255, 0-255, 0-255)
  is written with millimeter resolution to 'pump00001.las'
  215329 invalid points were omitted
processing scan 2 ...
  contains grid of 233 by 1672 equaling 389576 points
  contains intensities (0-1)
  contains RGB colors (0-255, 0-255, 0-255)
  is written with millimeter resolution to 'pump00002.las'
  227957 invalid points were omitted
processing scan 3 ...
  contains grid of 190 by 1358 equaling 258020 points
  contains intensities (0-1)
  contains RGB colors (0-255, 0-255, 0-255)
  is written with millimeter resolution to 'pump00003.las'
  139967 invalid points were omitted
processing scan 4 ...
  contains grid of 720 by 2000 equaling 1440000 points
  contains intensities (0-1)
  contains RGB colors (0-255, 0-255, 0-255)
  is written with millimeter resolution to 'pump00004.las'
  835362 invalid points were omitted
scans of 'pump.e57' contain 1664974 invalid points that were omitted



---------------

if you find bugs let me (martin@rapidlasso.com) know
