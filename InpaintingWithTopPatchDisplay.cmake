QT4_WRAP_UI(InpaintingWithTopPatchDisplayUISrcs Interactive/BasicViewerWidget.ui Interactive/TopPatchesWidget.ui)
QT4_WRAP_CPP(InpaintingWithTopPatchDisplayMOCSrcs Interactive/BasicViewerWidget.h Visitors/InformationVisitors/DisplayVisitor.hpp
                      Visitors/NearestNeighborsDisplayVisitor.hpp
                      Interactive/TopPatchesWidget.h Interactive/Delegates/PixmapDelegate.h
)

ADD_EXECUTABLE(InpaintingWithTopPatchDisplay InpaintingWithTopPatchDisplay.cpp
              Interactive/Layer.cpp
              Interactive/PatchHighlighter.cpp
              Interactive/Delegates/PixmapDelegate.cpp
              Interactive/InteractorStyleImageWithDrag.cpp Interactive/ImageCamera.cpp
              ${InpaintingWithTopPatchDisplayUISrcs} ${InpaintingWithTopPatchDisplayMOCSrcs})

TARGET_LINK_LIBRARIES(InpaintingWithTopPatchDisplay ${PatchBasedInpainting_libraries})
INSTALL( TARGETS InpaintingWithTopPatchDisplay RUNTIME DESTINATION ${INSTALL_DIR} )
