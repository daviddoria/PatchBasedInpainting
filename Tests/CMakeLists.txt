include_directories(../) # so we can access the headers normally (e.g. #include "Patch.h") from the tests
include_directories(../Testing ../Helpers)

#####################

add_executable(TestValidRegionIterator TestValidRegionIterator.cpp ../ValidRegionIterator.cpp ../Mask.cpp)
target_link_libraries(TestValidRegionIterator ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting libHelpers)
add_test(TestValidRegionIterator TestValidRegionIterator)

add_executable(TestImage TestImage.cpp)
target_link_libraries(TestImage ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestImage TestImage)

add_executable(TestPixelWiseDifferencePatchPairVisitor TestPixelWiseDifferencePatchPairVisitor.cpp ../ImagePatchItem.cpp ../PatchPair.cpp ../PatchPairDifferences.cpp ../SourcePatchCollection.cpp ../CandidatePairs.cpp ../Mask.cpp)
target_link_libraries(TestPixelWiseDifferencePatchPairVisitor ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting libHelpers)
add_test(TestPixelWiseDifferencePatchPairVisitor TestPixelWiseDifferencePatchPairVisitor)

add_executable(TestImagePatchItem TestImagePatchItem.cpp ../ImagePatchItem.cpp)
target_link_libraries(TestImagePatchItem ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestImagePatchItem TestImagePatchItem)

add_executable(TestScalarItem TestScalarItem.cpp)
target_link_libraries(TestScalarItem ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestScalarItem TestScalarItem)

add_executable(TestItemDifferenceVisitor TestItemDifferenceVisitor.cpp)
target_link_libraries(TestItemDifferenceVisitor ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestItemDifferenceVisitor TestItemDifferenceVisitor)

add_executable(TestPointCloudDescriptorCreator TestPointCloudDescriptorCreator.cpp)
target_link_libraries(TestPointCloudDescriptorCreator ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting)
add_test(TestPointCloudDescriptorCreator TestPointCloudDescriptorCreator)

add_executable(TestPixelCollection TestPixelCollection.cpp ../PixelCollection.cpp)
target_link_libraries(TestPixelCollection ${ITK_LIBRARIES})
add_test(TestPixelCollection TestPixelCollection)

add_executable(TestPixelSumAccumulator TestPixelSumAccumulator.cpp ../ImagePatchItem.cpp ../Mask.cpp)
target_link_libraries(TestPixelSumAccumulator ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting)
add_test(TestPixelSumAccumulator TestPixelSumAccumulator)

add_executable(TestPatchPair TestPatchPair.cpp ../ImagePatchItem.cpp ../PatchPair.cpp ../PatchPairDifferences.cpp)
target_link_libraries(TestPatchPair ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestPatchPair TestPatchPair)

add_executable(TestImagePatchCreator TestImagePatchCreator.cpp)
target_link_libraries(TestImagePatchCreator ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers)
add_test(TestImagePatchCreator TestImagePatchCreator)

add_executable(TestPatchPairDifferences TestPatchPairDifferences.cpp ../PatchPairDifferences.cpp)
target_link_libraries(TestPatchPairDifferences ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestPatchPairDifferences TestPatchPairDifferences)

# TODO: Fix this test.
# add_executable(TestCandidatePairs TestCandidatePairs.cpp ../CandidatePairs.cpp
# ../PatchPair.cpp ../ImagePatchItem.cpp ../SourcePatchCollection.cpp ../Mask.cpp ../PatchPairDifferences.cpp)
# target_link_libraries(TestCandidatePairs ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting libHelpers)
# add_test(TestCandidatePairs TestCandidatePairs)

add_executable(TestMask TestMask.cpp ../Mask.cpp)
target_link_libraries(TestMask ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestMask TestMask)

add_executable(TestSourcePatchCollection TestSourcePatchCollection.cpp ../SourcePatchCollection.cpp ../Mask.cpp ../ImagePatchItem.cpp)
target_link_libraries(TestSourcePatchCollection ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers)
add_test(TestSourcePatchCollection TestSourcePatchCollection)

# TODO: Fix this test.
# add_executable(TestDifferenceSumPixelPairVisitor TestDifferenceSumPixelPairVisitor.cpp ../ImagePatchItem.cpp ../PatchPair.cpp ../Mask.cpp ../PatchPairDifferences.cpp)
# target_link_libraries(TestDifferenceSumPixelPairVisitor ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting)
# add_test(TestDifferenceSumPixelPairVisitor TestDifferenceSumPixelPairVisitor)

add_executable(TestPixelDifferences TestPixelDifferences.cpp)
target_link_libraries(TestPixelDifferences ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting)
add_test(TestPixelDifferences TestPixelDifferences)

add_executable(TestPatchDifference TestPatchDifference.cpp ../ImagePatchItem.cpp ../PatchPair.cpp ../PatchPairDifferences.cpp)
target_link_libraries(TestPatchDifference ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting)
add_test(TestPatchDifference TestPatchDifference)

# add_executable(TestPriorityFactory TestPriorityFactory.cpp ../PriorityFactory.cpp ../Priority.cpp ../PriorityCriminisi.cpp ../PriorityDepth.cpp ../PriorityManual.cpp
# ../PriorityOnionPeel.cpp ../DebugOutputs.cpp ../Mask.cpp ../PriorityRandom.cpp ../Isophotes.cpp ../Derivatives.cpp ../MaskOperations.cpp ../NamedVTKImage.cpp)
# target_link_libraries(TestPriorityFactory ${ITK_LIBRARIES} ${VTK_LIBRARIES} libHelpers libTesting)
# add_test(TestPriorityFactory TestPriorityFactory)

add_executable(TestPriorityDepth TestPriorityDepth.cpp ../PriorityDepth.cpp ../Priority.cpp ../Mask.cpp ../Derivatives.cpp ../MaskOperations.cpp
../DebugOutputs.cpp ../Isophotes.cpp ../NamedVTKImage.cpp)
target_link_libraries(TestPriorityDepth ${ITK_LIBRARIES} ${VTK_LIBRARIES} libHelpers libTesting)
add_test(TestPriorityDepth TestPriorityDepth)

add_executable(TestPriorityManual TestPriorityManual.cpp ../PriorityManual.cpp ../Priority.cpp ../Mask.cpp ../PriorityOnionPeel.cpp ../DebugOutputs.cpp
../NamedVTKImage.cpp)
target_link_libraries(TestPriorityManual ${ITK_LIBRARIES} ${VTK_LIBRARIES} libHelpers libTesting)
add_test(TestPriorityManual TestPriorityManual)

add_executable(TestPriorityOnionPeel TestPriorityOnionPeel.cpp ../PriorityOnionPeel.cpp ../Priority.cpp ../Mask.cpp ../DebugOutputs.cpp ../NamedVTKImage.cpp)
target_link_libraries(TestPriorityOnionPeel ${ITK_LIBRARIES} ${VTK_LIBRARIES} libHelpers libTesting)
add_test(TestPriorityOnionPeel TestPriorityOnionPeel)

add_executable(TestPriorityRandom TestPriorityRandom.cpp ../PriorityRandom.cpp ../Priority.cpp ../Mask.cpp ../NamedVTKImage.cpp
../DebugOutputs.cpp)
target_link_libraries(TestPriorityRandom ${ITK_LIBRARIES} ${VTK_LIBRARIES} libTesting)
add_test(TestPriorityRandom TestPriorityRandom)

add_executable(TestPriorityCriminisi TestPriorityCriminisi.cpp ../PriorityCriminisi.cpp ../PriorityOnionPeel.cpp ../Priority.cpp
../Isophotes.cpp ../Mask.cpp ../DebugOutputs.cpp ../Derivatives.cpp ../NamedVTKImage.cpp)
target_link_libraries(TestPriorityCriminisi ${ITK_LIBRARIES} ${VTK_LIBRARIES} libHelpers libTesting)
add_test(TestPriorityCriminisi TestPriorityCriminisi)

add_executable(TestITKImageCollection TestITKImageCollection.cpp ../ITKImageCollection.cpp ../Mask.cpp)
target_link_libraries(TestITKImageCollection ${ITK_LIBRARIES})
add_test(TestITKImageCollection TestITKImageCollection)

add_executable(TestNamedITKImage TestNamedITKImage.cpp)
target_link_libraries(TestNamedITKImage ${ITK_LIBRARIES})
add_test(TestNamedITKImage TestNamedITKImage)

add_executable(TestNamedVTKImage TestNamedVTKImage.cpp ../NamedVTKImage.cpp)
target_link_libraries(TestNamedVTKImage ${VTK_LIBRARIES})
add_test(TestNamedVTKImage TestNamedVTKImage)

add_executable(TestPatchBasedInpainting TestPatchBasedInpainting.cpp ../PatchBasedInpainting.cpp ../PatchBasedInpaintingDebugging.cpp
../DebugOutputs.cpp ../CandidatePairs.cpp ../PatchPairDifferences.cpp
../Priority.cpp ../PriorityCriminisi.cpp ../PriorityOnionPeel.cpp ../PriorityRandom.cpp ../PriorityManual.cpp ../PriorityDepth.cpp
../PrioritySearchHighest.cpp ../PixelCollection.cpp
../SourcePatchCollection.cpp ../ImagePatchItem.cpp ../PatchPair.cpp
../Mask.cpp ../MaskOperations.cpp ../BoundaryNormals.cpp
../Isophotes.cpp ../Derivatives.cpp)
target_link_libraries(TestPatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers libTesting)
add_test(TestPatchBasedInpainting TestPatchBasedInpainting)

######## Missing tests ##########

add_executable(TestDerivatives TestDerivatives.cpp ../Derivatives.cpp ../Mask.cpp)
target_link_libraries(TestDerivatives ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting libHelpers)
add_test(TestDerivatives TestDerivatives)

add_executable(TestIsophotes TestIsophotes.cpp ../Derivatives.cpp ../Isophotes.cpp ../Mask.cpp)
target_link_libraries(TestIsophotes ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting libHelpers)
add_test(TestIsophotes TestIsophotes)

add_executable(TestMaskOperations TestMaskOperations.cpp ../MaskOperations.cpp ../Mask.cpp)
target_link_libraries(TestMaskOperations ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting libHelpers)
add_test(TestMaskOperations TestMaskOperations)

add_executable(TestNamedITKImageCollection TestNamedITKImageCollection.cpp ../NamedITKImageCollection.cpp ../MaskOperations.cpp ../Mask.cpp)
target_link_libraries(TestNamedITKImageCollection ${ITK_LIBRARIES} libHelpers)
add_test(TestNamedITKImageCollection TestNamedITKImageCollection)

