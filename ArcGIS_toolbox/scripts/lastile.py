#
# lastile.py
#
# (c) 2013, martin isenburg - http://rapidlasso.com
#     rapidlasso GmbH - fast tools to catch reality
#
# uses lastile.exe to tile a LiDAR files with a user
# specified tile size (and an optional buffer)
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
gp.AddMessage("Starting lastile ...")

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

### create the full path to the lastile executable
lastile_path = lastools_path+"\\lastile.exe"

### check if executable exists
if os.path.exists(lastools_path) == False:
    gp.AddMessage("Cannot find lastile.exe at " + lastile_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lastile_path + " ...")

### create the command string for lastile.exe
command = ['"'+lastile_path+'"']

### maybe use '-verbose' option
if sys.argv[argc-1] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+sys.argv[1]+'"')

### maybe use a user-defined tile size
if sys.argv[2] != "1000":
    command.append("-tile_size")
    command.append(sys.argv[2].replace(",","."))

### maybe create a buffer around the tiles
if sys.argv[3] != "0":
    command.append("-buffer")
    command.append(sys.argv[3].replace(",","."))

### maybe the output will be over 2000 tiles
if sys.argv[4] == "true":
    command.append("-extra_pass")
        
### maybe an output format was selected
if sys.argv[5] != "#":
    if sys.argv[5] == "las":
        command.append("-olas")
    elif sys.argv[5] == "laz":
        command.append("-olaz")
    elif sys.argv[5] == "bin":
        command.append("-obin")
    elif sys.argv[5] == "txt":
        command.append("-otxt")
    elif sys.argv[5] == "xyzi":
        command.append("-otxt")
        command.append("-oparse")
        command.append("xyzi")
    elif sys.argv[5] == "txyzi":
        command.append("-otxt")
        command.append("-oparse")
        command.append("txyzi")

### maybe an output file name was selected
if sys.argv[6] != "#":
    command.append("-o")
    command.append('"'+sys.argv[6]+'"')
    
### maybe an output directory was selected
if sys.argv[7] != "#":
    command.append("-odir")
    command.append('"'+sys.argv[7]+'"')

### maybe an output appendix was selected
if sys.argv[8] != "#":
    command.append("-odix")
    command.append('"'+sys.argv[8]+'"')

### maybe there are additional command-line options
if sys.argv[9] != "#":
    additional_options = sys.argv[9].split()
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

### report output of lastile
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. lastile failed.")
    sys.exit(1)

### report happy end
gp.AddMessage("Success. lastile done.")
