****************************************************************

  demzip:

  Compresses and uncompresses raster data from ASC, BIL, TIF, IMG
  format to the compressed RasterLAZ format. The expected inputs
  are rasters containing elevation data such as DTM, DSM, or CHM
  rasters, or GEOID difference grids, or forestry metrics. These
  values will be compressed into the z coordinate of RasterLAZ.

  For updates check the website or join the LAStools mailing list.

  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @rapidlasso

****************************************************************

for example usage see:

https://groups.google.com/d/topic/lastools/39hR_4BvvIA/discussion
https://groups.google.com/d/topic/lastools/nMPU75zpqPw/discussion

for more info:

C:\software\LAStools\bin>demzip -h
usage:
demzip -i dem.tif -o dem.laz
demzip -i dem.laz -o dem.tif
demzip -i dem.asc -o dem.laz
demzip -i dem.laz -o dem.asc
demzip -i dem.img -o dem.laz
demzip -i dem.laz -o dem.img
demzip -i dem.bil -o dem.laz
demzip -i dem.laz -o dem.bil
demzip -i dem\*.tif -olaz -cores 3
demzip -i dem\*.asc -olaz -cores 3
demzip -i dem\*.img -olaz -cores 3
demzip -i dem\*.bil -olaz -cores 3
demzip -i dem\*.laz -otif -cores 3
demzip -i dem\*.laz -oasc -cores 3
demzip -i dem\*.laz -oimg -cores 3
demzip -i dem\*.laz -obil -cores 3
demzip -i dem\*.asc -odir compressed_dem -olaz -cores 2
demzip -i compressed_dem\*.laz -odir dem -oasc -cores 2
demzip -h

other options:
 -nodata_value -9999      : raster value -9999 considered nodata
 -nodata_min -1000        : raster values -1000 or below considered nodata
 -nodata_max 32768        : raster values 32768 or above considered nodata
 -scale 1.0               : set vertical resolution to meter (or feet)
 -scale 0.1               : set vertical resolution to decimeter (or decifeet)
 -scale 0.01              : set vertical resolution to centimeter (or centifeet)
 -longlat -wgs84          : set horizontal datum to longlat on WGS84
 -longlat -etrs89         : set horizontal datum to longlat on ETRS89
 -longlat -gda94          : set horizontal datum to longlat on GDA94
 -longlat -nad83          : set horizontal datum to longlat on NAD83
 -longlat -nad83_csrs     : set horizontal datum to longlat on NAD83(CSRS)
 -longlat -nad83_2011     : set horizontal datum to longlat on NAD83(2001)
 -longlat -nad83_harn     : set horizontal datum to longlat on NAD83(HARN)
 -utm 32north -wgs84      : set horizontal datum to UTM32 north on WGS84
 -epsg 27700              : set horizontal datum to EPSG code 27700
 -vertical_wgs84          : set vertical datum to WGS84
 -vertical_navd88         : set vertical datum to NAVD88
 -vertical_cgvd2013       : set vertical datum to CGVD2013
 -vertical_nn2000         : set vertical datum to NN2000
 -vertical_dhhn92         : set vertical datum to DHHN92
 -vertical_dhhn2016       : set vertical datum to DHHN2016
 -elevation_survey_feet   : set vertical units from meters to US survey feet
 -sigmaxy 0.5             : horizontal accuracy expected at 0.5 meters (inactive)
---------------

if you find bugs let me (martin.isenburg@rapidlasso.com) know.