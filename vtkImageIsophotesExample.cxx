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

#include "vtkImageIsophotes.h"

int main(int, char *[])
{
  // Create an image of a white square on a black background
  
  vtkSmartPointer<vtkImageCanvasSource2D> drawing =
    vtkSmartPointer<vtkImageCanvasSource2D>::New();
  drawing->SetScalarTypeToUnsignedChar();
  drawing->SetNumberOfScalarComponents(1);
  drawing->SetExtent(0, 20, 0, 50, 0, 0);
  drawing->SetDrawColor(0, 0, 0, 0);
  drawing->FillBox(0,20,0,50);
  drawing->SetDrawColor(255, 0, 0, 0);
  drawing->FillBox(10, 15, 10, 15);
  drawing->Update();
  

  /*
  // Simple 2x2 image to use for detailed inspection
  vtkSmartPointer<vtkImageCanvasSource2D> drawing =
    vtkSmartPointer<vtkImageCanvasSource2D>::New();
  drawing->SetScalarTypeToUnsignedChar();
  drawing->SetNumberOfScalarComponents(1);
  drawing->SetExtent(0, 1, 0, 1, 0, 0);
  drawing->SetDrawColor(0, 0, 0, 0);
  drawing->FillBox(0,1,0,1);
  drawing->SetDrawColor(255, 0, 0, 0);
  drawing->FillBox(0, 0, 0, 10);
  drawing->Update();
  */
  
  vtkSmartPointer<vtkImageIsophotes> isophotesFilter =
    vtkSmartPointer<vtkImageIsophotes>::New();
  isophotesFilter->SetInputConnection(drawing->GetOutputPort());
  isophotesFilter->Update();

  vtkSmartPointer<vtkImageActor> originalActor =
    vtkSmartPointer<vtkImageActor>::New();
  originalActor->SetInput(drawing->GetOutput());

  vtkSmartPointer<vtkArrowSource> arrowSource =
    vtkSmartPointer<vtkArrowSource>::New();
  isophotesFilter->GetOutput()->GetPointData()->SetActiveVectors("ImageScalarsGradient");
  
  vtkSmartPointer<vtkGlyph3DMapper> isophotesMapper =
    vtkSmartPointer<vtkGlyph3DMapper>::New();
  //isophotesMapper->ScalingOn();
  //isophotesMapper->SetScaleFactor(.01);
  isophotesMapper->SetSourceConnection(arrowSource->GetOutputPort());
  isophotesMapper->SetInputConnection(isophotesFilter->GetOutputPort());
  isophotesMapper->Update();
  
  vtkSmartPointer<vtkActor> isophotesActor =
    vtkSmartPointer<vtkActor>::New();
  isophotesActor->SetMapper(isophotesMapper);

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
  rightRenderer->AddActor(isophotesActor);

  leftRenderer->ResetCamera();
  rightRenderer->ResetCamera();

  renderWindow->Render();

  vtkSmartPointer<vtkInteractorStyleImage> style =
    vtkSmartPointer<vtkInteractorStyleImage>::New();
  interactor->SetInteractorStyle(style);
  
  interactor->Start();

  return EXIT_SUCCESS;
}