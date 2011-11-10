cmake_minimum_required(VERSION 2.6)
PROJECT(CriminisiInpainting)

# Where to copy executables when 'make install' is run
SET(INSTALL_DIR ${CMAKE_INSTALL_PREFIX} )
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) #fix the dynamic linking error (RPATH) that would occur without this

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

QT4_WRAP_UI(UISrcs CriminisiInpainting.ui FileSelector.ui FileSelectionWidget.ui)
QT4_WRAP_CPP(MOCSrcs Form.h ProgressThread.h FileSelector.h FileSelectionWidget.h PixmapDelegate.h)

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

add_library(CriminisiInpainting
CandidatePairs.cpp
CriminisiInpainting.cpp
CriminisiInpaintingDebugging.cpp
CriminisiInpaintingInterface.cpp
Derivatives.cpp
Helpers.cpp
Histograms.cpp
Mask.cpp
Patch.cpp
PatchPair.cpp
SelfPatchCompare.cpp
)

# Interactive
SET(BuildInteractive CACHE BOOL "Build the interactive version.")
if(BuildInteractive)
  ADD_EXECUTABLE(CriminisiInpaintingInteractive
  FileSelector.cpp
  FileSelectionWidget.cpp
  Form.cxx
  ForwardLookTableModel.cpp
  FormGUIElements.cxx
  InpaintingVisualizationStack.cpp
  Interactive.cpp #driver (contains main())
  InteractorStyleImageNoLevel.cxx
  Layer.cpp
  PixmapDelegate.cpp
  ProgressThread.cpp
  TopPatchesTableModel.cpp
  VectorLayer.cpp
  ${UISrcs} ${MOCSrcs})
  TARGET_LINK_LIBRARIES(CriminisiInpaintingInteractive CriminisiInpainting ${VTK_LIBRARIES} QVTK ${ITK_LIBRARIES} ${QT_LIBRARIES})
  INSTALL( TARGETS CriminisiInpaintingInteractive RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

# Non-interactive
SET(BuildNonInteractive CACHE BOOL "Build the non-interactive version.")
if(BuildNonInteractive)
  ADD_EXECUTABLE(CriminisiInpaintingNonInteractive
  CriminisiInpaintingNonInteractive.cpp)
  TARGET_LINK_LIBRARIES(CriminisiInpaintingNonInteractive CriminisiInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})
  INSTALL( TARGETS CriminisiInpaintingNonInteractive RUNTIME DESTINATION ${INSTALL_DIR} )
endif()


# ADD_EXECUTABLE(LearnLambda LearnLambda.cpp)
# TARGET_LINK_LIBRARIES(LearnLambda CriminisiInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

ADD_EXECUTABLE(TestColorImageByScore Tests/TestColorImageByScore.cpp)
TARGET_LINK_LIBRARIES(TestColorImageByScore CriminisiInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})


# ADD_EXECUTABLE(ExamplePatchDifference Examples/ExamplePatchDifference.cpp)
# TARGET_LINK_LIBRARIES(ExamplePatchDifference CriminisiInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

#--------------
# --- Tests ---
#--------------
# ADD_EXECUTABLE(TestIsophotes Tests/TestIsophotes.cpp)
# TARGET_LINK_LIBRARIES(TestIsophotes CriminisiInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})
# 
# ADD_EXECUTABLE(TestGradient Tests/TestGradient.cpp)
# TARGET_LINK_LIBRARIES(TestGradient CriminisiInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

# ADD_EXECUTABLE(TestBoundaryNormals Tests/TestBoundaryNormals.cpp)
# TARGET_LINK_LIBRARIES(TestBoundaryNormals CriminisiInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

# ADD_EXECUTABLE(TestFollowIsophotesAcrossBoundary Tests/TestFollowIsophotesAcrossBoundary.cpp)
# TARGET_LINK_LIBRARIES(TestFollowIsophotesAcrossBoundary CriminisiInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

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
