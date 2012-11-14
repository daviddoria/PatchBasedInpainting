add_subdirectory(Patch)
add_subdirectory(Pixel)
add_subdirectory(Other)

option(PatchBasedInpainting_DifferenceFunctions_BuildTests "Build PatchBasedInpainting DifferenceFunctions tests.")
if(PatchBasedInpainting_DifferenceFunctions_BuildTests)
  add_subdirectory(DifferenceFunctions/Tests)
ENDIF()
