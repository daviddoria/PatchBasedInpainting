include_directories(../) # so we can access the headers normally (e.g. #include "Patch.h") from the tests
include_directories(../Testing ../Helpers)


# add_executable(TestPriorityFactory TestPriorityFactory.cpp ../PriorityFactory.cpp ../Priority.cpp ../PriorityCriminisi.cpp ../PriorityDepth.cpp ../PriorityManual.cpp
# ../PriorityOnionPeel.cpp ../DebugOutputs.cpp ../Mask.cpp ../PriorityRandom.cpp ../Isophotes.cpp ../Derivatives.cpp ../MaskOperations.cpp)
# target_link_libraries(TestPriorityFactory ${ITK_LIBRARIES} ${VTK_LIBRARIES} libHelpers libTesting)
# add_test(TestPriorityFactory TestPriorityFactory)

add_executable(TestPriorityDepth TestPriorityDepth.cpp ../PriorityDepth.cpp ../Priority.cpp ../../ImageProcessing/Mask.cpp ../../ImageProcessing/Derivatives.cpp ../../ImageProcessing/MaskOperations.cpp
../../ImageProcessing/Isophotes.cpp)
target_link_libraries(TestPriorityDepth ${ITK_LIBRARIES} ${VTK_LIBRARIES} libHelpers libTesting)
add_test(TestPriorityDepth TestPriorityDepth)

add_executable(TestPriorityManual TestPriorityManual.cpp ../PriorityManual.cpp ../Priority.cpp ../../ImageProcessing/Mask.cpp ../PriorityOnionPeel.cpp)
target_link_libraries(TestPriorityManual ${ITK_LIBRARIES} ${VTK_LIBRARIES} libHelpers libTesting)
add_test(TestPriorityManual TestPriorityManual)

add_executable(TestPriorityOnionPeel TestPriorityOnionPeel.cpp ../PriorityOnionPeel.cpp ../Priority.cpp ../../ImageProcessing/Mask.cpp)
target_link_libraries(TestPriorityOnionPeel ${ITK_LIBRARIES} ${VTK_LIBRARIES} libHelpers libTesting)
add_test(TestPriorityOnionPeel TestPriorityOnionPeel)

add_executable(TestPriorityRandom TestPriorityRandom.cpp ../PriorityRandom.cpp ../Priority.cpp ../../ImageProcessing/Mask.cpp)
target_link_libraries(TestPriorityRandom ${ITK_LIBRARIES} ${VTK_LIBRARIES} libTesting)
add_test(TestPriorityRandom TestPriorityRandom)

add_executable(TestPriorityCriminisi TestPriorityCriminisi.cpp ../PriorityCriminisi.cpp ../PriorityOnionPeel.cpp ../Priority.cpp
../../ImageProcessing/Isophotes.cpp ../../ImageProcessing/Mask.cpp ../../ImageProcessing/Derivatives.cpp)
target_link_libraries(TestPriorityCriminisi ${ITK_LIBRARIES} ${VTK_LIBRARIES} libHelpers libTesting)
add_test(TestPriorityCriminisi TestPriorityCriminisi)
