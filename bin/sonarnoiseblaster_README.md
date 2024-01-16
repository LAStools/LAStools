# sonarnoiseblaster

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
				 \   /   \   /
			/     \ /     \ /          
		   X- - - -D-------D
					\     /
						 
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


## License

Please license from info@rapidlasso.de to use the tool
commercially. 
You may use the tool to do tests with up to 3 mio points.
Please note that the unlicensed version may will adjust
some data and add a bit of white noise to the coordinates.

## Support

To get more information about a tool just goto the
[LAStools Google Group](http://groups.google.com/group/lastools/)
and enter the tool name in the search function.
You will get plenty of samples to this tool.

To get further support see our
[rapidlasso service page](https://rapidlasso.de/service/)

Check for latest updates at
https://rapidlasso.de/category/blog/releases/

If you have any suggestions please let us (info@rapidlasso.de) know.

