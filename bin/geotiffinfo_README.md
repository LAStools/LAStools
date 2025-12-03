# geotiffinfo

This tiny tool informs about all header fields in a GeoTIFF file.
Other TIFF tools just report the well known TIFF tags. This tool
also analyzes and reports the GeoKey directory entries.
  
## Examples

    geotiffinfo in.tif
or
    geotiffinfo -i in.tif

outputs the header information of a TIFF file include the GeoTIFF tags and their interpretation:

File [in.tif]:  
TIFF Directory at offset 0x1da88 (121480)  
  Image Width: 268 Image Length: 258  
  Bits/Sample: 32  
  Sample Format: IEEE floating point  
  Compression Scheme: LZW  
  Photometric Interpretation: min-is-black  
  Samples/Pixel: 1  
  Rows/Strip: 20  
  Planar Configuration: single image plane  
  Software: LAStools is a product of rapidlasso GmbH, Germany  
  Artist: created by LAStools (c) info@rapidlasso.de  
  GeoPixelScale: 1.000000,1.000000,0.000000  
  GeoTiePoints: 0.000000,0.000000,0.000000,476941.000000,4366727.000000,0.000000  
  GeoKeyDirectory: 1,1,0,13,1024,0,1,1,1025,0,1,1,1026,34737,33,0,2048,0,1,4617,2049,34737,9,33,2050,0,1,6617,2052,0,1,9001,2056,0,1,7019,3072,0,1,3157,3073,34737,22,42,3076,0,1,9001,4096,0,1,6647,4099,0,1,9001  
  GeoASCIIParams: NAD83(CSRS) / UTM 10N | GRS 1980|GRS 1980|NAD83(CSRS) / UTM 10N|  
  GDALNoDataValue: -9999  
  GeoKeyDirectory details (13 entries):  
KeyID   Loc Cnt Value: KeyDesc = Value  
 1024     0   1     1: GTModelTypeGeoKey = ModelTypeProjected  
 1025     0   1     1: GTRasterTypeGeoKey = RasterPixelIsArea  
 1026 34737  33     0: CitationGeoKeys(GTCitationGeoKey) = NAD83(CSRS) / UTM 10N | GRS 1980  
 2048     0   1  4617: GeodeticCRSGeoKey = GCS_NAD83_CSRS  
 2049 34737   9    33: CitationGeoKeys(GeodeticCitationGeoKey) = GRS 1980  
 2050     0   1  6617: GeodeticDatumGeoKey = Description for [6617] not found  
 2052     0   1  9001: UnitsGeoKey(Linear Units) = Linear_Meter  
 2056     0   1  7019: EllipsoidGeoKey = Ellipse_GRS_1980  
 3072     0   1  3157: ProjectedCRSGeoKey = NAD83(CSRS) / UTM 10N  
 3073 34737  22    42: CitationGeoKeys(ProjectedCitationGeoKey) = NAD83(CSRS) / UTM 10N  
 3076     0   1  9001: UnitsGeoKey(Linear Units) = Linear_Meter  
 4096     0   1  6647: VerticalGeoKey = Canadian Geodetic Vertical Datum of 2013  
 4099     0   1  9001: UnitsGeoKey(Vertical Units) = Linear_Meter  

## geotiffinfo specific arguments
-i [n]       : input file or input wildcard  

if only one argument is given this is assumed as input file or input wildcard

## Licensing

This tool is free to use.

## Support

1. We invite you to join our LAStools Google Group (http://groups.google.com/group/lastools/).
   If you are looking for information about a specific tool, enter the tool name in the search 
   function and you'll find all discussions related to the respective tool. 
2. Customer Support Page: https://rapidlasso.de/customer-support/.  
3. Download LAStools: https://rapidlasso.de/downloads/.  
4. Changelog: https://rapidlasso.de/changelog/.  


If you want to send us feedback or have questions that are not answered in the resources above, 
please email to info@rapidlasso.de.
