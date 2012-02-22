# include_directories(${ROOT_SOURCE_DIR})

SET(LearnGoodMatchParameters CACHE BOOL "Learn good match parameters.")
if(LearnGoodMatchParameters)
  ADD_EXECUTABLE(LearnGoodMatchParameters LearnGoodMatchParameters.cpp)
  TARGET_LINK_LIBRARIES(LearnGoodMatchParameters PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} ${PCL_LIBRARIES} libHelpers)
  INSTALL( TARGETS LearnGoodMatchParameters RUNTIME DESTINATION ${INSTALL_DIR} )
endif()
