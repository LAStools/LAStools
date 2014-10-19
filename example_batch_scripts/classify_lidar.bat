::
:: an example batch script for classifying a single LAS file
::

echo off

set PATH=%PATH%;C:\lastools\bin;

lasground -v -i %1 -city -fine -o temp1.las
lasheight -i temp1.las -o temp2.las
del temp1.las
lasclassify -v -i temp2.las -o %1 -odix _classified -olaz
del temp2.las
