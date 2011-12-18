# If we did not build the PatchBasedInpainting library with Qt, then find Qt now, it is definitely required for the interactive executable.
if(NOT QT4_FOUND)
  FIND_PACKAGE(Qt4 REQUIRED)
  INCLUDE(${QT_USE_FILE})
endif(NOT QT4_FOUND)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

QT4_WRAP_UI(UISrcs PatchBasedInpainting.ui FileSelector.ui FileSelectionWidget.ui)
QT4_WRAP_CPP(MOCSrcs PatchBasedInpaintingGUI.h InpaintingComputationObject.h FileSelector.h FileSelectionWidget.h PixmapDelegate.h)

ADD_EXECUTABLE(PatchBasedInpaintingInteractive
FileSelector.cpp
FileSelectionWidget.cpp
ForwardLookTableModel.cpp
HelpersDisplay.cpp
HelpersQt.cpp
InpaintingComputationObject.cpp
InpaintingIterationRecord.cpp
InteractorStyleImageWithDrag.cpp
Layer.cpp
ListModelDisplay.cpp
ListModelImageInput.cpp
ListModelSave.cpp
PatchBasedInpaintingGUI.cpp
PatchBasedInpaintingGUISlots.cpp
PatchBasedInpaintingInteractive.cpp #driver (contains main())
PixmapDelegate.cpp
TopPatchesTableModel.cpp
VectorLayer.cpp
${UISrcs} ${MOCSrcs})

TARGET_LINK_LIBRARIES(PatchBasedInpaintingInteractive PatchBasedInpainting ${VTK_LIBRARIES} QVTK ${ITK_LIBRARIES} ${QT_LIBRARIES})
INSTALL( TARGETS PatchBasedInpaintingInteractive RUNTIME DESTINATION ${INSTALL_DIR} )
