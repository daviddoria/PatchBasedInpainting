
  ADD_EXECUTABLE(InpaintingWithLocalSearch InpaintingWithLocalSearch.cpp)

  TARGET_LINK_LIBRARIES(InpaintingWithLocalSearch ${PatchBasedInpainting_libraries})
  INSTALL( TARGETS InpaintingWithLocalSearch RUNTIME DESTINATION ${INSTALL_DIR} )
