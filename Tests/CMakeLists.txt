ENABLE_TESTING()

include_directories(../) # so we can access the headers normally (e.g. #include "Patch.h") from the tests
include_directories(../Testing)

#####################

# Build an inpainting algorithm that uses the GUI objects but goes very fast by simply choosing the first patch.
QT4_WRAP_UI(DummyPatchesInpaintingUISrcs
            ../Interactive/BasicViewerWidget.ui)
ADD_EXECUTABLE(DummyPatchesInpainting DummyPatchesInpainting.cpp
               DummyPatchesDriver.hpp
               ../Interactive/BasicViewerWidget.h
               ../Interactive/MovablePatch.h
               ../Visitors/InformationVisitors/DisplayVisitor.hpp
               ${DummyPatchesInpaintingUISrcs})

TARGET_LINK_LIBRARIES(DummyPatchesInpainting ${PatchBasedInpainting_libraries} InpaintingGUI)
INSTALL( TARGETS DummyPatchesInpainting RUNTIME DESTINATION ${INSTALL_DIR} )

# add_executable(TestPixelWiseDifferencePatchPairVisitor TestPixelWiseDifferencePatchPairVisitor.cpp ../ImagePatchItem.cpp ../PatchPair.cpp ../PatchPairDifferences.cpp ../SourcePatchCollection.cpp ../CandidatePairs.cpp 
# ../ImageProcessing/Mask.cpp)
# target_link_libraries(TestPixelWiseDifferencePatchPairVisitor ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting libHelpers)
# add_test(TestPixelWiseDifferencePatchPairVisitor TestPixelWiseDifferencePatchPairVisitor)

#add_executable(TestImagePatchPixelDescriptor TestImagePatchPixelDescriptor.cpp)
#target_link_libraries(TestImagePatchPixelDescriptor ${VTK_LIBRARIES} ${ITK_LIBRARIES})
#add_test(TestImagePatchPixelDescriptor TestImagePatchPixelDescriptor)

# add_executable(TestItemDifferenceVisitor TestItemDifferenceVisitor.cpp ../PatchPairDifferences.cpp)
# target_link_libraries(TestItemDifferenceVisitor ${VTK_LIBRARIES} ${ITK_LIBRARIES})
# add_test(TestItemDifferenceVisitor TestItemDifferenceVisitor)

# add_executable(TestPointCloudDescriptorCreator TestPointCloudDescriptorCreator.cpp)
# target_link_libraries(TestPointCloudDescriptorCreator ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting)
# add_test(TestPointCloudDescriptorCreator TestPointCloudDescriptorCreator)

#add_executable(TestPixelCollection TestPixelCollection.cpp ../PixelCollection.cpp)
#target_link_libraries(TestPixelCollection ${ITK_LIBRARIES})
#add_test(TestPixelCollection TestPixelCollection)

# add_executable(TestPixelSumAccumulator TestPixelSumAccumulator.cpp ../ImagePatchItem.cpp ../ImageProcessing/Mask.cpp)
# target_link_libraries(TestPixelSumAccumulator ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting)
# add_test(TestPixelSumAccumulator TestPixelSumAccumulator)

# add_executable(TestPatchPair TestPatchPair.cpp ../ImagePatchItem.cpp ../PatchPair.cpp ../PatchPairDifferences.cpp)
# target_link_libraries(TestPatchPair ${VTK_LIBRARIES} ${ITK_LIBRARIES})
# add_test(TestPatchPair TestPatchPair)

# add_executable(TestImagePatchCreator TestImagePatchCreator.cpp)
# target_link_libraries(TestImagePatchCreator ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers)
# add_test(TestImagePatchCreator TestImagePatchCreator)

#add_executable(TestPatchPairDifferences TestPatchPairDifferences.cpp ../PatchPairDifferences.cpp)
#target_link_libraries(TestPatchPairDifferences ${VTK_LIBRARIES} ${ITK_LIBRARIES})
#add_test(TestPatchPairDifferences TestPatchPairDifferences)

# add_executable(TestSourcePatchCollection TestSourcePatchCollection.cpp ../SourcePatchCollection.cpp ../ImageProcessing/Mask.cpp ../ImagePatchItem.cpp)
# target_link_libraries(TestSourcePatchCollection ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers)
# add_test(TestSourcePatchCollection TestSourcePatchCollection)

# TODO: Fix this test.
# add_executable(TestDifferenceSumPixelPairVisitor TestDifferenceSumPixelPairVisitor.cpp ../ImagePatchItem.cpp ../PatchPair.cpp ../Mask.cpp ../PatchPairDifferences.cpp)
# target_link_libraries(TestDifferenceSumPixelPairVisitor ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting)
# add_test(TestDifferenceSumPixelPairVisitor TestDifferenceSumPixelPairVisitor)

#add_executable(TestPixelDifferences TestPixelDifferences.cpp)
#target_link_libraries(TestPixelDifferences ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting)
#add_test(TestPixelDifferences TestPixelDifferences)

# add_executable(TestPatchDifference TestPatchDifference.cpp ../ImagePatchItem.cpp ../PatchPair.cpp ../PatchPairDifferences.cpp)
# target_link_libraries(TestPatchDifference ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting)
# add_test(TestPatchDifference TestPatchDifference)

# add_executable(TestPatchBasedInpainting TestPatchBasedInpainting.cpp ../PatchBasedInpainting.cpp
# ../PatchPairDifferences.cpp
# ../Priority/Priority.cpp ../Priority/PriorityCriminisi.cpp ../Priority/PriorityOnionPeel.cpp ../Priority/PriorityRandom.cpp ../Priority/PriorityManual.cpp ../Priority/PriorityDepth.cpp
# ../Priority/PrioritySearchHighest.cpp ../PixelCollection.cpp
# ../SourcePatchCollection.cpp ../ImagePatchItem.cpp ../PatchPair.cpp
# ../ImageProcessing/Mask.cpp ../ImageProcessing/MaskOperations.cpp ../ImageProcessing/BoundaryNormals.cpp
# ../ImageProcessing/Isophotes.cpp ../ImageProcessing/Derivatives.cpp)
# target_link_libraries(TestPatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers libTesting)
# add_test(TestPatchBasedInpainting TestPatchBasedInpainting)
