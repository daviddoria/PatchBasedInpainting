ENABLE_TESTING()

include_directories(../)

add_library(libTesting Testing.cpp ../Mask.cpp)
target_link_libraries(libTesting ${ITK_LIBRARIES})
