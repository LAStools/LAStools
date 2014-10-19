#
# lasgridPro.py
#
# (c) 2013, martin isenburg - http://rapidlasso.com
#     rapidlasso GmbH - fast tools to catch reality
#
# uses lasgrid.exe to grid a folder of LiDAR files
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
gp.AddMessage("Starting lasgrid production ...")

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

### create the full path to the lasgrid executable
lasgrid_path = lastools_path+"\\lasgrid.exe"

### check if executable exists
if os.path.exists(lastools_path) == False:
    gp.AddMessage("Cannot find lasgrid.exe at " + lasgrid_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lasgrid_path + " ...")

### create the command string for lasgrid.exe
command = ['"'+lasgrid_path+'"']

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

### maybe use a user-defined step size
if sys.argv[c] != "1":
    command.append("-step")
    command.append(sys.argv[c].replace(",","."))
c = c + 1

### what should we raster
if sys.argv[c] != "elevation":
    command.append("-" + sys.argv[c])
c = c + 1
        
### what operation should we use
if sys.argv[c] != "lowest":
    command.append("-" + sys.argv[c])
c = c + 1

### should we fill a few pixels
if sys.argv[c] != "0":
    command.append("-fill")
    command.append(sys.argv[c])
c = c + 1

### what should we output
if sys.argv[c] == "gray ramp":
    command.append("-gray")
elif sys.argv[c] == "false colors":
    command.append("-false")

### do we have a min max value for colors
if (sys.argv[c] == "gray ramp") or (sys.argv[c] == "false colors"):
    if (sys.argv[c+1] != "#") and (sys.argv[c+2] != "#"):
        command.append("-set_min_max")
        command.append(sys.argv[c+1].replace(",","."))
        command.append(sys.argv[c+2].replace(",","."))
c = c + 3

### what should we triangulate
if sys.argv[c] == "ground points only":
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
elif sys.argv[c] == "last return only":
    command.append("-last_only")
elif sys.argv[c] == "first return only":
    command.append("-first_only")
c = c + 1

### should we use the bounding box
if sys.argv[c] == "true":
    command.append("-use_bb")
c = c + 1

### should we use the tile bounding box
if sys.argv[c] == "true":
    command.append("-use_tile_bb")
c = c + 1

### maybe an output format was selected
if sys.argv[c] != "#":
    command.append("-o" + sys.argv[c])
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

### report output of lasgrid
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. lasgrid failed.")
    sys.exit(1)

### report happy end
gp.AddMessage("Success. lasgrid done.")
