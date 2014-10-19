::
:: one approach to create a huge seamless hillshaded DTM of a densely
:: forested area and steep terrain as archeologists often want to do
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

:: remove buffer points from all tiles

lastile -i folder3\tile_*.laz ^
        -remove_buffer ^
        -odir folder4\ -olaz

:: create one seamlessly hillshaded DTM with BLAST from the ground
:: points of all tiles. maybe play with the lighting to see more.

blast2dem -i folder4\tile_*.laz -merged –keep_class 2 ^
          -step 1.0 -hillshade ^
          -o hillshaded_dtm1.png

blast2dem -i folder4\tile_*.laz -merged –keep_class 2 ^
          -step 1.0 -hillshade -light 1 0 0.2 ^
          -o hillshaded_dtm2.png

blast2dem -i folder4\tile_*.laz -merged –keep_class 2 ^
          -step 1.0 -hillshade -light 1 1 0.1 ^
          -o hillshaded_dtm3.png

blast2dem -i folder4\tile_*.laz -merged –keep_class 2 ^
          -step 1.0 -hillshade -light 0 1 0.3 ^
          -o hillshaded_dtm4.png
