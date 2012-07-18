  QT4_WRAP_UI(ClassicalImageInpaintingWithVisualOutputUISrcs Interactive/BasicViewerWidget.ui)
  QT4_WRAP_CPP(ClassicalImageInpaintingWithVisualOutputMOCSrcs Interactive/BasicViewerWidget.h Visitors/InformationVisitors/DisplayVisitor.hpp)

  ADD_EXECUTABLE(ClassicalImageInpaintingWithVisualOutput ClassicalImageInpaintingWithVisualOutput.cpp
    Interactive/Layer.cpp
    Interactive/InteractorStyleImageWithDrag.cpp Interactive/ImageCamera.cpp
    ${ClassicalImageInpaintingWithVisualOutputUISrcs} ${ClassicalImageInpaintingWithVisualOutputMOCSrcs})

  TARGET_LINK_LIBRARIES(ClassicalImageInpaintingWithVisualOutput PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES}
                       ${QT_LIBRARIES} Helpers QtHelpers VTKHelpers ITKHelpers ITKVTKHelpers)
  INSTALL( TARGETS ClassicalImageInpaintingWithVisualOutput RUNTIME DESTINATION ${INSTALL_DIR} )