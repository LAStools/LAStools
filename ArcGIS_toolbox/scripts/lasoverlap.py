#
# lasoverlap.py
#
# (c) 2013, martin isenburg - http://rapidlasso.com
#     rapidlasso GmbH - fast tools to catch reality
#
# uses lasoverlap.exe to check the flightline overlap
# and alignment for a folder of LiDAR files
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
gp.AddMessage("Starting lasoverlap ...")

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

### create the full path to the lasoverlap executable
lasoverlap_path = lastools_path+"\\lasoverlap.exe"

### check if executable exists
if os.path.exists(lastools_path) == False:
    gp.AddMessage("Cannot find lasoverlap.exe at " + lasoverlap_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lasoverlap_path + " ...")

### create the command string for lasoverlap.exe
command = ['"'+lasoverlap_path+'"']

### maybe use '-verbose' option
if sys.argv[argc-1] == "true":
    command.append("-v")

### counting up the arguments
c = 1

### add input LiDAR
command.append("-i")
command.append('"' + sys.argv[c] + '"')
c = c + 1

### maybe use a user-defined step size
if sys.argv[c] != "2":
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
if sys.argv[c] == "actual values":
    command.append("-values")
c = c + 1

### maybe no overlap raster
if sys.argv[c] != "true":
    command.append("-no_over")
c = c + 1

### maybe use a user-defined max diff
if sys.argv[c] != "5":
    command.append("-max_over")
    command.append(sys.argv[c])
c = c + 1

### maybe no difference raster
if sys.argv[c] != "true":
    command.append("-no_diff")
c = c + 1

### maybe use a user-defined max diff
if sys.argv[c].replace(",",".") != "0.5":
    command.append("-max_diff")
    command.append(sys.argv[c].replace(",","."))
c = c + 1

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

### report output of lasoverlap
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. lasoverlap failed.")
    sys.exit(1)

### report happy end
gp.AddMessage("Success. lasoverlap done.")
