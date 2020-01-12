****************************************************************

  lasvalidate:

  A simple tool (open source LGPL) to determine if LAS files
  conform to the ASPRS LAS 1.0 to 1.4 specifications.
 
  For updates check the website or join the LAStools mailing list.

  http://github.com/LASvalidator/

  http://rapidlasso.com/
  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  https://www.linkedin.com/groups/4408378/

  Martin @lastools

****************************************************************

Example calls to lasvalidate:

C:\lastools\bin> lasvalidate -i ..\data\*.las -oxml

and

C:\lastools\bin> lasvalidate -i ..\data\*.las -o ..\data\summary.xml

The source code compiles under Windows and Linux is available from
github at http://github.com/LASvalidator

mkdir LASvalidator
cd LASvalidator
git clone https://github.com/LASvalidator/LASread.git 
cd LASread 
make 
cd .. 
git clone https://github.com/LASvalidator/lasvalidate.git 
cd lasvalidate 
make 
cd bin 
E:\software\LAStools\bin>lasvalidate -h
This is version 200104 of the LAS validator. Please contact
me at 'martin.isenburg@rapidlasso.com' if you disagree with
validation reports, want additional checks, or find bugs as
the software is still under development. Your feedback will
help to finish it sooner.
Supported Inputs:
  -i lidar.las
  -i lidar1.las lidar2.las lidar3.las
  -i *.las
  -i flight0??.las flight1??.las
  -lof file_list.txt
Usage:
lasvalidate -i lidar.las
lasvalidate -i lidar.laz -no_CRS_fail
lasvalidate -v -i lidar.las -o report.xml
lasvalidate -v -i lidar.laz -oxml
lasvalidate -vv -i tile1.las tile2.las tile3.las -oxml
lasvalidate -i tile1.laz tile2.laz tile3.laz -o summary.xml
lasvalidate -i *.las -no_CRS_fail -o report.xml
lasvalidate -i *.laz -o summary.xml
lasvalidate -i *.laz -tile_size 1000 -o summary.xml
lasvalidate -i *.las -oxml
lasvalidate -i c:\data\lidar.las -oxml
lasvalidate -i ..\subfolder\*.las -o summary.xml
lasvalidate -v -i ..\..\flight\*.laz -o oxml
lasvalidate -h

--

Below is a test run on the unit tests. 

C:\LASvalidator\lasvalidate\bin> lasvalidate -i ..\unit\*.las -o ..\unit\validate.xml

This is version 200104 of the LAS validator. Please contact
me at 'martin.isenburg@rapidlasso.com' if you disagree with
validation reports, want additional checks, or find bugs as
the software is still under development. Your feedback will
help to finish it sooner.
needed 0.01 sec for 'las12.las' warning
needed 0.00 sec for 'las12_bounding_box.las' fail
needed 0.00 sec for 'las12_creation_date.las' fail
needed 0.00 sec for 'las12_global_encoding.las' fail
needed 0.00 sec for 'las12_header_size.las' fail
needed 0.00 sec for 'las12_number_of_points_by_return.las' fail
WARNING: end-of-file after 8144 of 8150 points
needed 0.01 sec for 'las12_number_of_point_records.las' fail
needed 0.00 sec for 'las12_offset_to_point_data.las' fail
needed 0.00 sec for 'las12_point_data_format.las' fail
needed 0.00 sec for 'las12_scale_factor.las' warning
done. total time 0.07 sec. total fail (pass=0,warning=2,fail=8)

C:\LASvalidator\lasvalidate\bin> more  ..\unit\validate.xml
<?xml version="1.0" encoding="UTF-8"?>
<LASvalidator>
  <report>
    <file>
      <name>las12.las</name>
      <path>..\unit\las12.las</path>
      <version>1.2</version>
      <system_identifier>LAStools (c) by Martin Isenburg</system_identifier>
      <generating_software>las2las (version 130506)</generating_software>
      <point_data_format>1</point_data_format>
      <CRS>UTM 55 southern hemisphere</CRS>
    </file>
    <summary>
      warning
    </summary>
    <details>
      <warning>
        <variable>return number</variable>
        <note>there are 1 points with a return number of 6</note>
      </warning>
      <warning>
        <variable>return number</variable>
        <note>there are 6 points with a number of returns of given pulse of 6</note>
      </warning>
    </details>
  </report>
  <report>
    <file>
      <name>las12_bounding_box.las</name>
      <path>..\unit\las12_bounding_box.las</path>
      <version>1.2</version>
      <system_identifier>LAStools (c) by Martin Isenburg</system_identifier>
      <generating_software>las2las (version 130506)</generating_software>
      <point_data_format>1</point_data_format>
      <CRS>UTM 55 southern hemisphere</CRS>
    </file>
    <summary>
      fail
    </summary>
    <details>
      <fail>
        <variable>bounding box</variable>
        <note>there are 1003 points outside of the bounding box specified in the LAS file header</note>
      </fail>
      <fail>
        <variable>min x</variable>
        <note>should be 309240.00 and not 309240.15</note>
      </fail>
      <fail>
        <variable>max x</variable>
        <note>should be 309254.99 and not 309254.50</note>
      </fail>
      <fail>
        <variable>min y</variable>
        <note>should be 6143455.00 and not 6143455.20</note>
      </fail>
      <fail>
        <variable>max y</variable>
        <note>should be 6143469.99 and not 6143468.50</note>
      </fail>
      <fail>
        <variable>min z</variable>
        <note>should be 455.07 and not 455.70</note>
      </fail>
      <fail>
        <variable>max z</variable>
        <note>should be 471.39 and not 471.30</note>
      </fail>
      <warning>
        <variable>return number</variable>
        <note>there are 1 points with a return number of 6</note>
      </warning>
      <warning>
        <variable>return number</variable>
        <note>there are 6 points with a number of returns of given pulse of 6</note>
      </warning>
    </details>
  </report>
  <report>
    <file>
      <name>las12_creation_date.las</name>
      <path>..\unit\las12_creation_date.las</path>
      <version>1.2</version>
      <system_identifier>LAStools (c) by Martin Isenburg</system_identifier>
      <generating_software>las2las (version 130506)</generating_software>
      <point_data_format>1</point_data_format>
      <CRS>UTM 55 southern hemisphere</CRS>
    </file>
    <summary>
      fail
    </summary>
    <details>
      <fail>
        <variable>file creation day</variable>
        <note>not set</note>
      </fail>
      <fail>
        <variable>file creation year</variable>
        <note>not set</note>
      </fail>
      <warning>
        <variable>return number</variable>
        <note>there are 1 points with a return number of 6</note>
      </warning>
      <warning>
        <variable>return number</variable>
        <note>there are 6 points with a number of returns of given pulse of 6</note>
      </warning>
    </details>
  </report>
  <report>
    <file>
      <name>las12_global_encoding.las</name>
      <path>..\unit\las12_global_encoding.las</path>
      <version>1.2</version>
      <system_identifier>LAStools (c) by Martin Isenburg</system_identifier>
      <generating_software>las2las (version 130506)</generating_software>
      <point_data_format>1</point_data_format>
      <CRS>UTM 55 southern hemisphere</CRS>
    </file>
    <summary>
      fail
    </summary>
    <details>
      <fail>
        <variable>global encoding</variable>
        <note>should not be greater than 1 for LAS version 1.2 but is 63</note>
      </fail>
      <fail>
        <variable>global encoding</variable>
        <note>set bit 4 not defined for LAS version 1.2</note>
      </fail>
      <fail>
        <variable>global encoding</variable>
        <note>set bit 3 not defined for LAS version 1.2</note>
      </fail>
      <fail>
        <variable>global encoding</variable>
        <note>set bit 2 not defined for LAS version 1.2</note>
      </fail>
      <fail>
        <variable>global encoding</variable>
        <note>set bit 2 not defined for point data format 1</note>
      </fail>
      <fail>
        <variable>global encoding</variable>
        <note>although bit 1 and bit 2 are mutually exclusive they are both set</note>
      </fail>
      <fail>
        <variable>global encoding</variable>
        <note>set bit 1 not defined for LAS version 1.2</note>
      </fail>
      <fail>
        <variable>global encoding</variable>
        <note>set bit 1 not defined for point data format 1</note>
      </fail>
      <warning>
        <variable>return number</variable>
        <note>there are 1 points with a return number of 6</note>
      </warning>
      <warning>
        <variable>return number</variable>
        <note>there are 6 points with a number of returns of given pulse of 6</note>
      </warning>
    </details>
  </report>
  <report>
    <file>
      <name>las12_header_size.las</name>
      <path>..\unit\las12_header_size.las</path>
      <version>1.2</version>
      <system_identifier>LAStools (c) by Martin Isenburg</system_identifier>
      <generating_software>las2las (version 130902)</generating_software>
      <point_data_format>0</point_data_format>
      <CRS>not valid or not specified</CRS>
    </file>
    <summary>
      fail
    </summary>
    <details>
      <fail>
        <variable>header size</variable>
        <note>the header_size of any LAS file is at least 227 but here it is only 226</note>
      </fail>
    </details>
  </report>
  <report>
    <file>
      <name>las12_number_of_points_by_return.las</name>
      <path>..\unit\las12_number_of_points_by_return.las</path>
      <version>1.2</version>
      <system_identifier>LAStools (c) by Martin Isenburg</system_identifier>
      <generating_software>las2las (version 130506)</generating_software>
      <point_data_format>1</point_data_format>
      <CRS>UTM 55 southern hemisphere</CRS>
    </file>
    <summary>
      fail
    </summary>
    <details>
      <fail>
        <variable>number of points by return[0]</variable>
        <note>the number of 1st returns is 4405 and not 4400</note>
      </fail>
      <fail>
        <variable>number of points by return[2]</variable>
        <note>the number of 3rd returns is 1031 and not 1030</note>
      </fail>
      <fail>
        <variable>number of points by return[3]</variable>
        <note>the number of 4th returns is 201 and not 200</note>
      </fail>
      <fail>
        <variable>number of points by return[4]</variable>
        <note>the number of 5th returns is 26 and not 30</note>
      </fail>
      <warning>
        <variable>return number</variable>
        <note>there are 1 points with a return number of 6</note>
      </warning>
      <warning>
        <variable>return number</variable>
        <note>there are 6 points with a number of returns of given pulse of 6</note>
      </warning>
    </details>
  </report>
  <report>
    <file>
      <name>las12_number_of_point_records.las</name>
      <path>..\unit\las12_number_of_point_records.las</path>
      <version>1.2</version>
      <system_identifier>LAStools (c) by Martin Isenburg</system_identifier>
      <generating_software>las2las (version 130506)</generating_software>
      <point_data_format>1</point_data_format>
      <CRS>UTM 55 southern hemisphere</CRS>
    </file>
    <summary>
      fail
    </summary>
    <details>
      <fail>
        <variable>number of point records</variable>
        <note>there are only 8144 point records and not 8150</note>
      </fail>
      <warning>
        <variable>return number</variable>
        <note>there are 1 points with a return number of 6</note>
      </warning>
      <warning>
        <variable>return number</variable>
        <note>there are 6 points with a number of returns of given pulse of 6</note>
      </warning>
    </details>
  </report>
  <report>
    <file>
      <name>las12_offset_to_point_data.las</name>
      <path>..\unit\las12_offset_to_point_data.las</path>
      <version>1.2</version>
      <system_identifier>LAStools (c) by Martin Isenburg</system_identifier>
      <generating_software>las2las (version 130902)</generating_software>
      <point_data_format>0</point_data_format>
      <CRS>not valid or not specified</CRS>
    </file>
    <summary>
      fail
    </summary>
    <details>
      <fail>
        <variable>offset to point data</variable>
        <note>the offset_to_point_data 225 must be equal or larger than the header_size 227</note>
      </fail>
    </details>
  </report>
  <report>
    <file>
      <name>las12_point_data_format.las</name>
      <path>..\unit\las12_point_data_format.las</path>
      <version>1.2</version>
      <system_identifier>LAStools (c) by Martin Isenburg</system_identifier>
      <generating_software>las2las (version 130902)</generating_software>
      <point_data_format>14</point_data_format>
      <CRS>not valid or not specified</CRS>
    </file>
    <summary>
      fail
    </summary>
    <details>
      <fail>
        <variable>point type or size</variable>
      </fail>
    </details>
  </report>
  <report>
    <file>
      <name>las12_scale_factor.las</name>
      <path>..\unit\las12_scale_factor.las</path>
      <version>1.2</version>
      <system_identifier>LAStools (c) by Martin Isenburg</system_identifier>
      <generating_software>las2las (version 130506)</generating_software>
      <point_data_format>1</point_data_format>
      <CRS>UTM 55 southern hemisphere</CRS>
    </file>
    <summary>
      warning
    </summary>
    <details>
      <warning>
        <variable>x scale factor</variable>
        <note>should be factor ten of 0.1 or 0.5 or 0.25 and not 0.003333</note>
      </warning>
      <warning>
        <variable>y scale factor</variable>
        <note>should be factor ten of 0.1 or 0.5 or 0.25 and not 0.0123456789</note>
      </warning>
      <warning>
        <variable>z scale factor</variable>
        <note>should be factor ten of 0.1 or 0.5 or 0.25 and not 0.00987654321</note>
      </warning>
      <warning>
        <variable>return number</variable>
        <note>there are 1 points with a return number of 6</note>
      </warning>
      <warning>
        <variable>return number</variable>
        <note>there are 6 points with a number of returns of given pulse of 6</note>
      </warning>
    </details>
  </report>
  <total>
    fail
    <details>
      <pass>0</pass>
      <warning>2</warning>
      <fail>8</fail>
    </details>
  </total>
  <version>
    200104 built with LASread version 1.1 (200104)
  </version>
  <command_line>
    lasvalidate -i ..\unit\*.las -o ..\unit\validate.xml
  </command_line>
</LASvalidator>
