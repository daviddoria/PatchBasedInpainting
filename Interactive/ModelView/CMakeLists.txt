add_library(libModelView
ListModelDisplay.cpp
ListModelImageInput.cpp
ListModelSave.cpp
TableModelForwardLook.cpp
TableModelImageInput.cpp
TableModelTopPatches.cpp
)

#TARGET_LINK_LIBRARIES(PatchBasedInpaintingInteractive PatchBasedInpainting ${VTK_LIBRARIES} QVTK ${ITK_LIBRARIES} ${QT_LIBRARIES})

