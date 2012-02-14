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


add_subdirectory(Helpers)

add_library(PatchBasedInpainting
#CandidatePairs.cpp
ImageProcessing/Derivatives.cpp
Utilities/itkCommandLineArgumentParser.cxx
ImageProcessing/Isophotes.cpp
ImageProcessing/Mask.cpp
ImageProcessing/MaskOperations.cpp
#PatchBasedInpainting.cpp
#PatchPair.cpp
#PatchPairDifferences.cpp
Priority/Priority.cpp
Priority/PriorityCriminisi.cpp
Priority/PriorityDepth.cpp
Priority/PriorityManual.cpp
Priority/PriorityOnionPeel.cpp
Priority/PriorityRandom.cpp
#SourcePatchCollection.cpp
#ValidRegionIterator.cpp
)

# Interactive
SET(BuildInteractive CACHE BOOL "Build the interactive version.")
if(BuildInteractive)
  add_subdirectory(Interactive)
endif()

SET(BuildViewer CACHE BOOL "Build the viewer.")
if(BuildViewer)
  message("Building the viewer.")
  add_subdirectory(Interactive)
endif()

SET(NormalImageInpainting CACHE BOOL "Build a traditional patch comparison image inpainting.")
if(NormalImageInpainting)
  ADD_EXECUTABLE(NormalImageInpainting
  NormalImageInpainting.cpp)
  TARGET_LINK_LIBRARIES(NormalImageInpainting PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} libHelpers)
  INSTALL( TARGETS NormalImageInpainting RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(TwoStepInpainting CACHE BOOL "Build a two step (features, then patch comparison) image inpainting.")
if(TwoStepInpainting)
  ADD_EXECUTABLE(TwoStepInpainting TwoStepInpainting.cpp PixelDescriptors/FeatureVectorPixelDescriptor.cpp)
  TARGET_LINK_LIBRARIES(TwoStepInpainting PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} libHelpers)
  INSTALL( TARGETS TwoStepInpainting RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(SmallPatchBigPatchInpainting CACHE BOOL "Build a two step (compare small patches, then compare big patches at the best of the small patches) image inpainting.")
if(SmallPatchBigPatchInpainting)
  ADD_EXECUTABLE(SmallPatchBigPatchInpainting SmallPatchBigPatchInpainting.cpp)
  TARGET_LINK_LIBRARIES(SmallPatchBigPatchInpainting PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} libHelpers)
  INSTALL( TARGETS SmallPatchBigPatchInpainting RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(BuildTests CACHE BOOL "Build tests.")
if(BuildTests)
  ENABLE_TESTING()
  add_subdirectory(Testing)
  #add_subdirectory(Tests)
  add_subdirectory(Priority/Tests)
  #add_subdirectory(ImageProcessing/Tests)
  add_subdirectory(NearestNeighbor/Tests)
  add_subdirectory(Helpers/Tests)
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
add_subdirectory(DifferenceFunctions)
add_subdirectory(Concepts)