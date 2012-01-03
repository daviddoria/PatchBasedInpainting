ENABLE_TESTING()
# set(BaselineDir ${CriminisiInpainting_SOURCE_DIR}/../Testing/Baselines/)
# set(DataDir ${CriminisiInpainting_SOURCE_DIR}/../Testing/Data/)
# set(ExeDir ${CriminisiInpainting_BINARY_DIR})
# set(OutDir ${CriminisiInpainting_BINARY_DIR}/Testing/)

include_directories(../)

add_executable(TestPatch TestPatch.cpp ../Patch.cpp)
target_link_libraries(TestPatch ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestPatch TestPatch)

add_executable(TestPatchPair TestPatchPair.cpp ../Patch.cpp ../PatchPair.cpp ../PairDifferences.cpp)
target_link_libraries(TestPatchPair ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestPatchPair TestPatchPair)

add_executable(TestPairDifferences TestPairDifferences.cpp ../PairDifferences.cpp)
target_link_libraries(TestPairDifferences ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestPairDifferences TestPairDifferences)

add_executable(TestCandidatePairs TestCandidatePairs.cpp ../CandidatePairs.cpp
../PatchPair.cpp ../Patch.cpp ../SourcePatchCollection.cpp ../Mask.cpp ../PatchSorting.cpp ../Helpers.cpp ../PairDifferences.cpp)
target_link_libraries(TestCandidatePairs ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestCandidatePairs TestCandidatePairs)

add_executable(TestMask TestMask.cpp ../Mask.cpp)
target_link_libraries(TestMask ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestMask TestMask)

add_executable(TestSourcePatchCollection TestSourcePatchCollection.cpp ../SourcePatchCollection.cpp ../Helpers.cpp ../Mask.cpp ../Patch.cpp)
target_link_libraries(TestSourcePatchCollection ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestSourcePatchCollection TestSourcePatchCollection)

# Failing tests:
add_executable(TestSelfPatchCompare TestSelfPatchCompare.cpp ../SelfPatchCompare.cpp
../DebugOutputs.cpp
../Derivatives.cpp ../Mask.cpp ../PatchPair.cpp ../PairDifferences.cpp ../Patch.cpp ../CandidatePairs.cpp ../Helpers.cpp ../SourcePatchCollection.cpp
../PatchSorting.cpp)
target_link_libraries(TestSelfPatchCompare ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestSelfPatchCompare TestSelfPatchCompare)

add_executable(TestDerivatives TestDerivatives.cpp ../Derivatives.cpp ../Mask.cpp ../Helpers.cpp)
target_link_libraries(TestDerivatives ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestDerivatives TestDerivatives)

add_executable(TestHelpers TestHelpers.cpp ../Helpers.cpp ../Derivatives.cpp ../Mask.cpp)
target_link_libraries(TestHelpers ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestHelpers TestHelpers)
