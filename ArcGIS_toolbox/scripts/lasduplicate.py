#
# lasduplicate.py
#
# (c) 2013, martin isenburg - http://rapidlasso.com
#     rapidlasso GmbH - fast tools to catch reality
#
# uses lasduplicate.exe to remove all duplicate points from a
# LiDAR file. By default the first point of those with identical
# x and y coordinates survives. It is also possible to keep the
# lowest of all xy-duplicates or to only remove xyz-duplicates.
#
# LiDAR input:   LAS/LAZ/BIN/TXT/SHP/BIL/ASC/DTM
# LiDAR output:  LAS/LAZ/BIN/TXT format
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
gp.AddMessage("Starting lasduplicate ...")

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

### create the full path to the lasduplicate executable
lasduplicate_path = lastools_path+"\\lasduplicate.exe"

### check if executable exists
if os.path.exists(lastools_path) == False:
    gp.AddMessage("Cannot find lasduplicate.exe at " + lasduplicate_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lasduplicate_path + " ...")

### create the command string for lasduplicate.exe
command = ['"'+lasduplicate_path+'"']

### maybe use '-verbose' option
if sys.argv[argc-1] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+sys.argv[1]+'"')

### maybe a user-specified grid size
if sys.argv[2] == "lowest_z":
    command.append("-lowest_z")
elif  sys.argv[2] == "unique_xyz":
    command.append("-unique_xyz")
        
### maybe an output format was selected
if sys.argv[3] != "#":
    if sys.argv[3] == "las":
        command.append("-olas")
    elif sys.argv[3] == "laz":
        command.append("-olaz")
    elif sys.argv[3] == "bin":
        command.append("-obin")
    elif sys.argv[3] == "xyz":
        command.append("-otxt")
    elif sys.argv[3] == "xyzi":
        command.append("-otxt")
        command.append("-oparse")
        command.append("xyzi")
    elif sys.argv[3] == "txyzi":
        command.append("-otxt")
        command.append("-oparse")
        command.append("txyzi")

### maybe an output file name was selected
if sys.argv[4] != "#":
    command.append("-o")
    command.append('"'+sys.argv[4]+'"')

### maybe an output directory was selected
if sys.argv[5] != "#":
    command.append("-odir")
    command.append('"'+sys.argv[5]+'"')

### maybe an output appendix was selected
if sys.argv[6] != "#":
    command.append("-odix")
    command.append('"'+sys.argv[6]+'"')

### maybe there are additional command-line options
if sys.argv[7] != "#":
    additional_options = sys.argv[7].split()
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

### report output of lasduplicate
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. lasduplicate failed.")
    sys.exit(1)

### report happy end
gp.AddMessage("Success. lasduplicate done.")
