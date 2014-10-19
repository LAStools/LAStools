****************************************************************

  las2las14:

  upconverts LiDAR data from LAS/LAZ/ASCII to the LAS 1.4 format

  DISCONTINUED: this functionality is now part of las2las.exe as
                shown in the examples below
 
  for updates check the website or join the LAStools mailing list

  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://groups.google.com/group/lasroom/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @lastools

****************************************************************

example usage:

las2las -i ..\data\TO_core_last_zoom.laz -set_version 1.4 -o to_14.laz
las2las -i ..\data\TO_core_last_zoom.laz -set_version 1.4 -o to_14__6.las -set_point_type 6
las2las -i ..\data\TO_core_last_zoom.laz -set_version 1.4 -o to_14__7.las -set_point_type 7
las2las -i ..\data\TO_core_last_zoom.laz -set_version 1.4 -o to_14__8.las -set_point_type 8
las2las -i ..\data\TO_core_last_zoom.laz -set_version 1.4 -o to_14__9.las -set_point_type 9
las2las -i ..\data\TO_core_last_zoom.laz -set_version 1.4 -o to_14_10.las -set_point_type 10

The option '-set_version 1.4' will upconvert the LAS 1.0 file
'TO_core_last_zoom.laz' to the LAS 1.4 format while leaving the
point data format of type 1 unchanged. This file should still
be readable by previous versions of legacy LAS software as long
as they implement forward compatible LAS readers.

The options '-set_point_type 6' to '-set_point_type 10' will in
addition upconvert the point data format from type 1 to the new
point types 6 to 10 that were newly introduced in LAS 1.4. These
LAS 1.4 file will no longer be compatible with earlier versions
of LAS software and will not be readable by forward compatible
LAS 1.0 to LAS 1.3 readers.

Compressed output in LAZ is not (yet) supported for the new point
types of LAS 1.4 format: 6, 7, 8, 9, and 10.

here some examples:

D:\lastools\bin>las2las -v -i ..\data\TO_core_last_zoom.laz -set_version 1.4 -o to_14.laz
reading 213093 and writing all surviving points ...
total time: 0.421 sec. written 213093 surviving points.

D:\lastools\bin>lasinfo -i to_14.laz
reporting all LAS header entries:
  file signature:             'LASF'
  file source ID:             0
  global_encoding:            0
  project ID GUID data 1-4:   00000000-0000-0000-0000-000000000000
  version major.minor:        1.4
  system identifier:          'LAStools (c) by Martin Isenburg'
  generating software:        'las2las (version 131205)'
  file creation day/year:     0/0
  header size:                375
  offset to point data:       377
  number var. length records: 0
  point data format:          1
  point data record length:   28
  number of point records:    213093
  number of points by return: 128621 84472 0 0 0
  scale factor x y z:         0.01 0.01 0.01
  offset x y z:               0 0 0
  min x y z:                  630250.00 4834500.00 46.83
  max x y z:                  630500.00 4834750.00 170.65
  start of waveform data packet record: 0
  start of first extended variable length record: 0
  number of extended_variable length records: 0
  extended number of point records: 213093
  extended number of points by return: 128621 84472 0 0 0 0 0 0 0 0 0 0 0 0 0
the header is followed by 2 user-defined bytes
LASzip compression (version 2.2r0 c2 50000): POINT10 2 GPSTIME11 2
reporting minimum and maximum for all LAS point record entries ...
  X   63025000   63050000
  Y  483450000  483475000
  Z       4683      17065
  intensity 10 50200
  edge_of_flight_line 0 0
  scan_direction_flag 0 0
  number_of_returns_of_given_pulse 1 2
  return_number                    1 2
  classification      1     1
  scan_angle_rank     0     0
  user_data           2     4
  point_source_ID     0     0
  gps_time 413162.560400 414095.322000
overview over number of returns of given pulse: 128621 84472 0 0 0 0 0
histogram of classification of points:
          213093  Unclassified (1)

D:\lastools\bin>las2las -v -i ..\data\TO_core_last_zoom.laz -set_version 1.4 -o to_14_06.las -set_point_type 6
reading 213093 and writing all surviving points ...
total time: 0.281 sec. written 213093 surviving points.

D:\lastools\bin>lasinfo -i to_14_06.las
reporting all LAS header entries:
  file signature:             'LASF'
  file source ID:             0
  global_encoding:            0
  project ID GUID data 1-4:   00000000-0000-0000-0000-000000000000
  version major.minor:        1.4
  system identifier:          'LAStools (c) by Martin Isenburg'
  generating software:        'las2las (version 131205)'
  file creation day/year:     0/0
  header size:                375
  offset to point data:       377
  number var. length records: 0
  point data format:          6
  point data record length:   30
  number of point records:    213093
  number of points by return: 128621 84472 0 0 0
  scale factor x y z:         0.01 0.01 0.01
  offset x y z:               0 0 0
  min x y z:                  630250.00 4834500.00 46.83
  max x y z:                  630500.00 4834750.00 170.65
  start of waveform data packet record: 0
  start of first extended variable length record: 0
  number of extended_variable length records: 0
  extended number of point records: 213093
  extended number of points by return: 128621 84472 0 0 0 0 0 0 0 0 0 0 0 0 0
the header is followed by 2 user-defined bytes
reporting minimum and maximum for all LAS point record entries ...
  X   63025000   63050000
  Y  483450000  483475000
  Z       4683      17065
  intensity 10 50200
  edge_of_flight_line 0 0
  scan_direction_flag 0 0
  number_of_returns_of_given_pulse 1 2
  return_number                    1 2
  classification      1     1
  scan_angle_rank     0     0
  user_data           2     4
  point_source_ID     0     0
  gps_time 413162.560400 414095.322000
  extended_number_of_returns_of_given_pulse 1 2
  extended_return_number                    1 2
  extended_classification       1      1
  extended_scan_angle       0.000  0.000
  extended_scanner_channel      0      0
overview over number of returns of given pulse: 128621 84472 0 0 0 0 0
histogram of classification of points:
          213093  Unclassified (1)

D:\lastools\bin>las2las -v -i ..\data\TO_core_last_zoom.laz -set_version 1.4 -o to_14_07.las -set_point_type 7
reading 213093 and writing all surviving points ...
total time: 0.296 sec. written 213093 surviving points.

D:\lastools\bin>lasinfo -i to_14_07.las
reporting all LAS header entries:
  file signature:             'LASF'
  file source ID:             0
  global_encoding:            0
  project ID GUID data 1-4:   00000000-0000-0000-0000-000000000000
  version major.minor:        1.4
  system identifier:          'LAStools (c) by Martin Isenburg'
  generating software:        'las2las (version 131205)'
  file creation day/year:     0/0
  header size:                375
  offset to point data:       377
  number var. length records: 0
  point data format:          7
  point data record length:   36
  number of point records:    213093
  number of points by return: 128621 84472 0 0 0
  scale factor x y z:         0.01 0.01 0.01
  offset x y z:               0 0 0
  min x y z:                  630250.00 4834500.00 46.83
  max x y z:                  630500.00 4834750.00 170.65
  start of waveform data packet record: 0
  start of first extended variable length record: 0
  number of extended_variable length records: 0
  extended number of point records: 213093
  extended number of points by return: 128621 84472 0 0 0 0 0 0 0 0 0 0 0 0 0
the header is followed by 2 user-defined bytes
reporting minimum and maximum for all LAS point record entries ...
  X   63025000   63050000
  Y  483450000  483475000
  Z       4683      17065
  intensity 10 50200
  edge_of_flight_line 0 0
  scan_direction_flag 0 0
  number_of_returns_of_given_pulse 1 2
  return_number                    1 2
  classification      1     1
  scan_angle_rank     0     0
  user_data           2     4
  point_source_ID     0     0
  gps_time 413162.560400 414095.322000
  Color R 0 0
        G 0 0
        B 0 0
  extended_number_of_returns_of_given_pulse 1 2
  extended_return_number                    1 2
  extended_classification       1      1
  extended_scan_angle       0.000  0.000
  extended_scanner_channel      0      0
overview over number of returns of given pulse: 128621 84472 0 0 0 0 0
histogram of classification of points:
          213093  Unclassified (1)

D:\lastools\bin>las2las -v -i ..\data\TO_core_last_zoom.laz -set_version 1.4 -o to_14_08.las -set_point_type 8
reading 213093 and writing all surviving points ...
total time: 0.312 sec. written 213093 surviving points.

D:\lastools\bin>lasinfo -i to_14_08.las
reporting all LAS header entries:
  file signature:             'LASF'
  file source ID:             0
  global_encoding:            0
  project ID GUID data 1-4:   00000000-0000-0000-0000-000000000000
  version major.minor:        1.4
  system identifier:          'LAStools (c) by Martin Isenburg'
  generating software:        'las2las (version 131205)'
  file creation day/year:     0/0
  header size:                375
  offset to point data:       377
  number var. length records: 0
  point data format:          8
  point data record length:   38
  number of point records:    213093
  number of points by return: 128621 84472 0 0 0
  scale factor x y z:         0.01 0.01 0.01
  offset x y z:               0 0 0
  min x y z:                  630250.00 4834500.00 46.83
  max x y z:                  630500.00 4834750.00 170.65
  start of waveform data packet record: 0
  start of first extended variable length record: 0
  number of extended_variable length records: 0
  extended number of point records: 213093
  extended number of points by return: 128621 84472 0 0 0 0 0 0 0 0 0 0 0 0 0
the header is followed by 2 user-defined bytes
reporting minimum and maximum for all LAS point record entries ...
  X   63025000   63050000
  Y  483450000  483475000
  Z       4683      17065
  intensity 10 50200
  edge_of_flight_line 0 0
  scan_direction_flag 0 0
  number_of_returns_of_given_pulse 1 2
  return_number                    1 2
  classification      1     1
  scan_angle_rank     0     0
  user_data           2     4
  point_source_ID     0     0
  gps_time 413162.560400 414095.322000
  Color R 0 0
        G 0 0
        B 0 0
      NIR 0 0
  extended_number_of_returns_of_given_pulse 1 2
  extended_return_number                    1 2
  extended_classification       1      1
  extended_scan_angle       0.000  0.000
  extended_scanner_channel      0      0
overview over number of returns of given pulse: 128621 84472 0 0 0 0 0
histogram of classification of points:
          213093  Unclassified (1)

D:\lastools\bin>las2las -v -i ..\data\TO_core_last_zoom.laz -set_version 1.4 -o to_14_09.las -set_point_type 9
reading 213093 and writing all surviving points ...
total time: 0.328 sec. written 213093 surviving points.

D:\lastools\bin>lasinfo -i to_14_09.las
reporting all LAS header entries:
  file signature:             'LASF'
  file source ID:             0
  global_encoding:            0
  project ID GUID data 1-4:   00000000-0000-0000-0000-000000000000
  version major.minor:        1.4
  system identifier:          'LAStools (c) by Martin Isenburg'
  generating software:        'las2las (version 131205)'
  file creation day/year:     0/0
  header size:                375
  offset to point data:       377
  number var. length records: 0
  point data format:          9
  point data record length:   59
  number of point records:    213093
  number of points by return: 128621 84472 0 0 0
  scale factor x y z:         0.01 0.01 0.01
  offset x y z:               0 0 0
  min x y z:                  630250.00 4834500.00 46.83
  max x y z:                  630500.00 4834750.00 170.65
  start of waveform data packet record: 0
  start of first extended variable length record: 0
  number of extended_variable length records: 0
  extended number of point records: 213093
  extended number of points by return: 128621 84472 0 0 0 0 0 0 0 0 0 0 0 0 0
the header is followed by 2 user-defined bytes
reporting minimum and maximum for all LAS point record entries ...
  X   63025000   63050000
  Y  483450000  483475000
  Z       4683      17065
  intensity 10 50200
  edge_of_flight_line 0 0
  scan_direction_flag 0 0
  number_of_returns_of_given_pulse 1 2
  return_number                    1 2
  classification      1     1
  scan_angle_rank     0     0
  user_data           2     4
  point_source_ID     0     0
  gps_time 413162.560400 414095.322000
  Wavepacket Index    0 0
             Offset   0 0
             Size     0 0
             Location 0 0
             Xt       0 0
             Yt       0 0
             Zt       0 0
  extended_number_of_returns_of_given_pulse 1 2
  extended_return_number                    1 2
  extended_classification       1      1
  extended_scan_angle       0.000  0.000
  extended_scanner_channel      0      0
overview over number of returns of given pulse: 128621 84472 0 0 0 0 0
histogram of classification of points:
          213093  Unclassified (1)

D:\lastools\bin>las2las -v -i ..\data\TO_core_last_zoom.laz -set_version 1.4 -o to_14_10.las -set_point_type 10
reading 213093 and writing all surviving points ...
total time: 0.296 sec. written 213093 surviving points.

D:\lastools\bin>lasinfo -i to_14_10.las
reporting all LAS header entries:
  file signature:             'LASF'
  file source ID:             0
  global_encoding:            0
  project ID GUID data 1-4:   00000000-0000-0000-0000-000000000000
  version major.minor:        1.4
  system identifier:          'LAStools (c) by Martin Isenburg'
  generating software:        'las2las (version 131205)'
  file creation day/year:     0/0
  header size:                375
  offset to point data:       377
  number var. length records: 0
  point data format:          10
  point data record length:   67
  number of point records:    213093
  number of points by return: 128621 84472 0 0 0
  scale factor x y z:         0.01 0.01 0.01
  offset x y z:               0 0 0
  min x y z:                  630250.00 4834500.00 46.83
  max x y z:                  630500.00 4834750.00 170.65
  start of waveform data packet record: 0
  start of first extended variable length record: 0
  number of extended_variable length records: 0
  extended number of point records: 213093
  extended number of points by return: 128621 84472 0 0 0 0 0 0 0 0 0 0 0 0 0
the header is followed by 2 user-defined bytes
reporting minimum and maximum for all LAS point record entries ...
  X   63025000   63050000
  Y  483450000  483475000
  Z       4683      17065
  intensity 10 50200
  edge_of_flight_line 0 0
  scan_direction_flag 0 0
  number_of_returns_of_given_pulse 1 2
  return_number                    1 2
  classification      1     1
  scan_angle_rank     0     0
  user_data           2     4
  point_source_ID     0     0
  gps_time 413162.560400 414095.322000
  Color R 0 0
        G 0 0
        B 0 0
      NIR 0 0
  Wavepacket Index    0 0
             Offset   0 0
             Size     0 0
             Location 0 0
             Xt       0 0
             Yt       0 0
             Zt       0 0
  extended_number_of_returns_of_given_pulse 1 2
  extended_return_number                    1 2
  extended_classification       1      1
  extended_scan_angle       0.000  0.000
  extended_scanner_channel      0      0
overview over number of returns of given pulse: 128621 84472 0 0 0 0 0
histogram of classification of points:
          213093  Unclassified (1)

D:\lastools\bin>dir to_14*
12/09/2013  11:10 AM           585,306 to_14.laz
12/09/2013  11:41 AM         6,393,167 to_14_06.las
12/09/2013  11:42 AM         7,671,725 to_14_07.las
12/09/2013  11:42 AM         8,097,911 to_14_08.las
12/09/2013  11:42 AM        12,572,864 to_14_09.las
12/09/2013  11:47 AM        14,277,608 to_14_10.las


---------------

if you find bugs let me (martin.isenburg@rapidlasso.com) know.
