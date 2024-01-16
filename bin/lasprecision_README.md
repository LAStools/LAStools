# lasprecision

reads LIDAR data in the LAS format and computes statistics that
tell us whether the precision "advertised" in the header is
really in the data. Often I find that the scaling factors of
a LAS/LAZ file is miss-leading because they make it appear as
if there was much more precision than there really is.

To judge that we compute coordinate difference histograms that
allow you to easily decide whether there is artificially high
precision in the LAS/LAZ/BIN file. This may have been introduced
during LAS conversion by "upsampling" to a higher precision
"to play it safe".

Once you figured out the "correct" precision you can also use
this tool to resample the LAS/LAZ file to an appropriate level
of precision. A big motivation to remove "fake" precision is
that LAS files compress much better with laszip without this
"fluff" in the low-order bits.

For example I find "fluff" in those examples:
- Grass Lake Small.las
- MARS_Sample_Filtered_LiDAR.las
- Mount St Helens Oct 4 2004.las
- IowaDNR-CloudPeakSoft-1.0-UTM15N.las
- LAS12_Sample_withRGB_QT_Modeler.las
- Lincoln.las
- Palm Beach Pre Hurricane.las

There are also options to look at the '-gps' time and the '-rgb'
colors in the same manner. You can change the amount of lines
that are output per statistic with '-lines 30'.


## Examples

What happens in the example below? We load -all the raw integer
x, y, and z coordinates of -i "MARS_Sample_Filtered_LiDAR.las"
into three separate arrays, sort each array in ascending sorted
orders, compute the difference between all neighboring values
and output a histogram in textural form.

What do these histograms tell us? We see that the original scale
factors are inflated/missleading. There is not actual millimeter
precision (=> 0.001) in the data. All x, and y, or, z values are
either 10, 20, 30, 40, or other multiple of 10 units spaced apart.
Hence the true precision is only centimeters (=> 0.01). This is
not nice and makes compression less efficient.

    lasprecision64 -i MARS_Sample_Filtered_LiDAR.las -all
original scale factors: 0.001 0.001 0.001
loading first 8146178 of 8146178 points
X differences
          0 :    7822346   0
         10 :     323326   0.01
         20 :        500   0.02
         30 :          5   0.03
Y differences
          0 :    7533456   0
         10 :     609382   0.01
         20 :       3303   0.02
         30 :         36   0.03
Z differences
          0 :    8124512   0
         10 :      21175   0.01
         20 :        346   0.02
         30 :         75   0.03
         40 :         23   0.04
         50 :          7   0.05
         60 :          4   0.06
...

Let can take a closer look at the *raw* XYZ integers in this LAS
file to understand what is happening:

    las2txt64 -parse XYZ -i "MARS_Sample_Filtered_LiDAR.las" -otxt | more
2136823840 1699144370 5192090  
2136824490 1699143020 5192110  
2136825130 1699141660 5192130  
2136825780 1699140310 5192150  
2136826420 1699138950 5192170  
2136827070 1699137600 5192190  
2136827710 1699136250 5192210  
2136828360 1699134890 5192220  
2136829000 1699133540 5192240  
2136829650 1699132180 5192260  
2136830290 1699130830 5192280  
2136830940 1699129470 5192300  
...  

This is obviously not the right way to use the LAS format. Anyone peeking
in the header with lasinfo.exe would be lead to mistakenly assume that
the file has more precision that it actually has. We can fix that by
changing the scale factor to reflect the actual precision in this file:

    lasprecision64 -i MARS_Sample_Filtered_LiDAR.las -rescale 0.01 0.01 0.01 -o MARS_Sample_Filtered_LiDAR_rescaled.las
new scale factors: 0.01 0.01 0.01

So lets look at the rescaled file and we see that now we get the type of
histogram one would expect.

    lasprecision64 -i MARS_Sample_Filtered_LiDAR_rescaled.las -all
original scale factors: 0.01 0.01 0.01  
loading first 8146178 of 8146178 points  
X differences  
          0 :    7822346   0  
          1 :     323326   0.01  
          2 :        500   0.02  
          3 :          5   0.03  
Y differences  
          0 :    7533456   0  
          1 :     609382   0.01  
          2 :       3303   0.02  
          3 :         36   0.03  
Z differences  
          0 :    8124512   0  
          1 :      21175   0.01  
          2 :        346   0.02  
          3 :         75   0.03  
          4 :         23   0.04  
          5 :          7   0.05  
          6 :          4   0.06  

And in this particular example we did not even loose precision at all. The
double-precision coordinates after scaling are *identical* as lasdiff.exe
tell us (see below). That might not always be the case because usually tiny
rounding-error are introduced when excessive "fake" precision is in the file. 

    lasdiff64 -i MARS_Sample_Filtered_LiDAR.las -i MARS_Sample_Filtered_LiDAR_rescaled.las
header is different  
  WARNING: different x_scale_factor: 0.001 0.01  
  WARNING: different y_scale_factor: 0.001 0.01  
  WARNING: different z_scale_factor: 0.001 0.01  
headers have 3 differences.  
scaled points are identical. files are identical. both have 8146178 points.  

Note that LAZ compression works *much* better on a properly scaled file:

01/10/2010  01:46 AM       163,225,753 MARS_Sample_Filtered_LiDAR.las    
12/02/2010  01:02 PM        19,637,873 MARS_Sample_Filtered_LiDAR.laz    
12/02/2010  11:04 AM       163,225,753 MARS_Sample_Filtered_LiDAR_rescale.las    
12/02/2010  01:01 PM        17,932,612 MARS_Sample_Filtered_LiDAR_rescale.laz    


another (very bad) example (note the scale factors!!!):

C:\released_code\lastools\bin>lasprecision -i "Mount St Helens Oct 4 2004.las" -all -lines 105  
original scale factors: 1.22154e-005 1.67604e-005 2.39289e-006  
loading first 6743176 of 6743176 points  
X differences  
          3 :    2698410   3.66462e-005  
          4 :    1591251   4.88616e-005  
        804 :      85952   0.00982118  
        805 :      34047   0.0098334  
        807 :     523989   0.00985783  
        808 :     993344   0.00987004  
        811 :     355349   0.00990669  
        812 :       9337   0.0099189  
        814 :     220057   0.00994333  
        815 :     144628   0.00995555  
        817 :       4811   0.00997998  
        818 :      15857   0.0099922  
       1632 :      17269   0.0199355  
       1633 :       3395   0.0199477  
       2446 :       9077   0.0298789  
       2447 :      11588   0.0298911  
       3260 :        880   0.0398222  
       3261 :      19783   0.0398344  
...  
Y differences  
          2 :    1797569   3.35208e-005  
          3 :    1511859   5.02812e-005  
        591 :    2301976   0.0099054  
        592 :     243796   0.00992216  
        593 :     358431   0.00993892  
        594 :     442732   0.00995568  
        596 :      20476   0.0099892  
        597 :        190   0.010006  
       1189 :       9028   0.0199281  
       1190 :      11637   0.0199449  
       1783 :      18302   0.0298838  
       1784 :       2363   0.0299006  
       2376 :       6904   0.0398227  
       2377 :      13763   0.0398395  
       2970 :        652   0.0497784  
       2971 :        183   0.0497951  
...  
Z differences  
          0 :    1223726   0  
          1 :         53   2.39289e-006  
          2 :         46   4.78578e-006  
...  
         24 :         64   5.74294e-005  
         25 :      13395   5.98223e-005  
         26 :      13788   6.22152e-005  
         27 :         66   6.46081e-005  
...  
         50 :         48   0.000119645  
         51 :     141693   0.000122037  
         52 :       1999   0.00012443  
         53 :         43   0.000126823  
...  
         74 :         99   0.000177074  
         75 :         74   0.000179467  
         76 :      11837   0.00018186  
         77 :      12752   0.000184253  
         78 :         56   0.000186646  
         79 :        112   0.000189038  
         80 :        159   0.000191431  
...  
        101 :         41   0.000241682  
        102 :    1527728   0.000244075  
        103 :      43356   0.000246468  
        104 :         57   0.000248861  
...  
        203 :         34   0.000485757  
        204 :     983926   0.00048815  
        205 :      57265   0.000490543  
        206 :         42   0.000492936  
...  
        305 :         30   0.000729832  
        306 :     500869   0.000732225  
        307 :      45069   0.000734618  
        308 :         37   0.000737011  
...  
        407 :         34   0.000973907  
        408 :     353998   0.0009763  
        409 :      43756   0.000978693  
        410 :         40   0.000981085  
...  

From the scale factors it is clear that someone was "abusing" the scaling
mechanism of LAS to "preserve" an insane precision level that certainly
the data never had to start with. I am not suprised that the data set was
created by http://prologic-inc.com/ ... (-; Want more evidence? Here is
some output from the header contents with lasinfo.exe. I very much dislike
the way the offset that was chosen as well as how the scaling was set such
that the raw integers of x, y, and z range from 0 to 2147483647. bad idea!

    lasinfo64 -i "Mount St Helens Oct 4 2004.las"
...    
  generating_software:       'Lidar Explorer by ProLogic, Inc.'    
...    
  scale factor x y z         1.22154e-005 1.67604e-005 2.39289e-006    
  offset x y z               1199921.7455906442 301025.6214073577 3191.5268554688    
  min x y z                  1199921.7455906442 301025.6214073577 3191.5268554688    
  max x y z                  1226154.1154316438 337018.3067043898 8330.221862793    
...    
  x 0 2147483647    
  y 0 2147483647    
  z 0 2147483647    

If you look closely at the histogram of "X differences" you will notice
that there is one grouping around 804-818, the next around 1632-1633,
the next around 2446-2447 ... notice a pattern? If you multiply these
groupings with the "fake" x_scale_factor we get 0.01, 0.02, 0.03, ...
The same holds true for the "Y differences" where the groupings are
591-597, 1189-1190, 1783-1784, ... it is less obvious what those groupings
are for the "Z differences" but 0.001 should be enough (i.e. millimeter )
precision given the data presents elevations in meters. We also move the
offset a bit. Why? This is left as a exercise for the reader ... (-;

    lasprecision64 -i "Mount St Helens Oct 4 2004.las" -o "Mount St Helens Oct 4 2004 rescaled.las" -rescale 0.01 0.01 0.001 -reoffset 1048576 262144 4096
new scale factors: 0.01 0.01 0.001

After rescaling the file we get a maximal floating point difference
of 0.005, 0.005, and 0.0005 for x, y, and z respectively.

    lasdiff64 -i "Mount St Helens Oct 4 2004.las" -i "Mount St Helens Oct 4 2004 rescaled.las"
header is different  
  WARNING: different x_scale_factor: 0.000012 0.01  
  WARNING: different y_scale_factor: 0.000017 0.01  
  WARNING: different z_scale_factor: 0.000002 0.001  
  WARNING: different x_offset: 1199921.745591 1048576  
  WARNING: different y_offset: 301025.621407 262144  
  WARNING: different z_offset: 3191.526855 4096  
  x: 493483974 15737385 scaled offset x 1.20595e+006 1.20595e+006  
  y: 37431 3888225 scaled offset y 301026 301026  
  z: 2307045 -898953 scaled offset z 3197.05 3197.05  
  x: 494302434 15738385 scaled offset x 1.20596e+006 1.20596e+006  
  y: 24954 3888204 scaled offset y 301026 301026  
  z: 3244473 -896709 scaled offset z 3199.29 3199.29  
  x: 495120893 15739384 scaled offset x 1.20597e+006 1.20597e+006  
  y: 12477 3888183 scaled offset y 301026 301026  
  z: 3296507 -896585 scaled offset z 3199.42 3199.42  
scaled offset points are different (max diff: 0.005 0.005 0.0005).  
both have 6743176 points.  

Please note that laszip.exe compression works *much much much* better on a
properly scaled file because the compressor does not have to compress the
excessive and "fake" presision in the lower bits that is really just noise.

12/03/2010  06:28 AM       134,868,035 Mount St Helens Oct 4 2004 rescaled.las  
12/03/2010  06:30 AM        12,984,744 Mount St Helens Oct 4 2004 rescaled.laz  
12/03/2009  08:29 PM       134,868,035 Mount St Helens Oct 4 2004.las  
12/03/2010  06:30 AM        20,337,000 Mount St Helens Oct 4 2004.laz  

A few other files where I found excessive precision with lasprecision.exe:

    lasprecision64 -i "Grass Lake Small.las"
original scale factors: 0.01 0.01 0.01  
loading first 5000000 of 6190800 points  
X differences  
          0 :    4997360   0  
         33 :       1760   0.33  
         34 :        879   0.34  
Y differences  
          0 :    4998106   0  
         33 :       1262   0.33  
         34 :        631   0.34  
Z differences  
          0 :    4996046   0  
          1 :       3905   0.01  
          2 :         30   0.02  
          3 :          8   0.03  
          4 :          5   0.04  
          5 :          1   0.05  
          8 :          1   0.08  
          9 :          1   0.09  
         13 :          1   0.13  
         17 :          1   0.17  
  
    lasprecision64 -i IowaDNR-CloudPeakSoft-1.0-UTM15N.las  
original scale factors: 0.001 0.001 0.001  
loading first 5000000 of 5847380 points  
X differences  
          0 :    4800004   0  
         10 :     199991   0.01  
         20 :          4   0.02  
Y differences  
          0 :    4800002   0  
         10 :     199995   0.01  
         20 :          2   0.02  
Z differences  
          0 :    4930616   0  
          1 :      66129   0.001  
          2 :       1167   0.002  
          3 :        356   0.003  
  
    lasprecision64 -i LAS12_Sample_withRGB_QT_Modeler.las  
original scale factors: 0.001 0.001 0.001  
loading first 3837973 of 3837973 points  
WARNING: end-of-file after 3813696 of 3837973 points  
X differences  
          0 :    3835738   0  
          1 :         28   0.001  
        333 :       1500   0.333  
        334 :        706   0.334  
Y differences  
          0 :    3836244   0  
        166 :          2   0.166  
        333 :       1150   0.333  
        334 :        576   0.334  
Z differences  
          0 :    3806893   0  
          1 :      30238   0.001  
          2 :        389   0.002  
          3 :        155   0.003  
          4 :         75   0.004  
          5 :         60   0.005  
          6 :         26   0.006  
  
    lasprecision64 -i Lincoln.las  
original scale factors: 1.45584e-006 1.38473e-006 6.72668e-008  
loading first 5000000 of 9278073 points  
X differences  
        128 :       1615   0.000186347  
        129 :     512010   0.000187803  
        392 :     723661   0.000570689  
        393 :    1284377   0.000572145  
        521 :     900990   0.000758492  
        522 :    1577342   0.000759948  
        914 :          3   0.00133064  
        915 :          1   0.00133209  
Y differences  
        135 :    1022794   0.000186938  
        136 :    1678978   0.000188323  
        277 :     959022   0.000383569  
        278 :     213386   0.000384954  
        412 :     220537   0.000570507  
        413 :     901455   0.000571892  
        548 :       1623   0.00075883  
        549 :       1205   0.000760215  
Z differences  
          0 :     744253   0  
          1 :        381   6.72668e-008  
          2 :        642   1.34534e-007  
          3 :       2001   2.018e-007  
          4 :       2262   2.69067e-007  
          5 :        640   3.36334e-007  
          6 :        440   4.03601e-007  
          7 :      57413   4.70867e-007  
  
    lasprecision64 -i line_27007_dd.las  
original scale factors: 1e-007 1e-007 1e-007  
loading first 5000000 of 5380156 points  
X differences  
          0 :    4772386   0  
          1 :     225217   1e-007  
          2 :       1454   2e-007  
          3 :        434   3e-007  
          4 :        179   4e-007  
          5 :         91   5e-007  
          6 :         57   6e-007  
          7 :         35   7e-007  
Y differences  
          0 :    4908448   0  
          1 :      90572   1e-007  
          2 :        679   2e-007  
          3 :        155   3e-007  
          4 :         64   4e-007  
          5 :         26   5e-007  
          6 :         15   6e-007  
          7 :         11   7e-007  
          8 :          9   8e-007  
Z differences  
          0 :    4995922   0  
      90000 :        500   0.009  
     100000 :       3058   0.01  
     110000 :        502   0.011  
     190000 :          2   0.019  
     200000 :          6   0.02  
     210000 :          1   0.021  
     290000 :          1   0.029  
     300000 :          1   0.03  
  
    lasprecision64 -i "Palm Beach Pre Hurricane.las"  
original scale factors: 3.22228e-006 5.53148e-006 2.28007e-007  
loading first 2580410 of 2580410 points  
X differences  
        250 :     592850   0.00080557  
        251 :     837488   0.000808793  
        501 :      65509   0.00161436  
        502 :      13534   0.00161759  
       1425 :     489084   0.00459175  
       1426 :      35190   0.00459497  
       1675 :     141943   0.00539732  
       1676 :     266781   0.00540054  
       1926 :      98603   0.00620611  
       1927 :      30862   0.00620934  
       2176 :        454   0.00701169  
       2177 :       2087   0.00701491  
       4103 :        851   0.013221  
Y differences  
        145 :      21380   0.000802064  
        146 :     831975   0.000807596  
        976 :     575719   0.00539872  
        977 :      83759   0.00540425  
       1122 :     909969   0.00620632  
       1123 :     103297   0.00621185  
       1268 :         71   0.00701391  
       1269 :          5   0.00701945  
       2098 :       3334   0.011605  
       2099 :        982   0.0116106  
       2244 :       4505   0.0124126  
Z differences  
          0 :      88603   0  
          1 :       4279   2.28007e-007  
          2 :       5345   4.56014e-007  
          3 :       3967   6.84022e-007  
          4 :      12520   9.12029e-007  
          5 :       5730   1.14004e-006  
          6 :       4621   1.36804e-006  
          7 :       3917   1.59605e-006  
          8 :      29901   1.82406e-006  
          9 :      18615   2.05207e-006  
         10 :       4138   2.28007e-006  
  
  
  
lasprecision64 -h  
lasprecision64 -i in.las  
lasprecision64 -i in.las -number 1000000  
lasprecision64 -i in.las -all -gps -lines 50  
lasprecision64 -i in.las -no_x -no_y -no_z -rgb  
lasprecision64 -i in.las -diff_diff  
lasprecision64 -i in.las -o out.las -rescale 0.01 0.01 0.001 -reoffset 300000 2000000 0  
lasprecision64 -i in.las -o out.las -rescale 0.333333333 0.333333333 0.01


## lasprecision specific arguments

-all                  : analyze all points (otherwise: limit to 5 mil. points)  
-cores [n]            : process multiple inputs on [n] cores in parallel  
-diff_diff            : report also differences of differences  
-diff_diff_only       : report only differences of differences  
-gps                  : report also gps timestamp statistics  
-gps                  : validate gps data  
-lines [n]            : limit report to [n] lines (default=20)  
-no_x                 : do not report x statistics  
-no_y                 : do not report y statistics  
-no_z                 : do not report z statistics  
-number [n]           : limit statistics to first [n] points  
-o [n]                : use [n] as output file  
-obin                 : output as BIN (terrasolid binary)  
-olas                 : output as LAS file  
-olaz                 : output as LAZ (compressed LAS)  
-otxt                 : output as textfile  
-reoffset [x] [y] [z] : puts a new offset [x] [y] [z] into the header and translates the points accordingly  
-rescale [x] [y] [z]  : puts a new scale [x] [y] [z] into the header and rescales the points accordingly  
-rgb                  : report also rgb statistics  
-rgb                  : validate rgb data  
-week_to_adjusted [n] : converts time stamps from GPS week [n] to Adjusted Standard GPS  

### Basics
-fail    : fail if license expired or invalid  
-gui     : start with files loaded into GUI  
-h       : print help output  
-help    : print help output  
-v       : verbose output (print extra information)  
-verbose : verbose output (print extra information)  
-version : reports this tool's version number  

## Module arguments

### General
-buffered [n]      : define read or write buffer of size [n]{default=262144}  
-chunk_size [n]    : set chunk size [n] in number of bytes  
-comma_not_point   : use comma instead of point as decimal separator  
-neighbors [n]     : set neighbors filename or wildcard [n]  
-neighbors_lof [n] : set neighbors list of files [fnf]  
-stored            : use in memory reader  
-unique            : remove duplicate points  

### Color
-clamp_RGB_to_8bit                  : limit RGB values to 8 bit (otherwise: 16 bit)  
-copy_B_into_NIR                    : copy blue color value into NearInfraRed value  
-copy_B_into_intensity              : copy blue color value to intensity  
-copy_B_into_register [n]           : copy blue color value into register [n]  
-copy_G_into_NIR                    : copy green color value into NearInfraRed value  
-copy_G_into_intensity              : copy green color value to intensity  
-copy_G_into_register [n]           : copy green color value into register [n]  
-copy_NIR_into_intensity            : copy NIR into intensity  
-copy_NIR_into_register [n]         : copy NearInfraRed value into register [n]  
-copy_RGB_into_intensity            : copy weighted RGB value to intensity  
-copy_R_into_NIR                    : copy red color value into NearInfraRed value  
-copy_R_into_intensity              : copy red color value to intensity  
-copy_R_into_register [n]           : copy red color value into register [n]  
-copy_attribute_into_B [n]          : copy attribute [n] value into blue  
-copy_attribute_into_G [n]          : copy attribute [n] value into green  
-copy_attribute_into_NIR [n]        : copy attribute [n] value into NIR (NearInfraRed)  
-copy_attribute_into_R [n]          : copy attribute [n] value into red  
-copy_intensity_into_NIR            : copy intensity into NIR (NearInfraRed) value  
-copy_register_into_B [n]           : copy register [n] into blue color value  
-copy_register_into_G [n]           : copy register [n] into green color value  
-copy_register_into_I [n]           : copy register [n] into NearInfraRed value  
-copy_register_into_NIR [n]         : copy register [n] into NearInfraRed value  
-copy_register_into_R [n]           : copy register [n] into red color value  
-drop_RGB_green [min] [max]         : drop points with green color value between [min] and [max]  
-drop_RGB_red [min] [max]           : drop points with red color value between [min] and [max]  
-force_RGB                          : force the use of the RGB value even if the point format does not support RGB  
-keep_NDVI_from_CIR [min] [max]     : keep NDVI (Normalized Difference Vegetation Index) from CIR between [min] [max]  
-keep_NDVI_green_is_NIR [min] [max] : keep NDVI (Normalized Difference Vegetation Index) where green is NIR between [min] [max]  
-keep_NDVI_intensity_is_NIR [min] [max]: keep NDVI (Normalized Difference Vegetation Index) where intensity is NIR between [min] [max]  
-keep_RGB_blue [m] [n]              : keep points with RGB blue color values between [min] [max]  
-keep_RGB_green [min] [max]         : keep points with green color value between [min] and [max]  
-keep_RGB_greenness [m] [n]         : keep points with RGB greenness values between [min] [max]  
-keep_RGB_nir [m] [n]               : keep points with RGB NIR values between [min] [max]  
-keep_RGB_red [min] [max]           : keep points with red color value between [min] and [max]  
-map_attribute_into_RGB [a] [fnm]   : map attribute [a] by table in file [fnm] to RGB values  
-oscale_rgb [n]                     : scale output RGB by [n]  
-scale_NIR [n]                      : scale NearInfraRed value by factor [n]  
-scale_NIR_down                     : scale NearInfraRed value down by 256  
-scale_NIR_to_16bit                 : scale 8 bit NearInfraRed value to 16 bit  
-scale_NIR_to_8bit                  : scale 16 bit NearInfraRed value downto 8 bit  
-scale_NIR_up                       : scale NearInfraRed value up by 256  
-scale_RGB [r] [g] [b]              : scale RGB values by factors in [r][g][b]  
-scale_RGB_down                     : scale RGB color values down by 256  
-scale_RGB_to_16bit                 : scale 8 bit color values to 16 bit  
-scale_RGB_to_8bit                  : scale 16 bit color values downto 8 bit  
-scale_RGB_up                       : scale RGB values from 8 bit up to 16 bit (multiply with 256)  
-scale_rgb_down                     : divides all RGB values by 256 (to go from 16 bit to 8 bit numbers)  
-scale_rgb_up                       : multiplies all RGB values by 256 (to go from 8 bit to 16 bit numbers)  
-set_NIR [n]                        : set NearInfraRed value to [n]  
-set_RGB [r] [g] [b]                : set color to [r] [g] [b]  
-set_RGB_of_class [c] [r] [g] [b]   : set RGB values of class [c] to [r][g][b] (8 or 16 bit)  
-switch_G_B                         : switch green and blue value  
-switch_RGBI_into_CIR               : set R to NIR; G to R; B to G  
-switch_RGB_intensity_into_CIR      : set R to intensity; G to R; B to G  
-switch_R_B                         : switch red and blue color value  
-switch_R_G                         : switch red and green color value  

### Coordinates
-add_attribute_to_z [n]             : add value of attribute [n] to z value  
-add_scaled_attribute_to_z [m] [n]  : scale attribute [m] value by [n] and add to z value  
-auto_reoffset                      : puts a reasonable offset in the header and translates the points accordingly  
-bin_Z_into_point_source [n]        : set point source to z/[n]  
-clamp_raw_z [min] [max]            : limit raw z values to [min] and [max]  
-clamp_z [min] [max]                : limit z values to [min] and [max]  
-clamp_z_above [n]                  : limit z values to maximal [n]  
-clamp_z_below [n]                  : limit z values to minimal [n]  
-classify_z_above_as [m] [n]        : for z value above [m] set class to [n]  
-classify_z_below_as [m] [n]        : for z value below [m] set class to [n]  
-classify_z_between_as [m] [n] [o]  : for z value between [m] and [n] set class to [o]  
-copy_attribute_into_x [n]          : copy attribute [n] value into x  
-copy_attribute_into_y [n]          : copy attribute [n] value into y  
-copy_attribute_into_z [n]          : copy attribute [n] value into z  
-copy_intensity_into_z              : copy intensity to z value  
-copy_register_into_x [n]           : copy register [n] to x value  
-copy_register_into_y [n]           : copy register [n] to y value  
-copy_register_into_z [n]           : copy register [n] to z value  
-copy_user_data_into_z              : copy user data into z  
-copy_z_into_attribute [n]          : copy z value into attribute [n] value  
-drop_x [m] [n]                     : drop points with x value between [m] and [n]  
-drop_x_above [n]                   : drop points with x value above [n]  
-drop_x_below [n]                   : drop points with x value below [n]  
-drop_xy [x1] [y1] [x2] [y2]        : drop points within the [x1] [y1] [x2] [y2] rectangle  
-drop_xyz [x1] [y1] [z1] [x2] [y2] [z2]: drop points within the given cube dimensions  
-drop_y [m] [n]                     : drop points with y value between [m] and [n]  
-drop_y_above [n]                   : drop points with y value above [n]  
-drop_y_below [n]                   : drop points with y value below [n]  
-drop_z [m] [n]                     : drop points with z value between [m] and [n]  
-drop_z_above [n]                   : drop points with z value above [n]  
-drop_z_below [n]                   : drop points with z value below [n]  
-inside [x1] [y1] [x2] [y2]         : use only points within the [x1] [y1] [x2] [y2] rectangle  
-inside_circle [x] [y] [r]          : keep circle at pos [x] [y] with radius [r]  
-inside_rectangle [x1] [y1] [x2] [y2]: use only points within the [x1] [y1] [x2] [y2] rectangle  
-inside_tile [m] [n] [o]            : use only points inside tile at lower-left [x] [y] with size [s]  
-keep_circle [x] [y] [r]            : keep circle at pos [x] [y] with radius [r]  
-keep_profile [x1] [y1] [x2] [y2] [w]: keep profile with [x1] [y1] [x2] [y2] [w]  
-keep_tile [x] [y] [size]           : keep tile at lower-left [x] [y] with size [s]  
-keep_x [m] [n]                     : keep points with x value between [m] and [n]  
-keep_xy [x1] [y1] [x2] [y2]        : keep points within the [x1] [y1] [x2] [y2] rectangle  
-keep_xyz [x1] [y1] [z1] [x2] [y2] [z2]: keep points within the given cube dimensions  
-keep_y [m] [n]                     : keep points with y value between [m] and [n]  
-keep_z [m] [n]                     : keep points with z value between [m] and [n]  
-keep_z_above [n]                   : keep points with z value above [n]  
-keep_z_below [n]                   : keep points with z value below [n]  
-rescale_xy [x] [y]                 : rescale x y by [x] [y]  
-rescale_z [z]                      : rescale z by [z]  
-rotate_xy [a] [x] [y]              : rotate points by [a] degrees, center at [x] [y]  
-rotate_xz [a] [x] [z]              : rotate points by [a] degrees, center at [x] [z]  
-rotate_yz [a] [y] [z]              : rotate points by [a] degrees, center at [y] [z]  
-scale_x [n]                        : scale x value by [n]  
-scale_xyz [m] [n] [o]              : scale xyz values by [m] [n] [o]  
-scale_y [n]                        : scale y value by [n]  
-scale_z [n]                        : scale z value by [n]  
-switch_x_y                         : exchange x and y value  
-switch_x_z                         : exchange x and z value  
-switch_y_z                         : exchange z and x value  
-transform_affine [a],[b],[c],[d]   : transform input using affine transformation with [a],[b],[c],[d]  
-transform_helmert [m] [n] [o]      : do a helmert transformation with 3 or 7 comma separated parameters [n] ...  
-transform_matrix [r11,r12,r13] [r21,r22,r23] [r31,r32,r33] [tr1,tr2,tr3]: transform input using matrix [r11,r12,r13] [r21,r22,r23] [r31,r32,r33] [tr1,tr2,tr3]  
-translate_raw_x [n]                : translate raw x value by [n]  
-translate_raw_xy_at_random [x] [y] : translate raw xy values by random and max offset of [x] [y]  
-translate_raw_xyz [x] [y] [z]      : translate raw coordinates by [x] [y] [z]  
-translate_raw_y [n]                : translate raw y value by [n]  
-translate_raw_z [n]                : translate raw z value by [n]  
-translate_then_scale_x [m] [n]     : translate x value by [m] and scale by [n]  
-translate_then_scale_y [m] [n]     : translate y value by [m] and scale by [n]  
-translate_then_scale_z [m] [n]     : translate z value by [m] and scale by [n]  
-translate_x [n]                    : translate y value by [n]  
-translate_xyz [x] [y] [z]          : translate point coordinates by [x] [y] [z]  
-translate_y [n]                    : translate y value by [n]  
-translate_z [n]                    : translate z value by [n]  

### Simple thinning
-drop_every_nth [n]           : drop every [n]th point  
-keep_every_nth [n]           : keep every [n]th point  
-keep_random_fraction [m] [n] : keep points by random fraction [m]{0-1}, optional seed [n]  
-thin_points_with_time [n]    : thin points with time, [n] = timespacing  
-thin_pulses_with_time [n]    : thin pulses with time, [n] = timespacing  
-thin_with_grid [n]           : thin points by min grid size of [n]  
-thin_with_time [n]           : thin pulses with time, [n] = timespacing  

### Return number
-change_extended_number_of_returns_from_to [m] [n]: change extended number of returns from [m] to [n]  
-change_extended_return_number_from_to [m] [n]: change extended return number from [m] to [n]  
-change_number_of_returns_from_to [m] [n]: change number of returns from [m] to [n]  
-change_return_number_from_to [m] [n]: change return number from [m] to [n]  
-drop_double                        : drop double returns  
-drop_first                         : drop first return  
-drop_first_of_many                 : drop first of many returns  
-drop_last                          : drop last return  
-drop_last_of_many                  : drop last of many returns  
-drop_middle                        : drop middle returns  
-drop_number_of_returns [n]         : drop points with [n] number of returns  
-drop_quadruple                     : drop quadruple returns  
-drop_quintuple                     : drop quintuple returns  
-drop_return [m] [n]...             : drop points with return [m] [n]...  
-drop_return_mask [n]               : drop points with return mask [n]  
-drop_second_last                   : drop points with second last return  
-drop_single                        : drop points with single return  
-drop_triple                        : drop points with triple return  
-first_only                         : use first return only  
-keep_double                        : keep double returns  
-keep_first                         : keep first return  
-keep_first_of_many                 : keep first of many returns  
-keep_last                          : keep last return  
-keep_last_of_many                  : keep last of many returns  
-keep_middle                        : keep mittle returns  
-keep_number_of_returns [n]         : keep points with [n] number of returns  
-keep_quadruple                     : keep quadruple returns  
-keep_quintuple                     : keep quintuple returns  
-keep_return [m] [n]...             : keep points with return [m] [n]...  
-keep_return_mask [n]               : keep points with return mask [n]  
-keep_second_last                   : keep points with second last return  
-keep_single                        : keep points with single return  
-keep_triple                        : keep points with triple return  
-last_only                          : use last return only  
-repair_zero_returns                : sets return counts and number of returns that are zero to one  
-set_extended_number_of_returns [n] : set extended number of returns to [n]  
-set_extended_return_number [n]     : set extended return number to [n]  
-set_number_of_returns [n]          : set number of returns to [n]  
-set_return_number [n]              : set return number to [n]  

### Scanline
-drop_scan_direction [n]       : drop points with scan direction [n]  
-faf                           : input files are flightlines. do ***NOT*** use this for tiled input  
-faf_index [n]                 : set files are flightlines index [n]  
-files_are_flightlines         : input files are flightlines. do ***NOT*** use this for tiled input  
-keep_edge_of_flight_line      : keep points with "Edge of Flight Line" flag set  
-keep_scan_direction_change    : keep points with changed scan direction flag  
-set_edge_of_flight_line [0/1] : set "Edge of Flight Line" flag to [0/1]  
-set_scan_direction_flag [0/1] : set scan direction flag to [0/1]  

### Scanner channel
-copy_scanner_channel_into_point_source: copy scanner channel into point_source  
-copy_scanner_channel_into_user_data: copy scanner channel into user data  
-copy_user_data_into_scanner_channel: copy user data into scanner channel  
-drop_scanner_channel [n]           : drop points with scanner channel [n]  
-keep_scanner_channel [n]           : keep points with scanner channel [n]  
-merge_scanner_channel_into_point_source: merge scanner channel to point source  
-set_extended_scanner_channel [n]   : set extended scanner channel to [n]  
-set_scanner_channel [n]            : set scanner channel to [n]  
-split_scanner_channel_from_point_source: split scanner channel from point source and save as extended scanner channel  

### Source ID
-apply_file_source_ID               : copy file source ID to target  
-bin_Z_into_point_source [n]        : set point source to z/[n]  
-bin_abs_scan_angle_into_point_source [n]: set point source to scan_angle/[n]  
-bin_gps_time_into_point_source [n] : set point source to gps/[n]  
-change_point_source_from_to [m] [n]: change point source from [m] to [n]  
-copy_attribute_into_point_source [n]: copy attribute [n] value into point source  
-copy_classification_into_point_source: copy classification to point source  
-copy_point_source_into_register [n]: copy point source into register [n]  
-copy_register_into_point_source [n]: copy register [n] to point source  
-copy_scanner_channel_into_point_source: copy scanner channel into point_source  
-copy_user_data_into_point_source   : copy user data into point source  
-drop_point_source [n]              : drop points with point source [n]  
-drop_point_source_above [n]        : drop points with with point source above [n]  
-drop_point_source_below [n]        : drop points with with point source below [n]  
-drop_point_source_between [m] [n]  : drop points with with point source between [n] and [m]  
-keep_point_source [n]              : keep points with point source [n]  
-keep_point_source_between [m] [n]  : keep points with with point source between [n] and [m]  
-map_point_source [fnm]             : set the point source by map in file [fnm]  
-merge_scanner_channel_into_point_source: merge scanner channel to point source  
-set_point_source [n]               : set point source to [n]  
-split_scanner_channel_from_point_source: split scanner channel from point source and save as extended scanner channel  

### User data
-add_scaled_attribute_to_user_data [m] [n]: scale attribute [m] value by [n] and add to user data  
-change_user_data_from_to [m] [n]   : change user data from [m] to [n]  
-copy_attribute_into_user_data [n]  : copy attribute [n] value into user data field  
-copy_classification_into_user_data : copy classification to user data  
-copy_register_into_user_data [n]   : copy register [n] to user data  
-copy_scanner_channel_into_user_data: copy scanner channel into user data  
-copy_user_data_into_attribute [n]  : copy user data into attribute [n] value  
-copy_user_data_into_classification : copy user data into classification  
-copy_user_data_into_point_source   : copy user data into point source  
-copy_user_data_into_register [n]   : copy user data to register [n]  
-copy_user_data_into_scanner_channel: copy user data into scanner channel  
-copy_user_data_into_z              : copy user data into z  
-drop_user_data [n]                 : drop points with user data value of [n]  
-drop_user_data_above [n]           : drop points with user data value above [n]  
-drop_user_data_below [n]           : drop points with user data value below [n]  
-drop_user_data_between [m] [n]     : drop points with user data between [m] and [n]  
-keep_user_data [n]                 : keep points with user data value of [n]  
-keep_user_data_above [n]           : keep points with user data value above [n]  
-keep_user_data_below [n]           : keep points with user data value below [n]  
-keep_user_data_between [m] [n]     : keep points with user data between [m] and [n]  
-map_user_data [fnm]                : set the user data by map in file [fnm]  
-scale_user_data [n]                : scale user data by [n]  
-set_user_data [n]                  : sets all user_data fields to [n]  

### Classification
-change_class_from_to [m] [n]       : change classification from [m] to [n]  
-change_classification_from_to [m] [n]: change classification from [m] to [n]  
-change_extended_class_from_to [m] [n]: change extended class from [m] to [n]  
-change_extended_classification_from_to [m] [n]: change extended class from [m] to [n]  
-classify_attribute_above_as [m] [n] [o]: for attribute [m] with value above [n] set class to [o]  
-classify_attribute_below_as [m] [n] [o]: for attribute [m] with value below [n] set class to [o]  
-classify_attribute_between_as [m] [n] [o] [p]: for attribute [m] with value between [n] and [o] set class to [p]  
-classify_intensity_above_as [m] [n]: for intensity value above [m] set class to [n]  
-classify_intensity_below_as [m] [n]: for intensity value below [m] set class to [n]  
-classify_intensity_between_as [m] [n] [o]: for intensity value between [m] and [n] set class to [o]  
-classify_z_above_as [m] [n]        : for z value above [m] set class to [n]  
-classify_z_below_as [m] [n]        : for z value below [m] set class to [n]  
-classify_z_between_as [m] [n] [o]  : for z value between [m] and [n] set class to [o]  
-copy_classification_into_point_source: copy classification to point source  
-copy_classification_into_user_data : copy classification to user data  
-copy_intensity_into_classification : copy intensity to classification  
-copy_user_data_into_classification : copy user data into classification  
-drop_class [m] [n] [o]...          : drop points with class in [m][n][o]...  
-drop_classification [m] [n] [o]... : drop points with class in [m][n][o]...  
-drop_classification_mask [n]       : drop points with classification mask matches [n]  
-drop_extended_class [m] [n]...     : drop extended class [m] [n]...  
-drop_extended_classification [n]   : drop points with extended classification [n]  
-drop_extended_classification_mask [a] [b] [c] [d] [e] [f] [g] [h]: drop points with extended classification mask matches [a] [b] [c] [d] [e] [f] [g] [h]  
-keep_class [m] [n] [o]...          : keep points with class in [m][n][o]...  
-keep_classification [m] [n] [o]... : keep points with class in [m][n][o]...  
-keep_classification_mask [n]       : keep points with classification mask matches [n]  
-keep_extended_class [m] [n]...     : keep extended class [m] [n]...  
-keep_extended_classification [n]   : keep points with extended class [n]  
-move_ancient_to_extended_classification: move old data to extended classification  
-set_RGB_of_class [c] [r] [g] [b]   : set RGB values of class [c] to [r][g][b] (8 or 16 bit)  
-set_classification [n]             : set classification to [n]  
-set_extended_classification [n]    : set extended classification to [n]  

### Extra byte
-add_attribute_to_z [n]             : add value of attribute [n] to z value  
-add_scaled_attribute_to_user_data [m] [n]: scale attribute [m] value by [n] and add to user data  
-add_scaled_attribute_to_z [m] [n]  : scale attribute [m] value by [n] and add to z value  
-classify_attribute_above_as [m] [n] [o]: for attribute [m] with value above [n] set class to [o]  
-classify_attribute_below_as [m] [n] [o]: for attribute [m] with value below [n] set class to [o]  
-classify_attribute_between_as [m] [n] [o] [p]: for attribute [m] with value between [n] and [o] set class to [p]  
-copy_attribute_into_B [n]          : copy attribute [n] value into blue  
-copy_attribute_into_G [n]          : copy attribute [n] value into green  
-copy_attribute_into_I [n]          : copy attribute [n] value into intensity  
-copy_attribute_into_NIR [n]        : copy attribute [n] value into NIR (NearInfraRed)  
-copy_attribute_into_R [n]          : copy attribute [n] value into red  
-copy_attribute_into_intensity [n]  : copy attribute [n] value into intensity  
-copy_attribute_into_point_source [n]: copy attribute [n] value into point source  
-copy_attribute_into_register [m] [n]: copy attribute [m] value into register [m]  
-copy_attribute_into_user_data [n]  : copy attribute [n] value into user data field  
-copy_attribute_into_x [n]          : copy attribute [n] value into x  
-copy_attribute_into_y [n]          : copy attribute [n] value into y  
-copy_attribute_into_z [n]          : copy attribute [n] value into z  
-copy_intensity_into_attribute [n]  : copy intensity to attribute [n] value  
-copy_register_into_attribute [m] [n]: copy register [m] to attribute [n] value  
-copy_user_data_into_attribute [n]  : copy user data into attribute [n] value  
-copy_z_into_attribute [n]          : copy z value into attribute [n] value  
-drop_attribute_above [m] [n]       : drop points with attribute [m] value > [n]  
-drop_attribute_below [m] [n]       : drop points with attribute [m] value < [n]  
-drop_attribute_between [m] [n] [o] : drop points with attribute [m] in range [n]...[o]  
-iadd_attribute [m] [n] [o] [p] [q] [r] [s] [t]: adds a new "extra_byte" attribute of data_type [m] name [n] description [o]; optional: scale[p] offset [q] pre_scale [r] pre_offset [s] no_data_value [t]  
-iadd_extra [m] [n] [o] [p] [q] [r] [s] [t]: adds a new "extra_byte" attribute of data_type [m] name [n] description [o]; optional: scale[p] offset [q] pre_scale [r] pre_offset [s] no_data_value [t]  
-keep_attribute_above [m] [n]       : keep points with attribute [m] value > [n]  
-keep_attribute_below [m] [n]       : keep points with attribute [m] value < [n]  
-keep_attribute_between [m] [n] [o] : keep points with attribute [m] in range [n]...[o]  
-load_attribute_from_text [m] [fnt] : load attribute [m] from file [fnt]  
-map_attribute_into_RGB [a] [fnm]   : map attribute [a] by table in file [fnm] to RGB values  
-scale_attribute [m] [n]            : scale attribute [m] by [n]  
-set_attribute [m] [n]              : set attribute [m] with value [n]  
-translate_attribute [m] [n]        : translate attribute [n] by [n]  

### Flags
-drop_keypoint                   : drop points flaged as keypoint  
-drop_overlap                    : drop points flaged as overlap  
-drop_scan_direction [n]         : drop points with scan direction [n]  
-drop_synthetic                  : drop points flaged as synthetic  
-drop_withheld                   : drop points flaged as withheld  
-keep_edge_of_flight_line        : keep points with "Edge of Flight Line" flag set  
-keep_keypoint                   : keep points flaged as keypoint  
-keep_overlap                    : keep points flaged as overlap  
-keep_scan_direction_change      : keep points with changed scan direction flag  
-keep_synthetic                  : keep points flaged as synthetic  
-keep_withheld                   : keep points flaged as withheld  
-set_edge_of_flight_line [0/1]   : set "Edge of Flight Line" flag to [0/1]  
-set_extended_overlap_flag [0/1] : set extended overlap flag to [0/1]  
-set_keypoint_flag [0/1]         : set keypoint flag to [0/1]  
-set_overlap_flag [0/1]          : set overlap flag to [0/1]  
-set_scan_direction_flag [0/1]   : set scan direction flag to [0/1]  
-set_synthetic_flag [0/1]        : set synthetic flag to [0/1]  
-set_withheld_flag [0/1]         : set withheld flag to [0/1]  

### GPS time
-adjusted_to_week                   : converts time stamps from Adjusted Standard GPS to GPS week  
-bin_gps_time_into_intensity [n]    : set intensity time to gps/[n]  
-bin_gps_time_into_point_source [n] : set point source to gps/[n]  
-drop_gps_time_above [n]            : drop points with GPS time above [n]  
-drop_gps_time_below [n]            : drop points with GPS time below [n]  
-drop_gps_time_between [m] [n]      : drop points with GPS time between [m] and [n]  
-drop_gpstime_above [n]             : drop points with GPS time above [n]  
-drop_gpstime_below [n]             : drop points with GPS time below [n]  
-drop_gpstime_between [m] [n]       : drop points with GPS time between [m] and [n]  
-keep_gps_time [m] [n]              : keep points with GPS time between [m] and [n]  
-keep_gps_time_above [n]            : keep points with GPS time above [n]  
-keep_gps_time_below [n]            : keep points with GPS time below [n]  
-keep_gps_time_between [m] [n]      : keep points with GPS time between [m] and [n]  
-keep_gpstime [m] [n]               : keep points with GPS time between [m] and [n]  
-keep_gpstime_above [n]             : keep points with GPS time above [n]  
-keep_gpstime_below [n]             : keep points with GPS time below [n]  
-keep_gpstime_between [m] [n]       : keep points with GPS time between [m] and [n]  
-set_gps_time [n]                   : set gps time to [n]  
-translate_gps_time [n]             : translate GPS time by [n]  

### Intensity
-bin_gps_time_into_intensity [n]    : set intensity time to gps/[n]  
-clamp_intensity [min] [max]        : limit intensity values to [min] and [max]  
-clamp_intensity_above [max]        : limit intensity values to maximal [max]  
-clamp_intensity_below [max]        : limit intensity values to minimal [min]  
-classify_intensity_above_as [m] [n]: for intensity value above [m] set class to [n]  
-classify_intensity_below_as [m] [n]: for intensity value below [m] set class to [n]  
-classify_intensity_between_as [m] [n] [o]: for intensity value between [m] and [n] set class to [o]  
-copy_B_into_intensity              : copy blue color value to intensity  
-copy_G_into_intensity              : copy green color value to intensity  
-copy_NIR_into_intensity            : copy NIR into intensity  
-copy_RGB_into_intensity            : copy weighted RGB value to intensity  
-copy_R_into_intensity              : copy red color value to intensity  
-copy_attribute_into_I [n]          : copy attribute [n] value into intensity  
-copy_attribute_into_intensity [n]  : copy attribute [n] value into intensity  
-copy_intensity_into_NIR            : copy intensity into NIR (NearInfraRed) value  
-copy_intensity_into_attribute [n]  : copy intensity to attribute [n] value  
-copy_intensity_into_classification : copy intensity to classification  
-copy_intensity_into_register [n]   : copy color intensitiy value into register [n]  
-copy_intensity_into_z              : copy intensity to z value  
-copy_register_into_intensity [n]   : copy register [n] into point intensitiy value  
-drop_intensity_above [n]           : drop points with intensity value above [n]  
-drop_intensity_below [n]           : drop points with intensity value below [n]  
-drop_intensity_between [m] [n]     : drop points with intensity value between [m] and [n]  
-iscale_intensity [n]               : scale intensity value by [n]  
-itranslate_intensity [n]           : translate input intensity by [n]  
-keep_NDVI_intensity_is_NIR [min] [max]: keep NDVI (Normalized Difference Vegetation Index) where intensity is NIR between [min] [max]  
-keep_intensity [m] [n]             : keep points with intensity between [m] and [n]  
-keep_intensity_above [n]           : keep points with intensity value above [n]  
-keep_intensity_below [n]           : keep points with intensity value below [n]  
-map_intensity [fnm]                : set the intensity by map in file [fnm]  
-scale_intensity [n]                : multiply intensity by [n]  
-set_intensity [n]                  : set intensity to [n]  
-switch_RGB_intensity_into_CIR      : set R to intensity; G to R; B to G  
-translate_intensity [n]            : translate intensity by [n]  
-translate_then_scale_intensity [m] [n]: translate intensity by [m] and scale by [n]  

### Raw point values
-clamp_raw_z [min] [max]            : limit raw z values to [min] and [max]  
-translate_raw_x [n]                : translate raw x value by [n]  
-translate_raw_xy_at_random [x] [y] : translate raw xy values by random and max offset of [x] [y]  
-translate_raw_xyz [x] [y] [z]      : translate raw coordinates by [x] [y] [z]  
-translate_raw_y [n]                : translate raw y value by [n]  
-translate_raw_z [n]                : translate raw z value by [n]  

### Registers
-add_registers [m] [n] [o]          : add register [m] and [n] and store result in register [o]  
-copy_B_into_register [n]           : copy blue color value into register [n]  
-copy_G_into_register [n]           : copy green color value into register [n]  
-copy_NIR_into_register [n]         : copy NearInfraRed value into register [n]  
-copy_R_into_register [n]           : copy red color value into register [n]  
-copy_attribute_into_register [m] [n]: copy attribute [m] value into register [m]  
-copy_intensity_into_register [n]   : copy color intensitiy value into register [n]  
-copy_point_source_into_register [n]: copy point source into register [n]  
-copy_register_into_B [n]           : copy register [n] into blue color value  
-copy_register_into_G [n]           : copy register [n] into green color value  
-copy_register_into_I [n]           : copy register [n] into NearInfraRed value  
-copy_register_into_NIR [n]         : copy register [n] into NearInfraRed value  
-copy_register_into_R [n]           : copy register [n] into red color value  
-copy_register_into_attribute [m] [n]: copy register [m] to attribute [n] value  
-copy_register_into_intensity [n]   : copy register [n] into point intensitiy value  
-copy_register_into_point_source [n]: copy register [n] to point source  
-copy_register_into_user_data [n]   : copy register [n] to user data  
-copy_register_into_x [n]           : copy register [n] to x value  
-copy_register_into_y [n]           : copy register [n] to y value  
-copy_register_into_z [n]           : copy register [n] to z value  
-copy_user_data_into_register [n]   : copy user data to register [n]  
-divide_registers [m] [n] [o]       : divide register [m] by register [n] and store result in register [o]  
-multiply_registers [m] [n] [o]     : Multiply register [m] with [n] and store result in register [o]  
-scale_register [m] [n]             : scale register index [m] with factor [n]  
-set_register [m] [n]               : set register [m] with value [n]  
-subtract_registers [m] [n] [o]     : subtract register [m] by register [n] and store result in register [o]  
-translate_register [m] [n]         : translate register index [m] value by [n]  

### Scan angle
-bin_abs_scan_angle_into_point_source [n]: set point source to scan_angle/[n]  
-drop_abs_scan_angle_above [max]    : drop points with absolute scan angle above [max]  
-drop_abs_scan_angle_below [min]    : drop points with absolute scan angle below [min]  
-drop_scan_angle_above [n]          : drop points with scan angle above [n]  
-drop_scan_angle_below [n]          : drop points with scan angle below [n]  
-drop_scan_angle_between [m] [n]    : drop points with scan angle between [m] and [n]  
-iscale_scan_angle [n]              : scale scan angle by [n]  
-itranslate_scan_angle [n]          : translate input scan angle by [n]  
-keep_scan_angle [m] [n]            : keep points with scan angle between [m] and [n]  
-keep_scan_angle_between [m] [n]    : keep points with scan angle between [m] and [n]  
-scale_scan_angle [n]               : scale scan angle by [n]  
-set_scan_angle [n]                 : set scan angle to [n]  
-translate_scan_angle [n]           : translate scan angle by [n]  
-translate_then_scale_scan_angle [m] [n]: translate scan angle by [m] and scale by [n]  

### Tiles
-keep_tile [x] [y] [size] : keep tile at lower-left [x] [y] with size [s]  

### Waveform packet
-drop_wavepacket [n]     : drop points with wavepacket value of [n]  
-flip_waveform_direction : flip the waveform direction in the waveform VLR  
-keep_wavepacket [n]     : keep points with wavepacket value of [n]  

### CRS
-aeac [m] [n] [meter/survey_feet/feet] [o] [p] [q] [r]: Albers Equal Area Conic Projection: False Easting [m] False Northing[n] [meter/survey_feet/feet] Central Meridian [o] Standard Parallel 1 [p] Standard Parallel 2 [q] Latitude of origin [r]  
-ecef                               : input is geocentric (Earth-centered Earth-fixed)  
-elevation_feet                     : use feet for elevation  
-elevation_meter                    : use meter for elevation  
-elevation_survey_feet              : set vertical units from meters to US survey feet  
-elevation_surveyfeet               : use survey feet for elevation  
-ellipsoid [n]                      : use the WGS-84 ellipsoid [n]{do -ellipsoid -1 for a list of ellipsoids}  
-epsg [n]                           : set datum to EPSG [n]  
-etrs89                             : use datum ETRS89  
-feet                               : use feet  
-gda2020                            : use datum GDA2020  
-gda94                              : use datum GDA94  
-grs80                              : use datum GRS1980  
-latlong                            : geometric coordinates in latitude/longitude order  
-lcc 609601.22 0.0 meter 33.75 -79 34.33333 36.16666: specifies a lambertian conic confomal projection  
-longlat                            : geometric coordinates in longitude/latitude order  
-meter                              : use meter  
-nad27                              : use the NAD27 ellipsoid  
-nad83                              : use the NAD83 ellipsoid  
-nad83_2011                         : use datum NAD83_2011  
-nad83_csrs                         : use datum NAD83_CSRS  
-nad83_harn                         : use datum NAD83_HARN  
-nad83_pa11                         : set horizontal datum to NAD83 PA11  
-osgb1936                           : use datum OSGB 1936  
-sp27 SC_N                          : use the NAD27 South Carolina North state plane  
-sp83 CO_S                          : use the NAD83 Colorado South state plane for georeferencing  
-survey_feet                        : use survey feet  
-surveyfeet                         : use survey feet as unit of measurement  
-target_aeac [m] [n] [meter/survey_feet/feet] [o] [p] [q] [r]: Albers Equal Area Conic Projection for target: False Easting [m] False Northing[n] [meter/survey_feet/feet] Central Meridian [o] Standard Parallel 1 [p] Standard Parallel 2 [q] Latitude of origin [r]  
-target_ecef                        : output is geocentric (Earth-centered Earth-fixed)  
-target_elevation_feet              : output uses feet for elevation  
-target_elevation_meter             : output uses meter for elevation  
-target_elevation_precision [n]     : output uses [n] (meter/feet) resolution for z  
-target_elevation_survey_feet       : use elevation survey feet as target unit  
-target_elevation_surveyfeet        : output uses survey feet for elevation  
-target_epsg [n]                    : output is EPSG code [n] (e.g. 2193=NZGD2000)  
-target_feet                        : output uses feet  
-target_latlong                     : output is geometric coordinates in latitude/longitude  
-target_lcc 609601.22 0.0 meter 33.75 -79 34.33333 36.16666: specifies a lambertian conic confomal projection at target  
-target_longlat                     : output is geometric coordinates in longitude/latitude  
-target_meter                       : output uses meter  
-target_precision [n]               : output uses [n] (meter/feet) resolution for x and y  
-target_sp27 SC_N                   : output is state plane NAD27 South Carolina North  
-target_sp83 CO_S                   : output is state plane NAD83 Colorado South  
-target_survey_feet                 : output uses survey feet  
-target_surveyfeet                  : use survey feet as target unit  
-target_tm                          : use transverse mercator projection for target  
-target_utm 12T                     : output is UTM zone 12T  
-tm 609601.22 0.0 meter 33.75 -79 0.99996: specifies a transverse mercator projection  
-transverse_mercator                : use transverse mercator projection  
-utm 12T                            : use UTM zone 12T  
-vertical_cgvd2013                  : set vertical datum to CGVD2013  
-vertical_cgvd28                    : set vertical datum to CGVD28  
-vertical_dhhn2016                  : set vertical datum to DHHN2016  
-vertical_dhhn92                    : set vertical datum to DHHN92  
-vertical_dvr90                     : set vertical datum to DVR90  
-vertical_epsg [n]                  : set vertical datum to EPSG [n]  
-vertical_evrf2007                  : set vertical datum to EVRF2007  
-vertical_navd29                    : set vertical datum to NAVD29  
-vertical_navd88                    : set vertical datum to NAVD88  
-vertical_ngvd29                    : set vertical datum to NGVD29  
-vertical_nn2000                    : set vertical datum to NN2000  
-vertical_nn54                      : set vertical datum to NN54  
-vertical_nzvd2016                  : set vertical datum to NZVD2016  
-vertical_wgs84                     : set vertical datum to WGS84  
-wgs72                              : use the WGS-72 ellipsoid  
-wgs84                              : use the WGS-84 ellipsoid  

### Logical
-filter_and         : boolean AND combination of last 2 filters  
-filter_or          : boolean OR combination of last 2 filters  
-filtered_transform : do the transformation only on points of the current filter  

### Input
-i [fnp]        : input file or input file mask [fnp] (e.g. *.laz;fo?.la?;esri.shp,...)  
-io_ibuffer [n] : use read-input-buffer of size [n] bytes  
-iparse [xyz]   : define fields [xyz] for text input parser  
-ipts           : input as PTS (plain text lidar source), store header in VLR  
-iptx           : input as PTX (plain text extended lidar data), store header in VLR  
-iptx_transform : use PTX file header to transform point data  
-iskip [n]      : skip [n] lines at the beginning of the text input  
-itxt           : expect input as text file  
-lof [fnf]      : use input out of a list of files [fnf]  
-merged         : merge input files  
-stdin          : pipe from stdin  

### Output
-compatible      : write LAS/LAZ output in compatibility mode  
-do_not_populate : do not populate header on output  
-io_obuffer [n]  : use write-out-buffer of size [n] bytes  
-native          : write LAS/LAZ output in native/actual mode  
-nil             : pipe output to NULL (suppress output)  
-ocut [n]        : cut the last [n] characters from name  
-odir [n]        : set output directory to [n]  
-odix [n]        : set output file name suffix to [n]  
-oforce          : force output creation also on errors or warnings  
-oparse [xyz]    : parse on-the-fly to ASCII using fields [xyz]  
-opts            : output as PTS (plain text lidar data)  
-optx            : output as PTX (plain text with header)  
-oqi             : output in QFIT format (.qi)(ATM project, NASA)  
-oscale_rgb [n]  : scale output RGB by [n]  
-osep [sep]      : set text output separator as [sep](see table below)  
-owrl            : output as VRLM (Virtual Reality Modeling Language) text  
-pipe_on         : write output to command pipe, see also -std_in  
-populate        : populate header on output  
-stdout          : pipe to stdout  
-target_ecef     : output is geocentric (Earth-centered Earth-fixed)  
-temp_files [n]  : set base file name [n] for temp files (example: E:\tmp)  

### parse
The '-parse [xyz]' flag specifies how to interpret
each line of the ASCII file. For example, 'tsxyzssa'
means that the first number is the gpstime, the next
number should be skipped, the next three numbers are
the x, y, and z coordinate, the next two should be
skipped, and the next number is the scan angle.

The other supported entries are:  
  x : [x] coordinate  
  y : [y] coordinate  
  z : [z] coordinate  
  t : gps [t]ime  
  R : RGB [R]ed channel  
  G : RGB [G]reen channel  
  B : RGB [B]lue channel  
  I : N[I]R channel of LAS 1.4 point type 8  
  s : [s]kip a string or a number that we don't care about  
  i : [i]ntensity  
  a : scan [a]ngle  
  n : [n]umber of returns of that given pulse  
  r : number of [r]eturn  
  h : with[h]eld flag  
  k : [k]eypoint flag  
  g : synthetic fla[g]  
  o : [o]verlap flag of LAS 1.4 point types 6, 7, 8  
  l : scanner channe[l] of LAS 1.4 point types 6, 7, 8  
  E : terrasolid [E]hco Encoding  
  c : [c]lassification  
  u : [u]ser data  
  p : [p]oint source ID  
  e : [e]dge of flight line flag  
  d : [d]irection of scan flag  
  0-9 : additional attributes described as extra bytes (0 through 9)  
  (13) : additional attributes described as extra bytes (10 and up)  
  H : a hexadecimal string encoding the RGB color  
  J : a hexadecimal string encoding the intensity  

### output separator
The '-osep [sep]' argument specifies the output format of a text(xyz or csv) output.
Supported [sep] values:

  comma
  tab
  dot
  colon
  semicolon
  hyphen
  space

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

