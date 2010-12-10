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

#ifndef CustomStyle_H
#define CustomStyle_H

#include <vtkInteractorStyleImage.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

class vtkRenderer;
class vtkImageData;
class vtkImageActor;

class CustomStyle : public vtkInteractorStyleImage
{
  public:
    static CustomStyle* New();
    vtkTypeMacro(CustomStyle, vtkInteractorStyleImage);

    CustomStyle();

    void CallbackFunction(vtkObject* caller,
                    long unsigned int eventId,
                    void* callData );

    void OnKeyPress();

  vtkRenderer* RightRenderer;
  vtkImageData* Image;
  vtkImageData* Mask;
  vtkSmartPointer<vtkImageActor> InpaintedActor;

};

#endif