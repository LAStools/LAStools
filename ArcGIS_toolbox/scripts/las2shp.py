#
# las2shp.py
#
# (c) 2013, martin isenburg - http://rapidlasso.com
#     rapidlasso GmbH - fast tools to catch reality
#
# uses las2shp.exe to covert LiDAR points to ESRI's
# Shapefile format utilizing shapetype PointZ or
# MultiPointZ.
#
# LiDAR input:   LAS/LAZ/BIN/TXT
# SHP output:    PointZ, MultiPointZ
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
gp.AddMessage("Starting las2shp ...")

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

### create the full path to the las2shp executable
las2shp_path = lastools_path+"\\las2shp.exe"

### check if executable exists
if os.path.exists(lastools_path) == False:
    gp.AddMessage("Cannot find las2shp.exe at " + las2shp_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + las2shp_path + " ...")

### create the command string for las2shp.exe
command = ['"'+las2shp_path+'"']

### maybe use '-verbose' option
if sys.argv[argc-1] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+sys.argv[1]+'"')

### maybe use shape type PointZ
if sys.argv[2] == "PointZ":
    command.append("-single_points")

### use a user defined record size
elif sys.argv[3] != "1024":
    command.append("-record_size")
    command.append(sys.argv[3])
            
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

### report output of las2shp
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. las2shp failed.")
    sys.exit(1)

### report happy end
gp.AddMessage("Success. las2shp done.")
