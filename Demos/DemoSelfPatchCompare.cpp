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
#include "CandidatePairs.h"
#include "Helpers.h"
#include "Mask.h"
#include "Patch.h"
#include "SelfPatchCompare.h"
#include "Types.h"

// ITK
#include "itkBinaryThresholdImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageFileWriter.h"
#include "itkRandomImageSource.h"
#include "itkTimeProbe.h"
#include "itkVectorImage.h"

// Boost
#include <boost/bind.hpp>

// STL
#include <cstdlib>
#include <ctime>
#include <vector>

int main(int argc, char *argv[])
{
  unsigned int t = time(NULL);
  srand(t);

  itk::Size<2> size;
  size.Fill(100);

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
    pixel[0] = drand48();
    pixel[1] = drand48();
    pixel[2] = drand48();
    imageIterator.Set(pixel);
    ++imageIterator;
    }
  }

  // Generate a random membership image
  IntImageType::Pointer membershipImage = IntImageType::New();
  membershipImage->SetRegions(region);
  membershipImage->Allocate();
  membershipImage->FillBuffer(0);

  {
  itk::ImageRegionIterator<IntImageType> membershipImageIterator(membershipImage, membershipImage->GetLargestPossibleRegion());

  while(!membershipImageIterator.IsAtEnd())
    {
    IntImageType::PixelType pixel;
    pixel = rand() / 1000;
    membershipImageIterator.Set(pixel);
    ++membershipImageIterator;
    }
  }

  // Write the image
  itk::ImageFileWriter<FloatVectorImageType>::Pointer imageWriter =
    itk::ImageFileWriter<FloatVectorImageType>::New();
  imageWriter->SetFileName("image.mha");
  imageWriter->SetInput(image);
  imageWriter->Update();

//   // Generate a random mask
//   itk::RandomImageSource<Mask>::Pointer maskSource = itk::RandomImageSource<Mask>::New();
//   maskSource->SetNumberOfThreads(1); // to produce non-random results
//   maskSource->SetSize(size);
//   maskSource->SetMin(0);
//   maskSource->SetMax(255);
//   maskSource->Update();
//
//   // Threshold the mask
//   //typedef itk::ThresholdImageFilter <UnsignedCharImageType> ThresholdImageFilterType;
//   typedef itk::BinaryThresholdImageFilter <Mask, Mask> ThresholdImageFilterType;
//   ThresholdImageFilterType::Pointer thresholdFilter = ThresholdImageFilterType::New();
//   thresholdFilter->SetInput(maskSource->GetOutput());
//   thresholdFilter->SetLowerThreshold(0);
//   thresholdFilter->SetUpperThreshold(122);
//   thresholdFilter->SetOutsideValue(1);
//   thresholdFilter->SetInsideValue(0);
//   thresholdFilter->Update();
//   Mask::Pointer mask = thresholdFilter->GetOutput();

  std::cout << "Creating mask..." << std::endl;
  Mask::Pointer mask = Mask::New();
  mask->SetRegions(region);
  mask->Allocate();

  {
  itk::ImageRegionIterator<Mask> maskIterator(mask, mask->GetLargestPossibleRegion());

  while(!maskIterator.IsAtEnd())
    {
    int randomNumber = rand()%10;
    //std::cout << "randomNumber: " << randomNumber << std::endl;
    if(randomNumber > 5)
      {
      maskIterator.Set(mask->GetHoleValue());
      }
    else
      {
      maskIterator.Set(mask->GetValidValue());
      }
    ++maskIterator;
    }
  }
  std::cout << "Writing mask..." << std::endl;
  // Write the mask
  itk::ImageFileWriter<Mask>::Pointer maskWriter = itk::ImageFileWriter<Mask>::New();
  maskWriter->SetFileName("mask.png");
  maskWriter->SetInput(mask);
  maskWriter->Update();

  std::cout << "Creating source patches..." << std::endl;
  unsigned int patchRadius = 10;
  // Create source patches
  itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(image, image->GetLargestPossibleRegion());
  std::vector<Patch> sourcePatches;
  while(!imageIterator.IsAtEnd())
    {
    itk::Index<2> currentPixel = imageIterator.GetIndex();
    itk::ImageRegion<2> region = Helpers::GetRegionInRadiusAroundPixel(currentPixel, patchRadius);
    if(image->GetLargestPossibleRegion().IsInside(region))
      {
      sourcePatches.push_back(Patch(region));
      }
    ++imageIterator;
    }
  std::cout << "Source patches: " << sourcePatches.size() << std::endl;
  itk::Size<2> targetSize;
  targetSize.Fill(patchRadius * 2 + 1);

  itk::Index<2> targetIndex;
  targetIndex.Fill(3);

  itk::ImageRegion<2> targetRegion(targetIndex, targetSize);
  Patch targetPatch(targetRegion);

  CandidatePairs pairs(targetPatch);
  pairs.AddPairFromPatch(targetPatch);

  itk::ImageRegion<2> adjacentRegion = targetRegion;
  itk::Index<2> adjacentIndex;
  adjacentIndex[0] = targetIndex[0] + 1;
  adjacentIndex[1] = targetIndex[1] + 1;
  adjacentRegion.SetIndex(adjacentIndex);
  Patch adjacentPatch(adjacentRegion);
  pairs.AddPairFromPatch(adjacentPatch);
  //pairs.AddPairFromPatch(sourcePatches[0]);

  SelfPatchCompare patchCompare;
  patchCompare.SetPairs(&pairs);
  patchCompare.SetImage(image);
  patchCompare.SetMask(mask);
  patchCompare.SetNumberOfComponentsPerPixel(3);
  patchCompare.SetMembershipImage(membershipImage);

  patchCompare.FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchMembershipDifference,&patchCompare,_1));
  patchCompare.ComputeAllSourceDifferences();

  std::cout << "pairs: " << pairs.size() << std::endl;
  for(unsigned int i = 0; i < pairs.size(); ++i)
    {
    std::cout << "MembershipDifference: " << pairs[i].DifferenceMap[PatchPair::MembershipDifference] << std::endl;
    }

  //unsigned int bestMatchSourcePatchId = patchCompare.FindBestPatch();
  //std::cout << "bestMatchSourcePatchId: " << bestMatchSourcePatchId << std::endl;
/*
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
  std::cout << "Fast Total: " << fastTimer.GetTotal() << std::endl;*/

  return EXIT_SUCCESS;
}
