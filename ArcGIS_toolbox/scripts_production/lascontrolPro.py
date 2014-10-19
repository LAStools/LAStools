#
# lascontrolPro.py
#
# (c) 2013, martin isenburg - http://rapidlasso.com
#     rapidlasso GmbH - fast tools to catch reality
#
# uses lascontrol.exe to verify a folder of LiDAR 
# files against a set of ground control points
#
# LiDAR input:   LAS/LAZ/BIN/TXT/SHP/BIL/ASC/DTM
# control point: CSV/TXT
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
gp.AddMessage("Starting lascontrol production ...")

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

### create the full path to the lascontrol executable
lascontrol_path = lastools_path+"\\lascontrol.exe"

### check if executable exists
if os.path.exists(lastools_path) == False:
    gp.AddMessage("Cannot find lascontrol.exe at " + lascontrol_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lascontrol_path + " ...")

### create the command string for lascontrol.exe
command = ['"'+lascontrol_path+'"']

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

### maybe we should merge all files into one
if sys.argv[c] == "true":
    command.append("-merged")
c = c + 1

### add input control points
command.append("-cp")
command.append('"'+sys.argv[c]+'"')
c = c + 1

### maybe add the control point parse string
if sys.argv[3] != "xyz":
    command.append("-parse")
    command.append(sys.argv[c])
c = c + 1

### maybe skip a few points in the control file
if sys.argv[4] != "0":
    command.append("-skip")
    command.append(sys.argv[c])
c = c + 1
        
### maybe use horizontal feet
if sys.argv[5] == "true":
    command.append("-feet")
    command.append(sys.argv[c])
c = c + 1

### which points to consider
if sys.argv[c] == "only ground points":
    command.append("-keep_class")
    command.append("2")
elif sys.argv[c] == "ground and keypoints":
    command.append("-keep_class")
    command.append("2")
    command.append("8")
elif sys.argv[c] == "ground and buildings":
    command.append("-keep_class")
    command.append("2")
    command.append("6")
c = c + 1

### maybe an output file name was selected
if sys.argv[c] != "#":
    command.append("-cp_out")
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

### report output of lascontrol
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. lascontrol failed.")
    sys.exit(1)

### report happy end
gp.AddMessage("Success. lascontrol done.")
