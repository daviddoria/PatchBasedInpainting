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