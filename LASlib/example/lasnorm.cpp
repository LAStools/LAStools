/*
===============================================================================

  FILE:  lasexample.cpp
  
  CONTENTS:
  
    This source code serves as an example how you can easily use laslib to
    write your own processing tools or how to import from and export to the
    LAS format or - its compressed, but identical twin - the LAZ format.

  PROGRAMMERS:
  
		daryl_van_dyke@fws.gov
	
	from template lasexample by:
		martin.isenburg@gmail.com
  
  COPYRIGHT:
  
    (c) 2011, Martin Isenburg, LASSO - tools to catch reality

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
      28 May 2012 -- Final Version 
	  3 January 2011 -- created while too homesick to go to Salzburg with Silke
  
===============================================================================
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lasreader.hpp"
#include "laswriter.hpp"
#include "lasutility.hpp"
 
//from GDAL -->
#include "gdal.h"
#include "gdal_frmts.h"
//#include "gdal_frmts.h"
//#include "squid.h"
//#include "gdal_alg.h"
//#include "gdal_priv.h"
#include "cpl_string.h"
//#include "cpl_conv.h"
#include "ogr_spatialref.h"
#include "cpl_minixml.h"
#include <vector>
//<-- End GDAL

	struct  bands{
float  Val;
};
bands rBand;

void usage(bool wait=false)
{
  fprintf(stderr,"usage:\n");
  fprintf(stderr,"lastest in_DEM.img in.las out.las\n");
  fprintf(stderr,"lastest -i in.las -o out.las -verbose\n");
  fprintf(stderr,"lastest -ilas -olas < in.las > out.las\n");
  fprintf(stderr,"lastest -h\n");
  fprintf(stderr,"Pret-ty Good!\n");
  if (wait)
  {
    fprintf(stderr,"<press ENTER>\n");
    getc(stdin);
  }
  exit(1);
}

static void byebye(bool error=false, bool wait=false)
{
  if (wait)
  {
    fprintf(stderr,"<press ENTER>\n");
    getc(stdin);
  }
  exit(error);
}

static double taketime()
{
  return (double)(clock())/CLOCKS_PER_SEC;
}

static int lidardouble2string(char* string, double value)
{
  int len;
  len = sprintf(string, "%.15f", value) - 1;
  while (string[len] == '0') len--;
  if (string[len] != '.') len++;
  string[len] = '\0';
  return len;
}

static int lidardouble2string(char* string, double value, double precision)
{
  if (precision == 0.1)
    sprintf(string, "%.1f", value);
  else if (precision == 0.01)
    sprintf(string, "%.2f", value);
  else if (precision == 0.001 || precision == 0.002 || precision == 0.005 || precision == 0.025) 
    sprintf(string, "%.3f", value);
  else if (precision == 0.0001 || precision == 0.0002 || precision == 0.0005 || precision == 0.0025)
    sprintf(string, "%.4f", value);
  else if (precision == 0.00001 || precision == 0.00002 || precision == 0.00005 || precision == 0.00025)
    sprintf(string, "%.5f", value);
  else if (precision == 0.000001)
    sprintf(string, "%.6f", value);
  else if (precision == 0.0000001)
    sprintf(string, "%.7f", value);
  else if (precision == 0.00000001)
    sprintf(string, "%.8f", value);
  else
    return lidardouble2string(string, value);
  return strlen(string)-1;
}

int main(int argc, char *argv[])
{
  int i;
  bool verbose = false;
  double start_time = 0.0;

  assert(CHAR_BIT * sizeof (float) == 32);
  
  LASreadOpener lasreadopener;
  LASwriteOpener laswriteopener;
  
  //start GDAL -->
  const char         *pszLocX = NULL, *pszLocY = NULL;
  const char         *pszSrcFilename = NULL;
  const char         *pszSourceSRS = NULL;
  std::vector<int>   anBandList;
  bool               bAsXML = false, bLIFOnly = false;
  bool               bQuiet = false, bValOnly = false;
  double			 adfGeoTransform[6];
  GDALDatasetH       *poDataset;
  GDALRasterBandH     *poBand;

  char printstring[4096];
  double xScale, yScale, zScale;
  double xOffset, yOffset, zOffset;

  GDALAllRegister();
  
  // end GDAL <--
  
  if (argc == 1)
  {
    fprintf(stderr,"lastest.exe is better run in the command line\n");
    char file_name[256];
    fprintf(stderr,"enter input file: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    lasreadopener.set_file_name(file_name);
    fprintf(stderr,"enter output file: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    laswriteopener.set_file_name(file_name);
  }
  else
  {
    lasreadopener.parse(argc, argv);
    laswriteopener.parse(argc, argv);
  }

  for (i = 1; i < argc; i++)
  {
    if (argv[i][0] == '\0')
    {
      continue;
    }
    else if (strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"-help") == 0)
    {
      usage();
    }
    else if (strcmp(argv[i],"-v") == 0 || strcmp(argv[i],"-verbose") == 0)
    {
      verbose = true;
    }
	
	else if (i == argc - 3 && !lasreadopener.active() && !laswriteopener.active())
    {
	  fprintf(stdout,"Good DEM Name\n");
	  //fprintf(stdout,argv[i]);
	  fprintf(stdout,"%i\n",i);
	  fprintf(stdout,"%c\n",argv[i]);
	  fprintf(stdout,"%c\n",argv[argc - 2]);
	  fprintf(stdout,"%c\n",argv[argc - 1]);
      pszSrcFilename = (argv[i]);
	  
    }
    
	else if (i == argc - 2 && !lasreadopener.active() && !laswriteopener.active())
    {
      lasreadopener.set_file_name(argv[i]);
    }
    else if (i == argc - 1 && !lasreadopener.active() && !laswriteopener.active())
    {
      lasreadopener.set_file_name(argv[i]);
    }
    else if (i == argc - 1 && lasreadopener.active() && !laswriteopener.active())
    {
      laswriteopener.set_file_name(argv[i]);
    }
    else
    {
      fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
      usage();
    }
  }

  if (verbose) start_time = taketime();

  // check input & output
  
  if (!lasreadopener.active())
  {
    fprintf(stderr,"ERROR: no input specified\n");
    usage(argc == 1);
  }

  if (!laswriteopener.active())
  {
    fprintf(stderr,"ERROR: no output specified\n");
    usage(argc == 1);
  }

  // open lasreader

  LASreader* lasreader = lasreadopener.open();
  if (lasreader == 0)
  {
    fprintf(stderr, "ERROR: could not open lasreader\n");
    byebye(argc==1);
  }

  // generate LASHeader for later
  LASheader* lasheader = &lasreader->header;

  // open laswriter

  LASwriter* laswriter = laswriteopener.open(&lasreader->header);
  if (laswriter == 0)
  {
    fprintf(stderr, "ERROR: could not open laswriter\n");
    byebye(argc==1);
  }

#ifdef _WIN32
  if (verbose) fprintf(stderr, "reading %I64d points from '%s' and writing them modified to '%s'.\n", lasreader->npoints, lasreadopener.get_file_name(), laswriteopener.get_file_name());
#else
  if (verbose) fprintf(stderr, "reading %lld points from '%s' and writing them modified to '%s'.\n", lasreader->npoints, lasreadopener.get_file_name(), laswriteopener.get_file_name());
#endif

  fprintf(stdout, "  scale factor x y z:         "); 
  lidardouble2string(printstring, lasheader->x_scale_factor); 
  fprintf(stdout, "%s ", printstring);  
  lidardouble2string(printstring, lasheader->y_scale_factor); 
  fprintf(stdout, "%s ", printstring);  
  lidardouble2string(printstring, lasheader->z_scale_factor);
  fprintf(stdout, "%s\012", printstring);
  xScale = lasheader->x_scale_factor;
  yScale = lasheader->y_scale_factor;
  zScale = lasheader->z_scale_factor;

  
	fprintf(stdout, "  offset x y z:               "); lidardouble2string(printstring, lasheader->x_offset); fprintf(stdout, "%s ", printstring);  lidardouble2string(printstring, lasheader->y_offset); fprintf(stdout, "%s ", printstring);  lidardouble2string(printstring, lasheader->z_offset); fprintf(stdout, "%s\012", printstring);
	fprintf(stdout, "  min x y z:                  "); lidardouble2string(printstring, lasheader->min_x, lasheader->x_scale_factor); fprintf(stdout, "%s ", printstring); lidardouble2string(printstring, lasheader->min_y, lasheader->y_scale_factor); fprintf(stdout, "%s ", printstring); lidardouble2string(printstring, lasheader->min_z, lasheader->z_scale_factor); fprintf(stdout, "%s\012", printstring);
	fprintf(stdout, "  max x y z:                  "); lidardouble2string(printstring, lasheader->max_x, lasheader->x_scale_factor); fprintf(stdout, "%s ", printstring); lidardouble2string(printstring, lasheader->max_y, lasheader->y_scale_factor); fprintf(stdout, "%s ", printstring); lidardouble2string(printstring, lasheader->max_z, lasheader->z_scale_factor); fprintf(stdout, "%s\012", printstring);
	
	xOffset = (lasheader->x_offset);
	yOffset = (lasheader->y_offset);
	zOffset = (lasheader->z_offset);


  //exit(1);
  // loop over points and modify them
  //GDALAllRegister();
  
/* -------------------------------------------------------------------- */
/*      Open source file.                                               */
/* -------------------------------------------------------------------- */
  
  GDALDatasetH hSrcDS = NULL;
  
  hSrcDS = GDALOpen( pszSrcFilename, GA_ReadOnly );
  if( hSrcDS == NULL )
    exit( 1 );
  double xLL, yLL;
  double xDelta, yDelta;
  printf( "Size is %d, %d\n",
            GDALGetRasterXSize( hSrcDS ), 
            GDALGetRasterYSize( hSrcDS ) );
			
  if( GDALGetGeoTransform( hSrcDS, adfGeoTransform ) == CE_None )
  {
    if( adfGeoTransform[2] == 0.0 && adfGeoTransform[4] == 0.0 )
    {
        printf( "Origin = (%.15f,%.15f)\n",
                    adfGeoTransform[0], adfGeoTransform[3] );
		xLL = adfGeoTransform[0];
		yLL = adfGeoTransform[3];

        printf( "Pixel Size = (%.15f,%.15f)\n",
                    adfGeoTransform[1], adfGeoTransform[5] );
		xDelta = adfGeoTransform[1];
		yDelta = adfGeoTransform[5];
    }
    else
        printf( "GeoTransform =\n"
                    "  %.16g, %.16g, %.16g\n"
                    "  %.16g, %.16g, %.16g\n", 
                    adfGeoTransform[0],
                    adfGeoTransform[1],
                    adfGeoTransform[2],
                    adfGeoTransform[3],
                    adfGeoTransform[4],
                    adfGeoTransform[5] );
  }
  
  fprintf(stdout,"header  /n");
  printf( "Origin = (%.15f,%.15f)\n",
                    adfGeoTransform[0], adfGeoTransform[3] );
  printf( "Pixel Size = (%.15f,%.15f)\n",
                    adfGeoTransform[1], adfGeoTransform[5] );
  printf( "GeoTransform =\n"
                    "  %.16g, %.16g, %.16g\n"
                    "  %.16g, %.16g, %.16g\n", 
					adfGeoTransform[0],
                    adfGeoTransform[1],
                    adfGeoTransform[2],
                    adfGeoTransform[3],
                    adfGeoTransform[4],
                    adfGeoTransform[5] );
 
  xDelta = adfGeoTransform[1];
  
    //  START Header Info
  GDALDriverH   hDriver;
  hDriver = GDALGetDatasetDriver( hSrcDS );
  printf( "Driver: %s/%s\n",
          GDALGetDriverShortName( hSrcDS ),
          GDALGetDriverLongName( hSrcDS ) );

  printf( "Size is %dx%dx%d\n",
          GDALGetRasterXSize( hSrcDS ), 
          GDALGetRasterYSize( hSrcDS ),
          GDALGetRasterCount( hSrcDS ) );

  if( GDALGetProjectionRef( hSrcDS ) != NULL )
      printf( "Projection is `%s'\n", GDALGetProjectionRef( hSrcDS ) );

  if( GDALGetGeoTransform( hSrcDS, adfGeoTransform ) == CE_None )
  {
       printf( "Origin = (%.6f,%.6f)\n   xLL yLL = (%.6f, %.6f) \n",
              adfGeoTransform[0], adfGeoTransform[3], xLL, yLL );
	    
      printf( "Pixel Size = (%.6f,%.6f)\n",
              adfGeoTransform[1], adfGeoTransform[5] );
  }
	//  END Header Info

  /*
  adfGeoTransform[0] /* top left x 
  adfGeoTransform[1] /* w-e pixel resolution 
    adfGeoTransform[2] /* 0 
    adfGeoTransform[3] /* top left y 
    adfGeoTransform[4] /* 0 
    adfGeoTransform[5] /* n-s pixel resolution (negative value) 
  */

  fprintf(stdout,"Nice Try \n");
  //fprintf(stdout," SPOT <----- \n");
  
  // where there is a point to read
  while (lasreader->read_point())
  {
    
	// modify the point
	/* -------------------------------------------------------------------- */
	/*      Turn the location into a pixel and line location.               */
	/* -------------------------------------------------------------------- */
	int inputAvailable = 1;
	double dfGeoX;
	double dfGeoY, dX,dY;
	double dfGeoZ, dZ;
	typedef double f64;
	double coords[3] ;
	
	int iPixel, iLine;
	// 
	// _______________________declare the input point coords here_(as Double)__________________
	//lasreader->point.z += 10;
	/*    dfGeoX = lasreader->point.X * 0.001;
	dfGeoY = lasreader->point.Y * 0.01;
	dfGeoZ = lasreader->point.Z * 0.01;   */
	//dfGeoX = lasreader->point.get_X() * xScale ;
	//dfGeoY = lasreader->point.get_Y() * yScale ;
	//dfGeoZ = lasreader->point.get_Z() * zScale ;

	//(lasreader->point.coordinates );
	//(lasreader->point.compute_coordinates() );
	dX = lasreader->point.get_X() * xScale + xOffset;
	dY = lasreader->point.get_Y() * yScale + yOffset;
	dZ = lasreader->point.get_Z() * zScale + zOffset;
	//fprintf (stdout,"\n df \n las: X,Y :  %18.5f  %20.5f  %10.2f  \n",
		     //dfGeoX, dfGeoY, dfGeoZ);
	fprintf (stdout,"\n dX \n las: X,Y :  %18.5f  %18.5f  %10.2f  \n",
		     dX, dY, dZ);
	//__________________________________________________________________________________________




	//fprintf(stdout," SPOT <----- \n");
	//GDALRasterBandH hBand = GDALGetRasterBand( hSrcDS, anBandList[0] );
	//fprintf (stdout, anBandList[0]);
	GDALRasterBandH hBand = GDALGetRasterBand( hSrcDS, 1 );
		
	if (hBand == NULL) 
		exit( 1 );
		
	/* -------------------------------------------------------------------- */
	float bufPixel[8];
	float adfPixel2[2];//[2]
	CPLString osValue;
	//osValue.Printf( "This val   %.15g   \n", adfPixel[0] );
	//fprintf(stdout, "This val  (fprintf)  %8.2f \n", adfPixel2[1] );
	int bSuccess;
		
	double dfOffset = GDALGetRasterOffset( hBand, &bSuccess );
	double dfScale  = GDALGetRasterScale( hBand, &bSuccess );
	//fprintf(stdout, "%.15g  %.15g \n", dfOffset,dfScale);
    //fprintf(stdout, "xLL yLL %.15g  %.15g \n", xLL,yLL);
	

	/*
	iPixel = (int) floor( ( dfGeoX - xLL )/xDelta );
    iLine  = (int) floor(( dfGeoY - yLL )/yDelta );
	fprintf(stdout, " yLL %.15g  %.15g \n", iPixel,iLine);
	*/
	double adfGeoTransform[6], adfInvGeoTransform[6];

	if( GDALGetGeoTransform( hSrcDS, adfGeoTransform ) != CE_None )
		exit( 1 );

    if( !GDALInvGeoTransform( adfGeoTransform, adfInvGeoTransform ) )
            {
                CPLError(CE_Failure, CPLE_AppDefined, "Cannot invert geotransform");
                exit( 1 );
			}

	GDALInvGeoTransform( adfGeoTransform, adfInvGeoTransform );



	iPixel = (int) floor(dX - xLL);
    iLine  = (int) floor(yLL - dY);

	int pX = iPixel;
	int pY = iLine;
	
	fprintf(stdout,"iPixel, iLine:  %10i %10i \n",pX, pY);
	fprintf(stdout,"xLL, dfGeoX, dfGeoX - xLL:  %18.7f %18.7f %18.7f \n",xLL,dfGeoX, dfGeoX - xLL);
		
	typedef std::vector<float> raster_data_t;
	raster_data_t scanline(1);

	GDALDatasetRasterIO(hSrcDS,GF_Read,iPixel,iLine,1,1,bufPixel,1,1,GDT_Float32, 1, NULL, 0, 0, 0);

	GDALRasterBandH  *poBand;
	int             nBlockXSize, nBlockYSize;
    int             bGotMin, bGotMax;
    double          adfMinMax[2];
	float			*bufferPixel;
	float			*pafScanline;
    GDALGetBlockSize( hBand, &nBlockXSize, &nBlockYSize );
	

	//fprintf(stdout, "This val bufPixel %10.3f \n", bufPixel );
	//fprintf(stdout, "This val bp 0 1 2 %18.7f %18.7f %18.7f \n", bufPixel[0], bufPixel[1], bufPixel[2] );
	//fprintf(stdout, "%.15g  \n" , bufPixel);
	
    lasreader->point.Z = (dZ - bufPixel[0]);
    fprintf (stdout," X, Y, Z, demZ, height :  %18.2f  %18.2f  %10.2f %10.2f %10.2f \n", dX, dY, dZ, bufPixel[0], dZ-bufPixel[0]);
    // write the modified point
    laswriter->write_point(&lasreader->point);
    // add it to the inventory
    laswriter->update_inventory(&lasreader->point);
    } 
  

  // END OF WHILE LOOP FOR ALL LAS Points
  // close out
  laswriter->update_header(&lasreader->header, TRUE);

  I64 total_bytes = laswriter->close();
  delete laswriter;

#ifdef _WIN32
  if (verbose) fprintf(stderr,"total time: %g sec %I64d bytes for %I64d points\n", taketime()-start_time, total_bytes, lasreader->p_count);
#else
  if (verbose) fprintf(stderr,"total time: %g sec %lld bytes for %lld points\n", taketime()-start_time, total_bytes, lasreader->p_count);
#endif

  lasreader->close();
  delete lasreader;

  return 0;
} 

