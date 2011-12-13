add_executable(DemoBoundaryNormals DemoBoundaryNormals.cpp)
target_link_libraries(DemoBoundaryNormals PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

add_executable(DemoBoundaryNormals2 DemoBoundaryNormals2.cpp)
target_link_libraries(DemoBoundaryNormals2 PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES})

add_executable(DemoClusterColors DemoClusterColors.cpp)
target_link_libraries(DemoClusterColors PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

add_executable(DemoClusterColorsClusterCenters DemoClusterColorsClusterCenters.cpp)
target_link_libraries(DemoClusterColorsClusterCenters PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES})

add_executable(DemoClusterColorsLabelImage DemoClusterColorsLabelImage.cpp)
target_link_libraries(DemoClusterColorsLabelImage PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES})

add_executable(DemoColorImageByScore DemoColorImageByScore.cpp)
target_link_libraries(DemoColorImageByScore PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES})

add_executable(DemoCreateImage DemoCreateImage.cpp)
target_link_libraries(DemoCreateImage PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

add_executable(DemoCreateMask DemoCreateMask.cpp)
target_link_libraries(DemoCreateMask PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

add_executable(DemoDerivatives DemoDerivatives.cpp)
target_link_libraries(DemoDerivatives PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

add_executable(DemoErroneousGradient DemoErroneousGradient.cpp)
target_link_libraries(DemoErroneousGradient PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

add_executable(DemoFindPixelAcrossHole DemoFindPixelAcrossHole.cpp)
target_link_libraries(DemoFindPixelAcrossHole PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

add_executable(DemoFollowIsophotesAcrossBoundary DemoFollowIsophotesAcrossBoundary.cpp)
target_link_libraries(DemoFollowIsophotesAcrossBoundary PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES})

add_executable(DemoGradient DemoGradient.cpp)
target_link_libraries(DemoGradient PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES})

add_executable(DemoInnerOuterBoundary DemoInnerOuterBoundary.cpp)
target_link_libraries(DemoInnerOuterBoundary PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

add_executable(DemoIsophotes DemoIsophotes.cpp)
target_link_libraries(DemoIsophotes PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES})

add_executable(DemoPatchDifference DemoPatchDifference.cpp)
target_link_libraries(DemoPatchDifference PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

add_executable(DemoPatchImageDifference DemoPatchImageDifference.cpp)
target_link_libraries(DemoPatchImageDifference PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

add_executable(DemoPatchSearchFunction DemoPatchSearchFunction.cpp)
target_link_libraries(DemoPatchSearchFunction PatchBasedInpainting ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${QT_LIBRARIES})

add_executable(DemoSelfPatchCompare DemoSelfPatchCompare.cpp)
target_link_libraries(DemoSelfPatchCompare PatchBasedInpainting ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${QT_LIBRARIES})
