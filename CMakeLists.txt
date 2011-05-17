cmake_minimum_required(VERSION 2.6)
PROJECT(CriminisiInpainting)

# Where to copy executables when 'make install' is run
SET( INSTALL_DIR ${CMAKE_INSTALL_PREFIX} )
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) #fix the dynamic linking error (RPATH) that would occur without this

#include_directories(/home/doriad/src/ITK/Wrapping/WrapITK/ExternalProjects/ItkVtkGlue/src/)

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

ADD_EXECUTABLE(CriminisiInpainting CriminisiInpaintingExample.cpp Helpers.cpp)
TARGET_LINK_LIBRARIES(CriminisiInpainting vtkHybrid ${ITK_LIBRARIES})
INSTALL( TARGETS CriminisiInpainting
  RUNTIME DESTINATION ${INSTALL_DIR} )
#
# ADD_EXECUTABLE(TestSelfPatchMatch TestSelfPatchMatch.cpp Helpers.cpp)
# TARGET_LINK_LIBRARIES(TestSelfPatchMatch vtkHybrid ITKBasicFilters ITKCommon ITKIO)
#
# ADD_EXECUTABLE(TestPatchDifference TestPatchDifference.cpp Helpers.cpp)
# TARGET_LINK_LIBRARIES(TestPatchDifference vtkHybrid ITKBasicFilters ITKCommon ITKIO)

# ADD_EXECUTABLE(DemoErroneousGradient DemoErroneousGradient.cpp Helpers.cpp )
# TARGET_LINK_LIBRARIES(DemoErroneousGradient ITKBasicFilters ITKCommon ITKIO)

# ADD_EXECUTABLE(DemoBoundaryNormals DemoBoundaryNormals.cpp Helpers.cpp )
# TARGET_LINK_LIBRARIES(DemoBoundaryNormals ITKBasicFilters ITKCommon ITKIO)

# ADD_EXECUTABLE(DemoInnerOuterBoundary DemoInnerOuterBoundary.cpp Helpers.cpp )
# TARGET_LINK_LIBRARIES(DemoInnerOuterBoundary ITKBasicFilters ITKCommon ITKIO)

# ADD_EXECUTABLE(PatchImageDifferenceExample PatchImageDifferenceExample.cpp SelfPatchMatch.h Helpers.h  Helpers.cpp)
# TARGET_LINK_LIBRARIES(PatchImageDifferenceExample vtkHybrid ITKBasicFilters ITKCommon ITKIO)
#
# ADD_EXECUTABLE(SelfPatchMatchExample SelfPatchMatchExample.cpp SelfPatchMatch.h Helpers.h  Helpers.cpp)
# TARGET_LINK_LIBRARIES(SelfPatchMatchExample vtkHybrid ITKBasicFilters ITKCommon ITKIO)

# ADD_EXECUTABLE(CriminisiInpaintingRGBDI CriminisiInpaintingExampleRGBDI.cxx CriminisiInpainting.cpp CriminisiInpaintingDebugging.cpp CustomStyle.cpp SelfPatchMatch.h Helpers.h Helpers.cpp RotateVectors.h CriminisiInpainting.h)
# TARGET_LINK_LIBRARIES(CriminisiInpaintingRGBDI vtkHybrid ITKBasicFilters ITKCommon ITKIO)
