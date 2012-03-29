#include "MovablePatch.h"

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
#include "ITKHelpers/ITKHelpers.h"
#include "VTKHelpers/VTKHelpers.h"
#include "ITKVTKHelpers/ITKVTKHelpers.h"
#include "QtHelpers/QtHelpers.h"
#include "InteractorStyleImageWithDrag.h"

// STL
#include <stdexcept>

MovablePatch::MovablePatch() : Radius(0), InteractorStyle(NULL), View(NULL), PatchScene(NULL)
{
  
}

//MovablePatch::MovablePatch(const unsigned int radius, vtkRenderer* const renderer,
MovablePatch::MovablePatch(const unsigned int radius, InteractorStyleImageWithDrag* const interactorStyle,
                           QGraphicsView* const view, const QColor color) :
                           Radius(radius), InteractorStyle(interactorStyle), View(view), Color(color),
                           PatchScene(new QGraphicsScene)
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
  QtHelpers::QColorToUCharColor(color, userPatchColor);
  VTKHelpers::BlankAndOutlineImage(this->PatchLayer.ImageData, userPatchColor);

  this->PatchLayer.ImageSlice->SetPosition(position);

  if(!InteractorStyle)
  {
    throw std::runtime_error("MovablePatch: Interactor is not valid!");
  }

  if(!this->InteractorStyle->GetCurrentRenderer())
  {
    throw std::runtime_error("MovablePatch: Renderer is not valid!");
  }
  this->InteractorStyle->GetCurrentRenderer()->AddViewProp(this->PatchLayer.ImageSlice);

  this->PatchLayer.ImageSlice->SetPosition(position);
  
  InteractorStyle->TrackballStyle->AddObserver(CustomTrackballStyle::PatchesMovedEvent, this, &MovablePatch::PatchMoved);

  //this->PatchScene->setBackgroundBrush(brush);
  //this->View->setScene(this->PatchScene.data());

}

void MovablePatch::SetVisibility(const bool visibility)
{
  this->PatchLayer.ImageSlice->SetVisibility(visibility);
}

bool MovablePatch::GetVisibility()
{
  return this->PatchLayer.ImageSlice->GetVisibility();
}

itk::ImageRegion<2> MovablePatch::GetRegion()
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

void MovablePatch::Display()
{
  QGraphicsPixmapItem* item = this->PatchScene->addPixmap(QPixmap::fromImage(this->PatchImage));
  this->View->fitInView(item);
}

void MovablePatch::PatchMoved()
{
  // Snap user patch to integer pixels
  double position[3];
  this->PatchLayer.ImageSlice->GetPosition(position);
  //std::cout << "position: " << position[0] << " " << position[1] << " " << position[2] << std::endl;

  position[0] = round(position[0]);
  position[1] = round(position[1]);

  //std::cout << "rounded position: " << position[0] << " " << position[1] << " " << position[2] << std::endl;
  
  this->PatchLayer.ImageSlice->SetPosition(position);

  // This will refresh the scene so that the old patch positions are erased (doesn't work...)
  // this->InteractorStyle->GetCurrentRenderer()->GetRenderWindow()->Render();

  emit signal_PatchMoved();
}

void MovablePatch::SetGraphicsView(QGraphicsView* const view)
{
  this->View = view;
}
