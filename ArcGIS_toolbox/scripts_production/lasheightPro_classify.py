#
# lasheightPro_classify.py
#
# (c) 2013, martin isenburg - http://rapidlasso.com
#     rapidlasso GmbH - fast tools to catch reality
#
# uses lasheight to compute the height of LiDAR points above the ground
# and then uses the height information to classify the points.
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
gp.AddMessage("Starting lasheight (classsify) production ...")

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

### create the full path to the lasheight executable
lasheight_path = lastools_path+"\\lasheight.exe"

### check if executable exists
if os.path.exists(lastools_path) == False:
    gp.AddMessage("Cannot find lasheight.exe at " + lasheight_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lasheight_path + " ...")

### create the command string for lasheight.exe
command = ['"'+lasheight_path+'"']

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

### maybe use ground points from external file
if sys.argv[c] != "#":
    command.append("-ground_points")
    command.append('"'+sys.argv[c]+'"')
        
### else maybe use points with a different classification as ground
elif sys.argv[c+1] != "2":
    command.append("-class")
    command.append(return_classification(sys.argv[c+1]))
c = c + 2

### maybe we should ignore/preserve some existing classifications when classifying
if sys.argv[c] != "#":
    command.append("-ignore_class")
    command.append(return_classification(sys.argv[c]))
c = c + 1

### maybe we should ignore/preserve some more existing classifications when classifying
if sys.argv[c] != "#":
    command.append("-ignore_class")
    command.append(return_classification(sys.argv[c]))
c = c + 1

### maybe we classify points below
if sys.argv[c] != "#":
    command.append("-classify_below")
    command.append(sys.argv[c+1].replace(",","."))
    command.append(return_classification(sys.argv[c]))
c = c + 2

### maybe we classify points between [interval 1]
if sys.argv[c] != "#":
    command.append("-classify_between")
    command.append(sys.argv[c+1].replace(",","."))
    command.append(sys.argv[c+2].replace(",","."))
    command.append(return_classification(sys.argv[c]))
c = c + 3

### maybe we classify points between [interval 2]
if sys.argv[c] != "#":
    command.append("-classify_between")
    command.append(sys.argv[c+1].replace(",","."))
    command.append(sys.argv[c+2].replace(",","."))
    command.append(return_classification(sys.argv[c]))
c = c + 3

### maybe we classify points between [interval 3]
if sys.argv[c] != "#":
    command.append("-classify_between")
    command.append(sys.argv[c+1].replace(",","."))
    command.append(sys.argv[c+2].replace(",","."))
    command.append(return_classification(sys.argv[c]))
c = c + 3

### maybe we classify points below
if sys.argv[c] != "#":
    command.append("-classify_above")
    command.append(sys.argv[c+1].replace(",","."))
    command.append(return_classification(sys.argv[c]))
c = c + 2

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

### report output of lasheight
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. lasheight failed.")
    sys.exit(1)

### report happy end
gp.AddMessage("Success. lasheight done.")
