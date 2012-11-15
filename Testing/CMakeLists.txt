include_directories(../)

add_library(Testing Testing.cpp)
target_link_libraries(Testing Mask)

add_custom_target(TestingSources SOURCES
Testing.h
Testing.hxx
)
