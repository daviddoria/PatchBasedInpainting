cmake_minimum_required(VERSION 2.6)

PROJECT(PatchBasedInpainting)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(ROOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Where to copy executables when 'make install' is run
SET(INSTALL_DIR ${CMAKE_INSTALL_PREFIX} )
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) #fix the dynamic linking error (RPATH) that would occur without this

# This is needed for shared_ptr and the trick using enable_if and if_fundamental to allow scalars to be treated as the 0th component of a vector.
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")

# Setup include directories for Qt
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# VTK
FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

if( "${VTK_MAJOR_VERSION}" LESS 6 )
  MESSAGE(FATAL_ERROR "You must build this code with VTK >= 6.0!")
endif( "${VTK_MAJOR_VERSION}" LESS 6 )

# ITK
FIND_PACKAGE(ITK REQUIRED ITKCommon ITKIOImageBase ITKIOPNG
ITKImageIntensity ITKImageFeature ITKMathematicalMorphology ITKBinaryMathematicalMorphology ITKDistanceMap)
INCLUDE(${ITK_USE_FILE})

#FIND_PACKAGE(Boost COMPONENTS array REQUIRED) # 'array' does not seem to be a library
#INCLUDE(${Boost_USE_FILE}) # Boost_USE_FILE is not a real variable

# Check for Qt4. If it is available, build the PatchBasedInpainting library using it so that SelfPatchCompare can use QtConcurrent.
# FIND_PACKAGE(Qt4)
# if(QT4_FOUND)
#   message ( "QT4 found successfully." )
#   INCLUDE(${QT_USE_FILE})
#   list(APPEND compileflags "USE_QT_PARALLEL")
# else ( QT4_FOUND )
#   message ( "QT4 NOT found successfully." )
# endif ( QT4_FOUND )

FIND_PACKAGE(Qt4)
INCLUDE(${QT_USE_FILE})
list(APPEND compileflags "USE_QT_PARALLEL")

# Submodules
if(NOT TARGET Mask)
  add_subdirectory(Mask)
  include_directories(${Mask_includes})
endif()

if(NOT TARGET VTKHelpers)
  add_subdirectory(VTKHelpers)
include_directories(${VTKHelpers_includes})
endif()

if(NOT TARGET ITKVTKHelpers)
  add_subdirectory(ITKVTKHelpers)
  include_directories(${ITKVTKHelpers_includes})
endif()

if(NOT TARGET QtHelpers)
  add_subdirectory(QtHelpers)
  include_directories(${QtHelpers_includes})
endif()

if(NOT TARGET ITKQtHelpers)
  add_subdirectory(ITKQtHelpers)
    include_directories(${ITKQtHelpers_includes})
endif()

if(NOT TARGET BoostHelpers)
  add_subdirectory(BoostHelpers)
    include_directories(${BoostHelpers_includes})
endif()

if( (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Mask/.git") OR
(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/VTKHelpers/.git") OR
(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ITKVTKHelpers/.git") OR
(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/QtHelpers/.git") OR
(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/BoostHelpers/.git") )
  message( SEND_ERROR "At least one of the git submodules is not available.
                       Please run git submodule update --init --recursive")
endif()


add_subdirectory(NearestNeighbor)
add_subdirectory(Visitors)
add_subdirectory(Inpainters)
add_subdirectory(ImageProcessing)
add_subdirectory(Initializers)
add_subdirectory(PixelDescriptors)
add_subdirectory(DifferenceFunctions)
add_subdirectory(Concepts)
add_subdirectory(Interactive/Delegates)
add_subdirectory(LearnParameters)
add_subdirectory(SpeedTests)
add_subdirectory(SearchRegions)

# Core source files
add_library(PatchBasedInpainting
ImageProcessing/Derivatives.cpp
Utilities/itkCommandLineArgumentParser.cxx
ImageProcessing/Isophotes.cpp
ImageProcessing/BoundaryNormals.cpp
Priority/Priority.cpp
Priority/PriorityCriminisi.cpp
Priority/PriorityDepth.cpp
Priority/PriorityManual.cpp
Priority/PriorityOnionPeel.cpp
Priority/PriorityRandom.cpp
)

# Interactive
SET(BuildInteractive CACHE BOOL "Build the interactive version.")
if(BuildInteractive)
    message("Building the interactive components.")
  add_subdirectory(Interactive)
endif()

SET(BuildViewer CACHE BOOL "Build the viewer.")
if(BuildViewer)
  message("Building the viewer.")
  add_subdirectory(Interactive)
endif()

SET(inpainting_InpaintingWithLocalSearch CACHE BOOL "Build a traditional patch comparison image inpainting that displays a list of the top patches for the user to select if the best automatically found patch is decided not to be very good.")
if(inpainting_InpaintingWithLocalSearch)
  include(InpaintingWithLocalSearch.cmake)
endif()

SET(inpainting_InpaintingRGBD CACHE BOOL "Build a traditional patch comparison image inpainting that displays a list of the top patches for the user to select if the best automatically found patch is decided not to be very good.")
if(inpainting_InpaintingRGBD)
  include(InpaintingRGBD.cmake)
endif()

SET(inpainting_InpaintingAutomatic CACHE BOOL "Build a traditional patch comparison image inpainting that displays a list of the top patches for the user to select if the best automatically found patch is decided not to be very good.")
if(inpainting_InpaintingAutomatic)
  include(InpaintingAutomatic.cmake)
endif()

SET(inpainting_InpaintingCIELab CACHE BOOL "Build a traditional patch comparison image inpainting that displays a list of the top patches for the user to select if the best automatically found patch is decided not to be very good.")
if(inpainting_InpaintingCIELab)
  include(InpaintingCIELab.cmake)
endif()

SET(inpainting_InpaintingWithVerification CACHE BOOL "Build an inpainting program that displays a list of the top patches for the user to select if the best patch does not pass some acceptance tests.")
if(inpainting_InpaintingWithVerification)
  include(InpaintingWithVerification.cmake)
endif()

SET(inpainting_InpaintingVectorized CACHE BOOL "Build a traditional patch comparison image inpainting that displays a list of the top patches for the user to select if the best automatically found patch is decided not to be very good.")
if(inpainting_InpaintingVectorized)
  include(InpaintingVectorized.cmake)
endif()

SET(inpainting_PrecomputedInpainting CACHE BOOL "Build a traditional patch comparison image inpainting that displays a list of the top patches for the user to select if the best automatically found patch is decided not to be very good.")
if(inpainting_PrecomputedInpainting)
  include(PrecomputedInpainting.cmake)
endif()

SET(inpainting_InpaintingWithManualSelection CACHE BOOL "Build a traditional patch comparison image inpainting that displays a list of the top patches at every iteration for manual selection.")
if(inpainting_InpaintingWithManualSelection)
  include(InpaintingWithManualSelection.cmake)
endif()

SET(inpainting_InpaintingWithTopPatchDisplay CACHE BOOL "Build a traditional patch comparison image inpainting that displays a list of the top patches at every iteration for passive inspection.")
if(inpainting_InpaintingWithTopPatchDisplay)
  include(InpaintingWithTopPatchDisplay.cmake)
endif()

SET(inpainting_ClassicalImageInpaintingWithVisualOutput CACHE BOOL "Build a traditional patch comparison image inpainting that displays the image at every iteration.")
if(inpainting_ClassicalImageInpaintingWithVisualOutput)
  include(ClassicalImageInpaintingWithVisualOutput.cmake)
endif()

SET(inpainting_ClassicalImageInpainting CACHE BOOL "Build a traditional patch comparison image inpainting.")
if(inpainting_ClassicalImageInpainting)
  ADD_EXECUTABLE(ClassicalImageInpainting ClassicalImageInpainting.cpp)
  TARGET_LINK_LIBRARIES(ClassicalImageInpainting PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES}
                       ${QT_LIBRARIES} Helpers ITKHelpers Mask)
  INSTALL( TARGETS ClassicalImageInpainting RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(inpainting_LocalOptimizationImageInpainting CACHE BOOL "Build a inpainting algorithm that uses locally (spatially and temporally) optimal choices instead of purely greedy choices.")
if(inpainting_LocalOptimizationImageInpainting)
  ADD_EXECUTABLE(LocalOptimizationImageInpainting LocalOptimizationImageInpainting.cpp)
  TARGET_LINK_LIBRARIES(LocalOptimizationImageInpainting PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES}
                       ${QT_LIBRARIES} Helpers ITKHelpers Mask)
  INSTALL( TARGETS LocalOptimizationImageInpainting RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(inpainting_ClassicalImageInpaintingWithTextOutput CACHE BOOL "Build a traditional patch comparison image inpainting.")
if(inpainting_ClassicalImageInpaintingWithTextOutput)
  ADD_EXECUTABLE(ClassicalImageInpaintingWithTextOutput ClassicalImageInpaintingWithTextOutput.cpp)
  TARGET_LINK_LIBRARIES(ClassicalImageInpaintingWithTextOutput PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} Helpers)
  INSTALL( TARGETS ClassicalImageInpaintingWithTextOutput RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(inpainting_SmallPatchBigPatchInpainting CACHE BOOL "Build a two step (compare small patches, then compare big patches at the best of the small patches) image inpainting.")
if(inpainting_SmallPatchBigPatchInpainting)
  ADD_EXECUTABLE(SmallPatchBigPatchInpainting SmallPatchBigPatchInpainting.cpp)
  TARGET_LINK_LIBRARIES(SmallPatchBigPatchInpainting PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} Helpers)
  INSTALL( TARGETS SmallPatchBigPatchInpainting RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(inpainting_NarrowSearchByFeaturesInpainting CACHE BOOL "Build a two step (features, then patch comparison) image inpainting.")
if(inpainting_NarrowSearchByFeaturesInpainting)
  ADD_EXECUTABLE(NarrowSearchByFeaturesInpainting NarrowSearchByFeaturesInpainting.cpp PixelDescriptors/FeatureVectorPixelDescriptor.cpp)
  TARGET_LINK_LIBRARIES(NarrowSearchByFeaturesInpainting PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} ${PCL_LIBRARIES} Helpers)
  INSTALL( TARGETS NarrowSearchByFeaturesInpainting RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(inpainting_NarrowSearchByNormalsInpainting CACHE BOOL "Build a two step (normals, then patch comparison) image inpainting.")
if(inpainting_NarrowSearchByNormalsInpainting)
  ADD_EXECUTABLE(NarrowSearchByNormalsInpainting NarrowSearchByNormalsInpainting.cpp PixelDescriptors/FeatureVectorPixelDescriptor.cpp)
  TARGET_LINK_LIBRARIES(NarrowSearchByNormalsInpainting PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} ${PCL_LIBRARIES} Helpers)
  INSTALL( TARGETS NarrowSearchByNormalsInpainting RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(inpainting_BuildTests CACHE BOOL "Build tests.")
if(inpainting_BuildTests)
  ENABLE_TESTING()
  add_subdirectory(Testing)
  #add_subdirectory(Tests)
  #add_subdirectory(Priority/Tests)
  #add_subdirectory(ImageProcessing/Tests)
  #add_subdirectory(NearestNeighbor/Tests)
  #add_subdirectory(Helpers/Tests)
  #add_subdirectory(Utilities/Tests)
   add_subdirectory(Interactive/Tests)
endif(inpainting_BuildTests)

############################
SET(BuildInteractiveTests CACHE BOOL "Build interactive tests.")
if(BuildInteractiveTests)
  ENABLE_TESTING()
  add_subdirectory(Interactive/Tests)
ENDIF(BuildInteractiveTests)

SET(inpainting_BuildDemos CACHE BOOL "Build demos.")
if(inpainting_BuildDemos)
  add_subdirectory(Demos)
ENDIF(inpainting_BuildDemos)
