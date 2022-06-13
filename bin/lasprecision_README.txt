****************************************************************
this file is deprecated - see *.md version of this file
****************************************************************

  lasprecision:

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

  For updates check the website or join the LAStools mailing list.

  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @lastools

****************************************************************

example usage:

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

>> lasprecision -i MARS_Sample_Filtered_LiDAR.las -all
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

>>las2txt -parse XYZ -i "MARS_Sample_Filtered_LiDAR.las" -otxt | more
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

>> lasprecision -i MARS_Sample_Filtered_LiDAR.las -rescale 0.01 0.01 0.01 -o MARS_Sample_Filtered_LiDAR_rescaled.las
new scale factors: 0.01 0.01 0.01

So lets look at the rescaled file and we see that now we get the type of
histogram one would expect.

>> lasprecision -i MARS_Sample_Filtered_LiDAR_rescaled.las -all
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

>>lasdiff -i MARS_Sample_Filtered_LiDAR.las -i MARS_Sample_Filtered_LiDAR_rescaled.las
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

****************************************************************

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

>> lasinfo -i "Mount St Helens Oct 4 2004.las"
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

>>lasprecision -i "Mount St Helens Oct 4 2004.las" -o "Mount St Helens Oct 4 2004 rescaled.las" -rescale 0.01 0.01 0.001 -reoffset 1048576 262144 4096
new scale factors: 0.01 0.01 0.001

After rescaling the file we get a maximal floating point difference
of 0.005, 0.005, and 0.0005 for x, y, and z respectively.

>>lasdiff -i "Mount St Helens Oct 4 2004.las" -i "Mount St Helens Oct 4 2004 rescaled.las"
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

>>lasprecision -i "Grass Lake Small.las"
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

>>lasprecision -i IowaDNR-CloudPeakSoft-1.0-UTM15N.las
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

>>lasprecision -i LAS12_Sample_withRGB_QT_Modeler.las
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

>>lasprecision -i Lincoln.las
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

>>lasprecision -i line_27007_dd.las
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

>>lasprecision -i "Palm Beach Pre Hurricane.las"
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

for more info:

D:\lastools\bin>lasprecision.exe -h
Filter points based on their coordinates.
  -keep_tile 631000 4834000 1000 (ll_x ll_y size)
  -keep_circle 630250.00 4834750.00 100 (x y radius)
  -keep_xy 630000 4834000 631000 4836000 (min_x min_y max_x max_y)
  -drop_xy 630000 4834000 631000 4836000 (min_x min_y max_x max_y)
  -keep_x 631500.50 631501.00 (min_x max_x)
  -drop_x 631500.50 631501.00 (min_x max_x)
  -drop_x_below 630000.50 (min_x)
  -drop_x_above 630500.50 (max_x)
  -keep_y 4834500.25 4834550.25 (min_y max_y)
  -drop_y 4834500.25 4834550.25 (min_y max_y)
  -drop_y_below 4834500.25 (min_y)
  -drop_y_above 4836000.75 (max_y)
  -keep_z 11.125 130.725 (min_z max_z)
  -drop_z 11.125 130.725 (min_z max_z)
  -drop_z_below 11.125 (min_z)
  -drop_z_above 130.725 (max_z)
  -keep_xyz 620000 4830000 100 621000 4831000 200 (min_x min_y min_z max_x max_y max_z)
  -drop_xyz 620000 4830000 100 621000 4831000 200 (min_x min_y min_z max_x max_y max_z)
Filter points based on their return number.
  -first_only -keep_first -drop_first
  -last_only -keep_last -drop_last
  -keep_middle -drop_middle
  -keep_return 1 2 3
  -drop_return 3 4
  -keep_single -drop_single
  -keep_double -drop_double
  -keep_triple -drop_triple
  -keep_quadruple -drop_quadruple
  -keep_quintuple -drop_quintuple
Filter points based on the scanline flags.
  -drop_scan_direction 0
  -scan_direction_change_only
  -edge_of_flight_line_only
Filter points based on their intensity.
  -keep_intensity 20 380
  -drop_intensity_below 20
  -drop_intensity_above 380
  -drop_intensity_between 4000 5000
Filter points based on their classification.
  -keep_class 1 3 7
  -drop_class 4 2
  -drop_synthetic -keep_synthetic
  -drop_keypoint -keep_keypoint
  -drop_withheld -keep_withheld
Filter points based on their user data.
  -keep_user_data 1
  -drop_user_data 255
  -keep_user_data_between 10 20
  -drop_user_data_below 1
  -drop_user_data_above 100
  -drop_user_data_between 10 40
Filter points based on their point source ID.
  -keep_point_source 3
  -keep_point_source_between 2 6
  -drop_point_source 27
  -drop_point_source_below 6
  -drop_point_source_above 15
  -drop_point_source_between 17 21
Filter points based on their scan angle.
  -keep_scan_angle -15 15
  -drop_abs_scan_angle_above 15
  -drop_scan_angle_below -15
  -drop_scan_angle_above 15
  -drop_scan_angle_between -25 -23
Filter points based on their gps time.
  -keep_gps_time 11.125 130.725
  -drop_gps_time_below 11.125
  -drop_gps_time_above 130.725
  -drop_gps_time_between 22.0 48.0
Filter points based on their wavepacket.
  -keep_wavepacket 0
  -drop_wavepacket 3
Filter points with simple thinning.
  -keep_every_nth 2
  -keep_random_fraction 0.1
  -thin_with_grid 1.0
Transform coordinates.
  -translate_x -2.5
  -scale_z 0.3048
  -rotate_xy 15.0 620000 4100000 (angle + origin)
  -translate_xyz 0.5 0.5 0
  -translate_then_scale_y -0.5 1.001
  -clamp_z_below 70.5
  -clamp_z 70.5 72.5
Transform raw xyz integers.
  -translate_raw_z 20
  -translate_raw_xyz 1 1 0
  -clamp_raw_z 500 800
Transform intensity.
  -scale_intensity 2.5
  -translate_intensity 50
  -translate_then_scale_intensity 0.5 3.1
  -clamp_intensity 0 255
  -clamp_intensity_above 255
Transform scan_angle.
  -scale_scan_angle 1.944445
  -translate_scan_angle -5
  -translate_then_scale_scan_angle -0.5 2.1
Change the return number or return count of points.
  -repair_zero_returns
  -set_return_number 1
  -change_return_number_from_to 2 1
  -set_number_of_returns 2
  -change_number_of_returns_from_to 0 2
Modify the classification.
  -set_classification 2
  -change_classification_from_to 2 4
  -classify_z_below_as -5.0 7
  -classify_z_above_as 70.0 7
  -classify_z_between_as 2.0 5.0 4
  -classify_intensity_above_as 200 9
  -classify_intensity_below_as 30 11
Modify the user data.
  -set_user_data 0
  -change_user_data_from_to 23 26
Modify the point source ID.
  -set_point_source 500
  -change_point_source_from_to 1023 1024
  -quantize_Z_into_point_source 200
Transform gps_time.
  -translate_gps_time 40.50
  -adjusted_to_week
  -week_to_adjusted 1671
Transform RGB colors.
  -scale_rgb_down (by 256)
  -scale_rgb_up (by 256)
Supported LAS Inputs
  -i lidar.las
  -i lidar.laz
  -i lidar1.las lidar2.las lidar3.las -merged
  -i *.las - merged
  -i flight0??.laz flight1??.laz
  -i terrasolid.bin
  -i esri.shp
  -i nasa.qi
  -i lidar.txt -iparse xyzti -iskip 2 (on-the-fly from ASCII)
  -i lidar.txt -iparse xyzi -itranslate_intensity 1024
  -lof file_list.txt
  -stdin (pipe from stdin)
  -rescale 0.01 0.01 0.001
  -rescale_xy 0.01 0.01
  -rescale_z 0.01
  -reoffset 600000 4000000 0
Supported LAS Outputs
  -o lidar.las
  -o lidar.laz
  -o xyzta.txt -oparse xyzta (on-the-fly to ASCII)
  -o terrasolid.bin
  -o nasa.qi
  -odir C:\data\ground (specify output directory)
  -odix _classified (specify file name appendix)
  -ocut 2 (cut the last two characters from name)
  -olas -olaz -otxt -obin -oqfit (specify format)
  -stdout (pipe to stdout)
  -nil    (pipe to NULL)
LAStools (by info@rapidlasso.de) version 140301
usage:
lasprecision -i in.las
lasprecision -i in.las -number 1000000
lasprecision -i in.las -all -gps -lines 50
lasprecision -i in.las -no_x -no_y -no_z -rgb
lasprecision -i in.las -diff_diff
lasprecision -i in.las -o out.las -rescale 0.01 0.01 0.001 -reoffset 300000 2000000 0
lasprecision -i in.las -o out.las -rescale 0.333333333 0.333333333 0.01
lasprecision -h

---------------

if you find bugs let me (info@rapidlasso.de) know.
