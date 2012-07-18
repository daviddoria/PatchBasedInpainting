 add_subdirectory(Interactive)
  QT4_WRAP_UI(InpaintingWithManualSelectionUISrcs Interactive/BasicViewerWidget.ui Interactive/TopPatchesWidget.ui Interactive/TopPatchesDialog.ui)
  QT4_WRAP_CPP(InpaintingWithManualSelectionMOCSrcs Interactive/BasicViewerWidget.h Visitors/InformationVisitors/DisplayVisitor.hpp
                       Visitors/NearestNeighborsDisplayVisitor.hpp
                       #NearestNeighbor/VisualSelectionBest.hpp
                       Interactive/TopPatchesWidget.h
                       Interactive/TopPatchesDialog.h
                       Interactive/Delegates/PixmapDelegate.h)

  ADD_EXECUTABLE(InpaintingWithManualSelection InpaintingWithManualSelection.cpp
                Interactive/Layer.cpp
                Interactive/Delegates/PixmapDelegate.cpp
                Interactive/InteractorStyleImageWithDrag.cpp Interactive/ImageCamera.cpp
                ${InpaintingWithManualSelectionUISrcs} ${InpaintingWithManualSelectionMOCSrcs})

  TARGET_LINK_LIBRARIES(InpaintingWithManualSelection PatchBasedInpainting
                        ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} Helpers)
  INSTALL( TARGETS InpaintingWithManualSelection RUNTIME DESTINATION ${INSTALL_DIR} )