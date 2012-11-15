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
