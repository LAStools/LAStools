﻿# sonarnoiseblaster

NOTE: This tool is *NOT* part of LAStools

Removes noise from multi-beam echo-sounder (MBES) data using a
streaming connected component algorithm. The two key parameters
to change are the '-cut_threshold 0.8' that specifies at which
vertical elevation difference the edges between the points are
cut and the '-connected_echoes 20000' value that determines the
size that a connected point cluster must minimally have to be
part of the output. All smaller connected point clusters will
be deleted. The default values are 0.5 and 10000.

The points are "Across=Delaunay-Connected" which means they do
not only have edges to their Delaunay triangulation neighbors
that is computed using only the points' x and y coordinates but
also to the corresponding fourth vertex that lies in the across
triangles of all triangles that are part of their Delaunay star.

Below an example where a vertex has 6 Delaunay-connected vertices
and 6 across-connected vertices. All of the Delaunay-connected 
vertices but only 2 of the across-connected vertices are shown:


             D-------D
            / \     / \          * - considered vertex 
           /   \   /   \         D - Delaunay-connected
          /     \ /     \        X - across-connected
         D-------*-------D
        / \     / \     /                       
       /   \   /   \   /
      /     \ /     \ /          
     X-------D-------D
              \     /
               \   /
                \ /
                 X


## Examples

    sonarnoiseblaster -i reef.laz -o reef_cleaned.laz

Cleans the file 'reef.laz' with the default cut_threshold of 0.5
and keep all connected components 10000 and bigger.

    sonarnoiseblaster -i reef.laz -o reef_cleaned.laz -large 

Same as above but prints some info about the 7 largest components
to the console.

    sonarnoiseblaster -i reef.laz -o reef_cleaned.laz -histo

Same as above but prints a histogram over all component sizes to
the console.

    sonarnoiseblaster -i reef.laz -o reef_cleaned.laz -cut_threshold 0.4 -connected_echoes 5000

Same as above but cutting edges already when the vertical different
of the end-points is 40 cm but keeping points once the size of their
connected component reaches 5000 or higher.


    sonarnoiseblaster -h
    sonarnoiseblaster -i sonar.las -o cleaned_sonar.las
    sonarnoiseblaster -i *.laz -merged -o cleaned_sonar.laz
    sonarnoiseblaster -i *.laz -merged -o cleaned_sonar.laz -large
    sonarnoiseblaster -i *.laz -merged -o cleaned_sonar.laz -histo
    sonarnoiseblaster -i *.laz -merged -o cleaned_sonar.laz -cut_threshold 0.7
    sonarnoiseblaster -i *.laz -merged -o cleaned_sonar.laz -connected_echoes 20000
    sonarnoiseblaster -i *.laz -merged -o cleaned_sonar.laz -cut_threshold 1.0 -connected_echoes 30000


## sonarnoiseblaster specific arguments

-week_to_adjusted [n] : converts time stamps from GPS week [n] to Adjusted Standard GPS  

## Licensing

This tool is free to use.

## Support

1. We invite you to join our LAStools Google Group (http://groups.google.com/group/lastools/).
   If you are looking for information about a specific tool, enter the tool name in the search 
   function and you'll find all discussions related to the respective tool. 
2. Customer Support Page: https://rapidlasso.de/customer-support/.  
3. Download LAStools: https://rapidlasso.de/downloads/.  
4. Changelog: https://rapidlasso.de/changelog/.  


If you want to send us feedback or have questions that are not answered in the resources above, 
please email to info@rapidlasso.de.
