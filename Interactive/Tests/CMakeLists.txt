# include_directories(../) # The Interactive directory
# include_directories(../../) # The main PatchBasedInpainting}directory
# include_directories(../../Testing)
# 
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
# add_executable(TestNamedITKImageCollection TestNamedITKImageCollection.cpp ../NamedITKImageCollection.cpp ../MaskOperations.cpp ../Mask.cpp)
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
# add_executable(TestHelpersQt TestHelpersQt.cpp ../HelpersQt.cpp)
# target_link_libraries(TestHelpersQt ${ITK_LIBRARIES} ${QT_LIBRARIES} libTesting)
# add_test(TestHelpersQt TestHelpersQt)
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
# add_executable(TestMovablePatch TestMovablePatch.cpp ../MovablePatch.cpp ../Layer.cpp ../HelpersQt.cpp)
# target_link_libraries(TestMovablePatch ${QT_LIBRARIES} ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers)
# add_test(TestMovablePatch TestMovablePatch)

# include_directories(${CMAKE_CURRENT_BINARY_DIR})
# QT4_WRAP_UI(TopPatchesWidgetUISrcs ../TopPatchesWidget.ui)
# QT4_WRAP_CPP(TopPatchesWidgetMOCSrcs ../TopPatchesWidget.h ../Delegates/PixmapDelegate.h)
# add_executable(TestTopPatchesWidget TestTopPatchesWidget.cpp ../Delegates/PixmapDelegate.cpp ../HelpersQt.cpp ${TopPatchesWidgetUISrcs} ${TopPatchesWidgetMOCSrcs})
# target_link_libraries(TestTopPatchesWidget ${QT_LIBRARIES} ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers)
# add_test(TestTopPatchesWidget TestTopPatchesWidget)


# Ideally we would like to not duplicate TopPatchesDialog and TopPatchesWidget, but rather derive one from the other.
# include_directories(${CMAKE_CURRENT_BINARY_DIR})
# QT4_WRAP_UI(TopPatchesWidgetUISrcs ../TopPatchesWidget.ui)
# QT4_WRAP_CPP(TopPatchesWidgetMOCSrcs ../TopPatchesWidget.h ../TopPatchesDialog.h ../Delegates/PixmapDelegate.h)
# add_executable(TestTopPatchesDialog TestTopPatchesDialog.cpp ../Delegates/PixmapDelegate.cpp ../HelpersQt.cpp
#                ${TopPatchesWidgetUISrcs} ${TopPatchesWidgetMOCSrcs})
# target_link_libraries(TestTopPatchesDialog ${QT_LIBRARIES} ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers)
# add_test(TestTopPatchesDialog TestTopPatchesDialog)

# 
# include_directories(${CMAKE_CURRENT_BINARY_DIR})
# include_directories(${CMAKE_CURRENT_BINARY_DIR}../)
# QT4_WRAP_UI(TopPatchesDialogUISrcs ../TopPatchesDialog.ui)
# QT4_WRAP_CPP(TopPatchesDialogMOCSrcs ../TopPatchesDialog.h ../Delegates/PixmapDelegate.h)
# add_executable(TestTopPatchesDialog TestTopPatchesDialog.cpp ../Delegates/PixmapDelegate.cpp ../HelpersQt.cpp
#                ../../ImageProcessing/Mask.cpp
#                ${TopPatchesDialogUISrcs} ${TopPatchesDialogMOCSrcs})
# target_link_libraries(TestTopPatchesDialog ${QT_LIBRARIES} ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers)
# add_test(TestTopPatchesDialog TestTopPatchesDialog)


# include_directories(${CMAKE_CURRENT_BINARY_DIR})
# include_directories(${CMAKE_CURRENT_BINARY_DIR}../)
# QT4_WRAP_UI(TestPriorityViewerWidgetUISrcs ../PriorityViewerWidget.ui)
# QT4_WRAP_CPP(TestPriorityViewerWidgetMOCSrcs ../PriorityViewerWidget.h)
# add_executable(TestPriorityViewerWidget TestPriorityViewerWidget.cpp # ../PriorityViewerWidget.cpp
#                ${TestPriorityViewerWidgetUISrcs} ${TestPriorityViewerWidgetMOCSrcs})
# target_link_libraries(TestPriorityViewerWidget ${QT_LIBRARIES} ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers libGUI)
# add_test(TestPriorityViewerWidget TestPriorityViewerWidget)

#include_directories(${CMAKE_CURRENT_BINARY_DIR})
#include_directories(${CMAKE_CURRENT_BINARY_DIR}../)
#QT4_WRAP_UI(TestManualPatchSelectionDialogUISrcs ../ManualPatchSelectionDialog.ui)
#QT4_WRAP_CPP(TestManualPatchSelectionDialogMOCSrcs ../ManualPatchSelectionDialog.h ../MovablePatch.h)
#add_executable(TestManualPatchSelectionDialog TestManualPatchSelectionDialog.cpp
#               ../MovablePatch.cpp
#../../ImageProcessing/Mask.cpp
#               ${TestManualPatchSelectionDialogUISrcs} ${TestManualPatchSelectionDialogMOCSrcs})
#target_link_libraries(TestManualPatchSelectionDialog ${QT_LIBRARIES} ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers libGUI)
#add_test(TestManualPatchSelectionDialog TestManualPatchSelectionDialog)
