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

#include "HelpersOutput.h"
#include "ITKHelpers.h"

#include <vtkSmartPointer.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkXMLPolyDataWriter.h>

namespace HelpersOutput
{
void WritePolyData(vtkPolyData* const polyData, const std::string& fileName)
{
  std::string extension = fileName.substr(fileName.size() - 3, fileName.size());
  if(extension.compare("vtp") != 0)
    {
    std::cerr << "Cannot write a vtkPolyData to a non .vtp file!" << std::endl;
    return;
    }

  vtkSmartPointer<vtkXMLPolyDataWriter> polyDataWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  polyDataWriter->SetFileName(fileName.c_str());
  polyDataWriter->SetInputConnection(polyData->GetProducerPort());
  polyDataWriter->Write();
}

void WriteImageData(vtkImageData* const imageData, const std::string& fileName)
{
  std::string extension = fileName.substr(fileName.size() - 3, fileName.size());
  if(extension.compare("vti") != 0)
    {
    std::cerr << "Cannot write a vtkImageData to a non .vti file!" << std::endl;
    return;
    }

  vtkSmartPointer<vtkXMLImageDataWriter> writer = vtkSmartPointer<vtkXMLImageDataWriter>::New();
  writer->SetFileName(fileName.c_str());
  writer->SetInputConnection(imageData->GetProducerPort());
  writer->Write();
}


void Write2DVectorImage(const FloatVector2ImageType* const image, const std::string& filename)
{
  Write2DVectorRegion(image, image->GetLargestPossibleRegion(), filename);
}

void Write2DVectorRegion(const FloatVector2ImageType* const image, const itk::ImageRegion<2>& region, const std::string& filename)
{
  // This is a separate function than WriteRegion because Paraview requires vectors to b 3D to glyph them.

  typedef itk::RegionOfInterestImageFilter<FloatVector2ImageType, FloatVector2ImageType> RegionOfInterestImageFilterType;

  typename RegionOfInterestImageFilterType::Pointer regionOfInterestImageFilter = RegionOfInterestImageFilterType::New();
  regionOfInterestImageFilter->SetRegionOfInterest(region);
  regionOfInterestImageFilter->SetInput(image);
  regionOfInterestImageFilter->Update();

  itk::Point<float, 2> origin;
  origin.Fill(0);
  regionOfInterestImageFilter->GetOutput()->SetOrigin(origin);

  FloatVector3ImageType::Pointer vectors3D = FloatVector3ImageType::New();
  vectors3D->SetRegions(regionOfInterestImageFilter->GetOutput()->GetLargestPossibleRegion());
  vectors3D->Allocate();

  itk::ImageRegionConstIterator<FloatVector2ImageType> iterator(regionOfInterestImageFilter->GetOutput(), regionOfInterestImageFilter->GetOutput()->GetLargestPossibleRegion());

  while(!iterator.IsAtEnd())
    {
    FloatVector2Type vec2d = iterator.Get();
    FloatVector3Type vec3d;
    vec3d[0] = vec2d[0];
    vec3d[1] = vec2d[1];
    vec3d[2] = 0;

    vectors3D->SetPixel(iterator.GetIndex(), vec3d);
    ++iterator;
    }

  //std::cout << "regionOfInterestImageFilter " << regionOfInterestImageFilter->GetOutput()->GetLargestPossibleRegion() << std::endl;

  itk::ImageFileWriter<FloatVector3ImageType>::Pointer writer = itk::ImageFileWriter<FloatVector3ImageType>::New();
  writer->SetFileName(filename);
  writer->SetInput(vectors3D);
  writer->Update();
}


void WriteVectorImageAsRGB(const FloatVectorImageType* const image, const std::string& fileName)
{
  RGBImageType::Pointer rgbImage = RGBImageType::New();
  ITKHelpers::VectorImageToRGBImage(image, rgbImage);
  WriteImage<RGBImageType>(rgbImage, fileName);
}

} // end namespace
