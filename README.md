# LAStools

**Award-winning software for efficient LiDAR processing (with LASzip)**

## Overview

Tools to process LiDAR data files.
The data files has the ASPRS LAS format (version 1.0-1.4) or the losslessly compressed, but otherwise identical twin, the LAZ format.
See the table below to see what tools and converters are provided.

LAStools consist of different parts:
* LASlib, the low level processing API.
* LASzip, the compressing/decompressing API.
Both parts are for free and open source.
* LAStools toolset.
Additional tools, based on LASlib and LASzip to process LiDAR data.
Some of the tools are also free to use.
Some of them are licensed.

All code is written in ultra-light-weight, very efficient and superfast C++.

LAStools are a collection of highly-efficient, scriptable tools with multi-core batching that process LAS, compressed LAZ, Terrasolid BIN, ESRI Shapefiles (SHP), ASCII and others.

## Open source tools:

* `las2las` extracts last returns, clips, subsamples, translates, etc ...
* `las2txt` turns LAS into human-readable and easy-to-parse ASCII
* `lasindex`  creates a spatial index LAX file for fast spatial queries
* `lasinfo` prints out a quick overview of the contents of a LAS file
* `lasmerge`  merges several LAS or LAZ files into a single LAS or LAZ file
* `lasprecision`  analyses the actual precision of the LIDAR points
* `laszip`  powerful, lossless LiDAR compressor that turns large LAS files into much smaller LAZ files that are only 7 - 20 percent of the original file size
* `txt2las` converts LIDAR data from ASCII text to binary LAS format
* `lascopcindex` create a copc indexed copc.laz file

## Closed source tools:

* `las2dem` rasters (via a TIN) into elevation/slope/intensity/rgb DEMs
* `las2iso` extracts, optionally simplified, elevation contours
* `las2shp` turns binary LAS into ESRI's Shapefile format
* `las2tin` triangulates the points of a LAS file into a TIN
* `lasboundary` extracts a boundary polygon that encloses the points
* `lascanopy` computes many raster and plot metrics for forestry applications
* `lasclassify` finds buildings and the vegetation above the ground
* `lasclip` clips points against building footprints / swath boundaries
* `lascolor`  colors the LAS points based on ortho imagery in TIF format 
* `lascontrol`  quality checks elevations for a list of control points 
* `lascopy` copies ttributes using the GPS-time stamp and the return number
* `lasdatum` transforms rom one horizontal datum to another
* `lasdistance` classifies,flags, or removes points based on distance from polygonal segments
* `lasduplicate` removes duplicate points (with identical x and y, z optional) 
* `lasgrid` grids onto min/max/avg/std elevation, intensity, or counter rasters
* `lasground` extracts the bare-earth by classifying all ground points
* `lasground_new` an improved version of lasground for complex terrains
* `lasheight` computes for each point its height above the ground
* `lasnoise` flags r removes noise points in LAS/LAZ/BIN/ASCII files
* `lasoptimize` optimizes ata for better compression and spatial coherency
* `lasoverage` finds he "overage" of a airborne collect that get covered by multiple flightline
* `lasoverlap` checks overlap & vertical/horizontal alignment of flight lines
* `lasplanes` finds planar patches in terrestrial, mobile, (airborne?) scans 
* `lasprecision` reads IDAR data in the LAS format and computes statistics about precision "advertised" in the header
* `lasprobe` probes he elevation of a LIDAR for a given x and y location
* `laspublish` do D visualization of LiDAR data in a web browser using the WebGL Potree
* `lasreturn` reports eometric return statistics and repairs 'number of returns' field based on GPS times
* `lassort` sorts points by gps_time, point_source, or into spatial proximity
* `lassplit` splits points of LAS file(s) into flightlines or other criteria
* `lasthin` thins lowest / highest / random LAS points via a grid
* `lastile` tiles huge amounts of LAS points into square tiles
* `lastool` is an old GUI for multiple LAStools (now each tool has its own GUI)
* `lastrack` classifies LiDAR point based on distance from a trajectory
* `lasvalidate` determine f LAS files are conform to the ASPRS LAS specifications
* `lasvdatum` transforms iDAR from ellipsoidal to orthometric elevations using a grid
* `lasview` visualizes a LAS file with a simple OpenGL viewer
* `lasvoxel` computes  voxelization of points
* `e572las` extracts the points from the E57 format and stores them as LAS/LAZ files
* `shp2las` turns an ESRI's Shapefile into binary LAS

## BLAST extension

* `blast2dem` rasters like las2dem but with streaming TINs for billions of points. 
* `blast2iso` contours like las2iso but with streaming TINs for billions of points. 

## Documentation

All documentation is provided as a readme markdown files (*.md) in the ".\bin" directory. 

## Installation

All binaries can be downloaded at https://rapidlasso.de/downloads/

Just download the files to a directory.
There is no installation needed. Just run the tools from commandline or use 
a client software to access the tools.

## Usage

### Console
This command will do a ground classification of the "lake.laz" file and writes the 
result into the file "out.laz".  
    las2ground64 -i lake.laz -o out.laz
It is common to use the commands in a batch script to process huge amount of data. 

### Client software: laslook
laslook is a graphical frontend and workbench for LAStools.
The software contains the full documentation of LAStools and is free to use.
See https://rapidlasso.de/laslook/ for details.

### Client software: QGIS plugin
The QGIS plugin is available within the QGIS plugin manager.

### Client software: ArcGIS pro plugin
The windows binary download contains the ArcGIS toolbox in `.\LAStools\ArcGIS_toolbox`.

## Compile the sources
To build own binaries out of this sources it is recommended to use cmake.

    cmake .
    cmake --build .
    
For windows there is also a MSVC solution file (*.sln) and project files in the repository.
This provides a simple "Release" build configuration.

A few example DOS batch scripts can be found in the `.\LAStools\example_batch_scripts` directory.

## Links

* download page: https://rapidlasso.de/downloads/
* company page:  https://rapidlasso.de
* user group:     http://groups.google.com/group/lastools

## License

Please read the `LICENSE.txt` file for information on the legal use and licensing
of LAStools. We appreciate any feedback. 

(c) 2007-2023 info@rapidlasso.de - https://rapidlasso.de
