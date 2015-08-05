#
# lasclassify.py
#
# (c) 2013, martin isenburg - http://rapidlasso.com
#     rapidlasso GmbH - fast tools to catch reality
#
# uses lasclassify.exe to classify the LiDAR points
# for a folder of files in building points (class 6)
# and high vegetation (class 5) points
#
# requires that height above ground is stored in the
# user data field of each input point which can be
# done with lasheight or (licensed) lasground 
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
gp.AddMessage("Starting lasclassify ...")

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

### create the full path to the lasclassify executable
lasclassify_path = lastools_path+"\\lasclassify.exe"

### check if executable exists
if os.path.exists(lastools_path) == False:
    gp.AddMessage("Cannot find lasclassify.exe at " + lasclassify_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lasclassify_path + " ...")

### create the command string for lasclassify.exe
command = ['"'+lasclassify_path+'"']

### maybe use '-verbose' option
if sys.argv[argc-1] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+sys.argv[1]+'"')

### maybe the units are in feet
if sys.argv[2] == "true":
    command.append("-feet")
        
### maybe the elevation is in feet
if sys.argv[3] == "true":
    command.append("-elevation_feet")
        
### maybe user-defined planarity
if sys.argv[4].replace(",",".") != "0.1":
    command.append("-planar")
    command.append(sys.argv[4].replace(",","."))

### maybe user-defined planarity
if sys.argv[5].replace(",",".") != "0.4":
    command.append("-rugged")
    command.append(sys.argv[5].replace(",","."))

### maybe user-defined planarity
if sys.argv[6] != "2":
    command.append("-ground_offset")
    command.append(sys.argv[6].replace(",","."))

### maybe no gutters
if sys.argv[7] == "false":
    command.append("-no_gutters")

### else maybe wide gutters
elif sys.argv[8] == "true":
    command.append("-wide_gutters")

### maybe also tiny buildings
if sys.argv[9] == "false":
    command.append("-small_buildings")

### maybe keep tree overhang
if sys.argv[10] == "false":
    command.append("-keep_overhang")

### this is where the output arguments start
out = 11

### maybe an output format was selected
if sys.argv[out] != "#":
    if sys.argv[out] == "las":
        command.append("-olas")
    elif sys.argv[out] == "laz":
        command.append("-olaz")
    elif sys.argv[out] == "bin":
        command.append("-obin")
    elif sys.argv[out] == "xyz":
        command.append("-otxt")
    elif sys.argv[out] == "xyzi":
        command.append("-otxt")
        command.append("-oparse")
        command.append("xyzi")
    elif sys.argv[out] == "txyzi":
        command.append("-otxt")
        command.append("-oparse")
        command.append("txyzi")

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

### report output of lasclassify
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. lasclassify failed.")
    sys.exit(1)

### report happy end
gp.AddMessage("Success. lasclassify done.")
