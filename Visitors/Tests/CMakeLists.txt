add_executable(TestFeatureVectorInpaintingVisitor TestFeatureVectorInpaintingVisitor.cpp 
../../ImageProcessing/Mask.cpp 
../../Priority/Priority.cpp ../../Priority/PriorityRandom.cpp
../../PixelDescriptors/FeatureVectorPixelDescriptor.cpp)
target_link_libraries(TestFeatureVectorInpaintingVisitor ${ITK_LIBRARIES} ${VTK_LIBRARIES} libHelpers)
add_test(TestFeatureVectorInpaintingVisitor TestFeatureVectorInpaintingVisitor)

add_executable(TestFeatureVectorDescriptorVisitor TestFeatureVectorDescriptorVisitor.cpp
../../PixelDescriptors/FeatureVectorPixelDescriptor.cpp)
target_link_libraries(TestFeatureVectorDescriptorVisitor ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestFeatureVectorDescriptorVisitor TestFeatureVectorDescriptorVisitor)

add_executable(TestInpaintingVisitor TestInpaintingVisitor.cpp
../../ImageProcessing/Mask.cpp 
../../PixelDescriptors/FeatureVectorPixelDescriptor.cpp
../../Priority/Priority.cpp ../../Priority/PriorityRandom.cpp)
target_link_libraries(TestInpaintingVisitor ${VTK_LIBRARIES} ${ITK_LIBRARIES} libHelpers)
add_test(TestInpaintingVisitor TestInpaintingVisitor)

