QT4_WRAP_UI(InpaintingWithTopPatchDisplayUISrcs Interactive/BasicViewerWidget.ui Interactive/TopPatchesWidget.ui)
QT4_WRAP_CPP(InpaintingWithTopPatchDisplayMOCSrcs Interactive/BasicViewerWidget.h Visitors/InformationVisitors/DisplayVisitor.hpp
                      Visitors/NearestNeighborsDisplayVisitor.hpp
                      Interactive/TopPatchesWidget.h Interactive/Delegates/PixmapDelegate.h
OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED # Fixes Parse error at "BOOST_JOIN" error (https://bugreports.qt-project.org/browse/QTBUG-22829)
)

ADD_EXECUTABLE(InpaintingWithTopPatchDisplay InpaintingWithTopPatchDisplay.cpp
              Interactive/Layer.cpp
              Interactive/PatchHighlighter.cpp
              Interactive/Delegates/PixmapDelegate.cpp
              Interactive/InteractorStyleImageWithDrag.cpp Interactive/ImageCamera.cpp
              ${InpaintingWithTopPatchDisplayUISrcs} ${InpaintingWithTopPatchDisplayMOCSrcs})

TARGET_LINK_LIBRARIES(InpaintingWithTopPatchDisplay ${PatchBasedInpainting_libraries} QtHelpers)
INSTALL( TARGETS InpaintingWithTopPatchDisplay RUNTIME DESTINATION ${INSTALL_DIR} )
