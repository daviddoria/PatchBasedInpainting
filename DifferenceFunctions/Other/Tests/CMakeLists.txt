ENABLE_TESTING()

include_directories(../)

add_executable(TestFeatureVectorDifference TestFeatureVectorDifference.cpp ../FeatureVectorDifference.hpp)
target_link_libraries(TestFeatureVectorDifference ${PatchBasedInpainting_libraries})
add_test(TestFeatureVectorDifference TestFeatureVectorDifference)
