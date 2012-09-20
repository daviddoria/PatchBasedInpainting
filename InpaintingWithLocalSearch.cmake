  QT4_WRAP_UI(InpaintingWithLocalSearchUISrcs Interactive/BasicViewerWidget.ui
              Interactive/TopPatchesWidget.ui Interactive/TopPatchesDialog.ui
              Interactive/PriorityViewerWidget.ui
              Interactive/ManualPatchSelectionDialog.ui)
  QT4_WRAP_CPP(InpaintingWithLocalSearchMOCSrcs Interactive/BasicViewerWidget.h Visitors/InformationVisitors/DisplayVisitor.hpp
                       Visitors/NearestNeighborsDisplayVisitor.hpp
                       #NearestNeighbor/VisualSelectionBest.hpp
                       Interactive/TopPatchesWidget.h
                       Interactive/TopPatchesDialog.h
                       Interactive/Delegates/PixmapDelegate.h
                       Interactive/PriorityViewerWidget.h
                       Interactive/ManualPatchSelectionDialog.h)

  ADD_EXECUTABLE(InpaintingWithLocalSearch InpaintingWithLocalSearch.cpp
                Interactive/Layer.cpp
                Interactive/Delegates/PixmapDelegate.cpp
                Interactive/MovablePatch.cpp
                Interactive/InteractorStyleImageWithDrag.cpp Interactive/ImageCamera.cpp
                ${InpaintingWithLocalSearchUISrcs} ${InpaintingWithLocalSearchMOCSrcs})

  TARGET_LINK_LIBRARIES(InpaintingWithLocalSearch ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS InpaintingWithLocalSearch RUNTIME DESTINATION ${INSTALL_DIR} )
