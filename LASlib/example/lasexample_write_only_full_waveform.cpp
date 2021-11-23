/*
===============================================================================

  FILE:  lasexample_write_full_waveform.cpp
  
  CONTENTS:
  
    This source code serves as an example how you can easily use LASlib to
    write to the LAS format or - its compressed, but identical twin - the
    LAZ format and also write full waveform data.

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de
    eric.rehm@gmail.com      

  COPYRIGHT:

    (c) 2007-2019, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    16 January 2019 -- full waveform example adapted from code by Eric Rehm
  
===============================================================================
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "laswriter.hpp"
#include "laswaveform13writer.hpp"

void usage(bool wait=false)
{
  fprintf(stderr,"usage:\n");
  fprintf(stderr,"lasexample_write_full_waveform out.las\n");
  fprintf(stderr,"lasexample_write_full_waveform -o out.las -verbose\n");
  fprintf(stderr,"lasexample_write_full_waveform -o out.las -verbose -waveform\n");
  fprintf(stderr,"lasexample_write_full_waveform > out.las\n");
  fprintf(stderr,"lasexample_write_full_waveform -h\n");
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

int main(int argc, char *argv[])
{
  int i, j;
  bool verbose = false;
  bool very_verbose = false;
  bool waveform = true;
  int num_points = 1;
  int num_returns = 3;
  double start_time = 0.0;

  LASwriteOpener laswriteopener;
  LASwaveform13writer* laswaveform13writer = 0;

  if (argc == 1)
  {
    fprintf(stderr,"%s is better run in the command line\n", argv[0]);
    char file_name[256];
    fprintf(stderr,"enter output file: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    laswriteopener.set_file_name(file_name);
  }
  else
  {
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
    else if (strcmp(argv[i],"-vv") == 0 || strcmp(argv[i],"-very_verbose") == 0)
    {
      verbose = true;
      very_verbose = true;
    }
    else if (i == argc - 1 && !laswriteopener.active())
    {
      laswriteopener.set_file_name(argv[i]);
    }
    else if (strcmp(argv[i],"-no_waveform") == 0 || strcmp(argv[i],"-no_waveforms") == 0)
    {
      waveform = false;
    }
    else if (strcmp(argv[i],"-num_points") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: number\n", argv[i]);
        byebye(true);
      }
      if (sscanf(argv[i+1], "%d", &num_points) != 1)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: number but '%s' is not a valid number\n", argv[i], argv[i+1]);
        byebye(true);
      }
      if (num_points <= 0)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: number but '%d' is not a valid number\n\n", argv[i], num_points);
        byebye(true);
      }
    }
    else if (strcmp(argv[i],"-num_returns") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: number\n", argv[i]);
        byebye(true);
      }
      if (sscanf(argv[i+1], "%d", &num_returns) != 1)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: number but '%s' is not a valid number\n", argv[i], argv[i+1]);
        byebye(true);
      }
      if (num_returns <= 0)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: number but '%d' is not a valid number\n\n", argv[i], num_returns);
        byebye(true);
      }
    }
    else
    {
      fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
      usage();
    }
  }

  if (verbose) start_time = taketime();

  // check output

  if (!laswriteopener.active())
  {
    fprintf(stderr,"ERROR: no output specified\n");
    usage(argc == 1);
  }

  // init header

  LASheader lasheader;
  sprintf(lasheader.generating_software, "LASlib of LAStools");
  sprintf(lasheader.system_identifier, "simple waveform example");
  lasheader.file_creation_day = 20;
  lasheader.file_creation_year = 2019;
  lasheader.x_scale_factor = 0.01;
  lasheader.y_scale_factor = 0.01;
  lasheader.z_scale_factor = 0.01;
  lasheader.x_offset =  500000.0;
  lasheader.y_offset = 4100000.0;
  lasheader.z_offset = 0.0;
  if (waveform)
  {
    lasheader.global_encoding = 0x05; // external waveform data packets 0x04 | adjusted standard GPS time 0x01
    lasheader.version_major = 1;
    lasheader.version_minor = 3;
    lasheader.header_size = 235; // 8 more bytes to specify start of waveform packet data
    lasheader.offset_to_point_data = 235;
    lasheader.point_data_format = 4; // essentially like format 1 but with wave packets
    lasheader.point_data_record_length = 28 + 29;
  }
  else
  {
    lasheader.global_encoding = 0x01; // adjusted standard GPS time 0x01
    lasheader.version_major = 1;
    lasheader.version_minor = 2;
    lasheader.header_size = 227;
    lasheader.offset_to_point_data = 227;
    lasheader.point_data_format = 1;
    lasheader.point_data_record_length = 28;
  }
    
  // update header with waveform information based on example from ASPRS wiki at:
  // http://github.com/ASPRSorg/LAS/wiki/Waveform-Data-Packet-Descriptors-Explained
    
  I32 idx = -1;
  LASvlr_wave_packet_descr* wave_packet_descr = 0;

  if (waveform)
  {
    // create an array of 256 wave packet descriptors (we only store two and use one)
    lasheader.vlr_wave_packet_descr = new LASvlr_wave_packet_descr*[256];
    if (lasheader.vlr_wave_packet_descr == 0)
    {
      fprintf(stderr, "ERROR: could not allocate LASvlr_wave_packet_descr*[256] array\n");
      byebye(argc==1);
    }
    for (j = 0; j < 256; j++) lasheader.vlr_wave_packet_descr[j] = (LASvlr_wave_packet_descr*)NULL;

    // no wave packet descriptor with index 0 is allowed to exist. always zero pointer. 
    idx = 0;
    lasheader.vlr_wave_packet_descr[idx] = (LASvlr_wave_packet_descr*)0;

    // create first wave packet descriptor (the one we use)
    idx = 1;
    lasheader.vlr_wave_packet_descr[idx] = new LASvlr_wave_packet_descr;

    // initialize first descriptor as per ASPRSorg Wiki example
    if (lasheader.vlr_wave_packet_descr[idx])
    {
      U8 nbits = 8;
      U8 compression = 0;
      U32 nsamples = 88;
      U32 temporalSpacing = 1000;  // ps
      F64 digitizerGain   = 0.0086453;
      F64 digitizerOffset = -0.1326341;
      lasheader.vlr_wave_packet_descr[idx]->setBitsPerSample(nbits) ;
      lasheader.vlr_wave_packet_descr[idx]->setCompressionType(compression);
      lasheader.vlr_wave_packet_descr[idx]->setNumberOfSamples(nsamples);
      lasheader.vlr_wave_packet_descr[idx]->setTemporalSpacing(temporalSpacing);
      lasheader.vlr_wave_packet_descr[idx]->setDigitizerGain(digitizerGain);
      lasheader.vlr_wave_packet_descr[idx]->setDigitizerOffset(digitizerOffset);

      // create a VLR for the first wave packet descriptor
      lasheader.add_vlr("LASF_Spec", 99+idx, sizeof(LASvlr_wave_packet_descr), (U8*)(lasheader.vlr_wave_packet_descr[idx]), FALSE, "Wave Packet Descr. 1", FALSE);
    }
    else
    {
      fprintf(stderr, "ERROR: could not allocate lasheader.vlr_wave_packet_descr[%d]\n", idx);
      byebye(argc==1);
    }

    // create second wave packet descriptor (the one we do *not* use)
    idx = 2;
    lasheader.vlr_wave_packet_descr[idx] = new LASvlr_wave_packet_descr;

    // initialize first descriptor as per ASPRSorg Wiki example
    if (lasheader.vlr_wave_packet_descr[idx])
    {
      U8 nbits = 8;
      U8 compression = 0;
      U32 nsamples = 96;
      U32 temporalSpacing = 1000;  // ps
      F64 digitizerGain   = 0.0086453;
      F64 digitizerOffset = -0.1326341;
      lasheader.vlr_wave_packet_descr[idx]->setBitsPerSample(nbits) ;
      lasheader.vlr_wave_packet_descr[idx]->setCompressionType(compression);
      lasheader.vlr_wave_packet_descr[idx]->setNumberOfSamples(nsamples);
      lasheader.vlr_wave_packet_descr[idx]->setTemporalSpacing(temporalSpacing);
      lasheader.vlr_wave_packet_descr[idx]->setDigitizerGain(digitizerGain);
      lasheader.vlr_wave_packet_descr[idx]->setDigitizerOffset(digitizerOffset);

      // create a VLR for the second wave packet descriptor
      lasheader.add_vlr("LASF_Spec", 99+2, sizeof(LASvlr_wave_packet_descr), (U8*)(lasheader.vlr_wave_packet_descr[idx]), FALSE, "Wave Packet Descr. 2 (unused)", FALSE);
    }
    else
    {
      fprintf(stderr, "ERROR: could not allocate lasheader.vlr_wave_packet_descr[%d]\n", idx);
      byebye(argc==1);
    }

    // in the following we use only the first wave packet descriptor

    idx = 1;
    wave_packet_descr = lasheader.vlr_wave_packet_descr[idx];
  }

  // update wave packets and waveform information based on example from ASPRS wiki at:
  // http://github.com/ASPRSorg/LAS/wiki/Waveform-Data-Packet-Descriptors-Explained

  F32 location[] = {23355.3f, 49297.5f, 78432.8f};           // ps
  F32 XYZt[3] =  {-4.9922e-5f, 5.89773e-6f, 0.000141175f};   // (m/ps) No refraction
  F64 XYZreturn[3];                                          // location of returns
  F64 XYZS0[3];                                              // start of digitized waveform
  U8* samples = NULL;
  U64 offset;
  U32 size;

  if (waveform)
  {
    // find start of first return: S0 = R0 + L1 * V
    //    where S0 = {sx, sy, sz}
    //          R0 = {px, py, pz}
    //          V  = {dx,dy,dz}
    //          L0 = return point location L_0 (return 1 of N)
    
    // find end of first return : E0 = S0 - Ns * dt * V
    //    where E0 = {ex, ey, ez}
    //          S0 = {sx, sy, sz}  (from above)
    //          V  = {dx,dy,dz}
    //          Ns = nsamples
    //          dt = temporalSpacing

    // each return itself
    // if you have S0, then Ri = S0 - Li*V  for any i

    // HACK: using ASPRS example data, back-calculate start of digitized waveform
    XYZS0[0] =  235374.66 + XYZt[0]*location[0];
    XYZS0[1] = 5800928.34 + XYZt[1]*location[0];
    XYZS0[2] =     276.50 + XYZt[2]*location[0];

    // allocate waveform data storage
    if (samples) delete [] samples;
    size = (wave_packet_descr->getBitsPerSample()/8) * wave_packet_descr->getNumberOfSamples();
    samples = new U8[size];
    memset(samples, 0, size);
    
    // create a fake spike sample at time / location of each return
    for (j = 0; j < num_returns; j++)
    {
      I32 ireturn = I32_QUANTIZE((F32)location[j] / (F32)wave_packet_descr->getTemporalSpacing());
      samples[ireturn-4] = (1<<4) * (num_returns - j);  // Fake
      samples[ireturn-3] = (1<<5) * (num_returns - j);  // Fake
      samples[ireturn-2] = (1<<5) * (num_returns - j);  // Fake
      samples[ireturn-1] = (1<<6) * (num_returns - j);  // Fake
      samples[ireturn] = (1<<6) * (num_returns - j);    // Fake
      samples[ireturn+1] = (1<<6) * (num_returns - j);  // Fake
      samples[ireturn+2] = (1<<5) * (num_returns - j);  // Fake
      samples[ireturn+3] = (1<<5) * (num_returns - j);  // Fake
      samples[ireturn+4] = (1<<4) * (num_returns - j);  // Fake
      if (very_verbose) fprintf(stderr, "samples[%d] = %x\n", ireturn, samples[ireturn]);
    }
  }

  // init point

  LASpoint laspoint;
  laspoint.init(&lasheader, lasheader.point_data_format, lasheader.point_data_record_length, 0);

  // open laswriter

  LASwriter* laswriter = laswriteopener.open(&lasheader);
  if (laswriter == 0)
  {
    fprintf(stderr, "ERROR: could not open laswriter\n");
    byebye(argc==1);
  }
    
  // open waveformwriter after laswriter

  if (waveform)
  {
    laswaveform13writer = laswriteopener.open_waveform13(&lasheader);
    if (laswaveform13writer == 0)
    {
      fprintf(stderr, "ERROR: could not open laswaveform13writer\n");
      byebye(argc==1);
    }
  }
    
  if (verbose) fprintf(stderr, "writing %d points with %d returns to '%s'.\n", num_points, num_returns, laswriteopener.get_file_name());

  // write points

  for (i = 0; i < num_points; i++)
  {
    for (j = 0; j< num_returns; j++)
    {
      // populate the point
      
      laspoint.set_intensity((U16)i);
      laspoint.set_gps_time(23365829.0 + 0.0006*i);
      
      if (waveform)
      {
        // add waveform attributes

        XYZreturn[0] = XYZS0[0] - location[j]*XYZt[0];   // notice minus sign
        XYZreturn[1] = XYZS0[1] - location[j]*XYZt[1];   // notice minus sign
        XYZreturn[2] = XYZS0[2] - location[j]*XYZt[2];   // notice minus sign
        laspoint.set_x(XYZreturn[0]);
        laspoint.set_y(XYZreturn[1]);
        laspoint.set_z(XYZreturn[2]);
        laspoint.set_return_number((U8)j+1);
        laspoint.set_number_of_returns((U8)num_returns);

        laspoint.wavepacket.setIndex((U8) idx);
        // laspoint.wavepacket.setOffset(offset) // set by LASwaveform13writer::write_waveform
        // laspoint.wavepacket.setSize(size)     // set by LASwaveform13writer::write_waveform
        laspoint.wavepacket.setLocation(location[j]);
        laspoint.wavepacket.setXt(XYZt[0]);
        laspoint.wavepacket.setYt(XYZt[1]);
        laspoint.wavepacket.setZt(XYZt[2]);
          
        if (very_verbose) fprintf(stderr, "%u t %g x %g y %g z %g i %d (%d of %d) d %d e %d c %d s %d %u p %d \012", (U32)i, laspoint.get_gps_time(), laspoint.get_x(), laspoint.get_y(), laspoint.get_z(), laspoint.get_intensity(), laspoint.get_return_number(), laspoint.get_number_of_returns(), laspoint.get_scan_direction_flag(), laspoint.get_edge_of_flight_line(), laspoint.get_classification(), laspoint.get_scan_angle_rank(), laspoint.get_user_data(), laspoint.get_point_source_ID());
      }
      else
      {
        laspoint.set_X(i);
        laspoint.set_Y(i);
        laspoint.set_Z(i);
      }
            
      // write the digitized waveform only on the first return because all three 
      // returns are sharing this *one* digitized waveform (which is stored in 
      // in the WDP or "waveform data packets" file). the write_waveform() call
      // also calculates the wavepacket offset and size which will stay the same
      // for subsequent when writing all subsequent returns.

      if (waveform && (j == 0))
      {
        laswaveform13writer->write_waveform(&laspoint, samples);
      }
      offset = laspoint.wavepacket.getOffset();
      size = laspoint.wavepacket.getSize();

      if (very_verbose) fprintf(stderr, "offset: %u, size: %d\n", (U32)offset, size);

      // write the point
      
      laswriter->write_point(&laspoint);
      
      // add it to the inventory
      
      laswriter->update_inventory(&laspoint);
    }
  }

  // update the header

  laswriter->update_header(&lasheader, TRUE);
    
  // close the writer

  I64 total_bytes = laswriter->close();

  if (waveform)
  {
    laswaveform13writer->close();
  }  

#ifdef _WIN32
  if (verbose) fprintf(stderr,"total time: %g sec %I64d bytes for %I64d points\n", taketime()-start_time, total_bytes, laswriter->npoints);
#else
  if (verbose) fprintf(stderr,"total time: %g sec %lld bytes for %lld points\n", taketime()-start_time, total_bytes, laswriter->npoints);
#endif

  delete laswriter;

  return 0;
}
