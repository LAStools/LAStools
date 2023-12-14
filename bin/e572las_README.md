# e572las

This tool reads LiDAR in the E57 format from a *.e57 file and
converts it to either standard LAS, compressed LAZ, or simple
ASCII TXT.

By default all scans contained in the E57 file are merged into
one output with all invalid points being omitted. The points of
different scans are given different point source IDs so that the
information which points belong to one scan is preserved.  It's
possible to request '-split_scans' and '-include_invalid' to put
points from different scans into separate files and/or to include 
invalid points as well.

Another useful option is '-v' or '-verbose' that informs about
the contents and the progress of processing.

Below are some typical example command lines for the data that
can be found here: http://www.libe57.org/data.html

e572las -v -i pumpACartesian.e57 -o pumpACartesian.laz   
e572las -v -i Station018.e57 -o Station018.laz -set_scale 0.002 0.002 0.002  
e572las -v -i trimble.e57 -o trimble.laz  
e572las -v -i pump.e57 -o pump.las -split_scans  
e572las -v -i trimble.e57 -o trimble.txt -oparse xyzi  
e572las -v -i pump.e57 -o pump.laz -include_invalid  

By default the tool will apply transformations and/or rotations
that it find stored in the pose of each scan. You can ask the tool
not to apply those with '-no_pose'. To selevtively suppress only
transformation or rotation use '-no_transformation' or '-no_rotation'

  
## Examples

    e572las -i Tikal.e57 -o Tikal.laz
convert from las e57 into LAZ. Result file is much smaller due compression:
    dir Tikal.*
     1,126,514,688 Tikal.e57
       172,269,240 Tikal.laz


    e572las -v -i pumpACartesian.e57 -o pumpACartesian.laz 
file 'pumpACartesian.e57' contains 1 scan  
processing scan 1 of 1 ...  
  contains grid of 345 by 1074 equaling 370530 points  
  contains intensities (0-1)  
  contains RGB colors (0-255, 0-255, 0-255)  
  is written with millimeter resolution to 'pumpACartesian.laz'  
  215329 invalid points were omitted  
scan of 'pumpACartesian.e57' contains 215329 invalid points that were omitted  
written a total 155201 points  

    dir pumpACartesian.*
     5,261,312 pumpACartesian.e57
       444,513 pumpACartesian.laz


    e572las -v -i pump.e57 -o pump.laz 
file 'pump.e57' contains 5 scans. merging ...  
processing scan 1 of 5 ...  
  contains grid of 242 by 1739 equaling 420838 points  
  has quaternion (0.999994,0.000820005,0.00268102,-0.00180426) which is applied  
  has translation (-3.02875,-3.81974,-1.38433) which is applied  
  contains intensities (0-1)  
  contains RGB colors (0-255, 0-255, 0-255)  
  all scans are written with millimeter resolution to 'pump.laz'  
  246359 invalid points were omitted  
processing scan 2 of 5 ...  
  contains grid of 345 by 1074 equaling 370530 points  
  contains intensities (0-1)  
  contains RGB colors (0-255, 0-255, 0-255)  
  215329 invalid points were omitted  
processing scan 3 of 5 ...  
  contains grid of 233 by 1672 equaling 389576 points  
  has quaternion (0.793531,-0.00198763,0.000784153,-0.608525) which is applied  
  has translation (0.676397,-5.7837,-1.40222) which is applied  
  contains intensities (0-1)  
  contains RGB colors (0-255, 0-255, 0-255)  
  227957 invalid points were omitted  
processing scan 4 of 5 ...  
  contains grid of 190 by 1358 equaling 258020 points  
  has quaternion (0.947786,0.000996269,-0.00143993,0.318902) which is applied  
  has translation (2.04813,-3.77893,-0.027761) which is applied  
  contains intensities (0-1)  
  contains RGB colors (0-255, 0-255, 0-255)  
  139967 invalid points were omitted  
processing scan 5 of 5 ...  
  contains grid of 720 by 2000 equaling 1440000 points  
  has quaternion (0.958191,-0.0112013,0.00244497,-0.2859) which is applied  
  has translation (-0.642414,-3.60494,-1.43066) which is applied  
  contains intensities (0-1)  
  contains RGB colors (0-255, 0-255, 0-255)  
  835362 invalid points were omitted  
scans of 'pump.e57' contain 1664974 invalid points that were omitted  
written a total 1213990 points  
    
    dir pump.*  
     52,496,384 pump.e57  
      3,374,559 pump.laz  
  
  
    e572las -v -i Station018.e57 -o Station018.laz -set_scale 0.002 0.002 0.002  
file 'Station018.e57' contains 1 scan  
processing scan 1 of 1 ...  
  contains grid of 5026 by 1664 equaling 8363264 points  
  has quaternion (0.381393,0,0,0.924413) which is applied  
  has translation (813.17,599.102,29.6163) which is applied  
  contains intensities (0-1)  
  contains RGB colors (0-255, 0-255, 0-255)  
  is written with resolution 0.002 0.002 0.002 to 'Station018.laz'  
written a total 4067815 points  
  
    dir Station018.*  
     117,242,880 Station018.e57  
      15,153,726 Station018.laz  
  
  
    e572las -v -i manitou.e57 -o manitou.laz  
file 'manitou.e57' contains 5 scans. merging ...  
processing scan 1 of 5 ...  
  contains grid of 3079 by 388 equaling 1194652 points  
  has quaternion (0.883724,0.00106198,-0.0013313,0.468005) which is applied  
  has translation (0.006588,-0.011522,0.003971) which is applied  
  contains intensities (0-1)  
  all scans are written with millimeter resolution to 'manitou.laz'  
  848208 invalid points were omitted  
processing scan 2 of 5 ...  
  contains grid of 2021 by 255 equaling 515355 points  
  has quaternion (0.84688,-0.148431,0.243503,0.448853) which is applied  
  has translation (0.0545705,0.0595203,-0.0199791) which is applied  
  contains intensities (0-1)  
  358947 invalid points were omitted  
processing scan 3 of 5 ...  
  contains grid of 3083 by 388 equaling 1196204 points  
  has quaternion (0.961504,0.000668005,0.00200447,-0.274783) which is applied  
  has translation (15.8151,-8.602,-0.192073) which is applied  
  contains intensities (0-1)  
  742577 invalid points were omitted  
processing scan 4 of 5 ...  
  contains grid of 1046 by 388 equaling 405848 points  
  has quaternion (0.951362,-0.0401362,0.297261,0.070251) which is applied  
  has translation (43.2483,-24.8545,-0.67416) which is applied  
  contains intensities (0-1)  
  371670 invalid points were omitted  
processing scan 5 of 5 ...  
  contains grid of 2089 by 388 equaling 810532 points  
  has quaternion (0.909579,0.0705078,0.315922,-0.260553) which is applied  
  has translation (15.9006,-8.65044,-0.222259) which is applied  
  contains intensities (0-1)  
  705487 invalid points were omitted  
scans of 'manitou.e57' contain 3026889 invalid points that were omitted  
written a total 1095702 points  
  
    dir manitou.*  
     72,008,704 manitou.e57  
      3,042,730 manitou.laz  
  
      
    e572las -v -i pump.e57 -o pump_scan.laz -split_scans  
file 'pump.e57' contains 5 scans. splitting ...  
processing scan 1 of 5 ...  
  contains grid of 242 by 1739 equaling 420838 points  
  has quaternion (0.999994,0.000820005,0.00268102,-0.00180426) which is applied  
  has translation (-3.02875,-3.81974,-1.38433) which is applied  
  contains intensities (0-1)  
  contains RGB colors (0-255, 0-255, 0-255)  
  is written with millimeter resolution to 'pump_scan00000.laz'  
  246359 invalid points were omitted  
processing scan 2 of 5 ...  
  contains grid of 345 by 1074 equaling 370530 points  
  contains intensities (0-1)  
  contains RGB colors (0-255, 0-255, 0-255)  
  is written with millimeter resolution to 'pump_scan00001.laz'  
  215329 invalid points were omitted  
processing scan 3 of 5 ...  
  contains grid of 233 by 1672 equaling 389576 points  
  has quaternion (0.793531,-0.00198763,0.000784153,-0.608525) which is applied  
  has translation (0.676397,-5.7837,-1.40222) which is applied  
  contains intensities (0-1)  
  contains RGB colors (0-255, 0-255, 0-255)  
  is written with millimeter resolution to 'pump_scan00002.laz'  
  227957 invalid points were omitted  
processing scan 4 of 5 ...  
  contains grid of 190 by 1358 equaling 258020 points  
  has quaternion (0.947786,0.000996269,-0.00143993,0.318902) which is applied  
  has translation (2.04813,-3.77893,-0.027761) which is applied  
  contains intensities (0-1)  
  contains RGB colors (0-255, 0-255, 0-255)  
  is written with millimeter resolution to 'pump_scan00003.laz'  
  139967 invalid points were omitted  
processing scan 5 of 5 ...  
  contains grid of 720 by 2000 equaling 1440000 points  
  has quaternion (0.958191,-0.0112013,0.00244497,-0.2859) which is applied  
  has translation (-0.642414,-3.60494,-1.43066) which is applied  
  contains intensities (0-1)  
  contains RGB colors (0-255, 0-255, 0-255)  
  is written with millimeter resolution to 'pump_scan00004.laz'  
  835362 invalid points were omitted  
scans of 'pump.e57' contain 1664974 invalid points that were omitted  
written a total 1213990 points  
  
    dir pump.e57 pump_scan*.laz  
     52,496,384 pump.e57  
        465,152 pump_scan00000.laz  
        444,513 pump_scan00001.laz  
        478,615 pump_scan00002.laz  
        372,742 pump_scan00003.laz  
      1,613,204 pump_scan00004.laz


## e572las specific arguments

-include_invalid       : include invalid points into target  
-set_scale [x] [y] [z] : quantize ASCII points with [x] [y] [z] (unit meters)  
-split_scans           : split output files by scan  


## License

This tool is free to use.

## Support

To get more information about a tool just goto the
[LAStools Google Group](http://groups.google.com/group/lastools/)
and enter the tool name in the search function.
You will get plenty of samples to this tool.

To get further support see our
[rapidlasso service page](https://rapidlasso.de/service/)

Check for latest updates at
https://rapidlasso.de/category/blog/releases/

If you have any suggestions please let us (info@rapidlasso.de) know.
