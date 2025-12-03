# PROJ lib integration

PROJ is a generic coordinate transformation software that transforms 
geospatial coordinates from one coordinate reference system (CRS) to 
another. This includes cartographic projections as well as geodetic 
transformations. 
PROJ is used and utilized in LAStools (las2las) to transform LiDAR data between 
various coordinate reference systems and to retrieve detailed  
represenations and information about CRS (lasinfo).

**Key features include:**
- Transformation of point coordinates between different CRS.  
- Conversion and representation of CRS in various formats (EPSG, 
WKT, PROJ string, PROJJSON).
- Retrieval of metadata such as ellipsoids, axes, and datum 
information related to a CRS.

The integration of the PROJ library provides a flexible and powerful 
means to perform geodetic transformations directly within LAStools.

The source code for the PROJ library is available at 
  https://github.com/OSGeo/PROJ.


## Installing the PROJ Library

To use the PROJ library in LAStools, it must be installed on your 
system (possibly via QGIS or OSGeo4W or Conda).
Please note: This software requires PROJ version 9.0.0 or later.
If the PROJ library is not already installed, here are several 
methods to install it (We recommend using the default standard installation paths):

### Package Managers

- **OSGeo4W**: Use this **Windows-specific** package manager to install PROJ.
 1. Download [OSGeo4W setup] - *https://trac.osgeo.org/osgeo4w/*
 2. Start the installer 'osgeo4w-setup.exe' and select mode: Advanced Install
 3. In the package selection window: 'Commandline_Utilities -> proj' or search for proj using the search bar and select it.
 4. Finish installation

- **Conda**: Use this **Cross-Platform** package manager to install PROJ with the following command:

    conda install -c conda-forge proj
  And for the PROJ data:

    conda install -c conda-forge proj-data

  If a warning appears in Linux stating that the library was found via the Conda installation but could not be loaded, this is usually because the dynamic loader finds the incompatible system libraries first. Using `export LD_LIBRARY_PATH=~/miniconda3/lib:$LD_LIBRARY_PATH` ensures that the Conda libraries and their dependencies are loaded first.  
  Note that in this case, the environment variable only applies to the current shell session.

The environment variable only applies to the current shell session.

- **Docker**: Get the Docker image with:

    docker pull osgeo/proj


### QGIS Installation

By installing QGIS, you automatically get the PROJ library installed 
and configured.
- **Windows**:
 1. Download the QGIS installer from the [QGIS setup] 
    (https://www.qgis.org/download/).
 2. Run the installer and follow the installation steps.
 3. Once QGIS is installed, the PROJ library will be available.
                
- **Linux**:
  **Debian/Ubuntu**:
  QGIS can also be installed on Linux distributions, which will 
  include the PROJ library.
 1. Add the QGIS repository:

    sudo add-apt-repository ppa:ubuntugis/ppa
    sudo apt update

 2. Install QGIS:

    sudo apt install qgis qgis-plugin-grass

   The PROJ library is now installed with QGIS.
     
   **Fedora:**:
    sudo dnf install qgis qgis-grass

### Further installation options
- **Linux**:
  **Debian/Ubuntu**:

    sudo apt-get install proj-bin

  **Fedora**:

    sudo dnf install proj

  **Red Hat**:

    sudo yum install proj


- **Mac OS X**:
  Using Homebrew:

    brew install proj

  Or MacPorts:

    sudo port install proj


### Required Files for PROJ in LASTools

To ensure that the PROJ library functions correctly with LASTools,
 you need to have the following library files available:

- `proj`  
- `libcrypto`  
- `libcurl`  
- `libssl`  
- `sqlite3`  
- `tiff`  
- `zlib`  

Additionally, ensure that the PROJ data files, including the 
`proj.db`, are installed for full functionality.

### File and Data Locations

- **`file` and `data` standart Directories**: If LAStools uses the 
PROJ library, the files and data of the library are automatically 
searched for in the default standard directories of the installation via 
QGIS or OSGeo4W or Conda.  

- **Custom Directory**: Alternatively, you can place the library 
files and PROJ data in custom directories of your choice via environment variables. We recommend only using this option if the PROJ lib is not found automatically via the QGIS or OSGeo4W or Conda installations.

    LASTOOLS_PROJ

  for the PROJ library directory.
  If the PROJ library cannot find the PROJ data directory itself set

    PROJ_LIB

  Which is a PROJ-specific environment variable and specifies the directory to PROJ data.

To use these custom directories, set the environment variables via command line (cmd) as shown in the following example:

- **Windows**:
  **Temporary** for the current session:

    set LASTOOLS_PROJ=C:\path\to\proj_lib

  and if required

    set PROJ_LIB=C:\path\to\proj_data


  Set environment variable **permanently** (applies to new CMD windows, but not to the current session):

    setx LASTOOLS_PROJ "C:\path\to\proj_lib"
  and if required

    setx PROJ_LIB "C:\path\to\proj_data"

  Set environment variable **permanently and system-wide** for all users (need to run with admin rights):

    setx LASTOOLS_PROJ "C:\path\to\proj_lib" /M

  and if required

    setx PROJ_LIB "C:\path\to\proj_data" /M

- **Linux**:
  **Temporary** for the current session:

    export LASTOOLS_PROJ=/path/to/proj_lib

  and if required

    export PROJ_LIB=/path/to/proj_data

  Set environment variable **permanently** for this user:
  Open your Bash configuration file:

    nano ~/.bashrc

  Add this line at the end of the file:

    export LASTOOLS_PROJ=/path/to/proj_lib

  and if required

    export PROJ_LIB=/path/to/proj_data

  save and close the file and then execute the command

    source ~/.bashrc

  Set environment variable **permanently and system-wide** for all users:
  Open the system file for environment variables:

    sudo nano /etc/environment
    
  Add the variable to the file:

    LASTOOLS_PROJ="/path/to/proj_lib"

  and if required

    PROJ_LIB="/path/to/proj_data"

  save and close the file and then execute the command or (best) reboot your system to ensure the changes are affecting  

    source /etc/environment


**Check the environment variables**:
- **cmd - windows**:

    echo %LASTOOLS_PROJ%
    echo %PROJ_LIB%

- **linux**:

    echo $LASTOOLS_PROJ
    echo $PROJ_LIB

If you want to use the PROJ library later via QGIS, OSGeo4W or another installation, you must first delete the environment variables LASTOOLS_PROJ and PROJ_LIB if they have been set permanently.

### Important Note

Ensure that the version of the PROJ data matches the version of the 
PROJ library to avoid compatibility issues.
For more information and detailed instructions about the 
installation, refer to the official PROJ installation page
  https://proj.org/install.html


## Licensing

This library is free to use.
PROJ is released under the X/MIT open source License.
For more information about the license and usage of the PROJ 
library, refer to the official PROJ repository or the official website 
  https://proj.org.

## Support

1. We invite you to join our LAStools Google Group (http://groups.google.com/group/lastools/).
   If you are looking for information about a specific tool, enter the tool name in the search 
   function and you'll find all discussions related to the respective tool. 
2. Customer Support Page: https://rapidlasso.de/customer-support/.  
3. Download LAStools: https://rapidlasso.de/downloads/.  
4. Changelog: https://rapidlasso.de/changelog/.  


If you want to send us feedback or have questions that are not answered in the resources above, 
please email to info@rapidlasso.de.
