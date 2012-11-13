cmake_minimum_required(VERSION 2.8.6) # Required for AutoMOC (http://blogs.kde.org/2011/11/01/cool-new-stuff-cmake-286-automoc)

# Suggested build flags are -fopenmp -msse3

# To use CMake's Automoc, headers (.h files, or .hpp if the class is declared directly in the .hpp)
# must be added to the add_executable list.

PROJECT(PatchBasedInpainting)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(ROOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_AUTOMOC_MOC_OPTIONS "-DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED") # Fixes Parse error at "BOOST_JOIN" error (https://bugreports.qt-project.org/browse/QTBUG-22829)

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
if(UNIX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++11")
endif(UNIX)

# Let Qt find it's MOCed headers in the build directory.
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# VTK
FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})
set(PatchBasedInpainting_libraries ${PatchBasedInpainting_libraries} ${VTK_LIBRARIES})

if( "${VTK_MAJOR_VERSION}" LESS 6 )
  MESSAGE(FATAL_ERROR "You must build this code with VTK >= 6.0!")
endif( "${VTK_MAJOR_VERSION}" LESS 6 )

# ITK
FIND_PACKAGE(ITK REQUIRED ITKCommon ITKIOImageBase ITKIOPNG ITKIOMeta ITKTestKernel
ITKImageIntensity ITKImageFeature ITKMathematicalMorphology ITKBinaryMathematicalMorphology ITKDistanceMap)
INCLUDE(${ITK_USE_FILE})
set(PatchBasedInpainting_libraries ${PatchBasedInpainting_libraries} ${ITK_LIBRARIES})

# Boost
FIND_PACKAGE(Boost 1.51 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# Check for Qt4. If it is available, build the PatchBasedInpainting library
# using it so that SelfPatchCompare can use QtConcurrent. We must do this
# AFTER including the submodules, as the Interactive directory
FIND_PACKAGE(Qt4)
if(QT4_FOUND)
  message ( "QT4 found successfully." )
  INCLUDE(${QT_USE_FILE})
  set(PatchBasedInpainting_libraries ${PatchBasedInpainting_libraries} ${QT_LIBRARIES})

  list(APPEND compileflags "USE_QT_PARALLEL")

else(QT4_FOUND)
  message("QT4 NOT found successfully.")
endif(QT4_FOUND)

#FIND_PACKAGE(Qt4)
#INCLUDE(${QT_USE_FILE})
#list(APPEND compileflags "USE_QT_PARALLEL")

# Submodules
# Mask
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

# QtHelpers
get_property(FoundQtHelpers GLOBAL PROPERTY QtHelpersIncludeDirs SET)
if(NOT FoundQtHelpers)
  add_subdirectory(QtHelpers)
endif()

get_property(QtHelpersIncludeDirs GLOBAL PROPERTY QtHelpersIncludeDirs)
set(PatchBasedInpainting_include_dirs ${PatchBasedInpainting_include_dirs} ${QtHelpersIncludeDirs})
get_property(QtHelpersLibs GLOBAL PROPERTY QtHelpersLibs)
set(PatchBasedInpainting_libraries ${PatchBasedInpainting_libraries} ${QtHelpersLibs})

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

# ITKVTKCamera
get_property(FoundITKVTKCamera GLOBAL PROPERTY ITKVTKCameraIncludeDirs SET)
if(NOT FoundITKVTKCamera)
  add_subdirectory(ITKVTKCamera)
  include_directories(${ITKVTKCamera_includes})
endif()
get_property(ITKVTKCameraIncludeDirs GLOBAL PROPERTY ITKVTKCameraIncludeDirs)
set(PatchBasedInpainting_include_dirs ${PatchBasedInpainting_include_dirs} ${ITKVTKCameraIncludeDirs})
get_property(ITKVTKCameraLibs GLOBAL PROPERTY ITKVTKCameraLibs)
set(PatchBasedInpainting_libraries ${PatchBasedInpainting_libraries} ${ITKVTKCameraLibs})

# BoostHelpers
if(NOT TARGET BoostHelpers)
  add_subdirectory(BoostHelpers)
    include_directories(${BoostHelpers_includes})
endif()

if( (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Mask/.git") OR
(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/VTKHelpers/.git") OR
(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ITKVTKCamera/.git") OR
(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ITKVTKHelpers/.git") OR
(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ITKQtHelpers/.git") OR
(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/QtHelpers/.git") OR
(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/BoostHelpers/.git") )
  message( SEND_ERROR "At least one of the git submodules is not available.
                       Please run git submodule update --init --recursive")
endif()

# Give the compiler all of the required include directories
include_directories(${PatchBasedInpainting_include_dirs})

# We must do this AFTER including the submodules, as the Interactive directory
if(QT4_FOUND)
  add_subdirectory(Interactive)
endif(QT4_FOUND)

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

option(inpainting_InpaintingWithLocalSearch "Build a traditional patch comparison image inpainting that displays a list of the top patches for the user to select if the best automatically found patch is decided not to be very good.")
if(inpainting_InpaintingWithLocalSearch)
  include(InpaintingWithLocalSearch.cmake)
endif()

option(inpainting_InpaintingRGBD "Build a traditional patch comparison image inpainting that displays a list of the top patches for the user to select if the best automatically found patch is decided not to be very good.")
if(inpainting_InpaintingRGBD)
  include(InpaintingRGBD.cmake)
endif()

option(inpainting_InpaintingAutomatic "Build a traditional patch comparison image inpainting that displays a list of the top patches for the user to select if the best automatically found patch is decided not to be very good.")
if(inpainting_InpaintingAutomatic)
  include(InpaintingAutomatic.cmake)
endif()

option(inpainting_InpaintingCIELab "Build a traditional patch comparison image inpainting that displays a list of the top patches for the user to select if the best automatically found patch is decided not to be very good.")
if(inpainting_InpaintingCIELab)
  include(InpaintingCIELab.cmake)
endif()

option(inpainting_InpaintingWithVerification "Build an inpainting program that displays a list of the top patches for the user to select if the best patch does not pass some acceptance tests.")
if(inpainting_InpaintingWithVerification)
  include(InpaintingWithVerification.cmake)
endif()

option(inpainting_InpaintingVectorized "Build a traditional patch comparison image inpainting that displays a list of the top patches for the user to select if the best automatically found patch is decided not to be very good.")
if(inpainting_InpaintingVectorized)
  include(InpaintingVectorized.cmake)
endif()

option(inpainting_PrecomputedInpainting "Build a traditional patch comparison image inpainting that displays a list of the top patches for the user to select if the best automatically found patch is decided not to be very good.")
if(inpainting_PrecomputedInpainting)
  include(PrecomputedInpainting.cmake)
endif()

option(inpainting_InpaintingWithManualSelection "Build a traditional patch comparison image inpainting that displays a list of the top patches at every iteration for manual selection.")
if(inpainting_InpaintingWithManualSelection)
  include(InpaintingWithManualSelection.cmake)
endif()

option(inpainting_InpaintingWithTopPatchDisplay "Build a traditional patch comparison image inpainting that displays a list of the top patches at every iteration for passive inspection.")
if(inpainting_InpaintingWithTopPatchDisplay)
  include(InpaintingWithTopPatchDisplay.cmake)
endif()

option(inpainting_ClassicalImageInpaintingWithVisualOutput "Build a traditional patch comparison image inpainting that displays the image at every iteration.")
if(inpainting_ClassicalImageInpaintingWithVisualOutput)
  include(ClassicalImageInpaintingWithVisualOutput.cmake)
endif()

option(inpainting_InpaintingTexture "Build a patch based image inpainting algorithm that uses a gradient magnitude histogram difference sort on the top N SSD matches (to determine if texture is similar).")
if(inpainting_InpaintingTexture)
  ADD_EXECUTABLE(InpaintingTexture InpaintingTexture.cpp)
  TARGET_LINK_LIBRARIES(InpaintingTexture ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS InpaintingTexture RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

option(inpainting_InpaintingHistogramSort "Build a patch based image inpainting algorithm that uses a histogram difference sort on the top N SSD matches.")
if(inpainting_InpaintingHistogramSort)
  ADD_EXECUTABLE(InpaintingHistogramSort InpaintingHistogramSort.cpp)
  TARGET_LINK_LIBRARIES(InpaintingHistogramSort ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS InpaintingHistogramSort RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

option(inpainting_InpaintingStrategySelection "Build a patch based image inpainting algorithm that selects a strategy to sort the top N SSD matches of each target patch based on properties of the target patch.")
if(inpainting_InpaintingStrategySelection)
  ADD_EXECUTABLE(InpaintingStrategySelection InpaintingStrategySelection.cpp)
  TARGET_LINK_LIBRARIES(InpaintingStrategySelection ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS InpaintingStrategySelection RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

option(inpainting_InpaintingMinimizeIntroducedEnergy "Build a patch based image inpainting algorithm that uses an introduced energy sort on the top N SSD matches.")
if(inpainting_InpaintingMinimizeIntroducedEnergy)
  ADD_EXECUTABLE(InpaintingMinimizeIntroducedEnergy InpaintingMinimizeIntroducedEnergy.cpp)
  TARGET_LINK_LIBRARIES(InpaintingMinimizeIntroducedEnergy ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS InpaintingMinimizeIntroducedEnergy RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

option(inpainting_InpaintingTwoStep "Build a patch based image inpainting program that uses a two step (knn + best) search strategy.")
if(inpainting_InpaintingTwoStep)
  ADD_EXECUTABLE(InpaintingTwoStep InpaintingTwoStep.cpp)
  TARGET_LINK_LIBRARIES(InpaintingTwoStep ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS InpaintingTwoStep RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

option(inpainting_InpaintingBlurredSSD "Build a traditional patch comparison image inpainting using SSD on the blurred image.")
if(inpainting_InpaintingBlurredSSD)
  ADD_EXECUTABLE(InpaintingBlurredSSD InpaintingBlurredSSD.cpp)
  TARGET_LINK_LIBRARIES(InpaintingBlurredSSD ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS InpaintingBlurredSSD RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

option(inpainting_ClassicalImageInpainting "Build a traditional patch comparison image inpainting.")
if(inpainting_ClassicalImageInpainting)
  ADD_EXECUTABLE(ClassicalImageInpainting ClassicalImageInpainting.cpp)
  TARGET_LINK_LIBRARIES(ClassicalImageInpainting ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS ClassicalImageInpainting RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

option(inpainting_ClassicalImageInpaintingDebug "Build a traditional patch comparison image inpainting with lots of debugging output.")
if(inpainting_ClassicalImageInpaintingDebug)
  ADD_EXECUTABLE(ClassicalImageInpaintingDebug ClassicalImageInpaintingDebug.cpp)
  TARGET_LINK_LIBRARIES(ClassicalImageInpaintingDebug ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS ClassicalImageInpaintingDebug RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

option(inpainting_LocalOptimizationImageInpainting "Build a inpainting algorithm that uses locally (spatially and temporally) optimal choices instead of purely greedy choices.")
if(inpainting_LocalOptimizationImageInpainting)
  ADD_EXECUTABLE(LocalOptimizationImageInpainting LocalOptimizationImageInpainting.cpp)
  TARGET_LINK_LIBRARIES(LocalOptimizationImageInpainting ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS LocalOptimizationImageInpainting RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

option(inpainting_ClassicalImageInpaintingWithTextOutput "Build a traditional patch comparison image inpainting.")
if(inpainting_ClassicalImageInpaintingWithTextOutput)
  ADD_EXECUTABLE(ClassicalImageInpaintingWithTextOutput ClassicalImageInpaintingWithTextOutput.cpp)
  TARGET_LINK_LIBRARIES(ClassicalImageInpaintingWithTextOutput ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS ClassicalImageInpaintingWithTextOutput RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

option(inpainting_SmallPatchBigPatchInpainting "Build a two step (compare small patches, then compare big patches at the best of the small patches) image inpainting.")
if(inpainting_SmallPatchBigPatchInpainting)
  ADD_EXECUTABLE(SmallPatchBigPatchInpainting SmallPatchBigPatchInpainting.cpp)
  TARGET_LINK_LIBRARIES(SmallPatchBigPatchInpainting ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS SmallPatchBigPatchInpainting RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

option(inpainting_NarrowSearchByFeaturesInpainting "Build a two step (features, then patch comparison) image inpainting.")
if(inpainting_NarrowSearchByFeaturesInpainting)
  ADD_EXECUTABLE(NarrowSearchByFeaturesInpainting NarrowSearchByFeaturesInpainting.cpp PixelDescriptors/FeatureVectorPixelDescriptor.cpp)
  TARGET_LINK_LIBRARIES(NarrowSearchByFeaturesInpainting ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS NarrowSearchByFeaturesInpainting RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

option(inpainting_NarrowSearchByNormalsInpainting "Build a two step (normals, then patch comparison) image inpainting.")
if(inpainting_NarrowSearchByNormalsInpainting)
  ADD_EXECUTABLE(NarrowSearchByNormalsInpainting NarrowSearchByNormalsInpainting.cpp PixelDescriptors/FeatureVectorPixelDescriptor.cpp)
  TARGET_LINK_LIBRARIES(NarrowSearchByNormalsInpainting ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS NarrowSearchByNormalsInpainting RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

option(inpainting_InpaintingGMH "Build an automatic inpainting algorithm that sorts top patches by their Gradient Magnitude Histogram differences." OFF)
if(inpainting_InpaintingGMH)
  ADD_EXECUTABLE(InpaintingGMH InpaintingGMH.cpp)
  TARGET_LINK_LIBRARIES(InpaintingGMH ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS InpaintingGMH RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

################## Interactive versions ################
option(inpainting_InteractiveInpaintingWithVerification "An inpainting executable that runs acceptance tests and asks the user to choose a patch if they fail.")
if(inpainting_InteractiveInpaintingWithVerification)
    QT4_WRAP_UI(InteractiveInpaintingWithVerificationUISrcs
                Interactive/BasicViewerWidget.ui
                Interactive/TopPatchesDialog.ui
                Interactive/PriorityViewerWidget.ui
                Interactive/ManualPatchSelectionDialog.ui)
    ADD_EXECUTABLE(InteractiveInpaintingWithVerification InteractiveInpaintingWithVerification.cpp
                   Drivers/InteractiveInpaintingWithVerification.hpp
                   Interactive/TopPatchesDialog.h
                   NearestNeighbor/TopPatchListOrManual.hpp
                   Interactive/ManualPatchSelectionDialog.h
                   Interactive/ManualPatchSelectionDialogParent.h
                   Interactive/PriorityViewerWidget.h
                   ${InteractiveInpaintingWithVerificationUISrcs})
    TARGET_LINK_LIBRARIES(InteractiveInpaintingWithVerification ${PatchBasedInpainting_libraries} InpaintingGUI)
    INSTALL( TARGETS InteractiveInpaintingWithVerification RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

# InteractiveInpaintingGMH
option(inpainting_InteractiveInpaintingGMH "An inpainting executable that uses a sort by
                 Gradient Magnitude Histogram score and asks the user to choose a patch if
                 the score is above a threshold.")
if(inpainting_InteractiveInpaintingGMH)
    QT4_WRAP_UI(InteractiveInpaintingGMHUISrcs
                Interactive/BasicViewerWidget.ui
                Interactive/TopPatchesDialog.ui
                Interactive/ManualPatchSelectionDialog.ui
                Interactive/PatchVerificationDialog.ui
                Interactive/PriorityViewerWidget.ui)
    ADD_EXECUTABLE(InteractiveInpaintingGMH InteractiveInpaintingGMH.cpp
                   Drivers/InteractiveInpaintingGMH.hpp
                   Interactive/TopPatchesDialog.h
                   Interactive/TopPatchesDialogHandler.h
                   Interactive/BasicViewerWidget.h
                   Interactive/ManualPatchSelectionDialogParent.h
                   Interactive/MovablePatch.h
                   Interactive/PriorityViewerWidget.h
                   Interactive/PatchVerificationDialog.h
                   Interactive/PatchVerificationDialogHandler.h
                   NearestNeighbor/VerifyOrManual.hpp
                   NearestNeighbor/TopPatchListOrManual.hpp
                   Visitors/InformationVisitors/DisplayVisitor.hpp
                   ${InteractiveInpaintingGMHUISrcs})

    TARGET_LINK_LIBRARIES(InteractiveInpaintingGMH ${PatchBasedInpainting_libraries} InpaintingGUI)
    INSTALL( TARGETS InteractiveInpaintingGMH RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

# ClassicalImageInpaintingBasicViewer
option(inpainting_ClassicalImageInpaintingBasicViewer "An inpainting executable that uses a traditional greedy SSD method and shows you the result at every iteration.")
if(inpainting_ClassicalImageInpaintingBasicViewer)
    QT4_WRAP_UI(ClassicalImageInpaintingBasicViewerUISrcs
                Interactive/BasicViewerWidget.ui
                Interactive/TopPatchesDialog.ui
                Interactive/ManualPatchSelectionDialog.ui
                Interactive/PriorityViewerWidget.ui)
    ADD_EXECUTABLE(ClassicalImageInpaintingBasicViewer ClassicalImageInpaintingBasicViewer.cpp
                   Drivers/ClassicalImageInpaintingBasicViewer.hpp
                   Interactive/BasicViewerWidget.h
                   Visitors/InformationVisitors/DisplayVisitor.hpp
                   ${ClassicalImageInpaintingBasicViewerUISrcs})

    TARGET_LINK_LIBRARIES(ClassicalImageInpaintingBasicViewer ${PatchBasedInpainting_libraries} InpaintingGUI)
    INSTALL( TARGETS ClassicalImageInpaintingBasicViewer RUNTIME DESTINATION ${INSTALL_DIR} )
endif()

option(BuildInpaintingTests "Build inpainting tests.")
if(BuildInpaintingTests)
  ENABLE_TESTING()
#  add_subdirectory(Testing)
  add_subdirectory(Tests)
  #add_subdirectory(Priority/Tests)
  #add_subdirectory(ImageProcessing/Tests)
  #add_subdirectory(NearestNeighbor/Tests)
  #add_subdirectory(Helpers/Tests)
  #add_subdirectory(Utilities/Tests)
   add_subdirectory(Interactive/Tests)
endif()

############################
option(BuildInteractiveInpaintingTests "Build interactive tests.")
if(BuildInteractiveInpaintingTests)
  ENABLE_TESTING()
  add_subdirectory(Interactive/Tests)
ENDIF()

option(BuildDifferenceFunctionsTests "Build difference functions tests.")
if(BuildDifferenceFunctionsTests)
  ENABLE_TESTING()
  add_subdirectory(DifferenceFunctions/Tests)
ENDIF()

option(BuildInpaintingDemos "Build demos.")
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
