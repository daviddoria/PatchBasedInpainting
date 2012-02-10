
add_executable(TestFeatureVectorInpaintingVisitor TestFeatureVectorInpaintingVisitor.cpp ../../ImageProcessing/Mask.cpp 
../../Priority/Priority.cpp ../../Priority/PriorityRandom.cpp
../../PixelDescriptors/FeatureVectorPixelDescriptor.cpp)
target_link_libraries(TestFeatureVectorInpaintingVisitor ${ITK_LIBRARIES} ${VTK_LIBRARIES} libHelpers)
add_test(TestFeatureVectorInpaintingVisitor TestFeatureVectorInpaintingVisitor)

