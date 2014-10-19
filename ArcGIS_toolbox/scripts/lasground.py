#
# lasground.py
#
# (c) 2013, martin isenburg - http://rapidlasso.com
#     rapidlasso GmbH - fast tools to catch reality
#
# uses lasground.exe to extract bare-earth points of
# a LiDAR file. classifies points into ground (class 2)
# and non-ground points (class 1).
#
# LiDAR input:   LAS/LAZ/BIN/TXT/SHP/BIL/ASC/DTM
# LiDAR output:  LAS/LAZ/BIN/TXT
#
# for licensing see http://lastools.org/LICENSE.txt
#

import sys, os, arcgisscripting, subprocess

def return_classification(classification):
    if (classification == "created, never classified (0)"):
        return "0"
    if (classification == "unclassified (1)"):
        return "1"
    if (classification == "ground (2)"):
        return "2"
    if (classification == "low vegetation (3)"):
        return "3"
    if (classification == "medium vegetation (4)"):
        return "4"
    if (classification == "high vegetation (5)"):
        return "5"
    if (classification == "building (6)"):
        return "6"
    if (classification == "low point (7)"):
        return "7"
    if (classification == "keypoint (8)"):
        return "8"
    if (classification == "water (9)"):
        return "9"
    if (classification == "high point (10)"):
        return "10"
    if (classification == "(11)"):
        return "11"
    if (classification == "overlap point (12)"):
        return "12"
    if (classification == "(13)"):
        return "13"
    if (classification == "(14)"):
        return "14"
    if (classification == "(15)"):
        return "15"
    if (classification == "(16)"):
        return "16"
    if (classification == "(17)"):
        return "17"
    if (classification == "(18)"):
        return "18"
    return "unknown"

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
gp.AddMessage("Starting lasground ...")

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

### create the full path to the lasground executable
lasground_path = lastools_path+"\\lasground.exe"

### check if executable exists
if os.path.exists(lastools_path) == False:
    gp.AddMessage("Cannot find lasground.exe at " + lasground_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lasground_path + " ...")

### create the command string for lasground.exe
command = ['"'+lasground_path+'"']

### maybe use '-verbose' option
if sys.argv[argc-1] == "true":
    command.append("-v")

### counting up the arguments
c = 1

### add input LiDAR
command.append("-i")
command.append('"'+sys.argv[c]+'"')
c = c + 1

### maybe no airborne LiDAR
if sys.argv[c] != "true":
    command.append("-not_airborne")
c = c + 1

### maybe horizontal feet
if sys.argv[c] == "true":
    command.append("-feet")
c = c + 1

### maybe vertical feet
if sys.argv[c] == "true":
    command.append("-elevation_feet")
c = c + 1

### what type of terrain do we have
if sys.argv[c] == "wilderness":
    command.append("-wilderness")
elif sys.argv[c] == "city or warehouses":
    command.append("-city")
elif sys.argv[c] == "towns or flats":
    command.append("-town")
elif sys.argv[c] == "metropolis":
    command.append("-metro")
c = c + 1

### what granularity should we operate with
if sys.argv[c] == "fine":
    command.append("-fine")
elif sys.argv[c] == "extra fine":
    command.append("-extra_fine")
elif sys.argv[c] == "ultra fine":
    command.append("-ultra_fine")
c = c + 1

### maybe we should ignore/preserve some existing classifications when classifying
if sys.argv[c] != "#":
    command.append("-ignore_class")
    command.append(return_classification(sys.argv[c]))
c = c + 1

### maybe also ignore/preserve some other existing classifications when classifying
if sys.argv[c] != "#":
    command.append("-ignore_class")
    command.append(return_classification(sys.argv[c]))
c = c + 1

### maybe we should compute the heights (e.g. for subsequent lasclassify)
if sys.argv[c] == "true":
    command.append("-compute_height")
c = c + 1

### maybe we should height-normalize the LiDAR (e.g. for subsequent canopy creation)
if sys.argv[c] == "true":
    command.append("-replace_z")
c = c + 1

### maybe an output format was selected
if sys.argv[c] != "#":
    if sys.argv[c] == "las":
        command.append("-olas")
    elif sys.argv[c] == "laz":
        command.append("-olaz")
    elif sys.argv[c] == "bin":
        command.append("-obin")
    elif sys.argv[c] == "xyzc":
        command.append("-otxt")
        command.append("-oparse")
        command.append("xyzc")
    elif sys.argv[c] == "xyzci":
        command.append("-otxt")
        command.append("-oparse")
        command.append("xyzci")
    elif sys.argv[c] == "txyzc":
        command.append("-otxt")
        command.append("-oparse")
        command.append("txyzc")
    elif sys.argv[c] == "txyzci":
        command.append("-otxt")
        command.append("-oparse")
        command.append("txyzci")
c = c + 1

### maybe an output file name was selected
if sys.argv[c] != "#":
    command.append("-o")
    command.append('"'+sys.argv[c]+'"')
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

### report output of lasground
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. lasground failed.")
    sys.exit(1)

### report happy end
gp.AddMessage("Success. lasground done.")
