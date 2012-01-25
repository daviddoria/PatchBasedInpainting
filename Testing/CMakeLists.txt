ENABLE_TESTING()

include_directories(../)

add_library(libTesting Testing.cpp ../ImageProcessing/Mask.cpp)
target_link_libraries(libTesting ${ITK_LIBRARIES})
