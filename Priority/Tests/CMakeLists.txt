include_directories(../) # so we can access the headers normally (e.g. #include "Patch.h") from the tests
include_directories(../Testing ../Helpers)


# add_executable(TestPriorityFactory TestPriorityFactory.cpp ../PriorityFactory.cpp ../Priority.cpp ../PriorityCriminisi.cpp ../PriorityDepth.cpp ../PriorityManual.cpp
# ../PriorityOnionPeel.cpp ../DebugOutputs.cpp ../Mask.cpp ../PriorityRandom.cpp ../Isophotes.cpp ../Derivatives.cpp ../MaskOperations.cpp)
# target_link_libraries(TestPriorityFactory ${ITK_LIBRARIES} ${VTK_LIBRARIES} libHelpers libTesting)
# add_test(TestPriorityFactory TestPriorityFactory)

add_executable(TestPriorityDepth TestPriorityDepth.cpp)
target_link_libraries(TestPriorityDepth ${PatchBasedInpainting_libraries} Testing)
add_test(TestPriorityDepth TestPriorityDepth)

add_executable(TestPriorityManual TestPriorityManual.cpp)
target_link_libraries(TestPriorityManual ${PatchBasedInpainting_libraries} Testing)
add_test(TestPriorityManual TestPriorityManual)

add_executable(TestPriorityConfidence TestPriorityConfidence.cpp)
target_link_libraries(TestPriorityConfidence ${PatchBasedInpainting_libraries} Testing)
add_test(TestPriorityConfidence TestPriorityConfidence)

add_executable(TestPriorityRandom TestPriorityRandom.cpp)
target_link_libraries(TestPriorityRandom ${PatchBasedInpainting_libraries} Testing)
add_test(TestPriorityRandom TestPriorityRandom)

add_executable(TestPriorityCriminisi TestPriorityCriminisi.cpp)
target_link_libraries(TestPriorityCriminisi ${PatchBasedInpainting_libraries} Testing)
add_test(TestPriorityCriminisi TestPriorityCriminisi)

add_executable(TestPriorityCurvature TestPriorityCurvature.cpp)
target_link_libraries(TestPriorityCurvature ${PatchBasedInpainting_libraries} Testing)
add_test(TestPriorityCurvature TestPriorityCurvature)
