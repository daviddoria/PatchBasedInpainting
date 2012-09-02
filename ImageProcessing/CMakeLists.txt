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

if(inpainting_BuildTests)
  add_subdirectory(Tests)
endif()
