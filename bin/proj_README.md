# proj lib integration

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
system (possibly via qigis or OSGeo4W).
If the PROJ library is not already installed, here are several 
methods to install it:

### Package Managers

- **Conda**: Install PROJ with the following command:
  ```
  conda install -c conda-forge proj
  ```
  And for the PROJ data:
  ```
  conda install -c conda-forge proj-data
  ```
- **Docker**: Get the Docker image with:
  ```
  docker pull osgeo/proj
  ```
### QGIS/OSGeo4W Installation

By installing QGIS, you automatically get the PROJ library installed 
and configured.
- **Windows**:
 1. Download the QGIS installer from the [QGIS setup] 
    (https://www.qgis.org/download/).
 2. Run the installer and follow the installation steps.
 3. Once QGIS is installed, the PROJ library will be available.
                
 - Or use the [OSGeo4W setup]
   (https://download.osgeo.org/osgeo4w/v2/) and select "proj" 
   under   "Commandline_Utilities".
  The OSGeo4W installer allows you to install QGIS along with the 
  PROJ library.

- **Linux**:
  **Debian/Ubuntu**:
  QGIS can also be installed on Linux distributions, which will 
  include the PROJ library.
 1. Add the QGIS repository:
    ```bash
    sudo add-apt-repository ppa:ubuntugis/ppa
    sudo apt update
    ```
 2. Install QGIS:
    ```
    sudo apt install qgis qgis-plugin-grass
    ```
     The PROJ library is now installed with QGIS.
     
     **Fedora:**:
      ```
      sudo dnf install qgis qgis-grass
      ```

### Further installation options
- **Linux**:
  **Debian/Ubuntu**:
  ```
  sudo apt-get install proj-bin
  ```
  **Fedora**:
  ```
  sudo dnf install proj
  ```
  **Red Hat**:
  ```
  sudo yum install proj
  ```

- **Mac OS X**:
  Using Homebrew:
  ```
  brew install proj
  ```
  Or MacPorts:
  ```
  sudo port install proj
  ```

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
searched for in the standard directories of the installation via 
Conda or QGIS/OSGeo4W.  

- **Custom Directory**: Alternatively, you can place the library 
files and PROJ data in custom directories of your choice. To use 
these custom directories, set the environment variables as follows:
  ```
  set LASTOOLS_PROJ=...
  ```
  for the PROJ library directory, and
  ```
  set PROJ_LIB=...
  ```
  for the PROJ data directory.

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
