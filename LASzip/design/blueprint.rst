LASzip for LAS 1.4 (native extension)
=====================================
a working document 
------------------
This document summarizes new features and implementation details that define the "native extension" of the LASzip compressor for the new point types 6 to 10 of the LAS 1.4 specification. This includes feedback from our `"Open Call for Input" <http://rapidlasso.com/2014/01/21/call-for-input-on-compression-of-las-1-4/>`_ from early 2014 as well as ideas from out initial proposal for a joint `LAS 1.4 compressor with ESRI <http://rapidlasso.com/2014/04/01/esri-and-rapidlasso-develop-joint-lidar-compressor/>`_ that was presented in form of a widely circulated and loudly applauded `April Fools' Day prank <http://rapidlasso.com/2014/04/01/esri-and-rapidlasso-develop-joint-lidar-compressor/>`_ after months of direct communication with ESRI had failed to produce any useful progress.

We should point out that the current `LASzip compressor <http://laszip.org>`_ can already handle (most) LAS 1.4 files via the `"LAS 1.4 compatibility mode" <http://rapidlasso.com/2014/10/06/rapidlasso-announces-laszip-compatibility-mode-for-las-1-4/>`_ that was sponsored in part by NOAA, Quantum Spatial, and Trimble. However, LAS 1.4 files are likely to stay with us for a long long time. The "native extension" of LASzip seeks to exploit the "natural break" in the lineage of the LAS specification to add entirely new features to the compressor that allow better exploitation of LAS files in the cloud or on Web portals as originally planned and suggested in the `joint proposal <http://rapidlasso.com/2014/04/01/esri-and-rapidlasso-develop-joint-lidar-compressor/>`_.

In the past ESRI has claimed to have identified very particular needs for processing LiDAR in the cloud that have warranted the development of their competing closed standard also know as the `"LAZ clone" <http://rapidlasso.com/2015/02/22/lidar-las-asprs-esri-and-the-laz-clone/>`_. The resulting format fragmentation has `upset <http://rapidlasso.com/2014/11/06/keeping-esri-honest/>`_ the geospatial community and led to an `Open Letter <http://wiki.osgeo.org/wiki/LIDAR_Format_Letter>`_ by OSGeo that has asked ESRI to change their course and work with the LiDAR users to avoid a format war. In parallel to the `on-going mediation <http://wiki.osgeo.org/wiki/LIDAR_Format_Letter>`_ under way via the Open Geospatial Consortium (OGC) in form of a new `Point Cloud Domain Working Group <http://www.opengeospatial.org/pressroom/pressreleases/2236>`_ we are intending to progress towards a set of useful features for compressed LAS 1.4 content in the open and transparent manner, yet at the speed that the needs of our industry demands. Therefore we invite in particular ESRI to tell us what exactly they want us to include in the "native extension" of `LASzip <http://laszip.org>`_ to the new point types:

..

  Dear ESRI, please tell us what features would you like in the native LAS 1.4 extension of LASzip? What exactly are the key differences of your own "optimized LAS" that are currently missing in LASzip?

..

Some of the things that ESRI has added to "optimized LAS" are already part of LASzip and others may already be on our TODO list. So as we wait for ESRI's answer, let's already start fleshing out the feature set that the rest of the community wants to see in the new LASzip:

Core Features
-------------
**1. Selective Decompression:**

Often only a subset of all the attributes of a LiDAR return stored in the LAS format are actually needed by an application. Currently all points attributes have to be read and decompressed by an application opening a LAS or a LAZ file. This is the most far reaching new feature of the native LAS 1.4 extension of LASzip: for the new point types, it will allow reading and decompressing only those points attributes that are relevant to the application. A Web-based LiDAR viewer, for example, may  only have to download, parse, and decompress the xyz coordinates and the intensities of the point cloud and a cloud-based service for DTM generation will only have to download, parse, and decompress the xyz coordinates and the classification values.

**2. Variable Chunking:**

Instead of having each atomic unit of points that are compressed together - a chunk - have the same size of a default 50,000 points the compressor offer the option to let each chunk have a different size. This is in particularly useful when creating point orders that support spatially indexing by rearranging them, for example, into the cells of a quad tree that are laid out along a space-filling curve. Variable chunking allows granular access to only those points of a cell that was selected for loading.

Optional Features
-----------------

**3. Tighter Integration of Spatial Indexing (likely)**

Already supported in the existing LASzip compressor as an optional item this will become a mandatory part of every new LAZ file that is written. Area-of-interest queries are also a form or "selective decompression" and require two things: Knowledge where in the file the points that fall into the interesting area are located (e.g. in the seventeen point intervals [5236312,5236312], [6523634,6634523], ....) and the ability to seek in the compressed file and decompress only those point intervals. The letter has been an integral part of the LASzip compressor since day one as this was one of the core features sponsored by USACE. The first has been added after little by little to LASzip in order not to disrupt anything since the concept (i.e. LASindex and tiny LAX files) was first introduced at ELMF in 2012. The concept needs to be reworked slightly to accommodate files with over 4 billion points.

**4. Re-writeable Flags and Classification (maybe, definitely via LASlayers of LASlib) **

Most of the point attribute of an LAS file will never change --- *at laest* once the LiDAR was published. The xyz coordinates are final once the LiDAR flightlines were aligned, the return counts are fix, the intensities are direct measured, and the original GPS time stamps and the point source IDs should be preserved for posterity. What often does change, however, are the point classifications into "ground", "building", "noise", or "water" points, as well as the flags describing which points are to be "withheld" or which ones are part of the not always desired "overlap".  This is the second far reaching new feature of the native LAS 1.4 extension of LASzip: based on mechanisms that have already been field-tested as part of the "LASlayers" effort the new LASzip will support overriding the existing classifications or flags with a new layer of values. 

**4. Attach-able Attributes (maybe, more likely via LASlayers of LASlib) **

Some LiDAR processing steps create additional per-point attributes such as RGB colors or NDVI values from a different source, error estimates, or point normals. During the design process of the the native LAS 1.4 extension of LASzip we want to consider to allow adding such attributes later without requiring a complete re-write of all existing points attributes that have not changed. 

**5. Explode-able Files (maybe, more likely via LASlayers of LASlib) **

Selective decompression - or more importantly selective download - of large files may in some cases be more feasible to implement for a 3D Web viewer or a LiDAR service portal when the data for selectable attributes is stored for download in separate files. During the design process of the the native LAS 1.4 extension of LASzip we want to accomodate to later add the option to store one compressed LAZ file as a number of compressed files each of which encodes a different set of point attributes.  

**6. Specification Document (`in progress <http://github.com/LASzip/LASzip/blob/master/design/specification.rst>`_) **

The LASzip compressor is currently only documented via an open source reference implementation in C++. In order to create LASzip compressors and decompressors in other programming languages it is currently necessary to step through the (reasonably well documented) C++ source code. We hope that funds can be made available that allow us to hire technical writers who can create a proper `specification document <http://groups.google.com/group/lasroom>`_ that describes the open LASzip compressed LiDAR format.

Open Forum
----------
Please join us to continue the already on-going discussion in the `"LAS room" <https://github.com/LASzip/LASzip/blob/master/design/specification.rst>`_ where we will finalize the feature set in an open censensus process.
