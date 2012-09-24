cmake_minimum_required(VERSION 2.6)

PROJECT(PatchBasedInpainting)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(ROOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# A function to get all user defined variables with a specified prefix
function (getListOfVarsStartingWith _prefix _varResult)
    get_cmake_property(_vars VARIABLES)
    string (REGEX MATCHALL "(^|;)${_prefix}[A-Za-z0-9_]*" _matchedVars "${_vars}")
    set (${_varResult} ${_matchedVars} PARENT_SCOPE)
endfunction()

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
set(PatchBasedInpainting_libraries ${PatchBasedInpainting_libraries} ${VTK_LIBRARIES})

if( "${VTK_MAJOR_VERSION}" LESS 6 )
  MESSAGE(FATAL_ERROR "You must build this code with VTK >= 6.0!")
endif( "${VTK_MAJOR_VERSION}" LESS 6 )

# ITK
FIND_PACKAGE(ITK REQUIRED ITKCommon ITKIOImageBase ITKIOPNG ITKIOMeta
ITKImageIntensity ITKImageFeature ITKMathematicalMorphology ITKBinaryMathematicalMorphology ITKDistanceMap)
INCLUDE(${ITK_USE_FILE})
set(PatchBasedInpainting_libraries ${PatchBasedInpainting_libraries} ${ITK_LIBRARIES})

# Boost
FIND_PACKAGE(Boost 1.51 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

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
get_property(FoundMask GLOBAL PROPERTY MaskIncludeDirs SET)
if(NOT FoundMask)
  add_subdirectory(Mask)
endif()

get_property(MaskIncludeDirs GLOBAL PROPERTY MaskIncludeDirs)
set(PatchBasedInpainting_include_dirs ${PatchBasedInpainting_include_dirs} ${MaskIncludeDirs})
get_property(MaskLibs GLOBAL PROPERTY MaskLibs)
set(PatchBasedInpainting_libraries ${PatchBasedInpainting_libraries} ${MaskLibs})

# VTKHelpers
get_property(FoundVTKHelpers GLOBAL PROPERTY VTKHelpersIncludeDirs SET)
if(NOT FoundVTKHelpers)
  add_subdirectory(VTKHelpers)
endif()

get_property(VTKHelpersIncludeDirs GLOBAL PROPERTY VTKHelpersIncludeDirs)
set(PatchBasedInpainting_include_dirs ${PatchBasedInpainting_include_dirs} ${VTKHelpersIncludeDirs})
get_property(VTKHelpersLibs GLOBAL PROPERTY VTKHelpersLibs)
set(PatchBasedInpainting_libraries ${PatchBasedInpainting_libraries} ${VTKHelpersLibs})

# ITKVTKHelpers
get_property(FoundITKVTKHelpers GLOBAL PROPERTY ITKVTKHelpersIncludeDirs SET)
if(NOT FoundITKVTKHelpers)
  add_subdirectory(ITKVTKHelpers)
endif()

get_property(ITKVTKHelpersIncludeDirs GLOBAL PROPERTY ITKVTKHelpersIncludeDirs)
set(PatchBasedInpainting_include_dirs ${PatchBasedInpainting_include_dirs} ${ITKVTKHelpersIncludeDirs})
get_property(ITKVTKHelpersLibs GLOBAL PROPERTY ITKVTKHelpersLibs)
set(PatchBasedInpainting_libraries ${PatchBasedInpainting_libraries} ${ITKVTKHelpersLibs})

if(NOT TARGET QtHelpers)
  add_subdirectory(QtHelpers)
  include_directories(${QtHelpers_includes})
endif()

# ITKQtHelpers
get_property(FoundITKQtHelpers GLOBAL PROPERTY ITKQtHelpersIncludeDirs SET)
if(NOT FoundITKQtHelpers)
  add_subdirectory(ITKQtHelpers)
  include_directories(${ITKVTKHelpers_includes})
endif()
get_property(ITKQtHelpersIncludeDirs GLOBAL PROPERTY ITKQtHelpersIncludeDirs)
set(PatchBasedInpainting_include_dirs ${PatchBasedInpainting_include_dirs} ${ITKQtHelpersIncludeDirs})
get_property(ITKQtHelpersLibs GLOBAL PROPERTY ITKQtHelpersLibs)
set(PatchBasedInpainting_libraries ${PatchBasedInpainting_libraries} ${ITKQtHelpersLibs})

# BoostHelpers
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

# Give the compiler all of the required include directories
include_directories(${PatchBasedInpainting_include_dirs})

add_subdirectory(Algorithms)
add_subdirectory(Concepts)
add_subdirectory(DifferenceFunctions)
add_subdirectory(Drivers)
add_subdirectory(Inpainters)
add_subdirectory(Interactive/Delegates)
add_subdirectory(ImageProcessing)
add_subdirectory(Initializers)
add_subdirectory(LearnParameters)
add_subdirectory(NearestNeighbor)
add_subdirectory(PixelDescriptors)
add_subdirectory(Priority)
add_subdirectory(Scripts)
add_subdirectory(SearchRegions)
add_subdirectory(SpeedTests)
add_subdirectory(Utilities)
add_subdirectory(Visitors)

# Core source files
add_library(PatchBasedInpainting
ImageProcessing/Derivatives.cpp
Utilities/itkCommandLineArgumentParser.cxx
Utilities/PatchHelpers.cpp
ImageProcessing/Isophotes.cpp
Priority/Priority.cpp
Priority/PriorityDepth.cpp
Priority/PriorityManual.cpp
Priority/PriorityConfidence.cpp
Priority/PriorityRandom.cpp
)

# Add the library to the list of libraries that are required to use this as a submodule.
set(PatchBasedInpainting_libraries ${PatchBasedInpainting_libraries} PatchBasedInpainting)

# Allow this project to be detected and used as a submodule
set(PatchBasedInpainting_include_dirs ${PatchBasedInpainting_include_dirs} ${CMAKE_CURRENT_SOURCE_DIR})
set_property(GLOBAL PROPERTY PatchBasedInpaintingIncludeDirs ${PatchBasedInpainting_include_dirs})
set_property(GLOBAL PROPERTY PatchBasedInpaintingLibs ${PatchBasedInpainting_libraries})

############ Executables ###########

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

SET(inpainting_InpaintingTexture CACHE BOOL "Build a patch based image inpainting algorithm that uses a gradient magnitude histogram difference sort on the top N SSD matches (to determine if texture is similar).")
if(inpainting_InpaintingTexture)
  ADD_EXECUTABLE(InpaintingTexture InpaintingTexture.cpp)
  TARGET_LINK_LIBRARIES(InpaintingTexture ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS InpaintingTexture RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(inpainting_InpaintingHistogramSort CACHE BOOL "Build a patch based image inpainting algorithm that uses a histogram difference sort on the top N SSD matches.")
if(inpainting_InpaintingHistogramSort)
  ADD_EXECUTABLE(InpaintingHistogramSort InpaintingHistogramSort.cpp)
  TARGET_LINK_LIBRARIES(InpaintingHistogramSort ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS InpaintingHistogramSort RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(inpainting_InpaintingStrategySelection CACHE BOOL "Build a patch based image inpainting algorithm that selects a strategy to sort the top N SSD matches of each target patch based on properties of the target patch.")
if(inpainting_InpaintingStrategySelection)
  ADD_EXECUTABLE(InpaintingStrategySelection InpaintingStrategySelection.cpp)
  TARGET_LINK_LIBRARIES(InpaintingStrategySelection ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS InpaintingStrategySelection RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(inpainting_InpaintingMinimizeIntroducedEnergy CACHE BOOL "Build a patch based image inpainting algorithm that uses an introduced energy sort on the top N SSD matches.")
if(inpainting_InpaintingMinimizeIntroducedEnergy)
  ADD_EXECUTABLE(InpaintingMinimizeIntroducedEnergy InpaintingMinimizeIntroducedEnergy.cpp)
  TARGET_LINK_LIBRARIES(InpaintingMinimizeIntroducedEnergy ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS InpaintingMinimizeIntroducedEnergy RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(inpainting_InpaintingTwoStep CACHE BOOL "Build a patch based image inpainting program that uses a two step (knn + best) search strategy.")
if(inpainting_InpaintingTwoStep)
  ADD_EXECUTABLE(InpaintingTwoStep InpaintingTwoStep.cpp)
  TARGET_LINK_LIBRARIES(InpaintingTwoStep ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS InpaintingTwoStep RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(inpainting_InpaintingBlurredSSD CACHE BOOL "Build a traditional patch comparison image inpainting using SSD on the blurred image.")
if(inpainting_InpaintingBlurredSSD)
  ADD_EXECUTABLE(InpaintingBlurredSSD InpaintingBlurredSSD.cpp)
  TARGET_LINK_LIBRARIES(InpaintingBlurredSSD ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS InpaintingBlurredSSD RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(inpainting_ClassicalImageInpainting CACHE BOOL "Build a traditional patch comparison image inpainting.")
if(inpainting_ClassicalImageInpainting)
  ADD_EXECUTABLE(ClassicalImageInpainting ClassicalImageInpainting.cpp)
  TARGET_LINK_LIBRARIES(ClassicalImageInpainting ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS ClassicalImageInpainting RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(inpainting_ClassicalImageInpaintingDebug CACHE BOOL "Build a traditional patch comparison image inpainting with lots of debugging output.")
if(inpainting_ClassicalImageInpaintingDebug)
  ADD_EXECUTABLE(ClassicalImageInpaintingDebug ClassicalImageInpaintingDebug.cpp)
  TARGET_LINK_LIBRARIES(ClassicalImageInpaintingDebug ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS ClassicalImageInpaintingDebug RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(inpainting_LocalOptimizationImageInpainting CACHE BOOL "Build a inpainting algorithm that uses locally (spatially and temporally) optimal choices instead of purely greedy choices.")
if(inpainting_LocalOptimizationImageInpainting)
  ADD_EXECUTABLE(LocalOptimizationImageInpainting LocalOptimizationImageInpainting.cpp)
  TARGET_LINK_LIBRARIES(LocalOptimizationImageInpainting ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS LocalOptimizationImageInpainting RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(inpainting_ClassicalImageInpaintingWithTextOutput CACHE BOOL "Build a traditional patch comparison image inpainting.")
if(inpainting_ClassicalImageInpaintingWithTextOutput)
  ADD_EXECUTABLE(ClassicalImageInpaintingWithTextOutput ClassicalImageInpaintingWithTextOutput.cpp)
  TARGET_LINK_LIBRARIES(ClassicalImageInpaintingWithTextOutput ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS ClassicalImageInpaintingWithTextOutput RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(inpainting_SmallPatchBigPatchInpainting CACHE BOOL "Build a two step (compare small patches, then compare big patches at the best of the small patches) image inpainting.")
if(inpainting_SmallPatchBigPatchInpainting)
  ADD_EXECUTABLE(SmallPatchBigPatchInpainting SmallPatchBigPatchInpainting.cpp)
  TARGET_LINK_LIBRARIES(SmallPatchBigPatchInpainting ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS SmallPatchBigPatchInpainting RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(inpainting_NarrowSearchByFeaturesInpainting CACHE BOOL "Build a two step (features, then patch comparison) image inpainting.")
if(inpainting_NarrowSearchByFeaturesInpainting)
  ADD_EXECUTABLE(NarrowSearchByFeaturesInpainting NarrowSearchByFeaturesInpainting.cpp PixelDescriptors/FeatureVectorPixelDescriptor.cpp)
  TARGET_LINK_LIBRARIES(NarrowSearchByFeaturesInpainting ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS NarrowSearchByFeaturesInpainting RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(inpainting_NarrowSearchByNormalsInpainting CACHE BOOL "Build a two step (normals, then patch comparison) image inpainting.")
if(inpainting_NarrowSearchByNormalsInpainting)
  ADD_EXECUTABLE(NarrowSearchByNormalsInpainting NarrowSearchByNormalsInpainting.cpp PixelDescriptors/FeatureVectorPixelDescriptor.cpp)
  TARGET_LINK_LIBRARIES(NarrowSearchByNormalsInpainting ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS NarrowSearchByNormalsInpainting RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

SET(BuildInpaintingTests CACHE BOOL "Build tests.")
if(BuildInpaintingTests)
  ENABLE_TESTING()
  add_subdirectory(Testing)
  #add_subdirectory(Tests)
  #add_subdirectory(Priority/Tests)
  #add_subdirectory(ImageProcessing/Tests)
  #add_subdirectory(NearestNeighbor/Tests)
  #add_subdirectory(Helpers/Tests)
  #add_subdirectory(Utilities/Tests)
   add_subdirectory(Interactive/Tests)
endif()

############################
SET(BuildInteractiveInpaintingTests CACHE BOOL "Build interactive tests.")
if(BuildInteractiveInpaintingTests)
  ENABLE_TESTING()
  add_subdirectory(Interactive/Tests)
ENDIF()

SET(BuildInpaintingDemos CACHE BOOL "Build demos.")
if(BuildInpaintingDemos)
  add_subdirectory(Demos)
endif()

# Add options to allow enabling/disabling of a set of targets
option(TurnOnAllInpainting "Turn on all variables starting with inpainting_")
option(TurnOffAllInpainting "Turn off all variables starting with inpainting_")

if(TurnOnAllInpainting AND TurnOffAllInpainting)
  message(FATAL_ERROR "You cannot turn targets on and off at the same time!")
endif()

if(TurnOnAllInpainting)
    getListOfVarsStartingWith("inpainting_" matchedVars)
    foreach (_var IN LISTS matchedVars)
        get_property(currentHelpString CACHE "${_var}" PROPERTY HELPSTRING)
        set(${_var} ON CACHE BOOL ${currentHelpString} FORCE)
    endforeach()

    get_property(turnOnHelpString CACHE "TurnOnAll" PROPERTY HELPSTRING)
    set(TurnOnAllInpainting OFF CACHE BOOL ${turnOnHelpString} FORCE)  # Set itself back to off, as this is a one time thing.
endif()

if(TurnOffAllInpainting)
    getListOfVarsStartingWith("inpainting_" matchedVars)
    foreach (_var IN LISTS matchedVars)
        get_property(currentHelpString CACHE "${_var}" PROPERTY HELPSTRING)
        set(${_var} OFF CACHE BOOL ${currentHelpString} FORCE)
    endforeach()

    get_property(turnOffHelpString CACHE "TurnOnAll" PROPERTY HELPSTRING)
    set(TurnOffAllInpainting OFF CACHE BOOL ${turnOffHelpString} FORCE)  # Set itself back to off, as this is a one time thing.
endif()

# Display where this code was used from (if it is used as a submodule, there may be multiple instances of this submodule in the project, only the first of which is used)
option(PatchBasedInpainting_ShowSubmoduleLocation "Show the path from which PatchBasedInpainting was used?" OFF)
if(PatchBasedInpainting_ShowSubmoduleLocation)
  message("PatchBasedInpainting used from ${CMAKE_CURRENT_SOURCE_DIR}")
endif(PatchBasedInpainting_ShowSubmoduleLocation)
