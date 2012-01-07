ENABLE_TESTING()

include_directories(../)

add_library(libTestHelpers Testing.cpp ../Mask.cpp)
target_link_libraries(libTestHelpers ${ITK_LIBRARIES})
