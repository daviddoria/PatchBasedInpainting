add_custom_target(ConceptsSources SOURCES
DescriptorConcept.hpp
DescriptorVisitorConcept.hpp
InpaintingVisitorConcept.hpp
NodeConcept.hpp
PriorityConcept.hpp
)

option(PatchBasedInpainting_Concepts_BuildTests "Build PatchBasedInpainting Concepts tests?" OFF)
if(PatchBasedInpainting_Concepts_BuildTests)
  add_subdirectory(Tests)
endif()
