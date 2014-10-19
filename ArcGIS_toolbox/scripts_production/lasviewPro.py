#
# lasviewPro.py
#
# (c) 2013, martin isenburg - http://rapidlasso.com
#     rapidlasso GmbH - fast tools to catch reality
#
# uses lasview.exe to visualize LiDAR files
#
# LiDAR input:   LAS/LAZ/BIN/TXT/SHP/BIL/ASC/DTM
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
gp.AddMessage("Starting lasview production ...")

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

### create the full path to the lasview executable
lasview_path = lastools_path+"\\lasview.exe"

### check if executable exists
if os.path.exists(lastools_path) == False:
    gp.AddMessage("Cannot find lasview.exe at " + lasview_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lasview_path + " ...")

### create the command string for lasview.exe
command = ['"'+lasview_path+'"']

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

### maybe input files are flightlines
if sys.argv[c] == "true":
    command.append("-files_are_flightlines")
c = c + 1

### maybe display different number of points
if sys.argv[c] != "5000000":
    command.append("-points")
    command.append(sys.argv[c])
c = c + 1

### which points to consider
if sys.argv[c] == "first returns":
    command.append("-only_first")
elif sys.argv[c] == "last returns":
    command.append("-only_last")
elif sys.argv[c] == "multi returns":
    command.append("-only_multi")
elif sys.argv[c] == "single returns":
    command.append("-only_single")
elif sys.argv[c] == "ground":
    command.append("-ground")
elif sys.argv[c] == "buildings":
    command.append("-buildings")
elif sys.argv[c] == "vegetation":
    command.append("-vegetation")
elif sys.argv[c] == "objects":
    command.append("-objects")
elif sys.argv[c] == "ground and buildings":
    command.append("-ground_buildings")
elif sys.argv[c] == "ground and vegetation":
    command.append("-ground_vegetation")
elif sys.argv[c] == "ground and objects":
    command.append("-ground_objects")
c = c + 1

### how to color
if sys.argv[c] == "elevation ramp 1":
    command.append("-color_by_elevation1")
elif sys.argv[c] == "elevation ramp 2":
    command.append("-color_by_elevation2")
elif sys.argv[c] == "classification":
    command.append("-color_by_classification")
elif sys.argv[c] == "rgb":
    command.append("-color_by_rgb")
elif sys.argv[c] == "flight line":
    command.append("-color_by_flight_line")
elif sys.argv[c] == "intensity":
    command.append("-color_by_intensity")
elif sys.argv[c] == "number returns":
    command.append("-color_by_returns")
c = c + 1

### maybe show control points
if sys.argv[c] != "#":
    command.append("-cp")
    command.append('"'+sys.argv[c]+'"')
c = c + 1

### maybe add the control point parse string
if sys.argv[c] != "#":
    command.append("-cp_parse")
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

### report output of lasview
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. lasview failed.")
    sys.exit(1)

### report happy end
gp.AddMessage("Success. lasview done.")
