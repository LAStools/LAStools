#
# flightlines_to_single_CHM_pit_free.py
#
# (c) 2014, martin isenburg - http://rapidlasso.com
#     rapidlasso GmbH - fast tools to catch reality
#
# This LAStools pipeline turns a folder full of LAS
# or LAZ files (assumed to raw flightlines) into a
# single pit-free CHM using the algorithms described
# by A. Khosravipour et al. in Silvilaser 2013. The
# input file is first tiled using lastile with the
# specified tile size. The specified buffer is used
# to avoid edge artifacts. All tiles are then ground
# classified using lasground marking points as ground
# (class 2) and non-ground (class 1). Next the height
# of all points above the ground is computed using
# lasheight and used to height-normalize all the tiles
# in the sense that the height is used to replace the
# z coordinates. Using lasthin the tiles are then both
# thinned and splatted using the laser beam with in an
# attempt to widen the LiDAR returns a little bit. From
# these height-normalized and point-splatted tiles
# the partial CHMs are computed (as detailed in the
# poster, the extended abstract, and the paper) that
# are merged into a single CHM in the final step.
#
# LiDAR input:   LAS/LAZ
# raster output: TIF/IMG/BIL/DTM/ASC/FLT/XYZ
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
gp.AddMessage("Starting flightlines_to_single_CHM_pit_free ...")

### define positions of arguments in argv array
arg_input_folder   =  1
arg_tile_size      =  2
arg_buffer         =  3
arg_terrain_type   =  4
arg_beam_width     =  5
arg_step           =  6
arg_cores          =  7
arg_empty_temp_dir =  8
arg_output_file    =  9
arg_verbose        = 10
arg_count_needed   = 11

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

### create the full path to the lasheight executable
lasheight_path = lastools_path+"\\lasheight.exe"

### check if the lasheight executable exists
if os.path.exists(lasheight_path) == False:
    gp.AddMessage("Cannot find lasheight.exe at " + lasheight_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lasheight_path + " ...")

### create the full path to the lasheight executable
lasthin_path = lastools_path+"\\lasthin.exe"

### check if the lasthin executable exists
if os.path.exists(lasthin_path) == False:
    gp.AddMessage("Cannot find lasthin.exe at " + lasthin_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lasthin_path + " ...")

### create the full path to the las2dem executable
las2dem_path = lastools_path+"\\las2dem.exe"

### check if the las2dem executable exists
if os.path.exists(las2dem_path) == False:
    gp.AddMessage("Cannot find las2dem.exe at " + las2dem_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + las2dem_path + " ...")

### create the full path to the lasgrid executable
lasgrid_path = lastools_path+"\\lasgrid.exe"

### check if the lasgrid executable exists
if os.path.exists(lasgrid_path) == False:
    gp.AddMessage("Cannot find lasgrid.exe at " + lasgrid_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lasgrid_path + " ...")

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
### first step: tile folder of input files
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

### give tiles a simple name
command.append("-o")
command.append("pit_free_temp_tile.laz")

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
    gp.AddMessage("Error. flightlines_to_single_CHM_pit_free failed in lastile step.")
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
command.append('"'+empty_temp_dir+"\\pit_free_temp_tile*.laz"+'"')

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
    gp.AddMessage("Error. flightlines_to_single_CHM_pit_free failed in lasground step.")
    sys.exit(1)

### report success
gp.AddMessage("lasground step done.")

###################################################
### third step: height-normalize each tile
###################################################

### create the command string for lasheight.exe
command = ['"'+lasheight_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+empty_temp_dir+"\\pit_free_temp_tile*_g.laz"+'"')

### height normalize
command.append("-replace_z")

### give height-classified tiles a meaningful appendix
command.append("-odix")
command.append("h")

### store height-classified tiles in compressed format
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

### report output of lasheight
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. flightlines_to_single_CHM_pit_free failed in lasheight step.")
    sys.exit(1)

### report success
gp.AddMessage("lasheight step done.")

###################################################
### fourth step: splat and thin-highest each tile
###################################################

### create the command string for lasheight.exe
command = ['"'+lasthin_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+empty_temp_dir+"\\pit_free_temp_tile*_gh.laz"+'"')

### keep the highest
command.append("-highest")

### on a grid with two by two times the final step size
command.append("-step")
command.append(str(0.5*float(sys.argv[arg_step].replace(",","."))))

### maybe splat with half the laser beam width
if sys.argv[arg_beam_width] != "#":
    command.append("-subcircle")
    command.append(str(0.5*float(sys.argv[arg_beam_width].replace(",","."))))

### give thin-classified tiles a meaningful appendix
command.append("-odix")
command.append("t")

### store thin-classified tiles in compressed format
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

### report output of lasthin
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. flightlines_to_single_CHM_pit_free failed in lasthin step.")
    sys.exit(1)

### report success
gp.AddMessage("lasthin step done.")

###################################################
### fifth step: raster all partial CHMs
###################################################
### at level 00
###################################################

### create the command string for las2dem.exe
command = ['"'+las2dem_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+empty_temp_dir+"\\pit_free_temp_tile*_ght.laz"+'"')

### raster tile with requested step
command.append("-step")
command.append(sys.argv[arg_step])

### raster only tile interiors
command.append("-use_tile_bb")

### store rastered tiles in temporary directory
command.append("-odir")
command.append(empty_temp_dir)

### give the tiles a meaningful appendix
command.append("-ocut")
command.append("4")
command.append("-odix")
command.append("_chm00")

### store rastered tiles in BIL format
command.append("-obil")

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
    gp.AddMessage("Error. flightlines_to_single_CHM_pit_free failed in las2dem (CHM00) step.")
    sys.exit(1)

### report success
gp.AddMessage("las2dem (CHM00) step done.")

###################################################
### fifth step: raster all partial CHMs
###################################################
### at level 02
###################################################

### create the command string for las2dem.exe
command = ['"'+las2dem_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+empty_temp_dir+"\\pit_free_temp_tile*_ght.laz"+'"')

### remove all points below 2 meters
command.append("-drop_z_below")
command.append("2")

### raster tile with requested step
command.append("-step")
command.append(sys.argv[arg_step].replace(",","."))

### kill triangles that are three times the requested step (or bigger)
command.append("-kill")
command.append(str(3.0*float(sys.argv[arg_step].replace(",","."))))

### raster only tile interiors
command.append("-use_tile_bb")

### store rastered tiles in temporary directory
command.append("-odir")
command.append(empty_temp_dir)

### give the tiles a meaningful appendix
command.append("-ocut")
command.append("4")
command.append("-odix")
command.append("_chm02")

### store rastered tiles in BIL format
command.append("-obil")

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
    gp.AddMessage("Error. flightlines_to_single_CHM_pit_free failed in las2dem (CHM02) step.")
    sys.exit(1)

### report success
gp.AddMessage("las2dem (CHM02) step done.")

###################################################
### fifth step: raster all partial CHMs
###################################################
### at level 05
###################################################

### create the command string for las2dem.exe
command = ['"'+las2dem_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+empty_temp_dir+"\\pit_free_temp_tile*_ght.laz"+'"')

### remove all points below 5 meters
command.append("-drop_z_below")
command.append("5")

### raster tile with requested step
command.append("-step")
command.append(sys.argv[arg_step].replace(",","."))

### kill triangles that are three times the requested step (or bigger)
command.append("-kill")
command.append(str(3.0*float(sys.argv[arg_step].replace(",","."))))

### raster only tile interiors
command.append("-use_tile_bb")

### store rastered tiles in temporary directory
command.append("-odir")
command.append(empty_temp_dir)

### give the tiles a meaningful appendix
command.append("-ocut")
command.append("4")
command.append("-odix")
command.append("_chm05")

### store rastered tiles in BIL format
command.append("-obil")

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
    gp.AddMessage("Error. flightlines_to_single_CHM_pit_free failed in las2dem (CHM05) step.")
    sys.exit(1)

### report success
gp.AddMessage("las2dem (CHM05) step done.")

###################################################
### fifth step: raster all partial CHMs
###################################################
### at level 10
###################################################

### create the command string for las2dem.exe
command = ['"'+las2dem_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+empty_temp_dir+"\\pit_free_temp_tile*_ght.laz"+'"')

### remove all points below 10 meters
command.append("-drop_z_below")
command.append("10")

### raster tile with requested step
command.append("-step")
command.append(sys.argv[arg_step].replace(",","."))

### kill triangles that are three times the requested step (or bigger)
command.append("-kill")
command.append(str(3.0*float(sys.argv[arg_step].replace(",","."))))

### raster only tile interiors
command.append("-use_tile_bb")

### store rastered tiles in temporary directory
command.append("-odir")
command.append(empty_temp_dir)

### give the tiles a meaningful appendix
command.append("-ocut")
command.append("4")
command.append("-odix")
command.append("_chm10")

### store rastered tiles in BIL format
command.append("-obil")

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
    gp.AddMessage("Error. flightlines_to_single_CHM_pit_free failed in las2dem (CHM10) step.")
    sys.exit(1)

### report success
gp.AddMessage("las2dem (CHM10) step done.")

###################################################
### fifth step: raster all partial CHMs
###################################################
### at level 15
###################################################

### create the command string for las2dem.exe
command = ['"'+las2dem_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+empty_temp_dir+"\\pit_free_temp_tile*_ght.laz"+'"')

### remove all points below 15 meters
command.append("-drop_z_below")
command.append("15")

### raster tile with requested step
command.append("-step")
command.append(sys.argv[arg_step].replace(",","."))

### kill triangles that are three times the requested step (or bigger)
command.append("-kill")
command.append(str(3.0*float(sys.argv[arg_step].replace(",","."))))

### raster only tile interiors
command.append("-use_tile_bb")

### store rastered tiles in temporary directory
command.append("-odir")
command.append(empty_temp_dir)

### give the tiles a meaningful appendix
command.append("-ocut")
command.append("4")
command.append("-odix")
command.append("_chm15")

### store rastered tiles in BIL format
command.append("-obil")

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
    gp.AddMessage("Error. flightlines_to_single_CHM_pit_free failed in las2dem (CHM15) step.")
    sys.exit(1)

### report success
gp.AddMessage("las2dem (CHM15) step done.")

###################################################
### fifth step: raster all partial CHMs
###################################################
### at level 20
###################################################

### create the command string for las2dem.exe
command = ['"'+las2dem_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+empty_temp_dir+"\\pit_free_temp_tile*_ght.laz"+'"')

### remove all points below 20 meters
command.append("-drop_z_below")
command.append("20")

### raster tile with requested step
command.append("-step")
command.append(sys.argv[arg_step].replace(",","."))

### kill triangles that are three times the requested step (or bigger)
command.append("-kill")
command.append(str(3.0*float(sys.argv[arg_step].replace(",","."))))

### raster only tile interiors
command.append("-use_tile_bb")

### store rastered tiles in temporary directory
command.append("-odir")
command.append(empty_temp_dir)

### give the tiles a meaningful appendix
command.append("-ocut")
command.append("4")
command.append("-odix")
command.append("_chm20")

### store rastered tiles in BIL format
command.append("-obil")

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
    gp.AddMessage("Error. flightlines_to_single_CHM_pit_free failed in las2dem (CHM20) step.")
    sys.exit(1)

### report success
gp.AddMessage("las2dem (CHM20) step done.")

###################################################
### sixth step: merge highest rasters to final CHM
###################################################

### create the command string for lasgrid.exe
command = ['"'+lasgrid_path+'"']

### maybe use '-verbose' option
if sys.argv[arg_verbose] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+empty_temp_dir+"\\pit_free_temp_tile*.bil"+'"')

### merge all files
command.append("-merged")

### keep the highest
command.append("-highest")

### raster tile with requested step
command.append("-step")
command.append(sys.argv[arg_step].replace(",","."))

### store rastered tiles in output file
command.append("-o")
command.append(sys.argv[arg_output_file])

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
    gp.AddMessage("Error. flightlines_to_single_CHM_pit_free failed in lasgrid step.")
    sys.exit(1)

### report success
gp.AddMessage("lasgrid step done.")

###################################################
### final step: clean-up all temporary files
###################################################

### create the command string for clean-up
command = ["del"]

### add temporary files wildcard
command.append('"'+empty_temp_dir+"\\pit_free_temp_tile*.*"+'"')

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
    gp.AddMessage("Error. flightlines_to_single_CHM_pit_free failed in clean-up step.")
    sys.exit(1)

### report success
gp.AddMessage("clean-up step done.")

### report happy end
gp.AddMessage("Success. flightlines_to_single_CHM_pit_free done.")
