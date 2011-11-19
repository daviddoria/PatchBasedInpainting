# If we did not build the PatchBasedInpainting library with Qt, then find Qt now, it is definitely required for the interactive executable.
if(NOT QT4_FOUND)
  FIND_PACKAGE(Qt4 REQUIRED)
  INCLUDE(${QT_USE_FILE})
endif(NOT QT4_FOUND)

QT4_WRAP_UI(UISrcs PatchBasedInpainting.ui FileSelector.ui FileSelectionWidget.ui)
QT4_WRAP_CPP(MOCSrcs PatchBasedInpaintingGUI.h ComputationThread.h FileSelector.h FileSelectionWidget.h PixmapDelegate.h)

ADD_EXECUTABLE(PatchBasedInpaintingInteractive
FileSelector.cpp
FileSelectionWidget.cpp
PatchBasedInpaintingGUI.cpp
ForwardLookTableModel.cpp
PatchBasedInpaintingGUISlots.cpp
HelpersDisplay.cpp
HelpersQt.cpp
InpaintingVisualizationStack.cpp
PatchBasedInpaintingInteractive.cpp #driver (contains main())
InteractorStyleImageNoLevel.cpp
InteractorStyleImageWithDrag.cpp
Layer.cpp
PixmapDelegate.cpp
ComputationThread.cpp
TopPatchesTableModel.cpp
VectorLayer.cpp
${UISrcs} ${MOCSrcs})
TARGET_LINK_LIBRARIES(PatchBasedInpaintingInteractive PatchBasedInpainting ${VTK_LIBRARIES} QVTK ${ITK_LIBRARIES} ${QT_LIBRARIES})
INSTALL( TARGETS PatchBasedInpaintingInteractive RUNTIME DESTINATION ${INSTALL_DIR} )
