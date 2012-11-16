add_custom_target(ImageProcessingSources SOURCES
BoundaryEnergy.h
BoundaryEnergy.hpp
BoundaryNormals.h
BoundaryNormals.hpp
Derivatives.h
Derivatives.hpp
ImageTypes.h
Isophotes.h
Isophotes.hpp
itkRGBToLabColorSpacePixelAccessor.h
PixelFilterFunctors.hpp
)

option(PatchBasedInpainting_ImageProcessing_BuildTests "Build PatchBasedInpainting ImageProcessing tests?")
if(PatchBasedInpainting_ImageProcessing_BuildTests)
  add_subdirectory(Tests)
endif()
