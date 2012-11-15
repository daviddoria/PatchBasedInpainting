add_custom_target(PrioritySources SOURCES
PriorityCriminisi.h
PriorityCriminisi.hpp
PriorityCurvature.h
PriorityCurvature.hpp
PriorityDepth.h
PriorityDepth.hpp
Priority.h
PriorityImprovedCriminisi.h
PriorityManual.h
PriorityManual.hpp
PriorityConfidence.h
PriorityConfidence.hpp
PriorityRandom.h
PriorityRandom.hpp
)

option(PatchBasedInpainting_Priority_BuildTests "Build PatchBasedInpainting Priority tests?")
if(PatchBasedInpainting_Priority_BuildTests)
  message("Building PatchBasedInpainting Priority tests.")
  add_subdirectory(Tests)
ENDIF()
