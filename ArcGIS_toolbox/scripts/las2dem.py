#
# las2dem.py
#
# (c) 2013, martin isenburg - http://rapidlasso.com
#     rapidlasso GmbH - fast tools to catch reality
#
# uses las2dem.exe to raster a LiDAR file
#
# LiDAR input:   LAS/LAZ/BIN/TXT/SHP/BIL/ASC/DTM
# raster output: BIL/ASC/IMG/TIF/DTM/PNG/JPG
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
gp.AddMessage("Starting las2dem ...")

### get number of arguments
argc = len(sys.argv)

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

### complete the path to where the LAStools executables are
lastools_path = lastools_path + "\\bin"

### check if path exists
if os.path.exists(lastools_path) == False:
    gp.AddMessage("Cannot find .\\lastools\\bin at " + lastools_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lastools_path + " ...")

### create the full path to the las2dem executable
las2dem_path = lastools_path+"\\las2dem.exe"

### check if executable exists
if os.path.exists(lastools_path) == False:
    gp.AddMessage("Cannot find las2dem.exe at " + las2dem_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + las2dem_path + " ...")

### create the command string for las2dem.exe
command = ['"'+las2dem_path+'"']

### maybe use '-verbose' option
if sys.argv[argc-1] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+sys.argv[1]+'"')

### maybe use a user-defined step size
if sys.argv[2] != "1":
    command.append("-step")
    command.append(sys.argv[2].replace(",","."))

### maybe use a user-defined kill
if sys.argv[3] != "100":
    command.append("-kill")
    command.append(sys.argv[3].replace(",","."))

### what should we raster
if sys.argv[4] == "slope":
    command.append("-slope")
elif sys.argv[4] == "intensity":
    command.append("-intensity")
elif sys.argv[4] == "rgb":
    command.append("-rgb")
        
### what should we output
if sys.argv[5] == "hillshade":
    command.append("-hillshade")
elif sys.argv[5] == "gray ramp":
    command.append("-gray")
elif sys.argv[5] == "false colors":
    command.append("-false")

### do we have special lighting for hillshade
if sys.argv[5] == "hillshade":
    if (sys.argv[6] != "north east") or (sys.argv[7] != "1 pm"):
        command.append("-light")
        if sys.argv[6] == "north":
            command.append("0")
            command.append("1.41421")
        elif sys.argv[6] == "south":
            command.append("0")
            command.append("-1.41421")
        elif sys.argv[6] == "east":
            command.append("1.41421")
            command.append("0")
        elif sys.argv[6] == "west":
            command.append("-1.41421")
            command.append("0")
        elif sys.argv[6] == "north east":
            command.append("1")
            command.append("1")
        elif sys.argv[6] == "south east":
            command.append("1")
            command.append("-1")
        elif sys.argv[6] == "north west":
            command.append("-1")
            command.append("1")
        else: ### if sys.argv[6] == "south west"
            command.append("-1")
            command.append("-1")
        if sys.argv[7] == "noon":
            command.append("100")
        elif sys.argv[7] == "1 pm":
            command.append("2")
        elif sys.argv[7] == "3 pm":
            command.append("1")
        elif sys.argv[7] == "6 pm":
            command.append("0.5")
        else: ### if sys.argv[7] == "9 pm"
            command.append("0.1")

### do we have a min max value for colors
if (sys.argv[5] == "gray ramp") or (sys.argv[5] == "false colors"):
    if (sys.argv[8] != "#") and (sys.argv[9] != "#"):
        command.append("-set_min_max")
        command.append(sys.argv[8].replace(",","."))
        command.append(sys.argv[9].replace(",","."))

### what should we triangulate
if sys.argv[10] == "ground points only":
    command.append("-keep_class")
    command.append("2")
    command.append("-extra_pass")
elif sys.argv[10] == "ground and keypoints":
    command.append("-keep_class")
    command.append("2")
    command.append("8")
    command.append("-extra_pass")
elif sys.argv[10] == "ground and buildings":
    command.append("-keep_class")
    command.append("2")
    command.append("6")
    command.append("-extra_pass")
elif sys.argv[10] == "ground and vegetation":
    command.append("-keep_class")
    command.append("2")
    command.append("3")
    command.append("4")
    command.append("5")
    command.append("-extra_pass")
elif sys.argv[10] == "ground and objects":
    command.append("-keep_class")
    command.append("2")
    command.append("3")
    command.append("4")
    command.append("5")
    command.append("6")
    command.append("-extra_pass")
elif sys.argv[10] == "last return only":
    command.append("-last_only")
    command.append("-extra_pass")
elif sys.argv[10] == "first return only":
    command.append("-first_only")
    command.append("-extra_pass")

### should we use the tile bounding box
if sys.argv[11] == "true":
    command.append("-use_tile_bb")

### do we have lakes
if sys.argv[12] != "#":
    command.append("-lakes")
    command.append('"'+sys.argv[12]+'"')

### do we have creeks
if sys.argv[13] != "#":
    command.append("-creeks")
    command.append('"'+sys.argv[13]+'"')

### this is where the output arguments start
out = 14

### maybe an output format was selected
if sys.argv[out] != "#":
    command.append("-o" + sys.argv[out])

### maybe an output file name was selected
if sys.argv[out+1] != "#":
    command.append("-o")
    command.append('"'+sys.argv[out+1]+'"')
    
### maybe an output directory was selected
if sys.argv[out+2] != "#":
    command.append("-odir")
    command.append('"'+sys.argv[out+2]+'"')

### maybe an output appendix was selected
if sys.argv[out+3] != "#":
    command.append("-odix")
    command.append('"'+sys.argv[out+3]+'"')

### maybe there are additional command-line options
if sys.argv[out+4] != "#":
    additional_options = sys.argv[out+4].split()
    for option in additional_options:
        command.append(option)

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
    gp.AddMessage("Error. las2dem failed.")
    sys.exit(1)

### report happy end
gp.AddMessage("Success. las2dem done.")
