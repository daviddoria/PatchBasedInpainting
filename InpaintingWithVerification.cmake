add_subdirectory(Interactive)
message("Building InpaintingWithVerification")
  QT4_WRAP_UI(InpaintingWithVerificationUISrcs Interactive/BasicViewerWidget.ui
              Interactive/TopPatchesWidget.ui Interactive/TopPatchesDialog.ui
              Interactive/PriorityViewerWidget.ui
              Interactive/ManualPatchSelectionDialog.ui)
  QT4_WRAP_CPP(InpaintingWithVerificationMOCSrcs Interactive/BasicViewerWidget.h Visitors/InformationVisitors/DisplayVisitor.hpp
                       Visitors/NearestNeighborsDisplayVisitor.hpp
                       Interactive/TopPatchesWidget.h
                       Interactive/TopPatchesDialog.h
                       Interactive/Delegates/PixmapDelegate.h
                       Interactive/PriorityViewerWidget.h
                       Interactive/ManualPatchSelectionDialog.h
                       Interactive/MovablePatch.h)

  ADD_EXECUTABLE(InpaintingWithVerification InpaintingWithVerification.cpp
                Interactive/Layer.cpp
                Interactive/Delegates/PixmapDelegate.cpp
                Interactive/MovablePatch.cpp
                Interactive/PatchHighlighter.cpp
                Interactive/InteractorStyleImageWithDrag.cpp
                Interactive/ImageCamera.cpp
                ${InpaintingWithVerificationUISrcs} ${InpaintingWithVerificationMOCSrcs})

  TARGET_LINK_LIBRARIES(InpaintingWithVerification ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS InpaintingWithVerification RUNTIME DESTINATION ${INSTALL_DIR} )
