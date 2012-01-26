cmake_minimum_required(VERSION 2.6)
PROJECT(PatchBasedInpainting)

# Where to copy executables when 'make install' is run
SET(INSTALL_DIR ${CMAKE_INSTALL_PREFIX} )
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) #fix the dynamic linking error (RPATH) that would occur without this

# This is needed for shared_ptr and the trick using enable_if and if_fundamental to allow scalars to be treated as the 0th component of a vector.
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

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

if(BuildTests OR BuildInteractive OR BuildNonInteractive)
  add_subdirectory(Helpers)
endif(BuildTests OR BuildInteractive OR BuildNonInteractive)

if(BuildInteractive OR BuildNonInteractive)
  add_library(PatchBasedInpainting
  CandidatePairs.cpp
  ImageProcessing/Derivatives.cpp
  Utilities/itkCommandLineArgumentParser.cxx
  ImageProcessing/Isophotes.cpp
  ImageProcessing/Mask.cpp
  ImageProcessing/MaskOperations.cpp
  PatchBasedInpainting.cpp
  PatchPair.cpp
  PatchPairDifferences.cpp
  PixelCollection.cpp
  Priority/Priority.cpp
  Priority/PriorityCriminisi.cpp
  Priority/PriorityDepth.cpp
  Priority/PriorityManual.cpp
  Priority/PriorityOnionPeel.cpp
  Priority/PriorityRandom.cpp
  Priority/PrioritySearchHighest.cpp
  SourcePatchCollection.cpp
  ValidRegionIterator.cpp
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
  TARGET_LINK_LIBRARIES(PatchBasedInpaintingNonInteractive PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} libHelpers)
  INSTALL( TARGETS PatchBasedInpaintingNonInteractive RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(BuildTests CACHE BOOL "Build tests.")
if(BuildTests)
  ENABLE_TESTING()
  add_subdirectory(Testing)
  add_subdirectory(Tests)
  add_subdirectory(Priority/Tests)
  add_subdirectory(ImageProcessing/Tests)
  add_subdirectory(NearestNeighbor/Tests)
  #add_subdirectory(Helpers/Tests)
ENDIF(BuildTests)

SET(BuildInteractiveTests CACHE BOOL "Build interactive tests.")
if(BuildInteractiveTests)
  ENABLE_TESTING()
  add_subdirectory(Interactive/Tests)
ENDIF(BuildInteractiveTests)

SET(BuildDemos CACHE BOOL "Build demos.")
if(BuildDemos)
  add_subdirectory(Demos)
ENDIF(BuildDemos)

add_subdirectory(Utilities/Tests)
add_subdirectory(NearestNeighbor)
add_subdirectory(Visitors)
add_subdirectory(Inpainters)
add_subdirectory(Initializers)
add_subdirectory(PixelDescriptors)