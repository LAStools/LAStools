#
# las2iso.py
#
# (c) 2013, martin isenburg - http://rapidlasso.com
#     rapidlasso GmbH - fast tools to catch reality
#
# uses las2iso.exe to contour a LiDAR file
#
# LiDAR input:   LAS/LAZ/BIN/TXT/SHP/BIL/ASC/DTM
# vector output: SHP/WKT/KML/TXT
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
gp.AddMessage("Starting las2iso ...")

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

### create the full path to the las2iso executable
las2iso_path = lastools_path+"\\las2iso.exe"

### check if executable exists
if os.path.exists(lastools_path) == False:
    gp.AddMessage("Cannot find las2iso.exe at " + las2iso_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + las2iso_path + " ...")

### create the command string for las2iso.exe
command = ['"'+las2iso_path+'"']

### maybe use '-verbose' option
if sys.argv[argc-1] == "true":
    command.append("-v")

### add input LiDAR
command.append("-i")
command.append('"'+sys.argv[1]+'"')

### maybe use user-defined concavity
if sys.argv[2] != "50":
    command.append("-concavity")
    command.append(sys.argv[2].replace(",","."))

### what should we contour
if sys.argv[3] == "only ground points":
    command.append("-keep_class")
    command.append("2")
    command.append("-extra_pass")
elif sys.argv[3] == "ground and keypoints":
    command.append("-keep_class")
    command.append("2")
    command.append("8")
    command.append("-extra_pass")
elif sys.argv[3] == "ground and buildings":
    command.append("-keep_class")
    command.append("2")
    command.append("6")
    command.append("-extra_pass")
elif sys.argv[3] == "ground and vegetation":
    command.append("-keep_class")
    command.append("2")
    command.append("3")
    command.append("4")
    command.append("5")
    command.append("-extra_pass")
elif sys.argv[3] == "ground and objects":
    command.append("-keep_class")
    command.append("2")
    command.append("3")
    command.append("4")
    command.append("5")
    command.append("6")
    command.append("-extra_pass")
            
### which isovalues should we extract
if sys.argv[4] == "a number of x equally spaced contours":
    if sys.argv[5] != "10":
        command.append("-iso_number")
        command.append(sys.argv[5])
elif sys.argv[4] == "a contour every x elevation units":
    command.append("-iso_every")
    command.append(sys.argv[5].replace(",","."))
elif sys.argv[4] == "the contour with the iso-value x":
    command.append("-iso_value")
    command.append(sys.argv[5].replace(",","."))

### maybe we should smooth the TIN
if sys.argv[6] != "do not smooth":
    command.append("-smooth")
    command.append(sys.argv[6])
    
### maybe we should simplify bumps
if sys.argv[7] != "do not simplify":
    command.append("-simplify")
    command.append(sys.argv[7])

### maybe we should clean short lines
if sys.argv[8] != "do not clean":
    command.append("-clean")
    command.append(sys.argv[8])

### do we have lakes
if sys.argv[9] != "#":
    command.append("-lakes")
    command.append('"'+sys.argv[9]+'"')

### do we have creeks
if sys.argv[10] != "#":
    command.append("-creeks")
    command.append('"'+sys.argv[10]+'"')

### this is where the output arguments start
out = 11

### maybe an output format was selected
if sys.argv[out] != "#":
    command.append("-o" + sys.argv[out])

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

### report output of las2iso
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. las2iso failed.")
    sys.exit(1)

### report happy end
gp.AddMessage("Success. las2iso done.")
