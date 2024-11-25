.. _home:

******************************************************************************
LASzip - free and lossless LiDAR compression
******************************************************************************

`LASzip <https://rapidlasso.de/laszip/>`_ – a free open source product by `rapidlasso GmbH <https://rapidlasso.de/>`_ – is a compression library to turn LAS files into LAZ files. LASzip is completely lossless: it compresses bulky LAS files into compact LAZ files that are only 7-20% of the original size, while accurately preserving every single bit. Likewise, LAZ files can be decompressed into a bitwise identical LAS file. LASzip allows compressed LAZ files to be treated like standard LAS files. They can be loaded directly from the compressed form into an application without first decompressing them to a disk.

LASzip is included in our full download package and available as source code to be compiled in Windows, Linux or MacOS environments. LASzip is provided as an Apache 2.0-licensed standalone software library.


Binary Download
..............................................................................

`download page <https://rapidlasso.de/downloads>`_


Source
..............................................................................

`github sources <https://github.com/LASzip/LASzip>`_


Specification
..............................................................................

`LAZ Specification 1.4 <https://rapidlasso.de/laszip/>`_


Example
..............................................................................

Compressing & decompressing the LAS file lidar.las with laszip.exe as shown below, results in lidar_copy.las that are bit-identical to lidar.las. However, the small size of lidar.laz makes it much easier to store, copy, transmit, or archive large amounts of LIDAR.

.. code-block:: bat

    laszip -i lidar.las -o lidar.laz  
    laszip -i lidar.laz -o lidar_copy.las


Background
..............................................................................

LASzip was developed by lidar pioneer `Dr. Martin Isenburg <https://lidarmag.com/2023/02/14/tools-for-a-better-tomorrow/>`_ for compressing LAS data in his software `LAStools <https://rapidlasso.de/product-overview/>`_. In November 2011, Martin presented his innovation at ELMF in Salzburg, Austria (`paper <https://downloads.rapidlasso.de/doc/laszip.pdf>`_ and `video <https://www.youtube.com/watch?v=A0s0fVktj6U>`_). LASzip, winner of the “2012 Geospatial World Forum Technology Innovation Award in LiDAR Processing” and runner-up for the “most innovative product at INTERGEO 2012”, quickly gained popularity and over the years became the de-facto standard for compressed LiDAR.

Beyond being a gifted software developer, Martin was keenly aware of our impact on the world. He wasn’t just about technology, but also about technology’s potential to improve our planet and the human condition. Martin was talking about reducing the carbon footprint of computing before almost anyone else. The carbon footprint he saved by giving away LASzip will have a lasting impact. Martin made outstanding contributions to the LiDAR community, and he was a source of inspiration to many. This will not be forgotten. Martin `passed away <https://lidarmag.com/2023/02/14/tools-for-a-better-tomorrow/>`_ on September 7, 2021. May he rest in peace.
