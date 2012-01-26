This software allows the user to inpaint an image using a greedy patch based method.

License: See LICENSE file in base directory.

Implementation notes:
Throughout the code, the size of square patches are described by their "radius". That is, a 9x9 square patch has radius 4.

Dependencies
-----------
Boost - Known to NOT work with 1.42, but does work with 1.46. (1.42 produces errors in TestDVPTreeGridGraph.cpp - something about grid_graph_vertex_at())

Thanks
------
Many thanks to Mikael Persson (http://www.cim.mcgill.ca/~mpersson) for many thoughtful discussions on the design and implementation of this program.
Additonally, the dynamic vantage point tree used is from his ReaK library (https://github.com/mikael-s-persson/ReaK).