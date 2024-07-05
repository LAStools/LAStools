.. _home:

******************************************************************************
LASzip - free and lossless LiDAR compression
******************************************************************************

LASzip - a free open source product of `rapidlasso GmbH <http://rapidlasso.com/>`_ - quickly turns bulky LAS files into compact LAZ files without information loss. Terabytes of LAZ data are now available for free download from various agencies making LASzip, winner of the 2012 Geospatial World Forum `Technology Innovation Award <http://www.facebook.com/photo.php?fbid=370334049695712>`_ in LiDAR Processing and runner-up for `innovative product at INTERGEO 2012 <http://gispoint.de/wia.html>`_, the de-facto standard for LiDAR compression.

Source
..............................................................................

* **2019-11-11**

  - `laszip-3.4.3.tar.gz <https://github.com/LASzip/LASzip/releases/download/3.4.3/laszip-src-3.4.3.tar.gz>`_
    `(md5) <https://github.com/LASzip/LASzip/releases/download/3.4.3/laszip-src-3.4.3.tar.gz.sha256sum>`__


Past Release(s)
~~~~~~~~~~~~~~~~~~~~~~~~~

* **2019-04-12**

  - `laszip-3.4.1.tar.gz <https://github.com/LASzip/LASzip/releases/download/3.4.1/laszip-src-3.4.1.tar.gz>`_
    `(md5) <https://github.com/LASzip/LASzip/releases/download/3.4.1/laszip-src-3.4.1.tar.gz.sha256sum>`__

* **2018-12-27**

  - `laszip-3.2.9.tar.gz <https://github.com/LASzip/LASzip/releases/download/3.2.9/laszip-src-3.2.9.tar.gz>`_
    `(md5) <https://github.com/LASzip/LASzip/releases/download/3.2.9/laszip-src-3.2.9.tar.gz.md5>`__

* **2018-11-19**

  - `laszip-3.2.8.tar.gz <https://github.com/LASzip/LASzip/releases/download/3.2.8/laszip-src-3.2.8.tar.gz>`_
    `(md5) <https://github.com/LASzip/LASzip/releases/download/3.2.8/laszip-src-3.2.8.tar.gz.md5>`__

* **2018-03-27**

  - `laszip-3.2.2.tar.gz <https://github.com/LASzip/LASzip/releases/download/3.2.2/laszip-src-3.2.2.tar.gz>`_
    `(md5) <https://github.com/LASzip/LASzip/releases/download/3.2.2/laszip-src-3.2.2.tar.gz.md5>`__

* **2017-10-10**

  - `laszip-3.1.1.tar.gz <https://github.com/LASzip/LASzip/releases/download/3.1.1/laszip-src-3.1.1.tar.gz>`_
    `(md5) <https://github.com/LASzip/LASzip/releases/download/3.1.1/laszip-src-3.1.1.tar.gz.md5>`__

* **2017-09-13**

  - `laszip-3.1.0.tar.gz <https://github.com/LASzip/LASzip/releases/download/3.1.0/laszip-src-3.1.0.tar.gz>`_

* **2013-08-05**

  - `laszip-2.2.0.tar.gz <https://github.com/LASzip/LASzip/releases/download/v2.2.0/laszip-src-2.2.0.tar.gz>`_
    `(md5) <https://github.com/LASzip/LASzip/releases/download/v2.2.0/laszip-src-2.2.0.tar.gz.md5>`__

  - `laszip-win32-msvc2009.zip <https://github.com/LASzip/LASzip/releases/download/v2.2.0/laszip-win32-msvc2009.zip>`_
    `(md5) <https://github.com/LASzip/LASzip/releases/download/v2.2.0/laszip-win32-msvc2009.zip.md5>`__


Platinum Sponsors (USD 25,000 or more)
------------------------------------------------------------------------------
* `Digital Coast - NOAA Office for Coastal Management <http://coast.noaa.gov/digitalcoast/>`_

Gold Sponsors (USD 10,000 or more)
------------------------------------------------------------------------------
* `CRREL - U.S. Army Corps of Engineers <http://www.crrel.usace.army.mil/>`_

Silver Sponsors (USD 5,000 or more)
------------------------------------------------------------------------------
* `RIEGL Laser Measurement Systems <http://riegl.com/>`_

Bronze Sponsors (USD 2,000 or more)
------------------------------------------------------------------------------
* `Quantum Spatial Inc. <http://quantumspatial.com/>`_
* `Trimble Geospatial <http://www.trimble.com/Industries/GeoSpatial/>`_

Download
------------------------------------------------------------------------------

The Apache 2.0-licensed LASzip library is easiest integrated via the DLL that is in
the LASzip subdirectory of the `LAStools`_ distribtion. Other options are to
link LASzip via the BSD-licensed `libLAS`_ library or to work with the `LASlib`_
library which fully integrates and enhances the LASzip codebase with spatial
indexing, filters, transforms, geo-referencing, ... of LAS and LAZ files.



Binaries
..............................................................................

  - See `OSGeo4W`_ for Windows .lib and
    include files to compile LASzip into your own application. `libLAS`_
    binaries through `OSGeo4W`_ also link LASzip.

  - For explicit compression (decompression) of LAS (LAZ) files a Windows
    binary `laszip.exe`_ is available (command-line only `laszip-cli.exe`_),
    as well as source code, examples, and makefiles to build your
    own `LASzip`_ on Windows, Linux, or MacOS.


Documentation
..............................................................................

  - The LASzip `paper`_ and `video`_ from the ELMF 2011 presentation in Salzburg, Austria. An `interview <http://geodatapoint.com/articles/view/expanding_access_to_lidar_through_compression>`_ with editor Christine Grahl of POB magazine.


Development Source
------------------------------------------------------------------------------

* Current Trunk: https://github.com/LASzip/LASzip


Background
------------------------------------------------------------------------------

LASzip is a compression library that was developed by `Martin Isenburg`_ for
compressing `ASPRS LAS format`_ data in his `LAStools`_. It has been provided
as an `LGPL`_-licensed stand-alone software library to allow other softwares
that handle LAS data to read and write LASzip-compressed data. The BSD-licensed
`libLAS`_ and the LGPL-licensed `LASlib`_ can take advantage of LASzip to read
and write compressed data.

LASzip was originally released as an `LGPL`_-licensed library, but in January
2022, it was relicensed as `Apache Public License 2.0
<https://www.apache.org/licenses/LICENSE-2.0>`__ by `Rapidlasso GmbH
<https://rapidlasso.de>`__. For those wishing for validation of this licensing
change, a notarized document to that effect is available by contacting
Howard Butler at howard@hobu.co or Rapidlasso, GmbH at info@rapidlasso.de.

LASzip is completely lossless. It compresses bulky LAS files into compact LAZ
files that are only 7-20 percent of the original size, accurately preserving
every single bit. For example, compressing and decompressing the LAS file
lidar.las with `laszip.exe`_ (command-line only `laszip-cli.exe`_) as shown below results in lidar_copy.las that
is bit-identical to lidar.las. However, the small size of lidar.laz makes it
much easier to store, copy, transmit, or archive large amounts of LIDAR.

* laszip -i lidar.las -o lidar.laz
* laszip -i lidar.laz -o lidar_copy.las

LASzip compression can be many times smaller and many times faster than
generic compressors like `bz2`_, `gzip`_, and `rar`_ because it knows what
the different bytes in a LAS file represent. Another advantage of LASzip
is that it allows you to treat compressed LAZ files just like standard LAS
files. You can load them directly from compressed form into your application
without needing to decompress them onto disk first. The availability of the
`LASzip`_ DLL and two APIs, `libLAS`_ and `LASlib`_, with LASzip capability
makes it easy to add native LAZ support to your own software package.

Software with native LAZ support
------------------------------------------------------------------------------

* Fugroviewer (2.0 and up) by `Fugro <http://www.fugroviewer.com/>`_
* QT Modeler (7.1.6 and up) by `Applied Imagery <http://www.appliedimagery.com/>`_
* ERDAS IMAGINE (14.1 and up) by `Hexagon Geospatial <http://www.hexagongeospatial.com/products/ERDAS-IMAGINE/Details.aspx>`_
* Global Mapper (13.1 and up) by `Blue Marble Geo <http://www.bluemarblegeo.com/>`_
* Trimble RealWorks (8.1 and up) by `Trimble Navigation Limited <http://www.trimble.com/3d-laser-scanning/realworks.aspx>`_
* ENVI LiDAR (5.1 and up) by `Exelis VIS <http://www.exelisvis.com/ProductsServices/ENVI/ENVILiDAR.aspx>`_
* FME (2012 and up) by `Safe Software <http://www.safe.com/>`_
* TopoDOT by `Certainty3D <http://www.certainty3d.com/products/topodot/>`_
* Pointools by `Bentley Systems <http://www.pointools.com/>`_
* Pix4uav by `Pix4D <http://www.pix4d.com/>`_
* CloudCompare by `Daniel Girardeau-Montaut <http://www.danielgm.net/cc/>`_
* SURE by `nframes <http://www.nframes.com/>`_
* Pointfuse by `Arithmetica <http://pointfuse.com/>`_
* LAStools by `rapidlasso - fast tools to catch reality <http://rapidlasso.com/>`_
* plas.io 3D Web Viewer by `Hobu Inc. <http://plas.io/>`_
* RiProcess by `RIEGL LMS GmbH <http://www.riegl.com/>`_
* CloudPro by `Leica Geosystems AG <http://www.leica-geosystems.com/en/About-us-News_360.htm?id=5380>`_
* Optech LMS (3.0 and up) by `Teledyne Optech <http://www.teledyneoptech.com/index.php/product/optech-lms/>`_
* Leica LIDAR Survey Studio LSS (2.2 and up) by `Leica Geosystems AG <http://www.leica-geosystems.com/en/About-us-News_360.htm?id=5960>`_
* FUSION (3.40 and up) by `Bob McGaughey of the USDA <http://forsys.cfr.washington.edu/fusion/fusionlatest.html/>`_
* ZEB1 by `3D Laser Mapping <http://www.3dlasermapping.com/index.php/products/handheld-mapping>`_
* OCAD (11.4.0 and up) by `OCAD Inc. <http://ocad.com/blog/2013/11/ocad-11-4-0-supports-laz-files/>`_
* Gexcel R3 by `Gexcel <http://www.gexcel.it/en/software/>`_
* Voyager (1.3 and up) by `Voyager GIS <http://www.voyagergis.com/>`_
* LIDAR Analyst (5.2 or 6.0 and up) by `TEXTRON Systems <http://www.textronsystems.com/>`_
* TerraScan, TerraMatch, TerraPhoto, and TerraSlave (015.xxx and up) by `Terrasolid <http://www.terrasolid.com/>`_
* Carlson (2016 and up) by `Carlson Software <http://carlsonsw.com/>`_
* Remote Sensing Software by `Joanneum Research Forschungsgesellschaft mbH <http://www.remotesensing.at/en/remote-sensing-software.html>`_
* LiMON Viewer by `Dephos Software <http://www.limon.eu/products/compare-versions/>`_
* Scanopy by `imagination <http://www.imagination.at/>`_
* WinGeoTransform by `KLT Assoc <http://www.kltassoc.com/>`_
* BayesStripAlign by `BayesMap Solutions, LLC <http://bayesmap.com/>`_
* Point Cloud Technology by `Point Cloud Technology, GmbH <http://www.pointcloudtechnology.com/>`_
* Scalypso by `Ingenieurbuero Dr.-Ing. R. Koenig <http://www.scalypso.com/>`_
* Photogrammetry Software by `Menci Software <http://www.menci.com/>`_
* TcpMDT by `Aplitop <http://www.aplitop.com/>`_
* PhotoMesh by `SmartEarth <http://www.skylineglobe.cn/>`_
* QINSy - Quality Integrated Navigation System by `Quality Positioning Services (QPS) B.V. <http://www.qps.nl/display/qinsy>`_
* Potree Converter (1.0 and up) by `Markus Schuetz <http://github.com/potree/PotreeConverter>`_
* FLAIM by `Fugro GeoServices B.V. <http://www.fugro.nl/werkgebieden/geo-informatie/geo-ict/software-productlijnen/flaim>`_
* ScanLook by `LiDAR USA <http://www.lidarusa.com/>`_
* GRASS GIS (7.0 and up) by `Open Source Geospatial Foundation (OSGeo) <http://grass.osgeo.org/>`_
* OPALS by `TU Vienna <http://www.ipf.tuwien.ac.at/opals/>`_
* Scop++ (5.5.3 and up) by `Trimble and TU Vienna <http://photo.geo.tuwien.ac.at/software/scop/>`_
* DTMaster (6.0 and up) by `Trimble INPHO <http://www.trimble.com/imaging/inpho/geo-modeling.aspx?dtID=DTMaster>`_
* Pythagoras by `Pythagoras BVBA <http://www.pythagoras.net/>`_
* EspaEngine by `ESPA Systems <http://www.espasystems.fi>`_
* Blaze Point Cloud and Blaze Terra by `Eternix Ltd <http://eternix.co.il/>`_
* ReportGen (2.9.0 and up), by `PDF3D <http://pdf3d.com/news/PDF3D_LiDAR_PressRelease_20dec2013.php>`_
* OrbitGIS by `Orbit <http://www.orbitgis.com/>`_
* PFMABE Software by `PFMABE Software <http://pfmabe.software/>`_
* K2Vi by `AAM Group <http://www.aamgroup.com/services-and-technology/3d-gis>`_
* Card/1 (8.4 and up) by `IB&T GmbH <http://www.card-1.com/en/product/the-system-card1/>`_
* SceneMark by `XtSense GmbH <http://scenemark.com/>`_
* MapWorks (6.0 and up) by `DotSoft <http://www.dotsoft.com/mapworks.htm>`_
* LiS by `LASERDATA <http://www.laserdata.at/>`_
* PointCloudViz by `mirage <http://www.mirage-tech.com/>`_
* Geoverse by `euclideon <http://www.euclideon.com/>`_
* PointCab by `PointCab GmbH <http://www.pointcab-software.com/en/technical-details/>`_
* Z+F SynCaT Mobile Mapping Software by `Z+F Zoller+Frohlich <http://www.zf-laser.com/Z-F-SynCaT-R.174.0.html>`_
* LidarViewer by `routescene <http://www.routescene.com/>`_
* 3Dsurvey by `Modri Planet d. o. o. <http://www.3dsurvey.si/>`_
* Vision Lidar by `geo-plus <http://www.geo-plus.com/>`_
* PPIMMS by `viametris <http://www.viametris.com/>`_
* SuperGIS by `Supergeo Technologies Inc. <http://www.supergeotek.com/>`_
* Smart Processing Lidar by `3D TARGET srl <http://www.3dtarget.it/>`_
* Geomapping+ by `GEOMAPPING <http://www.geomapping.fr/>`_
* GEOgraph 3D by `HHK Datentechnik GmbH <http://www.hhk.de/>`_
* Dot3DEdit (version 2.0 and up) by `DotProduct LLC <http://www.dotproduct3d.com/>`_
* HydroVish and KomVish by `AirborneHydroMapping GmbH <http://www.ahm.co.at/>`_

Download LAZ data
------------------------------------------------------------------------------
* USGS `LiDAR bulk download <ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/LPC/projects/>`_ such as

  * `AK_BrooksCamp_2012 <ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/LPC/projects/AK_BrooksCamp_2012/laz/>`_, `AK_Coastal_2009 <ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/LPC/projects/AK_Coastal_2009/laz/>`_, `AK_Fairbanks-NSBorough_2010 <ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/LPC/projects/AK_Fairbanks-NSBorough_2010/laz/>`_, `AK_Juneau_2012 <ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/LPC/projects/AK_Juneau_2012/laz/>`_, ...

  * `CA_AlamedaCo_2006 <ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/LPC/projects/CA_AlamedaCo_2006/laz/>`_, `CA-OR_KlamathRiver_2010 <ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/LPC/projects/CA-OR_KlamathRiver_2010/laz/>`_, `CA-NV_LakeTahoe_2010 <ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/LPC/projects/CA-NV_LakeTahoe_2010/laz/>`_, `CA_CanyonFire_2007 <ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/LPC/projects/CA_CanyonFire_2007/laz/>`_, ...

  * `... many more here ... <ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/LPC/projects/>`_

* Sonoma County LiDAR from `SonomaVegMap Project <http://sonomavegmap.org/data-downloads/>`_
* WMS served LiDAR by `Government of La Rioja in Spain <http://rapidlasso.com/2014/04/08/lidar-downloads-via-wms/>`_
* `Puget Sound LiDAR Consortium <http://pugetsoundlidar.ess.washington.edu/lidardata/>`_

  * 2007: `sumpter <http://pugetsoundlidar.ess.washington.edu/lidardata/restricted/las/pslc2007/sumpter/>`_

  * 2009: `douglasco <http://delta.ess.washington.edu/lazfiles/pslc2009/douglasco/>`_, `snohoriver <http://delta.ess.washington.edu/lazfiles/pslc2009/snohoriver/>`_, `umpqua <http://delta.ess.washington.edu/lazfiles/pslc2009/umpqua/>`_, `wenas <http://delta.ess.washington.edu/lazfiles/pslc2009/wenas/>`_, `wenatchee <http://delta.ess.washington.edu/lazfiles/pslc2009/wenatchee/>`_

  * 2011: `kittitas <http://delta.ess.washington.edu/lazfiles/pslc2011/kittitas/>`_, `quinault <http://delta.ess.washington.edu/lazfiles/pslc2011/quinault/>`_, `rattlesnake <http://delta.ess.washington.edu/lazfiles/pslc2011/rattlesnake/>`_

  * 2012: `chehalis <http://pugetsoundlidar.ess.washington.edu/lidardata/restricted/las/pslc2012/chehalis/>`_, `hoh <http://pugetsoundlidar.ess.washington.edu/lidardata/restricted/las/pslc2012/hoh/>`_, `jefferson_clallam <http://pugetsoundlidar.ess.washington.edu/lidardata/restricted/las/pslc2012/jefferson_clallam/>`_, `quinault <http://pugetsoundlidar.ess.washington.edu/lidardata/restricted/las/pslc2012/usgs_quinault/>`_, `upper_naches <http://pugetsoundlidar.ess.washington.edu/lidardata/restricted/las/pslc2012/upper_naches/>`_

* open LiDAR data strategy of the `National Land Survey of Finland <https://tiedostopalvelu.maanmittauslaitos.fi/tp/kartta?lang=en>`_
* nationwide open LiDAR data of `the Netherlands <https://groups.google.com/d/topic/lastools/Y0fdLGEJ6XE/discussion>`_
* `Digital Coast LiDAR <http://coast.noaa.gov/dataviewer>`_ by NOAA (+ `how to download <http://geozoneblog.wordpress.com/2013/01/28/how-download-lots-lidar-digital-coast>`_) and `folders for batch download <http://coast.noaa.gov/htdata/lidar1_z/>`_
* LiDAR of `Kanton Zurich, Switzerland <http://maps.zh.ch/download/hoehen/2014/lidar/>`_
* LiDAR of `Kanton Solothurn, Switzerland <http://www.catais.org/geodaten/ch/so/agi/hoehen/2014/lidar/>`_
* GRAFCAN `LiDAR of the Canary Islands <http://tiendavirtual.grafcan.es/visor.jsf?currentSeriePk=263585792>`_
* Alaska LiDAR in the `Matanuska-Susitna Borough <http://matsu.gina.alaska.edu/LiDAR/Point_MacKenzie/Point_Cloud/Classified.laz/>`_
* NSF-funded LiDAR hosting facility `OpenTopography <http://opentopo.sdsc.edu/gridsphere/gridsphere?cid=geonlidar>`_
* Clark County Kentucky LiDAR data `Clark County GIS <http://www.ccgisonline.com/LAZ/IndexMap.pdf>`_
* Bay of Plenty, New Zealand `BOPLASS LiDAR <http://www.arcgis.com/home/item.html?id=5be5a4460c064f0692aac7e288ddcef7>`_
* Nevada Barringer Meteorite Crater of the `Lunar and Planetary Institute <http://www.lpi.usra.edu/publications/books/barringer_crater_guidebook/LiDAR/>`_
* Alaska LiDAR hosted by `Division of Geological and Geophysical Surveys (DGGS) <http://maps.dggs.alaska.gov/lidar/#-16434084:9589812:5>`_

* Statewide LiDAR of `Elevation Mapping Project by Minnesota DNR <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/>`_

  * Counties: `aitkin <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/aitkin/laz/>`_, `anoka <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/anoka/laz/>`_, `becker <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/becker/laz/>`_, `beltrami <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/beltrami/laz/>`_, `benton <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/benton/laz/>`_, `bigstone <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/bigstone/laz/>`_, `blueearth <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/blueearth/2012/laz/>`_, `brown <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/brown/laz/>`_, `carlton <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/carlton/laz/>`_, `carver <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/carver/laz/>`_, `cass <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/cass/laz/>`_, `chippewa <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/chippewa/laz/>`_, `chisago <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/chisago/laz/>`_, `clay <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/clay/laz/>`_, `clearwater <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/clearwater/laz/>`_, `cook <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/cook/laz/>`_, `cottonwood <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/cottonwood/laz/>`_, `crowwing <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/crowwing/laz/>`_, `dakota <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/dakota/laz/>`_, `dodge <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/dodge/laz/>`_, `douglas <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/douglas/laz/>`_, `faribault <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/faribault/laz/>`_, `fillmore <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/fillmore/laz/>`_, `freeborn <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/freeborn/laz/>`_, `goodhue <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/goodhue/laz/>`_, `grant <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/grant/laz/>`_, `hennepin <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/hennepin/laz/>`_, `houston <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/houston/laz/>`_, `hubbard <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/hubbard/laz/>`_, `isanti <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/isanti/laz/>`_, `itasca <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/itasca/laz/>`_, `jackson <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/jackson/laz/>`_, `kanabec <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/kanabec/laz/>`_, `kandiyohi <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/kandiyohi/laz/>`_, `kittson <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/kittson/laz/>`_, `koochiching <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/koochiching/laz/>`_, `lacquiparle <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/lacquiparle/laz/>`_, `lake <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/lake/laz/>`_, `lakeofthewoods <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/lakeofthewoods/laz/>`_, `lesueur <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/lesueur/laz/>`_, `lincoln <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/lincoln/laz/>`_, `lyon <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/lyon/laz/>`_, `mahnomen <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/mahnomen/laz/>`_, `marshall <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/marshall/laz/>`_, `martin <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/martin/laz/>`_, `meeker <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/meeker/laz/>`_, `millelacs <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/millelacs/laz/>`_, `morrison <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/morrison/laz/>`_, `mower <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/mower/laz/>`_, `murray <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/murray/laz/>`_, `nicollet <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/nicollet/laz/>`_, `nobles <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/nobles/laz/>`_, `norman <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/norman/laz/>`_, `olmsted <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/olmsted/laz/>`_, `ottertail <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/ottertail/laz/>`_, `pennington <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/pennington/laz/>`_, `pine <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/pine/laz/>`_, `pipestone <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/pipestone/laz/>`_, `polk <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/polk/laz/>`_, `pope <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/pope/laz/>`_, `ramsey <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/ramsey/laz/>`_, `redlake <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/redlake/laz/>`_, `redwood <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/redwood/laz/>`_, `renville <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/renville/laz/>`_, `rice <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/rice/laz/>`_, `rock <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/rock/laz/>`_, `roseau <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/roseau/laz/>`_, `scott <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/scott/laz/>`_, `sherburne <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/sherburne/laz/>`_, `sibley <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/sibley/laz/>`_, `stearns <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/stearns/laz/>`_, `steele <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/steele/laz/>`_, `stevens <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/stevens/laz/>`_, `swift <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/swift/laz/>`_, `todd <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/todd/laz/>`_, `traverse <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/traverse/laz/>`_, `wabasha <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/wabasha/laz/>`_, `wadena <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/wadena/laz/>`_, `waseca <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/waseca/laz/>`_, `washington <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/washington/laz/>`_, `watonwan <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/watonwan/laz/>`_, `wilkin <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/wilkin/laz/>`_, `winona <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/winona/laz/>`_, `wright <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/wright/laz/>`_, `yellowmedicine <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/county/yellowmedicine/laz/>`_

  * Arrowhead: `block 1 <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/projects/arrowhead/block_1/laz/>`_, `block 2 <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/projects/arrowhead/block_2/laz/>`_, `block 3 <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/projects/arrowhead/block_3/laz/>`_, `block 4 <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/projects/arrowhead/block_4/laz/>`_, `block 5 <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/projects/arrowhead/block_5/laz/>`_

  * Twin Cities Metro: `block a c <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/projects/metro/block_a_c/laz/>`_, `block b <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/projects/metro/block_b/laz/>`_, `block d <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/projects/metro/block_d/laz/>`_, `block e <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/projects/metro/block_e/laz/>`_, `block f <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/projects/metro/block_f/laz/>`_, `block g <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/projects/metro/block_g/laz/>`_, `block h <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/projects/metro/block_h/laz/>`_, `block dakota <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/projects/metro/block_dakota/laz/>`_, `block maple grove <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/projects/metro/block_maple_grove/laz/>`_, `block metro <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/projects/metro/block_metro/laz/>`_

  * Central Lakes: `block 1 <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/projects/central_lakes/block_1/laz/>`_, `block 2 <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/projects/central_lakes/block_2/laz/>`_, `block 3 <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/projects/central_lakes/block_3/laz/>`_, `block 4 <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/projects/central_lakes/block_4/laz/>`_, `block 5 <ftp://ftp.lmic.state.mn.us/pub/data/elevation/lidar/projects/central_lakes/block_5/laz/>`_

* TERN AusCover Airborne LiDAR: `tumbarumba <http://tern-auscover.science.uq.edu.au/thredds/catalog/auscover/field_and_airborne_datasets/airborne/lidar/tumbarumba/catalog.html>`_, `tsliig <http://tern-auscover.science.uq.edu.au/thredds/catalog/auscover/field_and_airborne_datasets/airborne/lidar/tsliig/catalog.html>`_, `robsons creek <http://tern-auscover.science.uq.edu.au/thredds/catalog/auscover/field_and_airborne_datasets/airborne/lidar/robsons_creek/catalog.html>`_, `credo <http://tern-auscover.science.uq.edu.au/thredds/catalog/auscover/field_and_airborne_datasets/airborne/lidar/credo/catalog.html>`_, `chowilla <http://tern-auscover.science.uq.edu.au/thredds/catalog/auscover/field_and_airborne_datasets/airborne/lidar/chowilla/catalog.html>`_

* http://liblas.org/samples/
* `Virginia LiDAR <http://www.wm.edu/as/cga/Data%20Services/VALIDAR/index.php>`_ of College of William and Mary: `Eleven Coastal Counties <http://gisfiles.wm.edu/files/lidar/a11county/LAZ/>`_, `Eastern Shore <http://gisfiles.wm.edu/files/lidar/ESLiDAR/LiDAR_ClassifiedPointCloud/>`_, `Shenandoah Valley <http://gisfiles.wm.edu/files/lidar/SV/laz/>`_
* `MassGIS LiDAR Terrain Data <http://www.mass.gov/anf/research-and-tech/it-serv-and-support/application-serv/office-of-geographic-information-massgis/datalayers/lidar.html>`_
* https://www.lidar-online.com/product-list.php

* `Nationwide LiDAR Data of Denmark <https://download.kortforsyningen.dk/content/dhmpunktsky>`_

Users of LASzip
------------------------------------------------------------------------------

* `Fugro <http://www.fugro.com/>`_
* `Blom <http://www.blomasa.com/>`_
* `COWI <http://www.cowi.dk/>`_
* `Watershed Sciences, Inc. <http://www.watershedsciences.com/>`_
* `Riegl <http://www.riegl.com/>`_
* `NOAA <http://www.noaa.gov/>`_
* `USGS <http://www.usgs.gov/>`_
* `Euclideon <http://www.euclideon.com/>`_
* `Dielmo 3D <http://www.dielmo.com/>`_
* `Oregon Lidar Consortium <http://www.oregongeology.org/sub/projects/olc/default.htm>`_
* `Minnesota Department of Natural Resources <http://www.dnr.state.mn.us/>`_
* `US Army Corps of Engineers <http://erdc.usace.army.mil/>`_
* and many many more ...

Download LAS data (yet to be laszipped)
------------------------------------------------------------------------------

* `Spring 2011 Rhode Island Statewide LiDAR Data <http://www.edc.uri.edu/rigis/data/download/lidar/2011USGS/tiles.aspx/>`_
* `PAMAP - LiDAR data of Pennsylvania <ftp://pamap.pasda.psu.edu/pamap_lidar/>`_
* `Iowa LiDAR Mapping Project <http://geotree2.geog.uni.edu/IowaLidar/>`_
* `Sacramento-San Joaquin Delta LiDAR <ftp://atlas.ca.gov/pub/delta-vision/lidar2009/>`_
* `Illinois Height Modernization Project LiDAR <http://www.isgs.illinois.edu/nsdihome/webdocs/ilhmp/data.html>`_
* `Northwest Florida Water Management District LiDAR <http://www.nwfwmdlidar.com/>`_
* Washington University in St. Louis: `Franklin <ftp://lidar.wustl.edu/from_WRC/Franklin/LAS_Files/>`_, `Jasper <ftp://lidar.wustl.edu/from_WRC/Jasper/Lidar_Photoscience/JasperCounty_Classified_LAS/>`_, `Washington Iron <ftp://lidar.wustl.edu/from_WRC/Washington_Iron/LAS_Files/>`_, `St. Francois <ftp://lidar.wustl.edu/from_WRC/St_Francois/LAS_Files/>`_, `Jefferson City (1) <ftp://lidar.wustl.edu/from_WRC/Cole_Callaway_Osage/08232010/JeffersonCity/Classified_LAS/>`_, `Jefferson City (2) <ftp://lidar.wustl.edu/from_WRC/Cole_Callaway_Osage/08272010/JeffersonCity/Classified_LAS/>`_, `Jefferson City (3) <ftp://lidar.wustl.edu/from_WRC/Cole_Callaway_Osage/09012010/JeffersonCity_Classified_LAS/>`_, `Jefferson Ste Genevieve <ftp://lidar.wustl.edu/from_WRC/Jefferson_Ste_Genevieve/LAS_Files/>`_, `USGS Drive <ftp://lidar.wustl.edu/USGS_Drive/BE_LIDAR/>`_, `Stone County MO <ftp://lidar.wustl.edu/Stone/Stone_County_MO_Classified_LAS_G10PD00579/>`_
* Alaska LiDAR: `Caswell Lakes <http://matsu.gina.alaska.edu/LiDAR/Caswell_Lakes/Point_Cloud/Classified/>`_, `Core Area <http://matsu.gina.alaska.edu/LiDAR/Core_Area/Point_Cloud/Classified/>`_, `Matanuska <http://matsu.gina.alaska.edu/LiDAR/Matanuska/Point_Cloud/Classified/>`_, `North Susitna <http://matsu.gina.alaska.edu/LiDAR/North_Susitna/Point_Cloud/Classified/>`_, `Point MacKenzie <http://matsu.gina.alaska.edu/LiDAR/Point_MacKenzie/Point_Cloud/Classified/>`_, `Willow <http://matsu.gina.alaska.edu/LiDAR/Willow/Point_Cloud/Classified/>`_, `Talkeetna <http://matsu.gina.alaska.edu/LiDAR/Talkeetna/Point_Cloud/Classified/>`_
* Spain: `Pais Vasco LiDAR <ftp://ftp.geo.euskadi.net/lidar/LIDAR_2012_ETRS89/LAS/>`_

.. toctree::
   :hidden:

.. _`OSGeo4W`: http://trac.osgeo.org/osgeo4w
.. _`Martin Isenburg`: http://www.cs.unc.edu/~isenburg
.. _`ASPRS LAS format`: http://www.asprs.org/society/committees/standards/lidar_exchange_format.html
.. _`LAS`: http://www.asprs.org/society/committees/standards/lidar_exchange_format.html
.. _`LGPL`: http://en.wikipedia.org/wiki/GNU_Lesser_General_Public_License
.. _`bz2`: http://en.wikipedia.org/wiki/Bzip2
.. _`gzip`: http://en.wikipedia.org/wiki/Gzip
.. _`rar`: http://en.wikipedia.org/wiki/Rar
.. _`LAStools`: http://lastools.org/download/lastools.zip
.. _`libLAS`: http://liblas.org
.. _`LASlib`: http://lastools.org/download/laslib.zip
.. _`LASzip`: http://lastools.org/download/laszip.zip
.. _`laszip.exe`: http://lastools.org/download/laszip.exe
.. _`laszip-cli.exe`: http://lastools.org/download/laszip-cli.exe
.. _`paper`: http://lastools.org/download/laszip.pdf
.. _`video`: http://www.youtube.com/watch?v=A0s0fVktj6U
