include_directories(../) # The Interactive directory
include_directories(../../) # The main PatchBasedInpainting}directory
include_directories(../../Testing)

# Allow Qt to find it's MOCed files
include_directories(${CMAKE_CURRENT_BINARY_DIR}../)

add_executable(TestGMHDifference TestGMHDifference.cpp ../GMHDifference.hpp)
target_link_libraries(TestGMHDifference ${PatchBasedInpainting_libraries} InpaintingGUI)
add_test(TestGMHDifference TestGMHDifference)
