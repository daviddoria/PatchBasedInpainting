SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# If we did not build the PatchBasedInpainting library with Qt, then find Qt now,
# it is definitely required for the interactive executable.
if(NOT QT4_FOUND)
  FIND_PACKAGE(Qt4 REQUIRED)
  INCLUDE(${QT_USE_FILE})
endif(NOT QT4_FOUND)

# Let Qt find it's MOCed headers in the build directory.
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(Interactive_includes ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

add_custom_target(InteractiveSources SOURCES
BasicViewerWidget.h
BasicViewerWidget.hpp
ColorPalette.h
DisplayPriority.h
DisplayState.h
DisplayStyle.h
FileSelector.h
FullInspectionWidget.h
ImageCamera.h
ImageInput.h
InpaintingComputationObject.h
InpaintingDisplaySettings.h
InpaintingIterationRecord.h
InpaintingIterationRecordViewer.h
InteractorStyleImageWithDrag.h
Layer.h
ManualPatchSelectionDialog.h
ManualPatchSelectionDialog.hpp
ManualPatchSelectionDialogParent.h
MovablePatch.h
NamedITKImageCollection.h
NamedITKImage.h
NamedVTKImage.h
PatchHighlighter.h
PriorityViewerWidget.h
PriorityViewerWidget.hpp
SourceTargetPair.h
TopPatchesDialog.h
TopPatchesDialog.hpp
TopPatchesWidget.h
TopPatchesWidget.hpp
VectorLayer.h
)

add_library(InpaintingGUI Layer.cpp InteractorStyleImageWithDrag.cpp
            ImageCamera.cpp MovablePatch.cpp PatchHighlighter.cpp
            Delegates/PixmapDelegate.cpp)
target_link_libraries(InpaintingGUI ${PatchBasedInpainting_libraries})


#add_subdirectory(Widgets)
#add_subdirectory(ModelView)

#QT4_WRAP_UI(UISrcs PatchBasedInpainting.ui FileSelector.ui)

#add_executable(PatchBasedInpaintingInteractive
#ColorPalette.cpp
#DisplayState.cpp
#FileSelector.cpp
#ImageInput.cpp
#InpaintingComputationObject.cpp
#InpaintingIterationRecord.cpp
#InpaintingIterationRecordViewer.cpp
#PatchBasedInpaintingGUI.cpp
#PatchBasedInpaintingGUISlots.cpp
#PatchBasedInpaintingInteractive.cpp #driver (contains main())
#InpaintingDisplaySettings.cpp
#VectorLayer.cpp
#${UISrcs})

#target_link_libraries(PatchBasedInpaintingInteractive
#${PatchBasedInpainting_libraries}
#InpaintingGUI
#libWidgets libModelView
#)
#INSTALL( TARGETS PatchBasedInpaintingInteractive RUNTIME DESTINATION ${INSTALL_DIR} )


# Inpainting viewer
# if(BuildViewer)
#   message("Interactive/Building the viewer.")
#   QT4_WRAP_UI(UISrcs FullInspectionWidget.ui)
# 
#   add_executable(PatchBasedInpaintingViewer
#   ColorPalette.cpp
#   FullInspectionWidget.cpp
#   PatchBasedInpaintingViewer.cpp #driver (contains main())
#   InpaintingDisplaySettings.cpp
#   ${UISrcs})
# 
#   target_link_libraries(PatchBasedInpaintingViewer ${PatchBasedInpainting_libraries} InpaintingGUI)
#   INSTALL( TARGETS PatchBasedInpaintingViewer RUNTIME DESTINATION ${INSTALL_DIR} )
# endif(BuildViewer)

option(PatchBasedInpainting_BuildInteractiveTests "Build Interactive tests?" OFF)
if(PatchBasedInpainting_BuildInteractiveTests)
 add_subdirectory(Tests)
endif(PatchBasedInpainting_BuildInteractiveTests)
