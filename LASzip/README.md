# LASzip

**Award-winning software for efficient LiDAR compression **

Open-source compression library for compressing LAS to LAZ.
LASzip is completely lossless: It compresses bulky LAS files into compact LAZ files that are only 7-20% of the original size, while accurately preserving every single bit. Likewise, LAZ files can be decompressed into a bitwise identical LAS file. LASzip allows compressed LAZ files to be treated like standard LAS files. They can be loaded directly from the compressed form into an application without first decompressing them to a disk.

This is the repository of the open source LAZ compressor under the terms of the Apache Public License 2.0.

For the full LASlib and LAStools repository include the popular open source tools las2las, las2txt, txt2las please see https://github.com/LAStools/LAStools  


# Installation

Binary downloads for Windows and Linux are available at 
  https://rapidlasso.de/downloads

# Compilation

Just go to the root directory and run  
    cmake -DCMAKE_BUILD_TYPE=Release CMakeLists.txt  
    cmake --build .  

# Links

* official website:  https://rapidlasso.de
* user group:     http://groups.google.com/group/lastools

# License

Apache Public License 2.0.
See `COPYING` file for the license text.

# Summary
Your feedback is highly appreciated. Feel free to let us know what you use LAStools for and what features and improvements you might need.

(c) 2007-2024 info@rapidlasso.de - https://rapidlasso.de
