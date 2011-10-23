cmake_minimum_required(VERSION 2.6)
PROJECT(CriminisiInpainting)

# Where to copy executables when 'make install' is run
SET(INSTALL_DIR ${CMAKE_INSTALL_PREFIX} )
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) #fix the dynamic linking error (RPATH) that would occur without this

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

QT4_WRAP_UI(UISrcs CriminisiInpainting.ui FileSelector.ui FileSelectionWidget.ui)
QT4_WRAP_CPP(MOCSrcs Form.h ProgressThread.h FileSelector.h FileSelectionWidget.h)

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

# Interactive
ADD_EXECUTABLE(CriminisiInpaintingInteractive
CandidatePairs.cpp
CriminisiInpainting.cpp 
CriminisiInpaintingDebugging.cpp
CriminisiInpaintingInterface.cpp
FileSelector.cpp
FileSelectionWidget.cpp
Form.cxx
FormGUIElements.cxx
Helpers.cpp
InpaintingVisualizationStack.cpp
Interactive.cpp
InteractorStyleImageNoLevel.cxx
Layer.cpp
Mask.cpp
Patch.cpp
PatchPair.cpp
ProgressThread.cpp
SelfPatchCompare.cpp
VectorLayer.cpp
${UISrcs} ${MOCSrcs})

TARGET_LINK_LIBRARIES(CriminisiInpaintingInteractive ${VTK_LIBRARIES} QVTK ${ITK_LIBRARIES} ${QT_LIBRARIES})
INSTALL( TARGETS CriminisiInpaintingInteractive RUNTIME DESTINATION ${INSTALL_DIR} )

# Non-interactive
# ADD_EXECUTABLE(CriminisiInpainting
# CriminisiInpaintingNonInteractive.cpp
# CriminisiInpainting.cpp
# CriminisiInpaintingDebugging.cpp
# Patch.cpp
# Helpers.cpp
# SelfPatchCompare.cpp
# )
# TARGET_LINK_LIBRARIES(CriminisiInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})
# INSTALL( TARGETS CriminisiInpainting RUNTIME DESTINATION ${INSTALL_DIR} )


# ADD_EXECUTABLE(TestPatchDifference TestPatchDifference.cpp Helpers.cpp)
# TARGET_LINK_LIBRARIES(TestPatchDifference ${VTK_LIBRARIES} ${ITK_LIBRARIES})

# ADD_EXECUTABLE(DemoErroneousGradient DemoErroneousGradient.cpp Helpers.cpp )
# TARGET_LINK_LIBRARIES(DemoErroneousGradient ${ITK_LIBRARIES})

# ADD_EXECUTABLE(DemoBoundaryNormals DemoBoundaryNormals.cpp Helpers.cpp )
# TARGET_LINK_LIBRARIES(DemoBoundaryNormals ${ITK_LIBRARIES})

# ADD_EXECUTABLE(DemoInnerOuterBoundary DemoInnerOuterBoundary.cpp Helpers.cpp )
# TARGET_LINK_LIBRARIES(DemoInnerOuterBoundary ${ITK_LIBRARIES})

# ADD_EXECUTABLE(PatchImageDifferenceExample PatchImageDifferenceExample.cpp SelfPatchMatch.h Helpers.h  Helpers.cpp)
# TARGET_LINK_LIBRARIES(PatchImageDifferenceExample ${VTK_LIBRARIES} ${ITK_LIBRARIES})
#
# ADD_EXECUTABLE(SelfPatchMatchExample SelfPatchMatchExample.cpp SelfPatchMatch.h Helpers.h  Helpers.cpp)
# TARGET_LINK_LIBRARIES(SelfPatchMatchExample ${VTK_LIBRARIES} ${ITK_LIBRARIES})

# ADD_EXECUTABLE(CriminisiInpaintingRGBDI CriminisiInpaintingExampleRGBDI.cxx CriminisiInpainting.cpp CriminisiInpaintingDebugging.cpp CustomStyle.cpp SelfPatchMatch.h Helpers.h Helpers.cpp RotateVectors.h CriminisiInpainting.h)
# TARGET_LINK_LIBRARIES(CriminisiInpaintingRGBDI ${VTK_LIBRARIES} ${ITK_LIBRARIES})

##################################################################
# Testing

# SET( BUILD_TESTING OFF CACHE BOOL "Perform some tests." )
# IF( ITKTOOLS_BUILD_TESTING )
#   ENABLE_TESTING()
#   set(BaselineDir ${CriminisiInpainting_SOURCE_DIR}/../Testing/Baselines/)
#   set(DataDir ${CriminisiInpainting_SOURCE_DIR}/../Testing/Data/)
#   set(ExeDir ${CriminisiInpainting_BINARY_DIR})
#   set(OutDir ${CriminisiInpainting_BINARY_DIR}/Testing/)
# ENDIF()
# 
# ADD_EXECUTABLE(TestSelfPatchCompare TestSelfPatchCompare.cpp SelfPatchCompare.cpp)
# TARGET_LINK_LIBRARIES(TestSelfPatchCompare ${ITK_LIBRARIES})
