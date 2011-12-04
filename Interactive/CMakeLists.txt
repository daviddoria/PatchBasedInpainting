# If we did not build the PatchBasedInpainting library with Qt, then find Qt now, it is definitely required for the interactive executable.
if(NOT QT4_FOUND)
  FIND_PACKAGE(Qt4 REQUIRED)
  INCLUDE(${QT_USE_FILE})
endif(NOT QT4_FOUND)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

QT4_WRAP_UI(UISrcs PatchBasedInpainting.ui FileSelector.ui FileSelectionWidget.ui PriorityViewer.ui)
QT4_WRAP_CPP(MOCSrcs PatchBasedInpaintingGUI.h ComputationThread.h FileSelector.h FileSelectionWidget.h PixmapDelegate.h PriorityViewer.h)

ADD_EXECUTABLE(PatchBasedInpaintingInteractive
ComputationThread.cpp
FileSelector.cpp
FileSelectionWidget.cpp
ForwardLookTableModel.cpp
HelpersDisplay.cpp
HelpersQt.cpp
InpaintingIterationRecord.cpp
InteractorStyleImageWithDrag.cpp
Layer.cpp
PatchBasedInpaintingGUI.cpp
PatchBasedInpaintingGUISlots.cpp
PatchBasedInpaintingInteractive.cpp #driver (contains main())
PixmapDelegate.cpp
PriorityViewer.cpp
TopPatchesTableModel.cpp
VectorLayer.cpp
${UISrcs} ${MOCSrcs})

TARGET_LINK_LIBRARIES(PatchBasedInpaintingInteractive PatchBasedInpainting ${VTK_LIBRARIES} QVTK ${ITK_LIBRARIES} ${QT_LIBRARIES})
INSTALL( TARGETS PatchBasedInpaintingInteractive RUNTIME DESTINATION ${INSTALL_DIR} )
