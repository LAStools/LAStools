/*
===============================================================================

  FILE:  lasvalidate.cpp
  
  CONTENTS:
  
    A tool to validate whether a LAS file conforms to the LAS specification

  PROGRAMMERS:
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2007-2016, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
     21 January 2026 -- The tool has been renewed. LAS version 1.5 check, as well as json, text and csv output format added
     2 August 2015 -- not failing but warning if OCG WRT has intentional empty payload 
    12 April 2015 -- not failing but warning for certain empty VLR payloads 
    20 March 2015 -- fail on files containing zero point records
    26 January 2015 -- more useful reports if CRS always missing with '-no_CRS_fail'
     3 September 2013 -- made open source after the ASPRS LVS contract fiasko
     1 April 2013 -- on Easter Monday all-nighting in Perth airport for PER->SYD
  
===============================================================================
*/
#include "mydefs.hpp"
#include "lastool.hpp"
#include "lasreader.hpp"
#include "laswriter.hpp"
#include "geoprojectionconverter.hpp"
#include "lasvalidate_result.hpp"
#include "validate_writer.hpp"
#include "format_writer.hpp" 
#include "lascheck.hpp"
#include "lasdefinitions.hpp"

#include <cstdint>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>

#define VALIDATE_VERSION  260319

class LasTool_lasvalidate : public LasTool {
 private:
 public:
  void usage() override {
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "lasvalidate -i lidar.las\n");
    fprintf(stderr, "lasvalidate -i lidar.laz -no_CRS_fail\n");
    fprintf(stderr, "lasvalidate -v -i lidar.las -o report.xml\n");
    fprintf(stderr, "lasvalidate -v -i lidar.laz -oxml\n");
    fprintf(stderr, "lasvalidate -vv -i tile1.las tile2.las tile3.las -oxml\n");
    fprintf(stderr, "lasvalidate -i tile1.laz tile2.laz tile3.laz -o summary.xml\n");
    fprintf(stderr, "lasvalidate -i *.las -no_CRS_fail -o report.xml\n");
    fprintf(stderr, "lasvalidate -i *.laz -o summary.xml\n");
    fprintf(stderr, "lasvalidate -i *.laz -tile_size 1000 -o summary.xml\n");
    fprintf(stderr, "lasvalidate -i *.las -oxml\n");
    fprintf(stderr, "lasvalidate -i c:\\data\\lidar.las -oxml\n");
    fprintf(stderr, "lasvalidate -i ..\\subfolder\\*.las -o summary.xml\n");
    fprintf(stderr, "lasvalidate -v -i ..\\..\\flight\\*.laz -o oxml\n");
    fprintf(stderr, "lasvalidate -h\n");
  }
};

static void write_version(ValidateWriter* writer) {
  std::ostringstream version_oss;
  version_oss << "lasvalidate (" << VALIDATE_VERSION << ") built with LAStools version " << LAS_TOOLS_VERSION;

  writer->write("version", version_oss.str());
}

static std::string get_command_line(int argc, char* argv[]) {
  std::ostringstream command_line_oss;
  for (int i = 0; i < argc; i++) {
    if (!argv[i]) continue;

    if (i > 0) command_line_oss << ' ';

    if (std::strchr(argv[i], ' '))
      command_line_oss << '"' << argv[i] << '"';
    else
      command_line_oss << argv[i];
  }

  return command_line_oss.str();
}

static void open_report_output(const LASwriteOpener& laswriteopener, FILE*& file_out, ValidateWriter*& writer, BOOL is_csv, BOOL consol_out) {
  if (consol_out == FALSE) {
    // open the text output file
    file_out = LASfopen(laswriteopener.get_file_name(), "w");
    
    if (file_out == nullptr) {
      LASMessage(LAS_WARNING, "could not open output text file '%s'", laswriteopener.get_file_name());
      file_out = stderr;
    }
  }
  
  // output logging
  
  writer = FormatWriterFactory::createWriter(laswriteopener.get_format(), file_out);

  if (writer == nullptr) {
    LASMessage(LAS_ERROR, "could not create writer to create validation report");
    return;
  }

  if (file_out != nullptr && !is_csv) writer->open("LASvalidator");
}

static double taketime() {
  return (double)(clock())/CLOCKS_PER_SEC;
}

#ifdef COMPILE_WITH_MULTI_CORE
extern void lasvalidate_multi_core(
    int argc, char* argv[], GeoProjectionConverter* geoprojectionconverter, LASreadOpener* lasreadopener, LASwriteOpener* laswriteopener, int cores,
    BOOL cpu64);
#endif

int main(int argc, char *argv[]) {
  LasTool_lasvalidate lastool;
  lastool.init(argc, argv, "lasvalidate");
  double start_time = 0.0;
  double full_start_time = 0.0;
  FILE* file_out = stderr;
  BOOL no_CRS_fail = FALSE;
  BOOL is_csv = FALSE;
  BOOL is_text = FALSE;
  BOOL is_xml = FALSE;
  F64 tile_size = 0.0;
  BOOL one_report_per_file = FALSE;
  U32 num_pass = 0;
  U32 num_fail = 0;
  U32 num_warning = 0;

  LASreadOpener lasreadopener;
  GeoProjectionConverter geoprojectionconverter;
  LASwriteOpener laswriteopener;

  std::string command_line = get_command_line(argc, argv);

  if (argc == 1) {
#ifdef COMPILE_WITH_GUI
    lasvalidate_gui(argc, argv, 0);
#else
    wait_on_exit();
    fprintf(stderr, "%s is better run in the command line\n", argv[0]);
    std::string file_name;
    fprintf(stderr, "enter input file: ");
    std::getline(std::cin, file_name);
    if (!file_name.empty() && (file_name.back() == '\n' || file_name.back() == '\r')) {
      file_name.pop_back();
    }
    lasreadopener.set_file_name(file_name.c_str());
#endif
  } else {
    for (int i = 1; i < argc; i++) {
      if ((unsigned char)argv[i][0] == 0x96) argv[i][0] = '-';
    }
    lasreadopener.parse(argc, argv);
    geoprojectionconverter.parse(argc, argv);
    laswriteopener.parse(argc, argv);
  }

  if (laswriteopener.is_piped()) {
    file_out = stdout;
  }

  U32 num_files = lasreadopener.get_file_name_number();

  if (num_files > 1) {
    halt_on_error(false);
    LASMessage(LAS_INFO, "more than 1 input files to check. '-errors_ignore' will be used by default for %s", lastool.name.c_str());
  }

  auto arg_local = [&](int& i) -> bool {
    if (strcmp(argv[i], "-quiet") == 0) {
      file_out = nullptr;
    } else if (strcmp(argv[i], "-txt") == 0 || strcmp(argv[i], "-TXT") == 0) {
      is_text = TRUE;
    } else if (strcmp(argv[i], "-csv") == 0 || strcmp(argv[i], "-CSV") == 0) {
      is_csv = TRUE;
    } else if (strcmp(argv[i], "-xml") == 0 || strcmp(argv[i], "-XML") == 0) {
      is_xml = TRUE;
    } else if (strcmp(argv[i], "-halt_on_errors") == 0) {
      halt_on_error(true);
    } else if (strcmp(argv[i], "-no_CRS_fail") == 0) {
      no_CRS_fail = TRUE;
    } else if (strcmp(argv[i], "-report_per_file") == 0) {
      one_report_per_file = TRUE;
    } else if (strcmp(argv[i], "-tile_size") == 0) {
      if ((i+1) >= argc)
      {
        laserror("'%s' needs at least 1 argument: tile size", argv[i]);
      }
      i++;
      tile_size = atof(argv[i]);
    } else if ((argv[i][0] != '-') && (lasreadopener.get_file_name_number() == 0)) {
      lasreadopener.add_file_name(argv[i]);
      argv[i][0] = '\0';
    } else {
      return false;
    }
    return true;
  };

  lastool.parse(arg_local);

#ifdef COMPILE_WITH_MULTI_CORE
  if (lastool.cores > 1) {
    if (one_report_per_file == FALSE) {
      LASMessage(LAS_WARNING, "more than 1 core will be used. Use '-report_per_file' to get output result files for %s", lastool.name.c_str());
    }
    if (lasreadopener.get_use_stdin()) {
      LASMessage(LAS_WARNING, "using stdin. ignoring '-cores %d' ...", lastool.cores);
    } else if (lasreadopener.get_file_name_number() < 2) {
      LASMessage(LAS_WARNING, "only %u input files. ignoring '-cores %d' ...", lasreadopener.get_file_name_number(), lastool.cores);
    } else if (lasreadopener.is_merged()) {
      LASMessage(LAS_WARNING, "input files merged on-the-fly. ignoring '-cores %d' ...", lastool.cores);
    } else {
      lasvalidate_multi_core(argc, argv, &geoprojectionconverter, &lasreadopener, &laswriteopener, lastool.cores, lastool.cpu64);
    }
  }
  if (lastool.cpu64) {
    lasvalidate_multi_core(argc, argv, &geoprojectionconverter, &lasreadopener, &laswriteopener, 1, TRUE);
  }
#endif

  // check input

  if (!lasreadopener.active()) {
    laserror("no input specified");
  }
  // Do not write log whose checks included in the validation report.
  lasreadopener.set_validation(TRUE);

  const I32 format = laswriteopener.get_format();
  BOOL consol_out = FALSE;

  if (laswriteopener.get_file_name() == nullptr && laswriteopener.get_directory() == nullptr && format != LAS_TOOLS_FORMAT_TXT &&
      format != LAS_TOOLS_FORMAT_JSON && format != LAS_TOOLS_FORMAT_XML && format != LAS_TOOLS_FORMAT_CSV) {
      consol_out = TRUE;
  }

  // accumulated pass

  ValidationStatus total_pass = ValidationStatus::passed;
  BOOL first_report = TRUE;
  if (is_csv == FALSE) is_csv = (laswriteopener.get_format() == LAS_TOOLS_FORMAT_CSV) ? TRUE : FALSE;

  full_start_time = taketime();

  // possibly loop over multiple input files

  ValidateWriter* writer = nullptr;

  while (lasreadopener.active()) {
    try {
      start_time = taketime();
      
      // open lasreader
      
      LASreader* lasreader = lasreadopener.open();

      // fileformula filtered
      if (lasreadopener.is_file_formula_filtered()) {
        LASMessage(LAS_VERBOSE, "file '%s' filtered by fileformula", lasreadopener.get_file_name());
        continue;
      }

      if (lasreader == nullptr) {
        laserror("cannot open lasreader");
        continue;
      }
      
      // get a pointer to the header
      LASheader* lasheader = &lasreader->header;
      ValidationResult results;
    
      // maybe we are doing one report per file

      if (one_report_per_file == TRUE) {
        if (lasreadopener.get_file_name()) {
          laswriteopener.make_file_name(lasreadopener.get_file_name(), -2);
        }
        // output logging
        open_report_output(laswriteopener, file_out, writer, is_csv, FALSE);
      } else if (first_report == TRUE) {
        if (laswriteopener.get_file_name() == nullptr) {
          if (laswriteopener.get_format() == LAS_TOOLS_FORMAT_DEFAULT) {
            std::string file_format = "json";

            if (is_csv == TRUE) {
              file_format = "csv";
            } else if (is_xml == TRUE) {
              file_format = "xml";
            } else if (is_text == TRUE) {
              file_format = "txt";
            }
            laswriteopener.set_format(file_format.c_str());

            if (laswriteopener.get_directory() == nullptr)
              laswriteopener.set_directory(lasreadopener.get_file_name_base());
          }
          laswriteopener.make_file_name("validation_reports.", -2);
        }
        // output logging
        open_report_output(laswriteopener, file_out, writer, is_csv, consol_out);
      }

      if (file_out != nullptr && writer != nullptr) {    
        // start a new report
        
        LASMessage(LAS_VERBOSE, "start the full report");
        if (!is_csv) {
          // write which validator was used
          write_version(writer);

          // write which command line was used
          if (!is_csv) writer->write("command_line", command_line);

          writer->begin("report", ValidateWriter::ContainerType::Array);
          
          // report description of file
          
          writer->beginsub("file");
          writer->write("name", lasreadopener.get_file_name_only());
          writer->write("path", lasreadopener.get_file_name_base());
        } else {
          writer->write("file", lasreadopener.get_file_name());
        }

        std::string version = std::to_string(static_cast<int>(lasheader->version_major)) + "." + std::to_string(static_cast<int>(lasheader->version_minor));
        writer->write("LAS_version", version.c_str());

        if (!is_csv) {
          std::string system_identifier = lasheader->system_identifier;
          writer->write("system_identifier", system_identifier.c_str());
          std::string generating_software = lasheader->generating_software;
          writer->write("generating_software", generating_software.c_str());
        }

        writer->write("point_data_format", static_cast<int>(lasheader->point_data_format));
        
        std::string crsdescription = "not valid or not specified";
        
        if (lasheader != nullptr) {
          // header was loaded. now parse and check.
        
          LAScheck lascheck(lasheader, &geoprojectionconverter);
        
          LASMessage(LAS_VERY_VERBOSE, "start with the point validation");
        
          while (lasreader->read_point()) {
            lascheck.check_parse(&lasreader->point, results);
          }

          LASMessage(LAS_VERY_VERBOSE, "start with the header validation");
        
          // check header and points and get CRS description
        
          lascheck.check(results, crsdescription, no_CRS_fail, tile_size);
        }
        
        writer->write("CRS", crsdescription);
        if (!is_csv) writer->endsub("file");    
        
        // report the verdict
        
        writer->write("summary", (results.status == ValidationStatus::passed ? "pass" : ((results.status == ValidationStatus::failed) ? "fail" : "warning")));
        
        // report details (if necessary)
        LASMessage(LAS_VERBOSE, "write the report details");
        
        if (results.status != ValidationStatus::passed) {
          if (!is_csv) writer->beginsub("details");

          for (size_t i = 0; i < results.fail_messages.size(); ++i) {
            const ValidationMessage& msg = results.fail_messages[i];
            writer->write(msg.key.c_str(), "fail", msg.note.c_str());
          }
          for (size_t i = 0; i < results.warning_messages.size(); ++i) {
            const ValidationMessage& msg = results.warning_messages[i];
            writer->write(msg.key.c_str(), "warning", msg.note.c_str());
          }

          if (!is_csv) writer->endsub("details");

          if (results.status == ValidationStatus::failed) {
            total_pass = ValidationStatus::failed;
          } else if (results.status == ValidationStatus::warning && total_pass == ValidationStatus::passed) {
            total_pass = ValidationStatus::warning;
          }
          if (results.status == ValidationStatus::failed) {
            num_fail++;
          } else {
            num_warning++;
          }
        } else {
          num_pass++;
        }
        
        // end the report
        
        if (!is_csv) writer->end("report");
        
        // maybe we are doing one report per file
        
        if (one_report_per_file == TRUE) {
          // report the total verdict
        
          if (!is_csv) {
            writer->begin("total");
            writer->write("files", num_files);
            writer->write("result", (total_pass == ValidationStatus::passed ? "pass" : (total_pass == ValidationStatus::failed ? "fail" : "warning")));
            writer->beginsub("file_summaries");
            writer->write("pass", num_pass);
            writer->write("warning", num_warning);
            writer->write("fail", num_fail);
            writer->endsub("file_summaries");
            writer->end("total");
        
            num_pass = 0;
            num_warning = 0;
            num_fail = 0;
          }
        
          // close the LASvalidator output file

          writer->end("LASvalidator");
          if (one_report_per_file == TRUE) writer->write_final();

          if (file_out && (file_out != stdout) && (file_out != stderr)) fclose(file_out);
          laswriteopener.set_file_name(nullptr);
        }
      }

      first_report = FALSE;
      writer->next_file();
      
      lasreader->close();
      delete lasreader;
      LASMessage(LAS_INFO, "needed %.2f sec for '%s' %s", taketime() - start_time, lasreadopener.get_file_name(),
          (results.status == ValidationStatus::passed ? "pass" : (results.status == ValidationStatus::failed ? "fail" : "warning")));
      start_time = taketime();
    }
    catch (...) {
      laserror("processing file '%s'. maybe file is corrupt?", lasreadopener.get_file_name());
    }
  }

  // maybe we are doing one summary report

  if (one_report_per_file == FALSE && writer != nullptr) {
    // report total 

    if (!is_csv) {
      writer->begin("total");
      writer->write("files", num_files);
      writer->write("result", (total_pass == ValidationStatus::passed ? "pass" : (total_pass == ValidationStatus::failed ? "fail" : "warning")));
      writer->beginsub("file_summaries");
      writer->write("pass", num_pass);
      writer->write("warning", num_warning);
      writer->write("fail", num_fail);
      writer->endsub("file_summaries");
      writer->end("total");
    }

    // close the LASvalidator XML file

    writer->end("LASvalidator");
    writer->write_final();

    if (file_out && (file_out != stdout) && (file_out != stderr)) fclose(file_out);
    laswriteopener.set_file_name(nullptr);
  }
  delete writer;

  if (lasreadopener.get_file_name_number() > 0) {
    LASMessage(LAS_VERBOSE, "done. total time %.2f sec. total %s (pass=%d,warning=%d,fail=%d)", taketime() - full_start_time,
        (total_pass == ValidationStatus::passed ? "pass" : (total_pass == ValidationStatus::failed ? "fail" : "warning")), 
        num_pass, num_warning, num_fail);
  }

  byebye();
}
