# GDAL lib integration

GDAL is used to read, write and process raster and vector data. 

The GDAL library is used in LAStools (lasboundary, lasclip, lasdistance and las3oly) 
to read, write and convert the vector formats SHP, KML, GeoJSON, GML, GPKG and GPX, 
including the handling of coordinate reference systems (CRS).

**Key features include:**
- Support for numerous vector formats for reading and writing 
(SHP, KML, GeoJSON, GML, GPKG, GPX).
- Management and interpretation of CRS.

The integration of the GDAL library provides a flexible and powerful 
means to handle a multitude of different vector formats within LAStools.

The source code for the GDAL library is available at 
  https://github.com/OSGeo/gdal.


## Installing the GDAL Library

To use the GDAL library in LAStools, it must be installed on your 
system (possibly via QGIS or OSGeo4W or Conda).
Please note: This software requires GDAL version 3.0 or later.
If the GDAL library is not already installed, here are several 
methods to install it (We recommend using the default standard installation paths):

### Package Managers

- **OSGeo4W**: Use this **Windows-specific** package manager to install GDAL.
 1. Download [OSGeo4W setup] - *https://trac.osgeo.org/osgeo4w/*
 2. Start the installer 'osgeo4w-setup.exe' and select mode: Advanced Install
 3. In the package selection window: 'Commandline_Utilities -> gdal' or search for gdal using the search bar and select it.
 4. Finish installation

- **Conda**: Use this **Cross-Platform** package manager to install GDAL with the following command:
  ```
  conda install -c conda-forge gdal
  ```
  If a warning appears in Linux stating that the library was found via the Conda installation but could not be loaded, this is usually because the dynamic loader finds the incompatible system libraries first. Using `export LD_LIBRARY_PATH=~/miniconda3/lib:$LD_LIBRARY_PATH` ensures that the Conda libraries and their dependencies are loaded first.  
  Note that in this case, the environment variable only applies to the current shell session.


- **Docker**: Get the Docker image with:
  ```
  docker pull ghcr.io/osgeo/gdal
  ```
### QGIS/OSGeo4W Installation

By installing QGIS, you automatically get the GDAL library installed 
and configured.
- **Windows**:
 1. Download the QGIS installer from the [QGIS setup] 
    (https://www.qgis.org/download/).
 2. Run the installer and follow the installation steps.
 3. Once QGIS is installed, the GDAL library will be available.
                
- **Linux**:
  **Debian/Ubuntu**:
  QGIS can also be installed on Linux distributions, which will 
  include the GDAL library.
 1. Add the QGIS repository:
    ```bash
    sudo add-apt-repository ppa:ubuntugis/ppa
    sudo apt update
    ```
 2. Install QGIS:
    ```
    sudo apt install qgis qgis-plugin-grass
    ```
     The GDAL library is now installed with QGIS.
     
     **Fedora:**:
      ```
      sudo dnf install qgis qgis-grass
      ```

### Further installation options
- **Linux**:
  **Debian/Ubuntu**:
  ```
  sudo apt-get install gdal-bin
  ```
  **Fedora**:
  ```
  sudo dnf install gdal
  ```
  **Red Hat**:
  ```
  sudo yum install gdal
  ```

- **Mac OS X**:
  Using Homebrew:
  ```
  brew install gdal
  ```
  Or MacPorts:
  ```
  sudo port install gdal
  ```

### Required Files for GDAL in LASTools

To ensure that the GDAL library functions correctly with LASTools,
 you need to have the following library files available:

- `gdal`
- `proj`
- `sqlite3`

Additionally, ensure that the PROJ data files, including the 
`proj.db`, are installed for full functionality.

### File and Data Locations

- **`file` and `data` standart Directories**: If LAStools uses the 
GDAL library, the files and data of the library are automatically 
searched for in the default standard directories of the installation via 
QGIS or OSGeo4W or Conda.  

- **Custom Directory**: Alternatively, you can place the library 
files and GDAL data in custom directories of your choice via environment variables. We recommend only using this option if the GDAL lib is not found automatically via the QGIS or OSGeo4W or Conda installations.
  ```
  LASTOOLS_GDAL
  ```
  for the GDAL library directory.
  If the GDAL library cannot find the GDAL data directory itself set
  ```
  GDAL_DATA
  ```
  Which is a GDAL-specific environment variable and specifies the directory to GDAL data.

To use these custom directories,set the environment variables via command line (cmd) as shown in the following example:

- **Windows**:
  **Temporary** for the current session:
  ```
  set LASTOOLS_GDAL=C:\path\to\gdal_lib
  ```
  and if required
  ```
  set GDAL_DATA=C:\path\to\gdal_data
  ```

  Set environment variable **permanently** (applies to new CMD windows, but not to the current session):
  ```
  setx LASTOOLS_GDAL "C:\path\to\gdal_lib"
  ```
  and if required
  ```
  setx GDAL_DATA "C:\path\to\gdal_data"
  ```

  Set environment variable **permanently and system-wide** for all users (need to run with admin rights):
  ```
  setx LASTOOLS_GDAL "C:\path\to\gdal_lib" /M
  ```
  and if required
  ```
  setx GDAL_DATA "C:\path\to\gdal_data" /M
  ```

- **Linux**:
  **Temporary** for the current session:
  ```
  export LASTOOLS_GDAL=/path/to/gdal_lib
  ```
  and if required
  ```
  export GDAL_DATA=/path/to/gdal_data
  ```
  Set environment variable **permanently** for this user:
  Open your Bash configuration file:
  ```
  nano ~/.bashrc
  ```
  Add this line at the end of the file:
  ```
  export LASTOOLS_GDAL=/path/to/gdal_lib
  ```
  and if required
  ```
  export GDAL_DATA=/path/to/gdal_data
  ```
  save and close the file and then execute the command
  ```
  source ~/.bashrc
  ```

  Set environment variable **permanently and system-wide** for all users:
  Open the system file for environment variables:
  ```
  sudo nano /etc/environment
  ```
  Add the variable to the file:
  ```
  LASTOOLS_GDAL="/path/to/gdal_lib"
  ```
  and if required
  ```
  GDAL_DATA="/path/to/gdal_data"
  ```
  save and close the file and then execute the command
  ```
  source /etc/environment
  ```

**Check the environment variables**:
- **cmd - windows**:
  ```
  echo %LASTOOLS_GDAL%
  ```
  ```
  echo %GDAL_DATA%
  ```
- **linux**:
  ```
  echo $LASTOOLS_GDAL
  ```
  ```
  echo $GDAL_DATA
  ```
If you want to use the GDAL library later via QGIS, OSGeo4W or another installation, you must first delete the environment variables LASTOOLS_GDAL and GDAL_DATA if they have been set permanently.

### Note

For more information and detailed instructions about the 
installation, refer to the official GDAL installation page
  https://gdal.org/en/stable/download.html


## Licensing

This library is free to use.
GDAL is released under the X/MIT open source License.
For more information about the license and usage of the GDAL 
library, refer to the official GDAL repository or the official website 
  https://gdal.org/.

## Support

1. We invite you to join our LAStools Google Group (http://groups.google.com/group/lastools/).
   If you are looking for information about a specific tool, enter the tool name in the search 
   function and you'll find all discussions related to the respective tool. 
2. Customer Support Page: https://rapidlasso.de/customer-support/.  
3. Download LAStools: https://rapidlasso.de/downloads/.  
4. Changelog: https://rapidlasso.de/changelog/.  


If you want to send us feedback or have questions that are not answered in the resources above, 
please email to info@rapidlasso.de.

