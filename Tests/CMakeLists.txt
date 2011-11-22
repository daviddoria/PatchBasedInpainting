
ADD_EXECUTABLE(TestDerivatives Tests/TestDerivatives.cpp)
TARGET_LINK_LIBRARIES(TestDerivatives PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

ADD_EXECUTABLE(TestFindPixelAcrossHole Tests/TestFindPixelAcrossHole.cpp)
TARGET_LINK_LIBRARIES(TestFindPixelAcrossHole PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})


# ADD_EXECUTABLE(LearnLambda LearnLambda.cpp)
# TARGET_LINK_LIBRARIES(LearnLambda PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

# ADD_EXECUTABLE(TestColorImageByScore Tests/TestColorImageByScore.cpp)
# TARGET_LINK_LIBRARIES(TestColorImageByScore PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})
#
# ADD_EXECUTABLE(TestPatchSearchFunction Tests/TestPatchSearchFunction.cpp)
# TARGET_LINK_LIBRARIES(TestPatchSearchFunction PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

# ADD_EXECUTABLE(ExamplePatchDifference Examples/ExamplePatchDifference.cpp)
# TARGET_LINK_LIBRARIES(ExamplePatchDifference PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

#--------------
# --- Tests ---
#--------------
# ADD_EXECUTABLE(TestIsophotes Tests/TestIsophotes.cpp)
# TARGET_LINK_LIBRARIES(TestIsophotes PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})
#
# ADD_EXECUTABLE(TestGradient Tests/TestGradient.cpp)
# TARGET_LINK_LIBRARIES(TestGradient PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

# ADD_EXECUTABLE(TestBoundaryNormals Tests/TestBoundaryNormals.cpp)
# TARGET_LINK_LIBRARIES(TestBoundaryNormals PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

# ADD_EXECUTABLE(TestFollowIsophotesAcrossBoundary Tests/TestFollowIsophotesAcrossBoundary.cpp)
# TARGET_LINK_LIBRARIES(TestFollowIsophotesAcrossBoundary PatchBasedInpainting ${VTK_LIBRARIES} ${ITK_LIBRARIES})

# ADD_EXECUTABLE(TestPatchDifference TestPatchDifference.cpp Helpers.cpp)
# TARGET_LINK_LIBRARIES(TestPatchDifference ${VTK_LIBRARIES} ${ITK_LIBRARIES})

# ADD_EXECUTABLE(DemoErroneousGradient DemoErroneousGradient.cpp Helpers.cpp )
# TARGET_LINK_LIBRARIES(DemoErroneousGradient ${ITK_LIBRARIES})

# ADD_EXECUTABLE(DemoBoundaryNormals DemoBoundaryNormals.cpp Helpers.cpp )
# TARGET_LINK_LIBRARIES(DemoBoundaryNormals ${ITK_LIBRARIES})

# ADD_EXECUTABLE(DemoInnerOuterBoundary DemoInnerOuterBoundary.cpp Helpers.cpp )
# TARGET_LINK_LIBRARIES(DemoInnerOuterBoundary ${ITK_LIBRARIES})

# ADD_EXECUTABLE(PatchImageDifferenceExample PatchImageDifferenceExample.cpp SelfPatchMatch.h Helpers.h  Helpers.cpp)
# TARGET_LINK_LIBRARIES(PatchImageDifferenceExample ${VTK_LIBRARIES} ${ITK_LIBRARIES})
#
# ADD_EXECUTABLE(SelfPatchMatchExample SelfPatchMatchExample.cpp SelfPatchMatch.h Helpers.h  Helpers.cpp)
# TARGET_LINK_LIBRARIES(SelfPatchMatchExample ${VTK_LIBRARIES} ${ITK_LIBRARIES})

# ADD_EXECUTABLE(CriminisiInpaintingRGBDI CriminisiInpaintingExampleRGBDI.cxx PatchBasedInpainting.cpp CriminisiInpaintingDebugging.cpp CustomStyle.cpp SelfPatchMatch.h Helpers.h Helpers.cpp RotateVectors.h PatchBasedInpainting.h)
# TARGET_LINK_LIBRARIES(CriminisiInpaintingRGBDI ${VTK_LIBRARIES} ${ITK_LIBRARIES})

##################################################################
# Testing

# SET( BUILD_TESTING OFF CACHE BOOL "Perform some tests." )
# IF( ITKTOOLS_BUILD_TESTING )
#   ENABLE_TESTING()
#   set(BaselineDir ${CriminisiInpainting_SOURCE_DIR}/../Testing/Baselines/)
#   set(DataDir ${CriminisiInpainting_SOURCE_DIR}/../Testing/Data/)
#   set(ExeDir ${CriminisiInpainting_BINARY_DIR})
#   set(OutDir ${CriminisiInpainting_BINARY_DIR}/Testing/)
# ENDIF()
#
# ADD_EXECUTABLE(TestSelfPatchCompare TestSelfPatchCompare.cpp SelfPatchCompare.cpp)
# TARGET_LINK_LIBRARIES(TestSelfPatchCompare ${ITK_LIBRARIES})
