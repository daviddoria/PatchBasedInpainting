#include "InteractorStyleImageWithDrag.h"

#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>

vtkStandardNewMacro(InteractorStyleImageWithDrag);

InteractorStyleImageWithDrag::InteractorStyleImageWithDrag()
{
  this->ImageStyle = vtkSmartPointer<CustomImageStyle>::New();

  this->TrackballStyle = vtkSmartPointer<CustomTrackballStyle>::New();

}

void InteractorStyleImageWithDrag::Init()
{

  this->ImageStyle->SetOtherStyle(this->TrackballStyle);
  this->TrackballStyle->SetOtherStyle(this->ImageStyle);

  this->Interactor->SetInteractorStyle(this->ImageStyle);
}

void InteractorStyleImageWithDrag::SetCurrentRenderer(vtkRenderer* renderer)
{
  this->ImageStyle->SetCurrentRenderer(renderer);
  this->TrackballStyle->SetCurrentRenderer(renderer);
}

void InteractorStyleImageWithDrag::SetImageOrientation(const double* leftToRight, const double* bottomToTop)
{
  this->ImageStyle->SetImageOrientation(leftToRight, bottomToTop);
}

////////////////////////////////
///// CustomTrackballStyle /////
////////////////////////////////
vtkStandardNewMacro(CustomTrackballStyle);

void CustomTrackballStyle::OnLeftButtonDown()
{
  //std::cout << "TrackballStyle::OnLeftButtonDown()" << std::endl;
  // Behave like the middle button instead
  vtkInteractorStyleTrackballActor::OnMiddleButtonDown();
}

void CustomTrackballStyle::OnLeftButtonUp()
{
  //std::cout << "TrackballStyle::OnLeftButtonUp()" << std::endl;
  // Behave like the middle button instead
  vtkInteractorStyleTrackballActor::OnMiddleButtonUp();
  this->InvokeEvent(this->PatchesMovedEvent, NULL);
}

void CustomTrackballStyle::OnMiddleButtonDown()
{
  this->Interactor->SetInteractorStyle(this->OtherStyle);
  this->OtherStyle->OnMiddleButtonDown();
}

void CustomTrackballStyle::OnRightButtonDown()
{
  this->Interactor->SetInteractorStyle(this->OtherStyle);
  this->OtherStyle->OnRightButtonDown();
}

void CustomTrackballStyle::SetOtherStyle(CustomImageStyle* style)
{
  this->OtherStyle = style;
}

////////////////////////////////
///// CustomImageStyle /////////
////////////////////////////////

vtkStandardNewMacro(CustomImageStyle);

void CustomImageStyle::OnLeftButtonDown()
{
  //std::cout << "OnLeftButtonDown()" << std::endl;
  // Behave like the middle button instead
  this->Interactor->SetInteractorStyle(this->OtherStyle);
  this->OtherStyle->OnLeftButtonDown();
}

void CustomImageStyle::SetOtherStyle(CustomTrackballStyle* style)
{
  this->OtherStyle = style;
}