/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
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
#include "ImageCamera.h"

// VTK
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

// Custom
#include "InteractorStyleImageWithDrag.h"

ImageCamera::ImageCamera(vtkRenderer* const renderer) : Renderer(renderer)
{
  this->CameraLeftToRightVector.resize(3);
  this->CameraLeftToRightVector[0] = -1;
  this->CameraLeftToRightVector[1] = 0;
  this->CameraLeftToRightVector[2] = 0;

  this->CameraBottomToTopVector.resize(3);
  this->CameraBottomToTopVector[0] = 0;
  this->CameraBottomToTopVector[1] = 1;
  this->CameraBottomToTopVector[2] = 0;
}

void ImageCamera::SetCameraPosition()
{
  double leftToRight[3] = {this->CameraLeftToRightVector[0], this->CameraLeftToRightVector[1], this->CameraLeftToRightVector[2]};
  double bottomToTop[3] = {this->CameraBottomToTopVector[0], this->CameraBottomToTopVector[1], this->CameraBottomToTopVector[2]};

  InteractorStyleImageWithDrag::SafeDownCast(this->Renderer->GetRenderWindow()->GetInteractor()->GetInteractorStyle())->SetImageOrientation(leftToRight, bottomToTop);

  this->Renderer->ResetCamera();
  this->Renderer->ResetCameraClippingRange();
  this->Renderer->GetRenderWindow()->Render();
}

void ImageCamera::FlipVertically()
{
  this->CameraBottomToTopVector[1] *= -1;
  SetCameraPosition();
}

void ImageCamera::FlipHorizontally()
{
  this->CameraLeftToRightVector[0] *= -1;
  SetCameraPosition();
}
