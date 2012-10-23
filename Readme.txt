This software allows the user to inpaint an image using a greedy patch based method.

Once you have cloned this repository, you must
git submodule update --init --recursive

to checkout all of the dependencies (besides the major libraries (ITK, VTK, Qt, Boost) ).

License: See LICENSE file in base directory.

Implementation notes
--------------------
Throughout the code, the size of square patches are described by their "radius" or "half width". That is, a 9x9 square patch has radius 4. 
This is to ensure that the patch has a well defined center pixel (odd side length) - (side length = 2*radius + 1)

Much time was spent testing the speeds of the following things in the patch search function:
- Explicitly inlining the SSD function
- Pasting the SSD function contents directly into the ImagePatchDifference function to avoid function call overhead
- SSD (sum of squared differences) vs SAD (sum of absolute differences)

None of the above seem to make much difference (< 5%).

Dependencies
-----------
Boost
VTK
ITK
Qt

Thanks
------
Many, many thanks to Mikael Persson (http://www.cim.mcgill.ca/~mpersson) for many thoughtful discussions on the design and implementation of this program.
