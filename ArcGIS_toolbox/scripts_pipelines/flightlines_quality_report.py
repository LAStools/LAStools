#
# flightlines_quality_check.py
#
# (c) 2014, martin isenburg - http://rapidlasso.com
#     rapidlasso GmbH - fast tools to catch reality
#
# This LAStools pipeline classifies very large LAS
# or LAZ files by operating with a tile-based multi-
# core pipeline. The input file is first tiled using
# lastile with the specified tile size. The specified
# buffer is used to avoid edge artifacts. All tiles
# are then ground classified using lasground marking
# points as ground (class 2) and non-gound (class 1).
# Next the height of all points above the ground is
# computed using lasheight with an optional removal
# of points above a specified height. Then buildings
# and vegetation are classified using lasclassify.
# Finally the processed tiles are rejoined back into
# a single file with points in their original order
# and all temporary files are deleted.
#
# LiDAR input:   LAS/LAZ
# output:        TXT, XML, PNG/JPG/TIF, SHP/WKT/KML/TXT
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
gp.AddMessage("Starting flightlines_quality_check ...")

### define positions of arguments in argv array
arg_input_folder     =  1
arg_step             =  2
arg_max_diff         =  3
arg_expected         =  4
arg_excessive        =  5
arg_output_dir       =  6
arg_output_validate  =  7
arg_output_info      =  8
arg_output_overlap   =  9
arg_output_expected  = 10
arg_output_excessive = 11
arg_output_boundary  = 12
arg_verbose          = 13
arg_count_needed     = 14

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

### create the full path to the lasvalidate executable
lasvalidate_path = lastools_path+"\\lasvalidate.exe"

### check if the lasvalidate executable exists
if os.path.exists(lasvalidate_path) == False:
    gp.AddMessage("Cannot find lasvalidate.exe at " + lasvalidate_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lasvalidate_path + " ...")

### create the full path to the lasinfo executable
lasinfo_path = lastools_path+"\\lasinfo.exe"

### check if the lasinfo executable exists
if os.path.exists(lasinfo_path) == False:
    gp.AddMessage("Cannot find lasinfo.exe at " + lasinfo_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lasinfo_path + " ...")

### create the full path to the lasoverlap executable
lasoverlap_path = lastools_path+"\\lasoverlap.exe"

### check if the lasoverlap executable exists
if os.path.exists(lasoverlap_path) == False:
    gp.AddMessage("Cannot find lasoverlap.exe at " + lasoverlap_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lasoverlap_path + " ...")

### create the full path to the lasgrid executable
lasgrid_path = lastools_path+"\\lasgrid.exe"

### check if the lasgrid executable exists
if os.path.exists(lasgrid_path) == False:
    gp.AddMessage("Cannot find lasgrid.exe at " + lasgrid_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lasgrid_path + " ...")

### create the full path to the lasboundary executable
lasboundary_path = lastools_path+"\\lasboundary.exe"

### check if the lasboundary executable exists
if os.path.exists(lasboundary_path) == False:
    gp.AddMessage("Cannot find lasboundary.exe at " + lasboundary_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lasboundary_path + " ...")

###################################################
### first step: validate the LAS or LAZ files
###################################################

### create the command string for lasvalidate.exe
command = ['"'+lasvalidate_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+sys.argv[arg_input_folder]+'\\*.las"')
command.append("-i")
command.append('"'+sys.argv[arg_input_folder]+'\\*.laz"')

### specify the output file
command.append("-o")
command.append('"'+sys.argv[arg_output_dir]+"\\"+sys.argv[arg_output_validate]+'"')

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

### report output of lasvalidate
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. flightlines_quality_check failed in lasvalidate step.")
    sys.exit(1)

### report success
gp.AddMessage("lasvalidate step done.")

###################################################
### second step: info report for merged files
###################################################

### create the command string for lasinfo.exe
command = ['"'+lasinfo_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+sys.argv[arg_input_folder]+'\\*.las"')
command.append("-i")
command.append('"'+sys.argv[arg_input_folder]+'\\*.laz"')

### merge them into one input
command.append("-merged")

### compute average point density
command.append("-cd")

### specify the output directory
command.append("-odir")
command.append('"'+sys.argv[arg_output_dir]+'"')

### specify the output file
command.append("-o")
command.append('"'+sys.argv[arg_output_info]+'"')

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

### report output of lasinfo
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. flightlines_quality_check failed in lasinfo step.")
    sys.exit(1)

### report success
gp.AddMessage("lasinfo step done.")

###################################################
### third step: compute overlap images
###################################################

### create the command string for lasoverlap.exe
command = ['"'+lasoverlap_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+sys.argv[arg_input_folder]+'\\*.las"')
command.append("-i")
command.append('"'+sys.argv[arg_input_folder]+'\\*.laz"')

### merge the files as different flightlines into one input
command.append("-files_are_flightlines")

### desired step size for overlap computations
command.append("-step")
command.append(sys.argv[arg_step].replace(",","."))

### desired max_diff for coloring the difference images
command.append("-max_diff")
command.append(sys.argv[arg_max_diff].replace(",","."))

### specify the output directory
command.append("-odir")
command.append('"'+sys.argv[arg_output_dir]+'"')

### specify the output file
command.append("-o")
command.append('"'+sys.argv[arg_output_overlap]+'"')

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

### report output of lasoverlap
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. flightlines_quality_check failed in lasoverlap step.")
    sys.exit(1)

### report success
gp.AddMessage("lasoverlap step done.")

###################################################
### fourth step: lasgrid (expected)
###################################################

### create the command string for lasgrid.exe
command = ['"'+lasgrid_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+sys.argv[arg_input_folder]+'\\*.las"')
command.append("-i")
command.append('"'+sys.argv[arg_input_folder]+'\\*.laz"')

### merge them into one input
command.append("-merged")

### keep only the last return
command.append("-last_only")

### desired step size for counting last returns
command.append("-step")
command.append(sys.argv[arg_step].replace(",","."))

### count the points per area of size step x step
command.append("-counter_16bit")

### map these counts to false colors
command.append("-false")

### desired min max range for mapping to false colors
command.append("-set_min_max")
command.append("0")
command.append(sys.argv[arg_expected].replace(",","."))

### specify the output directory
command.append("-odir")
command.append('"'+sys.argv[arg_output_dir]+'"')

### specify the output file
command.append("-o")
command.append('"'+sys.argv[arg_output_expected]+'"')

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

### report output of lasgrid
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. flightlines_quality_check failed in lasgrid (expected) step.")
    sys.exit(1)

### report success
gp.AddMessage("lasgrid (expected) step done.")

###################################################
### fifth step: lasgrid (excessive)
###################################################

### create the command string for lasgrid.exe
command = ['"'+lasgrid_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+sys.argv[arg_input_folder]+'\\*.las"')
command.append("-i")
command.append('"'+sys.argv[arg_input_folder]+'\\*.laz"')

### merge them into one input
command.append("-merged")

### keep only the last return
command.append("-last_only")

### desired step size for counting last returns
command.append("-step")
command.append(sys.argv[arg_step].replace(",","."))

### count the points per area of size step x step
command.append("-counter_16bit")

### map these counts to false colors
command.append("-false")

### desired min max range for mapping to false colors
command.append("-set_min_max")
command.append("0")
command.append(sys.argv[arg_excessive].replace(",","."))

### specify the output directory
command.append("-odir")
command.append('"'+sys.argv[arg_output_dir]+'"')

### specify the output file
command.append("-o")
command.append('"'+sys.argv[arg_output_excessive]+'"')

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

### report output of lasgrid
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. flightlines_quality_check failed in lasgrid (excessive) step.")
    sys.exit(1)

### report success
gp.AddMessage("lasgrid (excessive) step done.")

###################################################
### sixth step: create boundary polygon (+ holes)
###################################################

### create the command string for lasboundary.exe
command = ['"'+lasboundary_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+sys.argv[arg_input_folder]+'\\*.las"')
command.append("-i")
command.append('"'+sys.argv[arg_input_folder]+'\\*.laz"')

### merge them into one input
command.append("-merged")

### keep only the last return
command.append("-last_only")

### request holes
command.append("-holes")

### allow the polygon to be disjoint
command.append("-disjoint")

### specify the output directory
command.append("-odir")
command.append('"'+sys.argv[arg_output_dir]+'"')

### specify the output file
command.append("-o")
command.append('"'+sys.argv[arg_output_boundary]+'"')

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

### report output of lasboundary
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. flightlines_quality_check failed in lasboundary step.")
    sys.exit(1)

### report success
gp.AddMessage("lasboundary step done.")

### report happy end
gp.AddMessage("Success. flightlines_quality_check done.")
