include_directories(../)

add_executable(TestFirstAndWrite TestFirstAndWrite.cpp ../FirstAndWrite.hpp)
target_link_libraries(TestFirstAndWrite ${PatchBasedInpainting_libraries})
add_test(TestFirstAndWrite TestFirstAndWrite)
