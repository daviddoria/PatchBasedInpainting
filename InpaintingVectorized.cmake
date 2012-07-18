add_subdirectory(Interactive)

  QT4_WRAP_UI(InpaintingVectorizedUISrcs Interactive/BasicViewerWidget.ui
              Interactive/TopPatchesWidget.ui Interactive/TopPatchesDialog.ui
              Interactive/PriorityViewerWidget.ui
              Interactive/ManualPatchSelectionDialog.ui)
  QT4_WRAP_CPP(InpaintingVectorizedMOCSrcs Interactive/BasicViewerWidget.h Visitors/InformationVisitors/DisplayVisitor.hpp
                       Visitors/NearestNeighborsDisplayVisitor.hpp
                       #NearestNeighbor/VisualSelectionBest.hpp
                       Interactive/TopPatchesWidget.h
                       Interactive/TopPatchesDialog.h
                       Interactive/Delegates/PixmapDelegate.h
                       Interactive/PriorityViewerWidget.h
                       Interactive/ManualPatchSelectionDialog.h)

  ADD_EXECUTABLE(InpaintingVectorized InpaintingVectorized.cpp
                Interactive/Layer.cpp
                Interactive/Delegates/PixmapDelegate.cpp
                Interactive/MovablePatch.cpp
                Interactive/InteractorStyleImageWithDrag.cpp Interactive/ImageCamera.cpp
                ${InpaintingVectorizedUISrcs} ${InpaintingVectorizedMOCSrcs})

  TARGET_LINK_LIBRARIES(InpaintingVectorized PatchBasedInpainting
                        ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} Helpers)
  INSTALL( TARGETS InpaintingVectorized RUNTIME DESTINATION ${INSTALL_DIR} )