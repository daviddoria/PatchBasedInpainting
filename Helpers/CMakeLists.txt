add_library(libHelpers Helpers.cpp ITKHelpers.cpp VTKHelpers.cpp ITKVTKHelpers.cpp HelpersOutput.cpp)
target_link_libraries(libHelpers ${ITK_LIBRARIES} ${VTK_LIBRARIES})

add_subdirectory(Tests)