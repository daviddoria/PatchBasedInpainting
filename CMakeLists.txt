cmake_minimum_required(VERSION 2.6)
PROJECT(CriminisiInpainting)

include_directories(/home/doriad/src/ITK/Wrapping/WrapITK/ExternalProjects/ItkVtkGlue/src/)

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

# ADD_EXECUTABLE(CriminisiInpainting CriminisiInpaintingExample.cpp CriminisiInpainting.cpp CriminisiInpaintingDebugging.cpp CustomStyle.cpp SelfPatchMatch.h Helpers.h Helpers.cpp RotateVectors.h CriminisiInpainting.h)
# TARGET_LINK_LIBRARIES(CriminisiInpainting vtkHybrid ITKBasicFilters ITKCommon ITKIO)

# ADD_EXECUTABLE(PatchImageDifferenceExample PatchImageDifferenceExample.cpp SelfPatchMatch.h Helpers.h  Helpers.cpp)
# TARGET_LINK_LIBRARIES(PatchImageDifferenceExample vtkHybrid ITKBasicFilters ITKCommon ITKIO)
#
# ADD_EXECUTABLE(SelfPatchMatchExample SelfPatchMatchExample.cpp SelfPatchMatch.h Helpers.h  Helpers.cpp)
# TARGET_LINK_LIBRARIES(SelfPatchMatchExample vtkHybrid ITKBasicFilters ITKCommon ITKIO)

ADD_EXECUTABLE(CriminisiInpaintingRGBDI CriminisiInpaintingExampleRGBDI.cxx CriminisiInpainting.cpp CriminisiInpaintingDebugging.cpp CustomStyle.cpp SelfPatchMatch.h Helpers.h Helpers.cpp RotateVectors.h CriminisiInpainting.h)
TARGET_LINK_LIBRARIES(CriminisiInpaintingRGBDI vtkHybrid ITKBasicFilters ITKCommon ITKIO)
