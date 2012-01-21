cmake_minimum_required(VERSION 2.6)
PROJECT(PatchBasedInpainting)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")# -std=gnu++0x") # ITK headers don't seem to compile under C++0x

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

add_executable(PatchBasedInpainting Driver.cpp Mask.cpp PriorityOnionPeel.cpp)
target_link_libraries(PatchBasedInpainting ${ITK_LIBRARIES})
