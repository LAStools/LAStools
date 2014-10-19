LAStools - award-winning software for efficient LiDAR processing (with LASzip)

You find the individual LAStools in the .\lastools\bin directory. Start
them by double-clicking or run them in the DOS command line. The ArcGIS
toolbox can be found in the .\lastools\ArcGIS_toolbox directory.

compant page:    http://rapidlasso.com
latest updates:  http://lastools.org
user group:      http://groups.google.com/group/lastools
twitter feed:    http://twitter.com/lastools
facebook page:   http://facebook.com/lastools
linkedin group:  http://linkedin.com/groups?gid=4408378

* lastool.exe is a combined GUI for multiple LAStools (each tool has its own GUI)

* lasground.exe extracts the bare-earth by classifying all ground points
* lasoverlap.exe checks overlap & vertical/horizontal alignment of flight lines
* lascontrol.exe quality checks elevations for a list of control points 
* lasclassify.exe finds buildings and the vegetation above the ground
* lascolor.exe colors the LAS points based on ortho imagery in TIF format 
* lasgrid.exe grids onto min/max/avg/std elevation, intensity, or counter rasters
* lascanopy.exe computes many raster and plot metrics for forestry applications
* lasboundary.exe extracts a boundary polygon that encloses the points
* lasheight.exe computes for each point its height above the ground
* lastrack.exe classifies LiDAR point based on distance from a trajectory
* lasplanes.exe finds planar patches in terrestrial, mobile, (airborne?) scans 
* lasclip.exe clips points against building footprints / swath boundaries
* lastile.exe tiles huge amounts of LAS points into square tiles
* laszip.exe compresses the LAS files in a completely lossless manner
* lasinfo.exe prints out a quick overview of the contents of a LAS file
* lasindex.exe creates a spatial index LAX file for fast spatial queries
* txt2las.exe converts LIDAR data from ASCII text to binary LAS format
* las2txt.exe turns LAS into human-readable and easy-to-parse ASCII
* lasmerge.exe merges several LAS files into one
* lassplit.exe splits points of LAS file(s) into flightlines or other criteria
* lassort.exe sorts points by gps_time, point_source, or into spatial proximity
* las2las.exe extracts last returns, clips, subsamples, translates, etc ...
* lasduplicate.exe removes duplicate points (with identical x and y, z optional) 
* lasthin.exe thins lowest / highest / random LAS points via a grid
* las2tin.exe triangulates the points of a LAS file into a TIN
* las2dem.exe rasters (via a TIN) into elevation/slope/intensity/rgb DEMs
* las2iso.exe extracts, optionally simplified, elevation contours
* lasview.exe visualizes a LAS file with a simple OpenGL viewer
* lasprecision.exe analyses the actual precision of the LIDAR points
* las2shp.exe turns binary LAS into ESRI's Shapefile format
* shp2las.exe turns an ESRI's Shapefile into binary LAS

* blast2dem.exe rasters like las2dem but with streaming TINs for billions of points. 
* blast2iso.exe contours like las2iso but with streaming TINs for billions of points. 

For Windows all binaries are included. They can also be compiled from the
source code and you can find the MSVC6.0 project files there as well. For
Linux the makefiles and many sources are included. Simply go into the root
directory and run 'make':

unzip lastools.zip
cd lastools/
make

The compiled binary executables are or will be in the ./lastools/bin directory.

---

Please read the "LICENSE.txt" file for information on the legal use and licensing
of LAStools. I would also really like it if you would send me an email and tell me
what you use LAStools for and what features and improvements you could need. 

(c) 2007-2014 martin.isenburg@rapidlasso.com - http://rapidlasso.com
