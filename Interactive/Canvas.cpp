#include "Canvas.h"

// VTK
#include <vtkImageSlice.h>
#include <vtkRenderer.h>

VTKCanvas::VTKCanvas(vtkRenderer* const renderer) : Renderer(renderer)
{
  this->ImageLayer.ImageSlice->SetPickable(false);
  this->MaskLayer.ImageSlice->SetPickable(false);
  this->UsedTargetPatchLayer.ImageSlice->SetPickable(false);
  this->UsedSourcePatchLayer.ImageSlice->SetPickable(false);
  this->AllSourcePatchOutlinesLayer.ImageSlice->SetPickable(false);
  this->AllForwardLookOutlinesLayer.ImageSlice->SetPickable(false);

  this->Renderer->AddViewProp(this->ImageLayer.ImageSlice);
  this->Renderer->AddViewProp(this->MaskLayer.ImageSlice);
  this->Renderer->AddViewProp(this->UsedTargetPatchLayer.ImageSlice);
  this->Renderer->AddViewProp(this->UsedSourcePatchLayer.ImageSlice);
  this->Renderer->AddViewProp(this->AllSourcePatchOutlinesLayer.ImageSlice);
  this->Renderer->AddViewProp(this->AllForwardLookOutlinesLayer.ImageSlice);
}