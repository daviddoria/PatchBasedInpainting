ENABLE_TESTING()

include_directories(../)

add_executable(TestSumSquaredPixelDifference TestSumSquaredPixelDifference.cpp ../SumSquaredPixelDifference.hpp)
target_link_libraries(TestSumSquaredPixelDifference ${PatchBasedInpainting_libraries})
add_test(TestSumSquaredPixelDifference TestSumSquaredPixelDifference)
