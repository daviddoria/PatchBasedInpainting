ENABLE_TESTING()

include_directories(../ ../../ ../../Testing)

add_executable(TestHelpers TestHelpers.cpp ../Helpers.cpp)
target_link_libraries(TestHelpers libTestHelpers)
add_test(TestHelpersClass TestHelpersClass)

add_executable(TestITKHelpers TestITKHelpers.cpp ../ITKHelpers.cpp ../Helpers.cpp)
target_link_libraries(TestITKHelpers ${ITK_LIBRARIES} libTestHelpers)
add_test(TestITKHelpers TestITKHelpers)

add_executable(TestVTKHelpers TestVTKHelpers.cpp ../VTKHelpers.cpp)
target_link_libraries(TestVTKHelpers ${VTK_LIBRARIES} libTestHelpers)
add_test(TestVTKHelpers TestVTKHelpers)

add_executable(TestITKVTKHelpers TestITKVTKHelpers.cpp ../ITKVTKHelpers.cpp ../ITKHelpers.cpp ../VTKHelpers.cpp ../Helpers.cpp)
target_link_libraries(TestITKVTKHelpers ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTestHelpers)
add_test(TestITKVTKHelpers TestITKVTKHelpers)
