#
# lasnoise.py
#
# (c) 2013, martin isenburg - http://rapidlasso.com
#     rapidlasso GmbH - fast tools to catch reality
#
# uses lasnoise.exe to remove spurious LiDAR points 
# that fulfill certain isolation criteria.
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
gp.AddMessage("Starting lasnoise ...")

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

### create the full path to the lasnoise executable
lasnoise_path = lastools_path+"\\lasnoise.exe"

### check if executable exists
if os.path.exists(lastools_path) == False:
    gp.AddMessage("Cannot find lasnoise.exe at " + lasnoise_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lasnoise_path + " ...")

### create the command string for lasnoise.exe
command = ['"'+lasnoise_path+'"']

### maybe use '-verbose' option
if sys.argv[argc-1] == "true":
    command.append("-v")

### counting up the arguments
c = 1

### add input LiDAR
command.append("-i")
command.append('"' + sys.argv[c] + '"')
c = c + 1

### maybe the units are in feet
if sys.argv[c] == "true":
    command.append("-feet")
c = c + 1
        
### maybe the elevation is in feet
if sys.argv[c] == "true":
    command.append("-elevation_feet")
c = c + 1

### maybe a user-specified grid size
if sys.argv[c] != "5":
    command.append("-isolated")
    command.append(sys.argv[c])
c = c + 1

### maybe a user-specified grid cell size in xy direction
if sys.argv[c] != "4":
    command.append("-step_xy")
    command.append(sys.argv[c].replace(",","."))
c = c + 1

### maybe a user-specified grid cell size in z direction
if sys.argv[c] != "4":
    command.append("-step_z")
    command.append(sys.argv[c].replace(",","."))
c = c + 1

### which point should we keep
if sys.argv[c] == "remove points":
    command.append("-remove_noise")
elif sys.argv[c] == "classify as 10":
    command.append("-classify_as")
    command.append("10")
elif sys.argv[c] == "classify as 18":
    command.append("-classify_as")
    command.append("18")
c = c + 1

### maybe we should ignore/preserve some existing classifications when classifying
if sys.argv[c] != "#":
    command.append("-ignore_class")
    command.append(return_classification(sys.argv[c]))
c = c + 1

### maybe we should ignore/preserve some other existing classifications when classifying
if sys.argv[c] != "#":
    command.append("-ignore_class")
    command.append(return_classification(sys.argv[c]))
c = c + 1

### maybe we should ignore/preserve yet another existing classifications when classifying
if sys.argv[c] != "#":
    command.append("-ignore_class")
    command.append(return_classification(sys.argv[c]))
c = c + 1

### finally, maybe we should ignore/preserve one more existing classifications when classifying
if sys.argv[c] != "#":
    command.append("-ignore_class")
    command.append(return_classification(sys.argv[c]))
c = c + 1

### maybe an output format was selected
if sys.argv[c] != "#":
    if sys.argv[c] == "las":
        command.append("-olas")
    elif sys.argv[c] == "laz":
        command.append("-olaz")
    elif sys.argv[c] == "bin":
        command.append("-obin")
    elif sys.argv[c] == "xyz":
        command.append("-otxt")
    elif sys.argv[c] == "xyzi":
        command.append("-otxt")
        command.append("-oparse")
        command.append("xyzi")
    elif sys.argv[c] == "txyzi":
        command.append("-otxt")
        command.append("-oparse")
        command.append("txyzi")
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

### maybe there are additional command-line options
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

### report output of lasnoise
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. lasnoise failed.")
    sys.exit(1)

### report happy end
gp.AddMessage("Success. lasnoise done.")
