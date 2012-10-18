include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_executable(DemoBoundaryNormals DemoBoundaryNormals.cpp)
target_link_libraries(DemoBoundaryNormals PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

add_executable(DemoBoundaryNormals2 DemoBoundaryNormals2.cpp)
target_link_libraries(DemoBoundaryNormals2 PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} Mask VTKHelpers)

# Need to build Clustering
#add_executable(DemoClusterColors DemoClusterColors.cpp)
#target_link_libraries(DemoClusterColors PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

# Need to build Clustering
#add_executable(DemoClusterColorsClusterCenters DemoClusterColorsClusterCenters.cpp)
#target_link_libraries(DemoClusterColorsClusterCenters PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES})

# Need to build Clustering
#add_executable(DemoClusterColorsLabelImage DemoClusterColorsLabelImage.cpp)
#target_link_libraries(DemoClusterColorsLabelImage PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES})

# Need to update to new API (SelfPatchCompare no longer exists)
#add_executable(DemoColorImageByScore DemoColorImageByScore.cpp)
#target_link_libraries(DemoColorImageByScore PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES})

add_executable(DemoDerivatives DemoDerivatives.cpp)
target_link_libraries(DemoDerivatives PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

add_executable(DemoErroneousGradient DemoErroneousGradient.cpp)
target_link_libraries(DemoErroneousGradient PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} Mask)

add_executable(DemoFindPixelAcrossHole DemoFindPixelAcrossHole.cpp)
target_link_libraries(DemoFindPixelAcrossHole PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} Mask)

add_executable(DemoFollowIsophotesAcrossBoundary DemoFollowIsophotesAcrossBoundary.cpp)
target_link_libraries(DemoFollowIsophotesAcrossBoundary PatchBasedInpainting
${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} Mask)

add_executable(DemoGradient DemoGradient.cpp)
target_link_libraries(DemoGradient PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} Mask VTKHelpers)

add_executable(DemoInnerOuterBoundary DemoInnerOuterBoundary.cpp)
target_link_libraries(DemoInnerOuterBoundary PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

add_executable(DemoIsophotes DemoIsophotes.cpp)
target_link_libraries(DemoIsophotes PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} Mask VTKHelpers)

# PatchDifference function no longer exists
#add_executable(DemoPatchDifference DemoPatchDifference.cpp)
#target_link_libraries(DemoPatchDifference PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} Mask)

add_executable(DemoPatchImageDifference DemoPatchImageDifference.cpp)
target_link_libraries(DemoPatchImageDifference PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES} ITKHelpers)

add_executable(DemoPatchSearchFunction DemoPatchSearchFunction.cpp)
target_link_libraries(DemoPatchSearchFunction PatchBasedInpainting ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${QT_LIBRARIES} Mask)

add_executable(DemoSelfPatchCompare DemoSelfPatchCompare.cpp)
target_link_libraries(DemoSelfPatchCompare PatchBasedInpainting ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${QT_LIBRARIES} Mask)

add_executable(DemoFillOrder DemoFillOrder.cpp)
target_link_libraries(DemoFillOrder PatchBasedInpainting ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${QT_LIBRARIES} Mask)

###################################
# add_executable(DemoBadPatchComparison DemoBadPatchComparison.cpp)
# target_link_libraries(DemoBadPatchComparison PatchBasedInpainting ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${QT_LIBRARIES} libHelpers)
# 
# QT4_WRAP_UI(DemoBadPatchRealImageUISrcs ${ROOT_SOURCE_DIR}/Interactive/TopPatchesDialog.ui)
# QT4_WRAP_CPP(DemoBadPatchRealImageMOCSrcs ${ROOT_SOURCE_DIR}/Interactive/TopPatchesDialog.h
#  ${ROOT_SOURCE_DIR}/Interactive/Delegates/PixmapDelegate.h)
# add_executable(DemoBadPatchRealImage DemoBadPatchRealImage.cpp
# ${ROOT_SOURCE_DIR}/Interactive/Delegates/PixmapDelegate.cpp
# ${DemoBadPatchRealImageUISrcs} ${DemoBadPatchRealImageMOCSrcs})
# target_link_libraries(DemoBadPatchRealImage PatchBasedInpainting ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${QT_LIBRARIES} libHelpers)
###################################

# New api - MaskedLaplacian() is no longer a function
#add_executable(DemoMaskedLaplacian DemoMaskedLaplacian.cpp)
#target_link_libraries(DemoMaskedLaplacian PatchBasedInpainting ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${QT_LIBRARIES} libHelpers)

# add_executable(DemoMaskedLaplacianSynthetic DemoMaskedLaplacianSynthetic.cpp)
# target_link_libraries(DemoMaskedLaplacianSynthetic PatchBasedInpainting ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${QT_LIBRARIES} libHelpers)
