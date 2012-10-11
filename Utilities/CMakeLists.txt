# Even with this line, we have to #include <Utilities/Histogram/Histogram.h>
# I don't understand why #include <Histogram/Histogram.h> doesn't work...
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

if(NOT TARGET Histogram)
  add_subdirectory(Histogram)
  include_directories(${Histogram_includes})
endif()

if(NOT TARGET Debug)
  add_subdirectory(Debug)
  include_directories(${Debug_includes})
endif()

add_custom_target(UtilitiesSources SOURCES
itkCommandLineArgumentParser.h
IndirectPriorityQueue.h
IntroducedEnergy.h
IntroducedEnergy.hpp
PatchHelpers.h
PatchHelpers.hpp
RotateVectors.h
Utilities.hpp
)

if(inpainting_BuildTests)
    message("Building Utilities/Tests")
  add_subdirectory(Tests)
else()
  message("Not building Utilities/Tests")
endif()
