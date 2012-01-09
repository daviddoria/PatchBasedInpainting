include_directories(../ ../../ ../../Testing)

add_executable(TestCanvas TestCanvas.cpp)
target_link_libraries(TestCanvas)
add_test(TestCanvas TestCanvas)

add_executable(TestColorPalette TestColorPalette.cpp)
target_link_libraries(TestColorPalette)
add_test(TestColorPalette TestColorPalette)

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
