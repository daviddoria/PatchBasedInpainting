cmake_minimum_required(VERSION 2.6)
PROJECT(PatchBasedInpainting)

# Where to copy executables when 'make install' is run
SET(INSTALL_DIR ${CMAKE_INSTALL_PREFIX} )
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) #fix the dynamic linking error (RPATH) that would occur without this

# This is needed for shared_ptr and the trick using enable_if and if_fundamental to allow scalars to be treated as the 0th component of a vector.
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")

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

SET(BuildLibrary CACHE BOOL "Build the library.")
if(BuildLibrary)
  add_library(PatchBasedInpainting
  CandidatePairs.cpp
  DebugOutputs.cpp
  Derivatives.cpp
  Helpers.cpp
  HelpersOutput.cpp
  Histograms.cpp
  itkCommandLineArgumentParser.cxx
  ITKHelpers.cpp
  ITKVTKHelpers.cpp
  ITKImageCollection.cpp
  Isophotes.cpp
  Mask.cpp
  MaskOperations.cpp
  NamedITKImageCollection.cpp
  NamedITKImage.cpp
  NamedVTKImage.cpp
  PairDifferences.cpp
  Patch.cpp
  PatchBasedInpainting.cpp
  PatchBasedInpaintingDebugging.cpp
  PatchPair.cpp
  Priority.cpp
  PriorityCriminisi.cpp
  PriorityDepth.cpp
  PriorityFactory.cpp
  PriorityManual.cpp
  PriorityOnionPeel.cpp
  PriorityRandom.cpp
  SelfPatchCompare.cpp
  SourcePatchCollection.cpp
  )
  set_target_properties(PatchBasedInpainting PROPERTIES COMPILE_DEFINITIONS "${compileflags}")
endif()

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
  TARGET_LINK_LIBRARIES(PatchBasedInpaintingNonInteractive PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES})
  INSTALL( TARGETS PatchBasedInpaintingNonInteractive RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(BuildTests CACHE BOOL "Build tests.")
if(BuildTests)
  message ("Building tests.")
  add_subdirectory(Tests)
ENDIF(BuildTests)

SET(BuildDemos CACHE BOOL "Build demos.")
if(BuildDemos)
  add_subdirectory(Demos)
ENDIF(BuildDemos)
