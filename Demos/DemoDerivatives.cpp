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

// Custom
#include "HelpersOutput.h"
#include "Mask.h"
#include "Types.h"

// ITK
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMaskImageFilter.h"
#include "itkLaplacianImageFilter.h"
#include "itkGradientImageFilter.h"

int main(int argc, char *argv[])
{
  std::string inputFileName = argv[1];
  //std::string outputFileName = argv[2];

  typedef itk::ImageFileReader<FloatScalarImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(inputFileName);
  reader->Update();

  typedef itk::GradientImageFilter< FloatScalarImageType, float>  GradientFilterType;
  GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
  gradientFilter->SetInput(reader->GetOutput());
  gradientFilter->Update();

  HelpersOutput::WriteImage<GradientFilterType::OutputImageType>(gradientFilter->GetOutput(), "gradient.mha");

  typedef itk::LaplacianImageFilter<FloatScalarImageType, FloatScalarImageType>  LaplacianFilterType;
  LaplacianFilterType::Pointer laplacianFilter = LaplacianFilterType::New();
  laplacianFilter->SetInput(reader->GetOutput());
  laplacianFilter->Update();

  HelpersOutput::WriteImage<LaplacianFilterType::OutputImageType>(laplacianFilter->GetOutput(), "laplacian.mha");

  return EXIT_SUCCESS;
}
/*
static void CreateMask(Mask::Pointer mask)
{
  itk::Size<2> size;
  size.Fill(20);

  itk::Index<2> start;
  start.Fill(0);

  itk::ImageRegion<2> region(start,size);

  mask->SetRegions(region);
  mask->Allocate();
  mask->FillBuffer(mask->GetValidValue());

  itk::ImageRegionIterator<Mask> iterator(mask, mask->GetLargestPossibleRegion());

  while(!iterator.IsAtEnd())
    {
    if(iterator.GetIndex()[0] > 5 && iterator.GetIndex()[0] < 15 &&
       iterator.GetIndex()[1] > 5 && iterator.GetIndex()[1] < 15)
      {
      mask->SetPixel(iterator.GetIndex(), mask->GetHoleValue());
      }

    ++iterator;
    }
}*/
