#include "PatchHighlighter.h"

// Qt
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>

// VTK
#include <vtkImageSlice.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/VTKHelpers.h"
#include "Helpers/ITKVTKHelpers.h"
#include "HelpersQt.h"
#include "InteractorStyleImageWithDrag.h"

PatchHighlighter::PatchHighlighter() : Radius(0), Renderer(NULL)
{

}

//MovablePatch::MovablePatch(const unsigned int radius, vtkRenderer* const renderer,
PatchHighlighter::PatchHighlighter(const unsigned int radius, vtkRenderer* const renderer,
                                   const QColor color) :
                                   Radius(radius), Color(color), Renderer(renderer)
{
  this->PatchLayer.ImageSlice->SetPickable(true);
  this->PatchLayer.ImageSlice->SetDragable(true);

  double position[3];
  this->PatchLayer.ImageSlice->GetPosition(position);

  position[0] = round(position[0]);
  position[1] = round(position[1]);

  this->PatchLayer.ImageSlice->SetPosition(position);

  // Create the patch (transparent center and solid outline)
  ITKVTKHelpers::CreateTransparentVTKImage(ITKHelpers::SizeFromRadius(radius), this->PatchLayer.ImageData);
  unsigned char userPatchColor[3];
  HelpersQt::QColorToUCharColor(color, userPatchColor);
  VTKHelpers::BlankAndOutlineImage(this->PatchLayer.ImageData, userPatchColor);

  this->PatchLayer.ImageSlice->SetPosition(position);

  Renderer->AddViewProp(this->PatchLayer.ImageSlice);

  this->PatchLayer.ImageSlice->SetPosition(position);

}

void PatchHighlighter::SetVisibility(const bool visibility)
{
  this->PatchLayer.ImageSlice->SetVisibility(visibility);
}

bool PatchHighlighter::GetVisibility()
{
  return this->PatchLayer.ImageSlice->GetVisibility();
}

void PatchHighlighter::SetRegion(const itk::ImageRegion<2>& region)
{
  // Snap user patch to integer pixels
  double position[3];
  position[0] = region.GetIndex()[0];
  position[1] = region.GetIndex()[1];
  position[2] = 0;

  this->PatchLayer.ImageSlice->SetPosition(position);
}

itk::ImageRegion<2> PatchHighlighter::GetRegion()
{
  double position[3];
  this->PatchLayer.ImageSlice->GetPosition(position);

  itk::Index<2> patchCorner;
  patchCorner[0] = position[0];
  patchCorner[1] = position[1];

  itk::Size<2> patchSize = ITKHelpers::SizeFromRadius(this->Radius);

  itk::ImageRegion<2> region(patchCorner, patchSize);

  return region;
}
