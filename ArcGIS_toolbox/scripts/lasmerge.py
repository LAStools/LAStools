#
# lasmerge.py
#
# (c) 2013, martin isenburg - http://rapidlasso.com
#     rapidlasso GmbH - fast tools to catch reality
#
# uses lasmerge.exe to merge LiDAR files into one
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
gp.AddMessage("Starting lasmerge ...")

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

### create the full path to the lasmerge executable
lasmerge_path = lastools_path+"\\lasmerge.exe"

### check if executable exists
if os.path.exists(lastools_path) == False:
    gp.AddMessage("Cannot find lasmerge.exe at " + lasmerge_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lasmerge_path + " ...")

### create the command string for lasmerge.exe
command = ['"'+lasmerge_path+'"']

### maybe use '-verbose' option
if sys.argv[argc-1] == "true":
    command.append("-v")

### add first and second input LiDAR
command.append("-i")
command.append('"'+sys.argv[1]+'"')
command.append('"'+sys.argv[2]+'"')
   
### maybe add another input LiDAR
if sys.argv[3] != "#":
    command.append('"'+sys.argv[3]+'"')

### maybe add another input LiDAR
if sys.argv[4] != "#":
    command.append('"'+sys.argv[4]+'"')

### maybe add another input LiDAR
if sys.argv[5] != "#":
    command.append('"'+sys.argv[5]+'"')

### maybe add another input LiDAR
if sys.argv[6] != "#":
    command.append('"'+sys.argv[6]+'"')

### maybe add another input LiDAR
if sys.argv[7] != "#":
    command.append('"'+sys.argv[7]+'"')

### maybe add another input LiDAR
if sys.argv[8] != "#":
    command.append('"'+sys.argv[8]+'"')

### maybe add another input LiDAR
if sys.argv[9] != "#":
    command.append('"'+sys.argv[9]+'"')

### add output LiDAR
command.append("-o")
command.append('"'+sys.argv[10]+'"')

### maybe there are additional command-line options
if sys.argv[11] != "#":
    additional_options = sys.argv[11].split()
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

### report output of lasmerge
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. lasmerge failed.")
    sys.exit(1)

### report happy end
gp.AddMessage("Success. lasmerge done.")
