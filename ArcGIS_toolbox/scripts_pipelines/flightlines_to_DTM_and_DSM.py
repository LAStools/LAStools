#
# flightlines_to_DTM_and_DSM.py
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
gp.AddMessage("Starting flightlines_to_DTM_and_DSM ...")

### define positions of arguments in argv array
arg_input_folder     =  1
arg_tile_size        =  2
arg_buffer           =  3
arg_terrain_type     =  4
arg_step             =  5
arg_cores            =  6
arg_empty_temp_dir   =  7
arg_output_dir       =  8
arg_output_base_name =  9
arg_output_rformat   = 10
arg_output_pformat   = 11
arg_verbose          = 12
arg_count_needed     = 13

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
output_base_name = sys.argv[arg_output_base_name]

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

### create the full path to the lastile executable
lastile_path = lastools_path+"\\lastile.exe"

### check if the lastile executable exists
if os.path.exists(lastile_path) == False:
    gp.AddMessage("Cannot find lastile.exe at " + lastile_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lastile_path + " ...")

### create the full path to the lasground executable
lasground_path = lastools_path+"\\lasground.exe"

### check if the lasground executable exists
if os.path.exists(lasground_path) == False:
    gp.AddMessage("Cannot find lasground.exe at " + lasground_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lasground_path + " ...")

### create the full path to the las2dem executable
las2dem_path = lastools_path+"\\las2dem.exe"

### check if the las2dem executable exists
if os.path.exists(las2dem_path) == False:
    gp.AddMessage("Cannot find las2dem.exe at " + las2dem_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + las2dem_path + " ...")

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
### first step: tile folder of flightlines
###################################################

### create the command string for lastile.exe
command = ['"'+lastile_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+sys.argv[arg_input_folder]+'\\*.las"')
command.append("-i")
command.append('"'+sys.argv[arg_input_folder]+'\\*.laz"')

### they are flight lines
command.append("-files_are_flightlines")

### maybe use a user-defined tile size
if sys.argv[arg_tile_size] != "1000":
    command.append("-tile_size")
    command.append(sys.argv[arg_tile_size].replace(",","."))

### maybe create a buffer around the tiles
if sys.argv[arg_buffer] != "0":
    command.append("-buffer")
    command.append(sys.argv[arg_buffer].replace(",","."))

### an empty temp directory must have been selected
if empty_temp_dir != "#":
    command.append("-odir")
    command.append('"'+empty_temp_dir+'"')
else:
    gp.AddMessage("Error. no empty temp directory was specified.")
    sys.exit(1)

### use default if base name not given
if output_base_name == "#":
    output_base_name = "tile"

### give tiles a base name
command.append("-o")
command.append('"' + output_base_name + '.laz"')

### store temporary tiles in compressed format
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

### report output of lastile
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. flightlines_to_DTM_and_DSM failed in lastile step.")
    sys.exit(1)

### report success
gp.AddMessage("lastile step done.")

###################################################
### second step: ground classify each tile
###################################################

### create the command string for lasground.exe
command = ['"'+lasground_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"' + empty_temp_dir + "\\" + output_base_name + "*.laz" + '"')

### what type of terrain do we have
if sys.argv[arg_terrain_type] == "wilderness":
    command.append("-wilderness")
elif sys.argv[arg_terrain_type] == "city or warehouses":
    command.append("-city")
    command.append("-extra_fine")
elif sys.argv[arg_terrain_type] == "towns or flats":
    command.append("-town")
    command.append("-fine")
elif sys.argv[arg_terrain_type] == "metropolis":
    command.append("-metro")
    command.append("-ultra_fine")

### give ground-classified tiles a meaningful appendix
command.append("-odix")
command.append("_g")

### store ground-classified tiles in compressed format
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

### report output of lasground
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. flightlines_to_DTM_and_DSM failed in lasground step.")
    sys.exit(1)

### report success
gp.AddMessage("lasground step done.")

###################################################
### third step: raster the DTMs
###################################################

### create the command string for las2dem.exe
command = ['"'+las2dem_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"' + empty_temp_dir + "\\" + output_base_name + "*_g.laz" + '"')

### filter ground points
command.append("-keep_class")
command.append("2")

### do an extra pass for smaller memory footprints
command.append("-extra_pass")

### raster tile with requested step
command.append("-step")
command.append(sys.argv[arg_step].replace(",","."))

### raster only tile interiors
command.append("-use_tile_bb")

### store rastered tiles in requested directory
command.append("-odir")
command.append(sys.argv[arg_output_dir])

### give the tiles a meaningful appendix
command.append("-ocut")
command.append("2")
command.append("-odix")
command.append("_dtm")

### store rastered tiles in requested format
command.append("-o" + sys.argv[arg_output_rformat])

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

### report output of las2dem 
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. flightlines_to_DTM_and_DSM failed in las2dem (DTM) step.")
    sys.exit(1)

### report success
gp.AddMessage("las2dem (DTM) step done.")

###################################################
### fourth step: raster the DSMs
###################################################

### create the command string for las2dem.exe
command = ['"'+las2dem_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"' + empty_temp_dir + "\\" + output_base_name + "*_g.laz" + '"')

### filter first returns
command.append("-first_only")

### do an extra pass for smaller memory footprints
command.append("-extra_pass")

### raster tile with requested step
command.append("-step")
command.append(sys.argv[arg_step].replace(",","."))

### raster only tile interiors
command.append("-use_tile_bb")

### store rastered tiles in requested directory
command.append("-odir")
command.append(sys.argv[arg_output_dir])

### give the tiles a meaningful appendix
command.append("-ocut")
command.append("2")
command.append("-odix")
command.append("_dsm")

### store rastered tiles in requested format
command.append("-o" + sys.argv[arg_output_rformat])

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

### report output of las2dem 
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. flightlines_to_DTM_and_DSM failed in las2dem (DSM) step.")
    sys.exit(1)

### report success
gp.AddMessage("las2dem (DSM) step done.")

###################################################
### fifth step: remove buffers from tiles
###################################################

### create the command string for lastile.exe
command = ['"'+lastile_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"' + empty_temp_dir + "\\" + output_base_name + "*_g.laz" + '"')

### use mode remove buffer
command.append("-remove_buffer")

### store rastered tiles in requested directory
command.append("-odir")
command.append(sys.argv[arg_output_dir])

### give the tiles a meaningful appendix
command.append("-ocut")
command.append("2")

### store classified tiles in requested format
command.append("-o" + sys.argv[arg_output_pformat])

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

### report output of lastile (remove)
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. flightlines_to_DTM_and_DSM failed in lastile (remove) step.")
    sys.exit(1)

### report success
gp.AddMessage("lastile (remove) step done.")

###################################################
### final step: clean-up all temporary files
###################################################

### create the command string for clean-up
command = ["del"]

### add temporary files wildcard
command.append('"' + empty_temp_dir + "\\" + output_base_name + "*.laz" + '"')

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
    gp.AddMessage("Error. flightlines_to_DTM_and_DSM failed in clean-up step.")
    sys.exit(1)

### report success
gp.AddMessage("clean-up step done.")

### report happy end
gp.AddMessage("Success. flightlines_to_DTM_and_DSM done.")
