cmake_minimum_required(VERSION 2.6)
PROJECT(PatchBasedInpainting)

# Where to copy executables when 'make install' is run
SET(INSTALL_DIR ${CMAKE_INSTALL_PREFIX} )
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) #fix the dynamic linking error (RPATH) that would occur without this

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

# Check for Qt4. If it is available, build the PatchBasedInpainting library using it so that SelfPatchCompare can use QtConcurrent.
FIND_PACKAGE(Qt4)
if(QT4_FOUND)
  message ( "QT4 found successfully." )
  INCLUDE(${QT_USE_FILE})
  list(APPEND compileflags "USE_QT_PARALLEL")
else ( QT4_FOUND )
  message ( "QT4 NOT found successfully." )
endif ( QT4_FOUND )

add_library(PatchBasedInpainting
CandidatePairs.cpp
ClusterColors.cpp
ClusterColorsAdaptive.cpp
ClusterColorsUniform.cpp
DebugOutputs.cpp
Derivatives.cpp
Helpers.cpp
HelpersOutput.cpp
Histograms.cpp
ImageCollection.cpp
Mask.cpp
Patch.cpp
PatchBasedInpainting.cpp
PatchBasedInpaintingDebugging.cpp
PatchBasedInpaintingInterface.cpp
PatchPair.cpp
PatchSorting.cpp
Priority.cpp
PriorityCriminisi.cpp
PriorityDepth.cpp
PriorityManual.cpp
PriorityOnionPeel.cpp
PriorityRandom.cpp
SelfPatchCompare.cpp
)
set_target_properties(PatchBasedInpainting PROPERTIES COMPILE_DEFINITIONS "${compileflags}")

# Interactive
SET(BuildInteractive CACHE BOOL "Build the interactive version.")
if(BuildInteractive)
  add_subdirectory(Interactive)
endif()

# Non-interactive
SET(BuildNonInteractive CACHE BOOL "Build the non-interactive version.")
if(BuildNonInteractive)
  ADD_EXECUTABLE(PatchBasedInpaintingNonInteractive
  PatchBasedInpaintingNonInteractive.cpp)
  TARGET_LINK_LIBRARIES(PatchBasedInpaintingNonInteractive PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})
  INSTALL( TARGETS PatchBasedInpaintingNonInteractive RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(BuildTests CACHE BOOL "Build tests.")
if(BuildTests)
  add_subdirectory(Tests)
ENDIF(BuildTests)
