#
# las2demPro.py
#
# (c) 2013, martin isenburg - http://rapidlasso.com
#     rapidlasso GmbH - fast tools to catch reality
#
# uses las2dem.exe to raster a folder of LiDAR files
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
gp.AddMessage("Starting las2dem production ...")

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

### counting up the arguments
c = 1

### add input LiDAR
wildcards = sys.argv[c+1].split()
for wildcard in wildcards:
    command.append("-i")
    command.append('"' + sys.argv[c] + "\\" + wildcard + '"')
c = c + 2

### maybe use a user-defined step size
if sys.argv[c] != "1":
    command.append("-step")
    command.append(sys.argv[c].replace(",","."))
c = c + 1

### maybe use a user-defined kill
if sys.argv[c] != "100":
    command.append("-kill")
    command.append(sys.argv[c].replace(",","."))
c = c + 1

### what should we raster
if sys.argv[c] == "slope":
    command.append("-slope")
elif sys.argv[c] == "intensity":
    command.append("-intensity")
elif sys.argv[c] == "rgb":
    command.append("-rgb")
c = c + 1
        
### what should we output
if sys.argv[c] == "hillshade":
    command.append("-hillshade")
elif sys.argv[c] == "gray ramp":
    command.append("-gray")
elif sys.argv[c] == "false colors":
    command.append("-false")

### do we have special lighting for hillshade
if sys.argv[c] == "hillshade":
    if (sys.argv[c+1] != "north east") or (sys.argv[c+2] != "1 pm"):
        command.append("-light")
        if sys.argv[c+1] == "north":
            command.append("0")
            command.append("1.41421")
        elif sys.argv[c+1] == "south":
            command.append("0")
            command.append("-1.41421")
        elif sys.argv[c+1] == "east":
            command.append("1.41421")
            command.append("0")
        elif sys.argv[c+1] == "west":
            command.append("-1.41421")
            command.append("0")
        elif sys.argv[c+1] == "north east":
            command.append("1")
            command.append("1")
        elif sys.argv[c+1] == "south east":
            command.append("1")
            command.append("-1")
        elif sys.argv[c+1] == "north west":
            command.append("-1")
            command.append("1")
        else: ### if sys.argv[c+1] == "south west"
            command.append("-1")
            command.append("-1")
        if sys.argv[c+2] == "noon":
            command.append("100")
        elif sys.argv[c+2] == "1 pm":
            command.append("2")
        elif sys.argv[c+2] == "3 pm":
            command.append("1")
        elif sys.argv[c+2] == "6 pm":
            command.append("0.5")
        else: ### if sys.argv[c+2] == "9 pm"
            command.append("0.1")

### do we have a min max value for colors
if (sys.argv[c] == "gray ramp") or (sys.argv[c] == "false colors"):
    if (sys.argv[c+3] != "#") and (sys.argv[c+4] != "#"):
        command.append("-set_min_max")
        command.append(sys.argv[c+3].replace(",","."))
        command.append(sys.argv[c+4].replace(",","."))
c = c + 5

### what should we triangulate
if sys.argv[c] == "ground points only":
    command.append("-keep_class")
    command.append("2")
    command.append("-extra_pass")
elif sys.argv[c] == "ground and keypoints":
    command.append("-keep_class")
    command.append("2")
    command.append("8")
    command.append("-extra_pass")
elif sys.argv[c] == "ground and buildings":
    command.append("-keep_class")
    command.append("2")
    command.append("6")
    command.append("-extra_pass")
elif sys.argv[c] == "ground and vegetation":
    command.append("-keep_class")
    command.append("2")
    command.append("3")
    command.append("4")
    command.append("5")
    command.append("-extra_pass")
elif sys.argv[c] == "ground and objects":
    command.append("-keep_class")
    command.append("2")
    command.append("3")
    command.append("4")
    command.append("5")
    command.append("6")
    command.append("-extra_pass")
elif sys.argv[c] == "last return only":
    command.append("-last_only")
    command.append("-extra_pass")
elif sys.argv[c] == "first return only":
    command.append("-first_only")
    command.append("-extra_pass")
c = c + 1

### should we use the tile bounding box
if sys.argv[c] == "true":
    command.append("-use_tile_bb")
c = c + 1

### do we have lakes
if sys.argv[c] != "#":
    command.append("-lakes")
    command.append('"'+sys.argv[c]+'"')
c = c + 1

### do we have creeks
if sys.argv[c] != "#":
    command.append("-creeks")
    command.append('"'+sys.argv[c]+'"')
c = c + 1

### maybe an output format was selected
if sys.argv[c] != "#":
    command.append("-o" + sys.argv[c])
c = c + 1
    
### maybe an output directory was selected
if sys.argv[c] != "#":
    command.append("-odir")
    command.append('"'+sys.argv[c]+'"')
c = c + 1

### maybe an output appendix was selected
if sys.argv[c] != "#":
    command.append("-odix")
    command.append('"'+sys.argv[c]+'"')
c = c + 1

### maybe we should run on multiple cores
if sys.argv[c] != "1":
    command.append("-cores")
    command.append(sys.argv[c])
c = c + 1

### maybe there are additional input options
if sys.argv[c] != "#":
    additional_options = sys.argv[c].split()
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
