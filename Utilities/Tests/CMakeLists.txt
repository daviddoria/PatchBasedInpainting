include_directories(../) # so we can access the headers normally (e.g. #include "Patch.h") from the tests
include_directories(../Testing ../Helpers)

add_executable(TestStatistics TestStatistics.cpp)
target_link_libraries(TestStatistics ${PatchBasedInpainting_libraries} Testing)
add_test(TestStatistics TestStatistics)

add_executable(TestParallelSort TestParallelSort.cpp)
target_link_libraries(TestParallelSort ${PatchBasedInpainting_libraries} Testing)
add_test(TestParallelSort TestParallelSort)

add_executable(TestIntroducedEnergy TestIntroducedEnergy.cpp)
target_link_libraries(TestIntroducedEnergy ${PatchBasedInpainting_libraries} Testing)
add_test(TestIntroducedEnergy TestIntroducedEnergy)

add_executable(TestPatchHelpers TestPatchHelpers.cpp)
target_link_libraries(TestPatchHelpers ${PatchBasedInpainting_libraries} Testing)
add_test(TestPatchHelpers TestPatchHelpers)
