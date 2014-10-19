::
:: one approach to create a huge hillshaded DTM as a number of tiles
:: without artifacts along the tile boundaries. this script is for a
:: densely forested area in steep terrain and small man made objects
:: are to be preserved as archeologists often want to do
::

:: tile input into compressed 500 by 500 tiles with 25 meter buffer
:: while also adding georeferecing / projection information (here I
:: assume state plane 83 for NC and the elevation in feet but you
:: may have to adjust that

lastile -i folder1\*.las -rescale 0.01 0.01 0.01 ^
        -sp83 NC -elevation_feet ^
        -tile_size 500 -buffer 25 ^
        -o folder2\tile.laz

:: ground classify tiles with a step 3 and extra fine on 7 cores

lasground -i folder2\tile_*.laz ^
          -step 3 -extra_fine ^
          -odir folder3\ -olaz ^
          -cores 7

:: create a tiling of hillshaded DTM in PNG format without tile edge
:: artifacts from the ground points on 7 cores

las2dem -i folder3\tile_*.laz –keep_class 2 ^
        -extra_pass -step 1.0 -hillshade -use_tile_bb ^
        -odir folder4\ -opng ^
        -cores 7

:: if the georeferencing was added you can now drag and drop the auto
:: created KML files into Google Earth. If georeferencing is correct
:: the tiles will appear in the area of interest. maybe play with the
:: lighting to see more.

las2dem -i folder3\tile_*.laz –keep_class 2 ^
        -extra_pass -step 1.0 -hillshade -use_tile_bb ^
        -odir folder5\ -opng ^
        -cores 7

las2dem -i folder3\tile_*.laz –keep_class 2 ^
        -extra_pass -step 1.0 -hillshade -use_tile_bb ^
        -light 0.0 1.0 0.1 ^
        -odir folder5\ -opng ^
        -cores 7

las2dem -i folder3\tile_*.laz –keep_class 2 ^
        -extra_pass -step 1.0 -hillshade -use_tile_bb ^
        -light 1.0 1.0 0.2 ^
        -odir folder5\ -opng ^
        -cores 7
