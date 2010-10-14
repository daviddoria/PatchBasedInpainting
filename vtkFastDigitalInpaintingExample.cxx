#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkJPEGReader.h>
#include <vtkImageActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderer.h>
#include <vtkObjectFactory.h>
#include <vtkCommand.h>

#include "vtkFastDigitalInpainting.h"

// Define interaction style
class CustomStyle : public vtkInteractorStyleImage
{
  public:
    static CustomStyle* New();
    vtkTypeMacro(CustomStyle, vtkInteractorStyleImage);

    CustomStyle()
    {
      this->InpaintedActor = vtkSmartPointer<vtkImageActor>::New();
      this->Inpainting = vtkSmartPointer<vtkFastDigitalInpainting>::New();
    }

    void CallbackFunction(vtkObject* caller,
                    long unsigned int eventId,
                    void* callData )
    {
      std::cout << "CustomStyle::CallbackFunction called." << std::endl;
      vtkImageData* intermediate = this->Inpainting->GetIntermediateOutput();
      intermediate->Modified();
      this->InpaintedActor->SetInput(intermediate);
      this->RightRenderer->ResetCamera();
      this->RightRenderer->Render();
      this->Interactor->GetRenderWindow()->Render();
    }

    void OnKeyPress()
    {
      // Get the keypress
      std::string key = this->Interactor->GetKeySym();

      if(key.compare("s") == 0) // 'S'tart
        {
        this->RightRenderer->AddActor(this->InpaintedActor);
        this->Inpainting->SetMaximumIterations(1000);
        this->Inpainting->SetInputConnection(0, this->Image->GetProducerPort());
        this->Inpainting->SetInputConnection(1, this->Mask->GetProducerPort());
        this->Inpainting->AddObserver(vtkCommand::WarningEvent, this, &CustomStyle::CallbackFunction);
        this->Inpainting->Update();

        this->InpaintedActor->SetInput(this->Inpainting->GetOutput());
        this->RightRenderer->ResetCamera();
        this->RightRenderer->Render();
        }

      // Forward events
      vtkInteractorStyleTrackballCamera::OnKeyPress();
    }

  vtkRenderer* RightRenderer;
  vtkImageData* Image;
  vtkImageData* Mask;
  vtkSmartPointer<vtkImageActor> InpaintedActor;
  vtkSmartPointer<vtkFastDigitalInpainting> Inpainting;
};
vtkStandardNewMacro(CustomStyle);

int main(int argc, char *argv[])
{
  if(argc != 3)
    {
    std::cerr << "Required arguments: image.jpg imageMask.jpg" << std::endl;
    return EXIT_FAILURE;
    }
  std::string imageFilename = argv[1];
  std::string imageMaskFilename = argv[2];

  vtkSmartPointer<vtkJPEGReader> imageReader =
    vtkSmartPointer<vtkJPEGReader>::New();
  imageReader->SetFileName(imageFilename.c_str());
  imageReader->Update();

  vtkSmartPointer<vtkJPEGReader> maskReader =
    vtkSmartPointer<vtkJPEGReader>::New();
  maskReader->SetFileName(imageMaskFilename.c_str());
  maskReader->Update();

  vtkSmartPointer<vtkImageActor> originalActor =
    vtkSmartPointer<vtkImageActor>::New();
  originalActor->SetInput(imageReader->GetOutput());

  vtkSmartPointer<vtkImageActor> maskActor =
    vtkSmartPointer<vtkImageActor>::New();
  maskActor->SetInput(maskReader->GetOutput());
  maskActor->SetOpacity(.5);

  // There will be one render window
  vtkSmartPointer<vtkRenderWindow> renderWindow =
    vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->SetSize(1000, 500);

  // And one interactor
  vtkSmartPointer<vtkRenderWindowInteractor> interactor =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  interactor->SetRenderWindow(renderWindow);

  // Define viewport ranges
  // (xmin, ymin, xmax, ymax)
  double leftViewport[4] = {0.0, 0.0, 0.5, 1.0};
  double rightViewport[4] = {0.5, 0.0, 1.0, 1.0};

  // Setup both renderers
  vtkSmartPointer<vtkRenderer> leftRenderer =
    vtkSmartPointer<vtkRenderer>::New();
  renderWindow->AddRenderer(leftRenderer);
  leftRenderer->SetViewport(leftViewport);
  leftRenderer->SetBackground(.6, .5, .4);

  vtkSmartPointer<vtkRenderer> rightRenderer =
    vtkSmartPointer<vtkRenderer>::New();
  renderWindow->AddRenderer(rightRenderer);
  rightRenderer->SetViewport(rightViewport);
  rightRenderer->SetBackground(.4, .5, .6);

  // Add the sphere to the left and the cube to the right
  leftRenderer->AddActor(originalActor);
  //leftRenderer->AddActor(maskActor);

  leftRenderer->ResetCamera();
  rightRenderer->ResetCamera();

  renderWindow->Render();

  vtkSmartPointer<CustomStyle> style =
    vtkSmartPointer<CustomStyle>::New();
  style->RightRenderer = rightRenderer;
  style->Image = imageReader->GetOutput();
  style->Mask = maskReader->GetOutput();
  interactor->SetInteractorStyle(style);

  interactor->Start();

  return EXIT_SUCCESS;
}