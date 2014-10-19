****************************************************************

  lasindex:

  Creates a *.lax file for a given *.las or *.laz file that
  contains spatial indexing information. When this LAX file is
  present it will be used to speed up access to the relevant
  areas of the LAS/LAZ file whenever a spatial queries of the
  type
    
    -inside_tile ll_x ll_y size
    -inside_circle center_x center_y radius
    -inside_rectangle min_x min_y max_x max_y  (or simply -inside)

  appears in the command line of any LAStools invocation. This
  acceleration is also available to users of the LASlib API. The
  LASreader class has three new functions called

    BOOL inside_tile(F32 ll_x, F32 ll_y, F32 size);
    BOOL inside_circle(F64 center_x, F64 center_y, F64 radius);
    BOOL inside_rectangle(F64 min_x, F64 min_y, F64 max_x, F64 max_y);

  if any of these functions is called the LASreader will only
  return the points that fall inside the specified region and
  use - when available - the spatial indexing information in the
  LAX file created by lasindex.

  For updates check the website or join the LAStools mailing list.

  http://lastools.org/
  http://groups.google.com/group/lastools/
  http://twitter.com/lastools/
  http://facebook.com/lastools/
  http://linkedin.com/groups?gid=4408378

  Martin @lastools

****************************************************************

example usage:

>> lasindex -i *.las

creates a spatial indexing file for all LAS files *.las

>> lasindex -i in.las

created a spatial indexing file called 'in.lax' that need to be
in the same folder as the file 'in.las' to be useful. If you
modify the spatial location of the points in the LAS file or 
their order then you need to recreate the LAX file.

---------------

if you find bugs let me (martin.isenburg@rapidlasso.com) know.