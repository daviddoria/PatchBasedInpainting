include_directories(../) # The Interactive directory
include_directories(../../) # The main PatchBasedInpainting}directory
include_directories(../../Testing)

add_executable(TestInpaintingIterationRecordViewer TestInpaintingIterationRecordViewer.cpp)
target_link_libraries(TestInpaintingIterationRecordViewer)
add_test(TestInpaintingIterationRecordViewer TestInpaintingIterationRecordViewer)

add_executable(TestNamedITKImage TestNamedITKImage.cpp)
target_link_libraries(TestNamedITKImage ${ITK_LIBRARIES})
add_test(TestNamedITKImage TestNamedITKImage)

add_executable(TestNamedVTKImage TestNamedVTKImage.cpp ../NamedVTKImage.cpp)
target_link_libraries(TestNamedVTKImage ${VTK_LIBRARIES})
add_test(TestNamedVTKImage TestNamedVTKImage)

add_executable(TestColorPalette TestColorPalette.cpp)
target_link_libraries(TestColorPalette)
add_test(TestColorPalette TestColorPalette)

add_executable(TestNamedITKImageCollection TestNamedITKImageCollection.cpp ../NamedITKImageCollection.cpp ../MaskOperations.cpp ../Mask.cpp)
target_link_libraries(TestNamedITKImageCollection ${ITK_LIBRARIES} libHelpers)
add_test(TestNamedITKImageCollection TestNamedITKImageCollection)

add_executable(TestDisplayState TestDisplayState.cpp ../DisplayState.cpp)
target_link_libraries(TestDisplayState)
add_test(TestDisplayState TestDisplayState)

add_executable(TestDisplayStyle TestDisplayStyle.cpp)
target_link_libraries(TestDisplayStyle)
add_test(TestDisplayStyle TestDisplayStyle)

add_executable(TestHelpersQt TestHelpersQt.cpp ../HelpersQt.cpp)
target_link_libraries(TestHelpersQt ${ITK_LIBRARIES} ${QT_LIBRARIES} libTesting)
add_test(TestHelpersQt TestHelpersQt)

add_executable(TestImageCamera TestImageCamera.cpp)
target_link_libraries(TestImageCamera)
add_test(TestImageCamera TestImageCamera)

add_executable(TestImageInput TestImageInput.cpp)
target_link_libraries(TestImageInput)
add_test(TestImageInput TestImageInput)

add_executable(TestInpaintingDisplaySettings TestInpaintingDisplaySettings.cpp)
target_link_libraries(TestInpaintingDisplaySettings)
add_test(TestInpaintingDisplaySettings TestInpaintingDisplaySettings)

add_executable(TestInpaintingIterationRecord TestInpaintingIterationRecord.cpp)
target_link_libraries(TestInpaintingIterationRecord)
add_test(TestInpaintingIterationRecord TestInpaintingIterationRecord)

add_executable(TestLayer TestLayer.cpp)
target_link_libraries(TestLayer)
add_test(TestLayer TestLayer)

add_executable(TestVectorLayer TestVectorLayer.cpp)
target_link_libraries(TestVectorLayer)
add_test(TestVectorLayer TestVectorLayer)

add_executable(TestMovablePatch TestMovablePatch.cpp ../MovablePatch.cpp ../Layer.cpp ../HelpersQt.cpp)
target_link_libraries(TestMovablePatch ${QT_LIBRARIES} ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers)
add_test(TestMovablePatch TestMovablePatch)
