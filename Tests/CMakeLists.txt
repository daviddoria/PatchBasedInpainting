ENABLE_TESTING()
# set(BaselineDir ${CriminisiInpainting_SOURCE_DIR}/../Testing/Baselines/)
# set(DataDir ${CriminisiInpainting_SOURCE_DIR}/../Testing/Data/)
# set(ExeDir ${CriminisiInpainting_BINARY_DIR})
# set(OutDir ${CriminisiInpainting_BINARY_DIR}/Testing/)

add_executable(TestSelfPatchCompare TestSelfPatchCompare.cpp)
target_link_libraries(TestSelfPatchCompare PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})
add_test(TestSelfPatchCompare TestSelfPatchCompare)