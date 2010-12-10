/*=========================================================================
 *
 *  Copyright David Doria 2010 daviddoria@gmail.com
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