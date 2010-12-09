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

#include "CriminisiInpainting.h"

int main(int argc, char *argv[])
{
  if(argc != 3)
    {
    std::cerr << "Required arguments: image.jpg imageMask.jpg" << std::endl;
    return EXIT_FAILURE;
    }
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;

  ColorImageReaderType::Pointer imageReader = ColorImageReaderType::New();
  imageReader->SetFileName(imageFilename.c_str());
  imageReader->Update();

  UnsignedCharImageReaderType::Pointer maskReader = UnsignedCharImageReaderType::New();
  maskReader->SetFileName(maskFilename.c_str());
  maskReader->Update();

  CriminisiInpainting Inpainting;
  Inpainting.SetWriteIntermediateImages(true);
  Inpainting.SetImage(imageReader->GetOutput());
  Inpainting.SetInputMask(maskReader->GetOutput());
  Inpainting.Inpaint();

  /*
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
  */
  return EXIT_SUCCESS;
}
