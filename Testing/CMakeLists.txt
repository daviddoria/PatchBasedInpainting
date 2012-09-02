ENABLE_TESTING()

include_directories(../)

add_library(libTesting Testing.cpp)
target_link_libraries(libTesting Mask)

add_custom_target(TestingSources SOURCES
Testing.h
Testing.hxx
)
