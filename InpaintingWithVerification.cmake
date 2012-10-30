
ADD_EXECUTABLE(InpaintingWithVerification InpaintingWithVerification.cpp
               Interactive/Layer.cpp
               Interactive/Delegates/PixmapDelegate.cpp
               Interactive/MovablePatch.cpp
               Interactive/PatchHighlighter.cpp
               Interactive/InteractorStyleImageWithDrag.cpp
               Interactive/ImageCamera.cpp)

TARGET_LINK_LIBRARIES(InpaintingWithVerification ${PatchBasedInpainting_libraries})
INSTALL( TARGETS InpaintingWithVerification RUNTIME DESTINATION ${INSTALL_DIR} )
