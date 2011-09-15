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

#include "Mask.h"
#include "SelfPatchCompare.h"
#include "Types.h"

#include "itkBinaryThresholdImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageFileWriter.h"
#include "itkRandomImageSource.h"
#include "itkTimeProbe.h"
#include "itkVectorImage.h"

#include <cstdlib>
#include <ctime>

itk::ImageRegion<2> GetRegionInRadiusAroundPixel(const itk::Index<2> pixel, const unsigned int radius);
float RandomFloat();

int main(int argc, char *argv[])
{
  unsigned int t = time(NULL);
  srand(t);
  
  itk::Size<2> size;
  size.Fill(10);

  itk::Index<2> index;
  index.Fill(0);

  itk::ImageRegion<2> region(index, size);
/*
  // Generate a random image (this method doesn't work with VectorImage)
  itk::RandomImageSource<FloatVectorImageType>::Pointer imageSource =
    itk::RandomImageSource<FloatVectorImageType>::New();
  imageSource->SetNumberOfThreads(1); // to produce non-random results
  imageSource->SetSize(size);
  imageSource->SetMin(0);
  imageSource->SetMax(100);
  imageSource->Update();
  FloatVectorImageType::Pointer image = imageSource->GetOutput();
*/
  // Generate a random image
  FloatVectorImageType::Pointer image = FloatVectorImageType::New();
  image->SetRegions(region);
  image->SetNumberOfComponentsPerPixel(3);
  image->Allocate();

  {
  itk::ImageRegionIterator<FloatVectorImageType> imageIterator(image, image->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    FloatVectorImageType::PixelType pixel;
    pixel.SetSize(3);
    pixel[0] = RandomFloat();
    pixel[1] = RandomFloat();
    pixel[2] = RandomFloat();
    imageIterator.Set(pixel);
    ++imageIterator;
    }
  }
  
  // Write the image
  itk::ImageFileWriter<FloatVectorImageType>::Pointer imageWriter =
    itk::ImageFileWriter<FloatVectorImageType>::New();
  imageWriter->SetFileName("image.mha");
  imageWriter->SetInput(image);
  imageWriter->Update();

  // Generate a random mask
  itk::RandomImageSource<Mask>::Pointer maskSource =
    itk::RandomImageSource<Mask>::New();
  maskSource->SetNumberOfThreads(1); // to produce non-random results
  maskSource->SetSize(size);
  maskSource->SetMin(0);
  maskSource->SetMax(255);
  maskSource->Update();

  // Threshold the mask
  //typedef itk::ThresholdImageFilter <UnsignedCharImageType> ThresholdImageFilterType;
  typedef itk::BinaryThresholdImageFilter <Mask, Mask> ThresholdImageFilterType;
  ThresholdImageFilterType::Pointer thresholdFilter = ThresholdImageFilterType::New();
  thresholdFilter->SetInput(maskSource->GetOutput());
  thresholdFilter->SetLowerThreshold(0);
  thresholdFilter->SetUpperThreshold(122);
  thresholdFilter->SetOutsideValue(1);
  thresholdFilter->SetInsideValue(0);
  thresholdFilter->Update();
  Mask::Pointer mask = thresholdFilter->GetOutput();

  // Write the mask
  itk::ImageFileWriter<Mask>::Pointer maskWriter =
    itk::ImageFileWriter<Mask>::New();
  maskWriter->SetFileName("mask.png");
  maskWriter->SetInput(mask);
  maskWriter->Update();

  unsigned int patchRadius = 1;
  // Create source patches
  itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(image, image->GetLargestPossibleRegion());
  std::vector<itk::ImageRegion<2> > sourcePatches;
  while(!imageIterator.IsAtEnd())
    {
    itk::Index<2> currentPixel = imageIterator.GetIndex();
    itk::ImageRegion<2> region = GetRegionInRadiusAroundPixel(currentPixel, patchRadius);
    if(image->GetLargestPossibleRegion().IsInside(region))
      {
      sourcePatches.push_back(region);
      }
    ++imageIterator;
    }

  itk::Size<2> targetSize;
  targetSize.Fill(patchRadius * 2 + 1);

  itk::Index<2> targetIndex;
  targetIndex.Fill(3);
  
  itk::ImageRegion<2> targetRegion(targetIndex, targetSize);
  SelfPatchCompare patchCompare;
  patchCompare.SetImage(image);
  patchCompare.SetMask(mask);
  patchCompare.SetSourceRegions(sourcePatches);
  patchCompare.SetTargetRegion(targetRegion);
  patchCompare.ComputeOffsets();
  
  //unsigned int bestMatchSourcePatchId = patchCompare.FindBestPatch();
  //std::cout << "bestMatchSourcePatchId: " << bestMatchSourcePatchId << std::endl;

  unsigned int patchId = 1;
  float slowPatchDifference = patchCompare.SlowDifference(sourcePatches[patchId]);
  std::cout << "slowPatchDifference: " << slowPatchDifference << std::endl;
  
  float fastPatchDifference = patchCompare.PatchDifference(sourcePatches[patchId]);
  std::cout << "fastPatchDifference: " << fastPatchDifference << std::endl;

  unsigned int iterations = 1e6;

  itk::TimeProbe slowTimer;
  slowTimer.Start();
  
  for(unsigned int i = 0; i < iterations; ++i)
    {
    float slowPatchDifference = patchCompare.SlowDifference(sourcePatches[patchId]);
    }

  slowTimer.Stop();
  std::cout << "Slow Total: " << slowTimer.GetTotal() << std::endl;

  itk::TimeProbe fastTimer;
  fastTimer.Start();

  for(unsigned int i = 0; i < iterations; ++i)
    {
    float fastPatchDifference = patchCompare.PatchDifference(sourcePatches[patchId]);
    }

  fastTimer.Stop();
  std::cout << "Fast Total: " << fastTimer.GetTotal() << std::endl;
  return EXIT_SUCCESS;
}

itk::ImageRegion<2> GetRegionInRadiusAroundPixel(const itk::Index<2> pixel, const unsigned int radius)
{
  // This function returns a Region with the specified 'radius' centered at 'pixel'. By the definition of the radius of a square patch, the output region is (radius*2 + 1)x(radius*2 + 1).
  // Note: This region is not necessarily entirely inside the image!

  // The "index" is the lower left corner, so we need to subtract the radius from the center to obtain it
  itk::Index<2> lowerLeft;
  lowerLeft[0] = pixel[0] - radius;
  lowerLeft[1] = pixel[1] - radius;

  itk::ImageRegion<2> region;
  region.SetIndex(lowerLeft);
  itk::Size<2> size;
  size[0] = radius*2 + 1;
  size[1] = radius*2 + 1;
  region.SetSize(size);

  return region;
}

float RandomFloat()
{
  /*
  typedef itk::Statistics::MersenneTwisterRandomVariateGenerator GeneratorType;
  GeneratorType::Pointer generator = GeneratorType::New();

  generator->Initialize();
  return generator->GetUniformVariate(0, 5);
  */
  return drand48();
}
