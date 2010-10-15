#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <vtkImageData.h>
#include <vtkImageActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderer.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkGlyph3DMapper.h>
#include <vtkArrowSource.h>
#include <vtkImageGradient.h>

#include "vtkImageVectorRotate.h"

int main(int, char *[])
{
  // Create an image of a white square on a black background
  vtkSmartPointer<vtkImageCanvasSource2D> drawing =
    vtkSmartPointer<vtkImageCanvasSource2D>::New();
  drawing->SetScalarTypeToUnsignedChar();
  drawing->SetNumberOfScalarComponents(1);
  drawing->SetExtent(0, 20, 0, 50, 0, 0);
  drawing->FillBox(0,20,0,50);
  drawing->SetDrawColor(255, 0, 0, 0);
  drawing->FillBox(10, 20, 10, 20);
  drawing->Update();

  vtkSmartPointer<vtkImageGradient> gradientFilter =
    vtkSmartPointer<vtkImageGradient>::New();
  gradientFilter->SetInputConnection(drawing->GetOutputPort());
  gradientFilter->SetDimensionality(3);
  gradientFilter->Update();

  vtkSmartPointer<vtkImageVectorRotate> vectorRotate =
    vtkSmartPointer<vtkImageVectorRotate>::New();
  vectorRotate->SetZRotation(15);
  vectorRotate->SetInputConnection(gradientFilter->GetOutputPort());
  vectorRotate->Update();

  vtkSmartPointer<vtkArrowSource> arrowSource =
    vtkSmartPointer<vtkArrowSource>::New();
  gradientFilter->GetOutput()->GetPointData()->SetActiveVectors("ImageScalarsGradient");
  vectorRotate->GetOutput()->GetPointData()->SetActiveVectors("ImageScalarsGradient");

  vtkSmartPointer<vtkGlyph3DMapper> originalMapper =
    vtkSmartPointer<vtkGlyph3DMapper>::New();
  originalMapper->ScalingOn();
  originalMapper->SetScaleFactor(.01);
  originalMapper->SetSourceConnection(arrowSource->GetOutputPort());
  originalMapper->SetInputConnection(gradientFilter->GetOutputPort());
  originalMapper->Update();

  vtkSmartPointer<vtkActor> originalActor =
    vtkSmartPointer<vtkActor>::New();
  originalActor->SetMapper(originalMapper);

  vtkSmartPointer<vtkGlyph3DMapper> rotatedMapper =
    vtkSmartPointer<vtkGlyph3DMapper>::New();
  rotatedMapper->ScalingOn();
  rotatedMapper->SetScaleFactor(.01);
  rotatedMapper->SetSourceConnection(arrowSource->GetOutputPort());
  rotatedMapper->SetInputConnection(vectorRotate->GetOutputPort());
  rotatedMapper->Update();
  
  vtkSmartPointer<vtkActor> rotatedActor =
    vtkSmartPointer<vtkActor>::New();
  rotatedActor->SetMapper(rotatedMapper);

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
  rightRenderer->AddActor(rotatedActor);

  leftRenderer->ResetCamera();
  rightRenderer->ResetCamera();

  renderWindow->Render();

  interactor->Start();

  return EXIT_SUCCESS;
}