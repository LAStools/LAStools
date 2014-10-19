#
# lasboundaryPro.py
#
# (c) 2013, martin isenburg - http://rapidlasso.com
#     rapidlasso GmbH - fast tools to catch reality
#
# uses lasboundary.exe to compute a boundary polygon for the
# points. This is a concave hull of the points with a "concavity"
# defined by the user that is 50 m (or 150 ft) by default. It is
# usually a single polygon where "islands of points" are connected
# by edges that are traversed in each direction once, but the user
# can chose to generate a disjoint set of polygons as well.
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
gp.AddMessage("Starting lasboundary production ...")

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

### create the full path to the lasboundary executable
lasboundary_path = lastools_path+"\\lasboundary.exe"

### check if executable exists
if os.path.exists(lastools_path) == False:
    gp.AddMessage("Cannot find lasboundary.exe at " + lasboundary_path)
    sys.exit(1)
else:
    gp.AddMessage("Found " + lasboundary_path + " ...")

### create the command string for lasboundary.exe
command = ['"'+lasboundary_path+'"']

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

### maybe use a user-specified concavity
if sys.argv[c] != "50":
    command.append("-concavity")
    command.append(sys.argv[c].replace(",","."))

### maybe thin with grid first
if sys.argv[c+1] == "true":
    command.append("-thin_with_grid")
    command.append(str(float(sys.argv[c].replace(",","."))/4))
c = c + 2

### what should we contour
if sys.argv[c] == "ground points only":
    command.append("-keep_class")
    command.append("2")
elif sys.argv[c] == "vegetation":
    command.append("-keep_class")
    command.append("3")
    command.append("4")
    command.append("5")
elif sys.argv[c] == "buildings":
    command.append("-keep_class")
    command.append("6")
elif sys.argv[c] == "keypoints":
    command.append("-keep_class")
    command.append("8")
elif sys.argv[c] == "water":
    command.append("-keep_class")
    command.append("9")
elif sys.argv[c] == "overlap points":
    command.append("-keep_class")
    command.append("12")
c = c + 1

### maybe discover disjoint polygons
if sys.argv[c] == "true":
    command.append("-disjoint")
c = c + 1

### maybe report holes in the interior
if sys.argv[c] == "true":
    command.append("-holes")
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

### report output of lasboundary
gp.AddMessage(str(output))

### check return code
if returncode != 0:
    gp.AddMessage("Error. lasboundary failed.")
    sys.exit(1)

### report happy end
gp.AddMessage("Success. lasboundary done.")
