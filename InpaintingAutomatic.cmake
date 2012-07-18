  QT4_WRAP_UI(InpaintingAutomatic_UISrcs Interactive/BasicViewerWidget.ui
              Interactive/TopPatchesWidget.ui Interactive/TopPatchesDialog.ui
              Interactive/PriorityViewerWidget.ui
              Interactive/ManualPatchSelectionDialog.ui)
  QT4_WRAP_CPP(InpaintingAutomatic_MOCSrcs Interactive/BasicViewerWidget.h Visitors/InformationVisitors/DisplayVisitor.hpp
                       Visitors/NearestNeighborsDisplayVisitor.hpp
                       #NearestNeighbor/VisualSelectionBest.hpp
                       Interactive/TopPatchesWidget.h
                       Interactive/TopPatchesDialog.h
                       Interactive/Delegates/PixmapDelegate.h
                       Interactive/PriorityViewerWidget.h
                       Interactive/ManualPatchSelectionDialog.h
                       Interactive/MovablePatch.h)

  ADD_EXECUTABLE(InpaintingAutomatic InpaintingAutomatic.cpp
                Interactive/Layer.cpp
                Interactive/Delegates/PixmapDelegate.cpp
                Interactive/MovablePatch.cpp
                Interactive/InteractorStyleImageWithDrag.cpp Interactive/ImageCamera.cpp
                ${InpaintingAutomatic_UISrcs} ${InpaintingAutomatic_MOCSrcs})

  TARGET_LINK_LIBRARIES(InpaintingAutomatic PatchBasedInpainting
                        ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} Helpers QtHelpers VTKHelpers ITKHelpers ITKVTKHelpers Mask)
  INSTALL( TARGETS InpaintingAutomatic RUNTIME DESTINATION ${INSTALL_DIR} )