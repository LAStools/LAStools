****************************************************************

  The LAStools LiDAR processing toolbox for QGIS 1.8.0 - 2.6.1

  (c) 2013-2014 - martin.isenburg@rapidlasso.com

  rapidlasso GmbH - fast tools to catch reality

  legal details: http://rapidlasso.com/LICENSE.txt

****************************************************************

For QGIS 1.8, 2.0, and 2.2 follow all steps in this blog

http://rapidlasso.com/2013/09/29/how-to-install-lastools-toolbox-in-qgis/

For the old QGIS 2.4 toolbox there is nothing to copy. Simply
enable the "Tools for LiDAR Data" and set the LAStools path to
your installation (e.g. skip directly to step 4 of the blog
and disregard most of step 6).

================================================================

For the new and improved LAStools toolbox for QGIS 2.4 created
during the QGIS hackfest in Essen use the QGIS_2_4_toolbox.zip
file. First, rename the current 'processing' (sub-)folder of
your QGIS install that is located here:

C:/Program Files/QGIS Chugiak/apps/qgis/python/plugins/processing

to something like this:

C:/Program Files/QGIS Chugiak/apps/qgis/python/plugins/processing_org

and then put the contents of the downloaded QGIS_2_4_toolbox.zip
ZIP archive - which contains a folder called 'processing' - in
the place of the folder that was just renamed. If you have trouble
there is some more information here:

http://rapidlasso.com/2013/09/29/how-to-install-lastools-toolbox-in-qgis/

If you have problems make sure you read *ALL* the comments to this
blog article or add some new comments detailing your troubles.

For our Spanish-speaking LiDAR friends ... check these two blogs:

http://digimapas.blogspot.com.es/2015/04/lidar-en-qgis-lastools.html
http://mappinggis.com/2015/04/como-configurar-lastools-en-qgis/

================================================================

For QGIS 2.6 the latest toolbox is already included. But maybe
have a look what was said in this discussion:

http://groups.google.com/d/topic/lastools/ktyrnfDjrQ4/discussion

================================================================

For QGIS 2.8 all is great. For small bug fixes copy and replace
the scripts of the "QGIS_2_8_toolbox_bug_fixes.zip" archive into
whatever path corresponds in your installation to this one:

C:\Program Files\QGIS Wien\apps\qgis\python\plugins\processing\algs\lidar\lastools

================================================================

For QGIS 2.10 all is great.
