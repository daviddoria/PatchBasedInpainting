add_executable(TestFeatureVectorInpaintingVisitor TestFeatureVectorInpaintingVisitor.cpp 
../../ImageProcessing/Mask.cpp 
../../Priority/Priority.cpp ../../Priority/PriorityRandom.cpp
../../PixelDescriptors/FeatureVectorPixelDescriptor.cpp)
target_link_libraries(TestFeatureVectorInpaintingVisitor ${ITK_LIBRARIES} ${VTK_LIBRARIES} libHelpers)
add_test(TestFeatureVectorInpaintingVisitor TestFeatureVectorInpaintingVisitor)

add_executable(FeatureVectorPrecomputedPolyDataDescriptorVisitor TestFeatureVectorPrecomputedPolyDataDescriptorVisitor.cpp
../../PixelDescriptors/FeatureVectorPixelDescriptor.cpp)
target_link_libraries(FeatureVectorPrecomputedPolyDataDescriptorVisitor ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers)
add_test(FeatureVectorPrecomputedPolyDataDescriptorVisitor FeatureVectorPrecomputedPolyDataDescriptorVisitor)

add_executable(FeatureVectorPrecomputedStructuredGridDescriptorVisitor TestFeatureVectorPrecomputedStructuredGridDescriptorVisitor.cpp
../../PixelDescriptors/FeatureVectorPixelDescriptor.cpp)
target_link_libraries(FeatureVectorPrecomputedStructuredGridDescriptorVisitor ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(FeatureVectorPrecomputedStructuredGridDescriptorVisitor FeatureVectorPrecomputedStructuredGridDescriptorVisitor)

add_executable(TestInpaintingVisitor TestInpaintingVisitor.cpp
../../ImageProcessing/Mask.cpp 
../../PixelDescriptors/FeatureVectorPixelDescriptor.cpp
../../Priority/Priority.cpp ../../Priority/PriorityRandom.cpp)
target_link_libraries(TestInpaintingVisitor ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers)
add_test(TestInpaintingVisitor TestInpaintingVisitor)

add_executable(TestImagePatchDescriptorVisitor TestImagePatchDescriptorVisitor.cpp
../../ImageProcessing/Mask.cpp )
target_link_libraries(TestImagePatchDescriptorVisitor ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers)
add_test(TestImagePatchDescriptorVisitor TestImagePatchDescriptorVisitor)

add_executable(TestCompositeDescriptorVisitor TestCompositeDescriptorVisitor.cpp
../../ImageProcessing/Mask.cpp 
../../PixelDescriptors/FeatureVectorPixelDescriptor.cpp)
target_link_libraries(TestCompositeDescriptorVisitor ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers)
add_test(TestCompositeDescriptorVisitor TestCompositeDescriptorVisitor)

