# Even with this line, we have to #include <Utilities/Histogram/Histogram.h>
# I don't understand why #include <Histogram/Histogram.h> doesn't work...
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

if(NOT TARGET Histogram)
  add_subdirectory(Histogram)
  include_directories(${Histogram_includes})
endif()
