  QT4_WRAP_UI(PrecomputedInpaintingUISrcs )
  QT4_WRAP_CPP(PrecomputedInpaintingMOCSrcs )

  ADD_EXECUTABLE(PrecomputedInpainting PrecomputedInpainting.cpp
                ${PrecomputedInpaintingUISrcs} ${PrecomputedInpaintingMOCSrcs})

  TARGET_LINK_LIBRARIES(PrecomputedInpainting ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS PrecomputedInpainting RUNTIME DESTINATION ${INSTALL_DIR} )
