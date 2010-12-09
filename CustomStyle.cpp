/*
Copyright (C) 2010 David Doria, daviddoria@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "CustomStyle.h"

#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkJPEGReader.h>
#include <vtkImageActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkObjectFactory.h>
#include <vtkCommand.h>

vtkStandardNewMacro(CustomStyle);


CustomStyle::CustomStyle()
{
  this->InpaintedActor = vtkSmartPointer<vtkImageActor>::New();
}

void CustomStyle::CallbackFunction(vtkObject* caller,
                long unsigned int eventId,
                void* callData )
{
  std::cout << "CustomStyle::CallbackFunction called." << std::endl;
  /*
  this->InpaintedActor->SetInput(intermediate);
  this->RightRenderer->ResetCamera();
  this->RightRenderer->Render();
  this->Interactor->GetRenderWindow()->Render();
  */
}

void CustomStyle::OnKeyPress()
{
  // Get the keypress
  std::string key = this->Interactor->GetKeySym();

  if(key.compare("s") == 0) // 'S'tart
    {
    /*
    this->RightRenderer->AddActor(this->InpaintedActor);
    this->Inpainting->SetMaximumIterations(1000);
    this->Inpainting->SetInputConnection(0, this->Image->GetProducerPort());
    this->Inpainting->SetInputConnection(1, this->Mask->GetProducerPort());
    this->Inpainting->AddObserver(vtkCommand::WarningEvent, this, &CustomStyle::CallbackFunction);
    this->Inpainting->Update();

    this->InpaintedActor->SetInput(this->Inpainting->GetOutput());
    this->RightRenderer->ResetCamera();
    this->RightRenderer->Render();
    */
    }

  // Forward events
  vtkInteractorStyleTrackballCamera::OnKeyPress();
}