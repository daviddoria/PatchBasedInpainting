  QT4_WRAP_UI(InpaintingCIELabUISrcs Interactive/BasicViewerWidget.ui
              Interactive/TopPatchesWidget.ui Interactive/TopPatchesDialog.ui
              Interactive/PriorityViewerWidget.ui
              Interactive/ManualPatchSelectionDialog.ui)
  QT4_WRAP_CPP(InpaintingCIELabMOCSrcs Interactive/BasicViewerWidget.h Visitors/InformationVisitors/DisplayVisitor.hpp
                       Visitors/NearestNeighborsDisplayVisitor.hpp
                       Interactive/TopPatchesWidget.h
                       Interactive/TopPatchesDialog.h
                       Interactive/Delegates/PixmapDelegate.h
                       Interactive/PriorityViewerWidget.h
                       Interactive/ManualPatchSelectionDialog.h
                       Interactive/MovablePatch.h)

  ADD_EXECUTABLE(InpaintingCIELab InpaintingCIELab.cpp
                Interactive/Layer.cpp
                Interactive/Delegates/PixmapDelegate.cpp
                Interactive/MovablePatch.cpp
                Interactive/InteractorStyleImageWithDrag.cpp Interactive/ImageCamera.cpp
                ${InpaintingCIELabUISrcs} ${InpaintingCIELabMOCSrcs})

  TARGET_LINK_LIBRARIES(InpaintingCIELab ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS InpaintingCIELab RUNTIME DESTINATION ${INSTALL_DIR} )
