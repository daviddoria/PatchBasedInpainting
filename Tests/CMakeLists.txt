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

add_executable(TestImagePatchPixelDescriptor TestImagePatchPixelDescriptor.cpp)
target_link_libraries(TestImagePatchPixelDescriptor ${PatchBasedInpainting_libraries})
add_test(TestImagePatchPixelDescriptor TestImagePatchPixelDescriptor)

add_executable(TestPixelDifferences TestPixelDifferences.cpp)
target_link_libraries(TestPixelDifferences ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting)
add_test(TestPixelDifferences TestPixelDifferences)

# add_executable(TestPatchBasedInpainting TestPatchBasedInpainting.cpp ../PatchBasedInpainting.cpp
# ../PatchPairDifferences.cpp
# ../Priority/Priority.cpp ../Priority/PriorityCriminisi.cpp ../Priority/PriorityOnionPeel.cpp ../Priority/PriorityRandom.cpp ../Priority/PriorityManual.cpp ../Priority/PriorityDepth.cpp
# ../Priority/PrioritySearchHighest.cpp ../PixelCollection.cpp
# ../SourcePatchCollection.cpp ../ImagePatchItem.cpp ../PatchPair.cpp
# ../ImageProcessing/Mask.cpp ../ImageProcessing/MaskOperations.cpp ../ImageProcessing/BoundaryNormals.cpp
# ../ImageProcessing/Isophotes.cpp ../ImageProcessing/Derivatives.cpp)
# target_link_libraries(TestPatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers libTesting)
# add_test(TestPatchBasedInpainting TestPatchBasedInpainting)
