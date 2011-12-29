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

#ifndef InteractorStyleImageWithDrag_H
#define InteractorStyleImageWithDrag_H

#include <vtkCommand.h>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkSmartPointer.h>

// To use this, you must do, in this order:
//  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
//  this->Interactor->SetInteractorStyle(this->InteractorStyle);
//  this->InteractorStyle->Init();

class CustomTrackballStyle;
class CustomImageStyle;

class InteractorStyleImageWithDrag : public vtkInteractorStyleTrackballActor
{
  public:

    static InteractorStyleImageWithDrag* New();
    vtkTypeMacro(InteractorStyleImageWithDrag,vtkInteractorStyleTrackballActor);

    InteractorStyleImageWithDrag();

    void Init();

    void SetCurrentRenderer(vtkRenderer* renderer);

    void SetImageOrientation(const double*, const double*);

    vtkSmartPointer<CustomImageStyle> ImageStyle;
    vtkSmartPointer<CustomTrackballStyle> TrackballStyle;
};


////////////////////////////////
///// CustomImageStyle /////////
////////////////////////////////

#include <vtkInteractorStyleImage.h>


class CustomTrackballStyle;

// Define interaction style
class CustomImageStyle : public vtkInteractorStyleImage
{
  public:
    static CustomImageStyle* New();
    vtkTypeMacro(CustomImageStyle,vtkInteractorStyleImage);

    void OnLeftButtonDown();

    void SetOtherStyle(CustomTrackballStyle* style);


private:
  CustomTrackballStyle* OtherStyle;

};

class CustomImageStyle;

////////////////////////////////
///// CustomTrackballStyle /////
////////////////////////////////

// Define interaction style
class CustomTrackballStyle : public vtkInteractorStyleTrackballActor
{
  public:
    const static unsigned int PatchesMovedEvent = vtkCommand::UserEvent + 1;

    static CustomTrackballStyle* New();
    vtkTypeMacro(CustomTrackballStyle,vtkInteractorStyleTrackballActor);

    void OnLeftButtonDown();

    void OnLeftButtonUp();

    void OnMiddleButtonDown();

    void OnRightButtonDown();

    void SetOtherStyle(CustomImageStyle* style);

private:
  CustomImageStyle* OtherStyle;

};


#endif
