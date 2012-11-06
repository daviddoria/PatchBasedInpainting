include_directories(../) # The Interactive directory
include_directories(../../) # The main PatchBasedInpainting}directory
include_directories(../../Testing)

# Allow Qt to find it's MOCed files
#include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}../)

add_executable(TestBasicViewer TestBasicViewer.cpp ../BasicViewerWidget.h)
target_link_libraries(TestBasicViewer ${PatchBasedInpainting_libraries} InpaintingGUI)
add_test(TestBasicViewer TestBasicViewer)

# add_executable(TestInpaintingIterationRecordViewer TestInpaintingIterationRecordViewer.cpp)
# target_link_libraries(TestInpaintingIterationRecordViewer)
# add_test(TestInpaintingIterationRecordViewer TestInpaintingIterationRecordViewer)
# 
# add_executable(TestNamedITKImage TestNamedITKImage.cpp)
# target_link_libraries(TestNamedITKImage ${ITK_LIBRARIES})
# add_test(TestNamedITKImage TestNamedITKImage)
# 
# add_executable(TestNamedVTKImage TestNamedVTKImage.cpp ../NamedVTKImage.cpp)
# target_link_libraries(TestNamedVTKImage ${VTK_LIBRARIES})
# add_test(TestNamedVTKImage TestNamedVTKImage)
# 
# add_executable(TestColorPalette TestColorPalette.cpp)
# target_link_libraries(TestColorPalette)
# add_test(TestColorPalette TestColorPalette)
# 
# add_executable(TestNamedITKImageCollection TestNamedITKImageCollection.cpp
#                ../NamedITKImageCollection.cpp)
# target_link_libraries(TestNamedITKImageCollection ${ITK_LIBRARIES} libHelpers)
# add_test(TestNamedITKImageCollection TestNamedITKImageCollection)
# 
# add_executable(TestDisplayState TestDisplayState.cpp ../DisplayState.cpp)
# target_link_libraries(TestDisplayState)
# add_test(TestDisplayState TestDisplayState)
# 
# add_executable(TestDisplayStyle TestDisplayStyle.cpp)
# target_link_libraries(TestDisplayStyle)
# add_test(TestDisplayStyle TestDisplayStyle)
# 
# add_executable(TestImageCamera TestImageCamera.cpp)
# target_link_libraries(TestImageCamera)
# add_test(TestImageCamera TestImageCamera)
# 
# add_executable(TestImageInput TestImageInput.cpp)
# target_link_libraries(TestImageInput)
# add_test(TestImageInput TestImageInput)
# 
# add_executable(TestInpaintingDisplaySettings TestInpaintingDisplaySettings.cpp)
# target_link_libraries(TestInpaintingDisplaySettings)
# add_test(TestInpaintingDisplaySettings TestInpaintingDisplaySettings)
# 
# add_executable(TestInpaintingIterationRecord TestInpaintingIterationRecord.cpp)
# target_link_libraries(TestInpaintingIterationRecord)
# add_test(TestInpaintingIterationRecord TestInpaintingIterationRecord)
# 
# add_executable(TestLayer TestLayer.cpp)
# target_link_libraries(TestLayer)
# add_test(TestLayer TestLayer)
# 
# add_executable(TestVectorLayer TestVectorLayer.cpp)
# target_link_libraries(TestVectorLayer)
# add_test(TestVectorLayer TestVectorLayer)
# 
 add_executable(TestMovablePatch TestMovablePatch.cpp ../MovablePatch.h)
 target_link_libraries(TestMovablePatch ${PatchBasedInpainting_libraries} InpaintingGUI)
 add_test(TestMovablePatch TestMovablePatch)

# QT4_WRAP_UI(TopPatchesWidgetUISrcs ../TopPatchesWidget.ui)
# add_executable(TestTopPatchesWidget TestTopPatchesWidget.cpp
#                 ../TopPatchesWidget.h ../Delegates/PixmapDelegate.h
#                 ../Delegates/PixmapDelegate.cpp ../HelpersQt.cpp ${TopPatchesWidgetUISrcs})
# target_link_libraries(TestTopPatchesWidget ${PatchBasedInpainting_libraries} InpaintingGUI)
# add_test(TestTopPatchesWidget TestTopPatchesWidget)


# Ideally we would like to not duplicate TopPatchesDialog and TopPatchesWidget, but rather derive one from the other.
# QT4_WRAP_UI(TopPatchesWidgetUISrcs ../TopPatchesWidget.ui)
# add_executable(TestTopPatchesDialog TestTopPatchesDialog.cpp ../Delegates/PixmapDelegate.cpp
#                ${TopPatchesWidgetUISrcs} ../TopPatchesWidget.h ../TopPatchesDialog.h ../Delegates/PixmapDelegate.h)
# target_link_libraries(TestTopPatchesDialog ${PatchBasedInpainting_libraries} InpaintingGUI)
# add_test(TestTopPatchesDialog TestTopPatchesDialog)

# QT4_WRAP_UI(TopPatchesDialogUISrcs ../TopPatchesDialog.ui)
# add_executable(TestTopPatchesDialog TestTopPatchesDialog.cpp ../Delegates/PixmapDelegate.cpp
# ../TopPatchesDialog.h ../Delegates/PixmapDelegate.h
#                ${TopPatchesDialogUISrcs} ${TopPatchesDialogMOCSrcs})
# target_link_libraries(TestTopPatchesDialog ${PatchBasedInpainting_libraries} InpaintingGUI)
# add_test(TestTopPatchesDialog TestTopPatchesDialog)


# QT4_WRAP_UI(TestPriorityViewerWidgetUISrcs ../PriorityViewerWidget.ui)
# add_executable(TestPriorityViewerWidget TestPriorityViewerWidget.cpp # ../PriorityViewerWidget.cpp
#                ${TestPriorityViewerWidgetUISrcs} ../PriorityViewerWidget.h)
# target_link_libraries(TestPriorityViewerWidget ${PatchBasedInpainting_libraries} InpaintingGUI)
# add_test(TestPriorityViewerWidget TestPriorityViewerWidget)

QT4_WRAP_UI(TestManualPatchSelectionDialogUISrcs ../ManualPatchSelectionDialog.ui)
QT4_WRAP_CPP(TestManualPatchSelectionDialogMOCSrcs )
add_executable(TestManualPatchSelectionDialog TestManualPatchSelectionDialog.cpp
               ../ManualPatchSelectionDialogParent.h ../MovablePatch.h
               ${TestManualPatchSelectionDialogUISrcs})
target_link_libraries(TestManualPatchSelectionDialog ${PatchBasedInpainting_libraries} InpaintingGUI)
add_test(TestManualPatchSelectionDialog TestManualPatchSelectionDialog)
