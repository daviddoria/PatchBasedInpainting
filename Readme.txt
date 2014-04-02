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
-----------
This code is known to work with
- Boost 1.51
- VTK 6.0
- Qt 4.7
- ITK 4

NOTE: If you get errors like: vxl/core/vnl/vnl_numeric_traits.h:366:29: error: ‘constexpr’ needed for in-class initialization of static data member ‘zero’ of non-integral type

It means that you have not built ITK with c++11 eneabled. To do this, you must add -std=gnu++11 to CMAKE_CXX_FLAGS when you configure ITK:

ccmake ~/src/ITK -DCMAKE_CXX_FLAGS=-std=gnu++11

NOTE: you cannot configure (ccmake) and THEN set CMAKE_CXX_FLAGS - you MUST include the gnu++11 in the ccmake command the very first time it is run.


Thanks
------
Many, many thanks to Mikael Persson (http://www.cim.mcgill.ca/~mpersson) for many thoughtful discussions on the design and implementation of this program.
