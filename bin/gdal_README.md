# gdal lib integration

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
system (possibly via QGIS or OSGeo4W).
Please note: This software requires GDAL version 3.0 or later.
If the GDAL library is not already installed, here are several 
methods to install it:

### Package Managers

- **Conda**: Install GDAL with the following command:
  ```
  conda install -c conda-forge gdal
  ```

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
                
 - Or use the [OSGeo4W setup]
   (https://download.osgeo.org/osgeo4w/v2/) and select "gdal" 
   under   "Commandline_Utilities".
  The OSGeo4W installer allows you to install QGIS along with the 
  GDAL library.

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
searched for in the standard directories of the installation via 
Conda or QGIS/OSGeo4W.  

- **Custom Directory**: Alternatively, you can place the library 
files and GDAL data in custom directories of your choice. To use 
these custom directories, set the environment variables as follows:
  ```
  set LASTOOLS_GDAL=...
  ```
  for the GDAL library directory, and
  ```
  set GDAL_DATA=...
  ```
  for the GDAL data directory.

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

