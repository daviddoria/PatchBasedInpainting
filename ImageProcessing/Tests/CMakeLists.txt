include_directories(../) # so we can access the headers normally (e.g. #include "Patch.h") from the tests
include_directories(../Testing ../Helpers)

#####################

add_executable(TestMask TestMask.cpp ../Mask.cpp ${ROOT_SOURCE_DIR}/Helpers/ITKHelpers.cpp ${ROOT_SOURCE_DIR}/Helpers/Helpers.cpp)
target_link_libraries(TestMask ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestMask TestMask)

# add_executable(TestDerivatives TestDerivatives.cpp ../Derivatives.cpp ../Mask.cpp)
# target_link_libraries(TestDerivatives ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting libHelpers)
# add_test(TestDerivatives TestDerivatives)
# 
# add_executable(TestIsophotes TestIsophotes.cpp ../Derivatives.cpp ../Isophotes.cpp ../Mask.cpp)
# target_link_libraries(TestIsophotes ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting libHelpers)
# add_test(TestIsophotes TestIsophotes)
# 
# add_executable(TestMaskOperations TestMaskOperations.cpp ../MaskOperations.cpp ../Mask.cpp)
# target_link_libraries(TestMaskOperations ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting libHelpers)
# add_test(TestMaskOperations TestMaskOperations)
# 
# add_executable(TestBoundaryEnergy TestBoundaryEnergy.cpp ../BoundaryEnergy.cpp ../Mask.cpp)
# target_link_libraries(TestBoundaryEnergy ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting libHelpers)
# add_test(TestBoundaryEnergy TestBoundaryEnergy)
# 
# add_executable(TestBoundaryNormals TestBoundaryNormals.cpp ../BoundaryNormals.cpp ../Mask.cpp)
# target_link_libraries(TestBoundaryNormals ${VTK_LIBRARIES} ${ITK_LIBRARIES} libTesting libHelpers)
# add_test(TestBoundaryNormals TestBoundaryNormals)
# 
# add_executable(TestPixelFilterFunctors TestPixelFilterFunctors.cpp)
# target_link_libraries(TestPixelFilterFunctors ${VTK_LIBRARIES} ${ITK_LIBRARIES})
# add_test(TestPixelFilterFunctors TestPixelFilterFunctors)

add_executable(TestMaskedLaplacian TestMaskedLaplacian.cpp ../MaskOperations.cpp)
target_link_libraries(TestMaskedLaplacian ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestMaskedLaplacian TestMaskedLaplacian)
