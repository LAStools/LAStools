#
# huge_file_sort_GPS_time.py
#
# (c) 2014, martin isenburg - http://rapidlasso.com
#     rapidlasso GmbH - fast tools to catch reality
#
# This LAStools pipeline sorts a huge LAS or LAZ
# files by operating with a chunk-based multi-core
# pipeline (i.e. bucket sort). The input file is
# first split into different GPS segments using
# lassplit with the specified duration in seconds.
# The points of each chunk are then sorted based
# on their GPS time stamps. The sorted chunks are
# then merged back into a single and all temporary
# chunks are deleted.
#
# LiDAR input:   LAS/LAZ/BIN/TXT/SHP/BIL/ASC/DTM
# LiDAR output:  LAS/LAZ/BIN/TXT
#
# for licensing see http://lastools.org/LICENSE.txt
#

import sys, os, arcgisscripting, subprocess

def check_output(command,console):
    if console == True:
        process = subprocess.Popen(command)
    else:
        process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
    output,error = process.communicate()
    returncode = process.poll()
    return returncode,output 

### create the geoprocessor object
gp = arcgisscripting.create(9.3)

### report that something is happening
gp.AddMessage("Starting huge_file_sort_GPS_time ...")

### define positions of arguments in argv array
arg_input_file     =  1
arg_time_interval  =  2
arg_cores          =  3
arg_empty_temp_dir =  4
arg_output_file    =  5
arg_output_format  =  6
arg_verbose        =  7
arg_count_needed   =  8

### get number of arguments
argc = len(sys.argv)

### make sure we have right number of arguments
if argc != arg_count_needed:
    gp.AddMessage("Error. Wrong number of arguments. Got " + str(argc) + " expected " + str(arg_count_needed))
    sys.exit(1)    

### report arguments (for debug)
#gp.AddMessage("Arguments:")
#for i in range(0, argc):
#    gp.AddMessage("[" + str(i) + "]" + sys.argv[i])

### get selected arguments
empty_temp_dir = sys.argv[arg_empty_temp_dir]

### get the path to LAStools
lastools_path = os.path.dirname(os.path.dirname(os.path.dirname(sys.argv[0])))

### make sure the path does not contain spaces
if lastools_path.count(" ") > 0:
    gp.AddMessage("Error. Path to .\\lastools installation contains spaces.")
    gp.AddMessage("This does not work: " + lastools_path)
    gp.AddMessage("This would work:    C:\\software\\lastools")
    sys.exit(1)    

### make sure the path does not contain open or closing brackets
if (lastools_path.count("(") > 0) or (lastools_path.count(")") > 0):
    gp.AddMessage("Error. Path to .\\lastools installation contains brackets.")
    gp.AddMessage("This does not work: " + lastools_path)
    gp.AddMessage("This would work:    C:\\software\\lastools")
    sys.exit(1)   

### complete the path to where the LAStools executables are
lastools_path = lastools_path + "\\bin"

### check if path exists
if os.path.exists(lastools_path) == False:
    gp.AddMessage("Cannot find .\\lastools\\bin at " + lastools_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lastools_path + " ...")

### create the full path to the lassplit executable
lassplit_path = lastools_path+"\\lassplit.exe"

### check if the lassplit executable exists
if os.path.exists(lassplit_path) == False:
    gp.AddMessage("Cannot find lassplit.exe at " + lassplit_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lassplit_path + " ...")

### create the full path to the lassort executable
lassort_path = lastools_path+"\\lassort.exe"

### check if the lassort executable exists
if os.path.exists(lassort_path) == False:
    gp.AddMessage("Cannot find lassort.exe at " + lassort_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lassort_path + " ...")

### create the full path to the lasmerge executable
lasmerge_path = lastools_path+"\\lasmerge.exe"

### check if the lasmerge executable exists
if os.path.exists(lasmerge_path) == False:
    gp.AddMessage("Cannot find lasmerge.exe at " + lasmerge_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lasmerge_path + " ...")

### check if the empty temp directory exists
if os.path.exists(empty_temp_dir) == False:
    gp.AddMessage("Cannot find empty temp dir " + empty_temp_dir)
    sys.exit(1)
else:
    gp.AddMessage("Found " + empty_temp_dir + " ...")

### make sure the empty temp directory is emtpy
if os.listdir(empty_temp_dir) != []:
    gp.AddMessage("Empty temp directory '" + empty_temp_dir + "' is not empty")
    sys.exit(1)
else:
    gp.AddMessage("And it's empty ...")

###################################################
### first step: split huge input file
###################################################

### create the command string for lassplit.exe
command = ['"'+lassplit_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+sys.argv[arg_input_file]+'"')

### specify how many seconds worth should go into one file
command.append("-by_gps_time_interval")
command.append(sys.argv[arg_time_interval].replace(",","."))

### use a generous amount of digits to number the file names
command.append("-digits")
command.append("8")

### maybe an output directory was selected
if empty_temp_dir != "#":
    command.append("-odir")
    command.append('"'+empty_temp_dir+'"')

### give temporary chunks a meaningful name
command.append("-o")
command.append("temp_huge_sort_GPS_time.laz")

### store temporary chunks in compressed format
command.append("-olaz")

### report command string
gp.AddMessage("LAStools command line:")
command_length = len(command)
command_string = str(command[0])
command[0] = command[0].strip('"')
for i in range(1, command_length):
    command_string = command_string + " " + str(command[i])
    command[i] = command[i].strip('"')
gp.AddMessage(command_string)

### run command
returncode,output = check_output(command, False)

### report output of lassplit
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. huge_file_sort_GPS_time failed in lassplit step.")
    sys.exit(1)

### report success
gp.AddMessage("lassplit step done.")

###################################################
### second step: sort each chunk by GPS time
###################################################

### create the command string for lassort.exe
command = ['"'+lassort_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
if empty_temp_dir != "#":
    command.append('"'+empty_temp_dir+"\\temp_huge_sort_GPS_time*.laz"+'"')
else:
    command.append("temp_huge_sort_GPS_time*.laz")

### specify by what we want to sort
command.append("-gps_time")

### give sorted chunks a meaningful appendix
command.append("-odix")
command.append("_s")

### store sorted chunks in compressed format
command.append("-olaz")

### maybe we should run on multiple cores
if sys.argv[arg_cores] != "1":
    command.append("-cores")
    command.append(sys.argv[arg_cores])

### report command string
gp.AddMessage("LAStools command line:")
command_length = len(command)
command_string = str(command[0])
command[0] = command[0].strip('"')
for i in range(1, command_length):
    command_string = command_string + " " + str(command[i])
    command[i] = command[i].strip('"')
gp.AddMessage(command_string)

### run command
returncode,output = check_output(command, False)

### report output of lassort
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. huge_file_sort_GPS_time failed in lassort step.")
    sys.exit(1)

### report success
gp.AddMessage("lassort step done.")

###################################################
### third step: merge sorted files into one output
###################################################

### create the command string for lasmerge.exe
command = ['"'+lasmerge_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
if empty_temp_dir != "#":
    command.append('"'+empty_temp_dir+"\\temp_huge_sort_GPS_time*_s.laz"+'"')
else:
    command.append("temp_huge_sort_GPS_time*_s.laz")

### maybe an output file name was selected
if sys.argv[arg_output_file] != "#":
    command.append("-o")
    command.append('"'+sys.argv[arg_output_file]+'"')

### maybe an output format was selected
if sys.argv[arg_output_format] != "#":
    if sys.argv[arg_output_format] == "las":
        command.append("-olas")
    elif sys.argv[arg_output_format] == "laz":
        command.append("-olaz")
    elif sys.argv[arg_output_format] == "bin":
        command.append("-obin")
    elif sys.argv[arg_output_format] == "xyzc":
        command.append("-otxt")
        command.append("-oparse")
        command.append("xyzc")
    elif sys.argv[arg_output_format] == "xyzci":
        command.append("-otxt")
        command.append("-oparse")
        command.append("xyzci")
    elif sys.argv[arg_output_format] == "txyzc":
        command.append("-otxt")
        command.append("-oparse")
        command.append("txyzc")
    elif sys.argv[arg_output_format] == "txyzci":
        command.append("-otxt")
        command.append("-oparse")
        command.append("txyzci")

### report command string
gp.AddMessage("LAStools command line:")
command_length = len(command)
command_string = str(command[0])
command[0] = command[0].strip('"')
for i in range(1, command_length):
    command_string = command_string + " " + str(command[i])
    command[i] = command[i].strip('"')
gp.AddMessage(command_string)

### run command
returncode,output = check_output(command, False)

### report output of lasmerge
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. lasmerge failed.")
    sys.exit(1)

### report success
gp.AddMessage("lasmerge step done.")

###################################################
### final step: clean-up all temporary files
###################################################

### create the command string for clean-up
command = ["del"]

### add temporary files wildcard
if empty_temp_dir != "#":
    command.append('"'+empty_temp_dir+"\\temp_huge_sort_GPS_time*.laz"+'"')
else:
    command.append("temp_huge_sort_GPS_time*.laz")

### report command string
gp.AddMessage("clean-up command line:")
command_length = len(command)
command_string = str(command[0])
command[0] = command[0].strip('"')
for i in range(1, command_length):
    command_string = command_string + " " + str(command[i])
    command[i] = command[i].strip('"')
gp.AddMessage(command_string)

### run command
returncode,output = check_output(command, False)

### report output of clean-up
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. huge_file_sort_GPS_time failed in clean-up step.")
    sys.exit(1)

### report success
gp.AddMessage("clean-up step done.")

### report happy end
gp.AddMessage("Success. huge_file_sort_GPS_time done.")
