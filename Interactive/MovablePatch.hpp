/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#ifndef MovablePatch_HPP
#define MovablePatch_HPP

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

// Submodules
#include <Helpers/Helpers.h>
#include <ITKHelpers/ITKHelpers.h>
#include <VTKHelpers/VTKHelpers.h>
#include <ITKVTKHelpers/ITKVTKHelpers.h>
#include <QtHelpers/QtHelpers.h>

// Custom
#include "InteractorStyleImageWithDrag.h"

// STL
#include <stdexcept>

template <typename TInteractorStyle>
MovablePatch<TInteractorStyle>::MovablePatch
(const unsigned int radius,
 TInteractorStyle* const interactorStyle,
 QGraphicsView* const view, const QColor color) :
 Radius(radius), InteractorStyle(interactorStyle), View(view), Color(color)
{
  if(!this->PatchScene)
  {
    this->PatchScene = new QGraphicsScene;
  }

  this->PatchLayer.ImageSlice->SetPickable(true);
  this->PatchLayer.ImageSlice->SetDragable(true);

  // Snap the square to an integer pixel
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

  if(!this->InteractorStyle)
  {
    throw std::runtime_error("MovablePatch: Interactor is not valid!");
  }

  if(!this->InteractorStyle->GetCurrentRenderer())
  {
    throw std::runtime_error("MovablePatch: Renderer is not valid!");
  }
  this->InteractorStyle->GetCurrentRenderer()->AddViewProp(this->PatchLayer.ImageSlice);

  this->PatchLayer.ImageSlice->SetPosition(position);
  
  this->InteractorStyle->AddObserver(CustomTrackballStyle::PatchesMovedEvent,
                                     this, &MovablePatch::PatchMoved);
}

template <typename TInteractorStyle>
void MovablePatch<TInteractorStyle>::SetVisibility(const bool visibility)
{
  this->PatchLayer.ImageSlice->SetVisibility(visibility);
}

template <typename TInteractorStyle>
bool MovablePatch<TInteractorStyle>::GetVisibility()
{
  return this->PatchLayer.ImageSlice->GetVisibility();
}

template <typename TInteractorStyle>
itk::ImageRegion<2> MovablePatch<TInteractorStyle>::GetRegion()
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

template <typename TInteractorStyle>
void MovablePatch<TInteractorStyle>::Display()
{
  if(this->View)
  {
    QGraphicsPixmapItem* item = this->PatchScene->addPixmap(QPixmap::fromImage(this->PatchImage));
    this->View->fitInView(item);
  }
}

template <typename TInteractorStyle>
void MovablePatch<TInteractorStyle>::PatchMoved()
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

template <typename TInteractorStyle>
void MovablePatch<TInteractorStyle>::SetGraphicsView(QGraphicsView* const view)
{
  this->View = view;
  if(!this->PatchScene)
  {
    this->PatchScene = new QGraphicsScene;
  }
}

template <typename TInteractorStyle>
void MovablePatch<TInteractorStyle>::SetRegion(const itk::ImageRegion<2>& region)
{
  // VTK requires us to set the position of the slice using it's corner, so we just have to get
  // the Index of the region.

  double position[3] = {static_cast<double>(region.GetIndex()[0]),
                        static_cast<double>(region.GetIndex()[1]), 0};
  this->PatchLayer.ImageSlice->SetPosition(position);
}

#endif
