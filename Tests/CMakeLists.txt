ENABLE_TESTING()
# set(BaselineDir ${CriminisiInpainting_SOURCE_DIR}/../Testing/Baselines/)
# set(DataDir ${CriminisiInpainting_SOURCE_DIR}/../Testing/Data/)
# set(ExeDir ${CriminisiInpainting_BINARY_DIR})
# set(OutDir ${CriminisiInpainting_BINARY_DIR}/Testing/)

include_directories(../)

add_library(libTestHelpers TestHelpers.cpp ../Mask.cpp)
target_link_libraries(libTestHelpers ${ITK_LIBRARIES})

add_library(libHelpers ../Helpers.cpp ../ITKHelpers.cpp ../VTKHelpers.cpp ../ITKVTKHelpers.cpp)
target_link_libraries(libTestHelpers ${ITK_LIBRARIES})

#####################

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
../PatchPair.cpp ../Patch.cpp ../SourcePatchCollection.cpp ../Mask.cpp ../PairDifferences.cpp)
target_link_libraries(TestCandidatePairs ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTestHelpers libHelpers)
add_test(TestCandidatePairs TestCandidatePairs)

add_executable(TestMask TestMask.cpp ../Mask.cpp)
target_link_libraries(TestMask ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestMask TestMask)

add_executable(TestSourcePatchCollection TestSourcePatchCollection.cpp ../SourcePatchCollection.cpp ../Mask.cpp ../Patch.cpp)
target_link_libraries(TestSourcePatchCollection ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers)
add_test(TestSourcePatchCollection TestSourcePatchCollection)

add_executable(TestPixelDifferences TestPixelDifferences.cpp)
target_link_libraries(TestPixelDifferences ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTestHelpers)
add_test(TestPixelDifferences TestPixelDifferences)

add_executable(TestPatchDifference TestPatchDifference.cpp ../Patch.cpp ../PatchPair.cpp)
target_link_libraries(TestPatchDifference ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTestHelpers)
add_test(TestPatchDifference TestPatchDifference)

add_executable(TestSelfPatchCompare TestSelfPatchCompare.cpp ../SelfPatchCompare.cpp
../DebugOutputs.cpp
../Derivatives.cpp ../Mask.cpp ../PatchPair.cpp ../PairDifferences.cpp ../Patch.cpp ../CandidatePairs.cpp ../SourcePatchCollection.cpp)
target_link_libraries(TestSelfPatchCompare ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTestHelpers libHelpers)
add_test(TestSelfPatchCompare TestSelfPatchCompare)

# Note this has nothing to do with the TestHelpers namespace, but rather is the Test for the Helpers namespace.
add_executable(TestHelpersClass TestHelpersClass.cpp ../Helpers.cpp)
target_link_libraries(TestHelpersClass libTestHelpers)
add_test(TestHelpersClass TestHelpersClass)

add_executable(TestITKHelpers TestITKHelpers.cpp)
target_link_libraries(TestITKHelpers ${ITK_LIBRARIES} libTestHelpers libHelpers)
add_test(TestITKHelpers TestITKHelpers)

add_executable(TestVTKHelpers TestVTKHelpers.cpp)
target_link_libraries(TestVTKHelpers ${VTK_LIBRARIES} libTestHelpers libHelpers)
add_test(TestVTKHelpers TestVTKHelpers)

add_executable(TestITKVTKHelpers TestITKVTKHelpers.cpp)
target_link_libraries(TestITKVTKHelpers ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers libTestHelpers)
add_test(TestITKVTKHelpers TestITKVTKHelpers)

add_executable(TestPatchBasedInpainting TestPatchBasedInpainting.cpp ../PatchBasedInpainting.cpp)
target_link_libraries(TestPatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers libTestHelpers)
add_test(TestPatchBasedInpainting TestPatchBasedInpainting)

######## Missing tests ##########

add_executable(TestDerivatives TestDerivatives.cpp ../Derivatives.cpp ../Mask.cpp)
target_link_libraries(TestDerivatives ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTestHelpers libHelpers)
add_test(TestDerivatives TestDerivatives)


add_executable(TestMaskOperations TestMaskOperations.cpp ../MaskOperations.cpp ../Mask.cpp)
target_link_libraries(TestMaskOperations ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTestHelpers libHelpers)
add_test(TestMaskOperations TestMaskOperations)

