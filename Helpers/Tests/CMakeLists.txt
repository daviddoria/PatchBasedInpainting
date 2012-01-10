include_directories(../) # The Helpers directory
include_directories(../../) # The PatchBasedInpainting directory
include_directories(../../Testing)

add_executable(TestHelpers TestHelpers.cpp ../Helpers.cpp)
target_link_libraries(TestHelpers libTesting)
add_test(TestHelpers TestHelpers)

add_executable(TestITKHelpers TestITKHelpers.cpp ../ITKHelpers.cpp ../Helpers.cpp)
target_link_libraries(TestITKHelpers ${ITK_LIBRARIES} libTesting)
add_test(TestITKHelpers TestITKHelpers)

add_executable(TestVTKHelpers TestVTKHelpers.cpp ../VTKHelpers.cpp)
target_link_libraries(TestVTKHelpers ${VTK_LIBRARIES} libTesting)
add_test(TestVTKHelpers TestVTKHelpers)

add_executable(TestITKVTKHelpers TestITKVTKHelpers.cpp ../ITKVTKHelpers.cpp ../ITKHelpers.cpp ../VTKHelpers.cpp ../Helpers.cpp)
target_link_libraries(TestITKVTKHelpers ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting)
add_test(TestITKVTKHelpers TestITKVTKHelpers)
