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

#include "PriorityConfidence.h" // Appease syntax parser

// Submodules
#include <Helpers/Helpers.h>
#include <ITKHelpers/ITKHelpers.h>
#include <ITKVTKHelpers/ITKVTKHelpers.h>

template <typename TNode>
void PriorityConfidence::Update(const TNode& sourceNode, const TNode& targetNode, const unsigned int patchNumber)
{
  float value = ComputeConfidenceTerm(targetNode);
  UpdateConfidences(targetNode, value);

  if(this->IsDebugOn())
  {
    ITKHelpers::WriteSequentialImage(this->ConfidenceMapImage.GetPointer(), "ConfidenceMap", patchNumber, 3, "mha");
  }
}

template <typename TNode>
float PriorityConfidence::ComputePriority(const TNode& queryPixel) const
{

  this->ComputePriorityCallCount++;
  float priority = ComputeConfidenceTerm(queryPixel);

  return priority;
}

template <typename TNode>
void PriorityConfidence::UpdateConfidences(const TNode& targetNode, const float value)
{
  // This is called once per inpainting iteration.

  itk::Index<2> targetPixel = ITKHelpers::CreateIndex(targetNode);

  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, this->PatchRadius);

  // Force the region to update to be entirely inside the image
  region.Crop(this->MaskImage->GetLargestPossibleRegion());

  // Set the hole pixels in the region to 'value'. Since this is sensitive to when we inpaint the mask (before or after this function)
  // we instead use the technique below.
//  itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, region);

//  while(!maskIterator.IsAtEnd())
//  {
//    if(maskIterator.Get() == this->MaskImage->GetHoleValue()) // avoid the GetPixel call in the above line.
//    {
//      this->ConfidenceMapImage->SetPixel(maskIterator.GetIndex(), value);
//      // std::cout << "Set " << maskIterator.GetIndex() << " to " << value << std::endl;
//    }
//    ++maskIterator;
//  }

  // Set the pixels which currently have a confidence of zero in the region to 'value'.
  itk::ImageRegionIterator<ConfidenceImageType> confidenceImageIterator(this->ConfidenceMapImage, region);

  while(!confidenceImageIterator.IsAtEnd())
  {
    if(confidenceImageIterator.Get() == 0.0f)
    {
      confidenceImageIterator.Set(value);
      // std::cout << "Set " << maskIterator.GetIndex() << " to " << value << std::endl;
    }
    ++confidenceImageIterator;
  }

}

// Two iterators
//template <typename TNode>
//float PriorityConfidence::ComputeConfidenceTerm(const TNode& queryNode) const
//{
//  // Sum the confidence map values in the valid region
//  // This is called ~50x per inpainting iteration (for 21x21 patches).
//  itk::Index<2> queryPixel = ITKHelpers::CreateIndex(queryNode);

//  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius);

//  // Ensure that the patch to use to compute the confidence is entirely inside the image
//  region.Crop(this->MaskImage->GetLargestPossibleRegion());

//  itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, region);
//  itk::ImageRegionConstIterator<ConfidenceImageType> confidenceImageIterator(this->ConfidenceMapImage, region);

//  // The confidence is computed as the sum of the confidences of patch pixels
//  // in the source region / area of the patch

//  float sum = 0.0f;

//  while(!maskIterator.IsAtEnd())
//  {
//    if(maskIterator.Get() == this->MaskImage->GetValidValue())
//    {
//      sum += confidenceImageIterator.Get();
//    }
//    ++confidenceImageIterator;
//    ++maskIterator;
//  }

////  if(sum == 0.0f)
////  {
////    throw std::runtime_error("Confidence is zero!");
////  }
//  assert(sum > 0.0f);

//  unsigned int numberOfPixels = region.GetNumberOfPixels();
//  float areaOfPatch = static_cast<float>(numberOfPixels);

//  float confidence = sum/areaOfPatch;

//  return confidence;
//}

// Single iterator, this is only marginally faster than the two iterator method above (~5s total in 300 iterations)
//template <typename TNode>
//float PriorityConfidence::ComputeConfidenceTerm(const TNode& queryNode) const
//{
//  // Sum the confidence map values in the valid region
//  // This is called ~50x per inpainting iteration (for 21x21 patches).
//  itk::Index<2> queryPixel = ITKHelpers::CreateIndex(queryNode);

//  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius);

//  // Ensure that the patch to use to compute the confidence is entirely inside the image
//  region.Crop(this->MaskImage->GetLargestPossibleRegion());

//  itk::ImageRegionConstIteratorWithIndex<ConfidenceImageType> confidenceImageIterator(this->ConfidenceMapImage, region);

//  // The confidence is computed as the sum of the confidences of patch pixels
//  // in the source region / area of the patch

//  float sum = 0.0f;

//  while(!confidenceImageIterator.IsAtEnd())
//  {
//    if(this->MaskImage->GetPixel(confidenceImageIterator.GetIndex()) == this->MaskImage->GetValidValue())
//    {
//      sum += confidenceImageIterator.Get();
//    }
//    ++confidenceImageIterator;
//  }

////  if(sum == 0.0f)
////  {
////    throw std::runtime_error("Confidence is zero!");
////  }
//  assert(sum > 0.0f);

//  unsigned int numberOfPixels = region.GetNumberOfPixels();
//  float areaOfPatch = static_cast<float>(numberOfPixels);

//  float confidence = sum/areaOfPatch;

//  return confidence;
//}

// Assume (correctly) that the confidence values are zero inside the masked region. This is only marginally faster than the single iterator method with the mask check (~5s total in 300 iterations)
//template <typename TNode>
//float PriorityConfidence::ComputeConfidenceTerm(const TNode& queryNode) const
//{
//  // Sum the confidence map values in the valid region
//  // This is called ~50x per inpainting iteration (for 21x21 patches).
//  itk::Index<2> queryPixel = ITKHelpers::CreateIndex(queryNode);

//  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius);

//  // Ensure that the patch to use to compute the confidence is entirely inside the image
//  region.Crop(this->MaskImage->GetLargestPossibleRegion());

//  itk::ImageRegionConstIterator<ConfidenceImageType> confidenceImageIterator(this->ConfidenceMapImage, region);

//  // The confidence is computed as the sum of the confidences of patch pixels
//  // in the source region / area of the patch

//  float sum = 0.0f;

//  while(!confidenceImageIterator.IsAtEnd())
//  {
//    sum += confidenceImageIterator.Get();
//    ++confidenceImageIterator;
//  }

////  if(sum == 0.0f)
////  {
////    throw std::runtime_error("Confidence is zero!");
////  }
//  assert(sum > 0.0f);

//  unsigned int numberOfPixels = region.GetNumberOfPixels();
//  float areaOfPatch = static_cast<float>(numberOfPixels);

//  float confidence = sum/areaOfPatch;

//  return confidence;
//}

// Manual loop, assuming (correctly) that the confidence values are zero inside the masked region. This is only marginally faster than the single iterator method with the mask check (~5s total in 300 iterations)
//template <typename TNode>
//float PriorityConfidence::ComputeConfidenceTerm(const TNode& queryNode) const
//{
//  // Sum the confidence map values in the valid region
//  // This is called ~50x per inpainting iteration (for 21x21 patches).
//  itk::Index<2> queryPixel = ITKHelpers::CreateIndex(queryNode);

//  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius);

//  // Ensure that the patch to use to compute the confidence is entirely inside the image
//  region.Crop(this->MaskImage->GetLargestPossibleRegion());

//  // The confidence is computed as the sum of the confidences of patch pixels
//  // in the source region / area of the patch

//  float sum = 0.0f;

//  for(unsigned int row = region.GetIndex()[1]; row < region.GetIndex()[1] + region.GetSize()[1]; ++row)
//  {
//    for(unsigned int col = region.GetIndex()[0]; col < region.GetIndex()[0] + region.GetSize()[0]; ++col)
//    {
//      itk::Index<2> index = {{col, row}};
//      sum += this->ConfidenceMapImage->GetPixel(index);
//    }
//  }

//  assert(sum > 0.0f);

//  unsigned int numberOfPixels = region.GetNumberOfPixels();
//  float areaOfPatch = static_cast<float>(numberOfPixels);

//  float confidence = sum/areaOfPatch;

//  return confidence;
//}

//// Manual buffer loop, assuming (correctly) that the confidence values are zero inside the masked region. This is only marginally faster than the single iterator method with the mask check (~5s total in 300 iterations)
//template <typename TNode>
//float PriorityConfidence::ComputeConfidenceTerm(const TNode& queryNode) const
//{
//  // Sum the confidence map values in the valid region
//  // This is called ~50x per inpainting iteration (for 21x21 patches).
//  itk::Index<2> queryPixel = ITKHelpers::CreateIndex(queryNode);

//  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius);

//  // Ensure that the patch to use to compute the confidence is entirely inside the image
//  itk::ImageRegion<2> fullRegion = this->ConfidenceMapImage->GetLargestPossibleRegion();
//  region.Crop(fullRegion);

//  ConfidenceImageType::PixelType* buffer = this->ConfidenceMapImage->GetBufferPointer();

//  // The confidence is computed as the sum of the confidences of patch pixels
//  // in the source region / area of the patch

//  float sum = 0.0f;

//  int width = fullRegion.GetSize()[0];
////  int height = fullRegion.GetSize()[1];

//  for(unsigned int row = region.GetIndex()[1]; row < region.GetIndex()[1] + region.GetSize()[1]; ++row)
//  {
//    for(unsigned int col = region.GetIndex()[0]; col < region.GetIndex()[0] + region.GetSize()[0]; ++col)
//    {
//      sum += buffer[row * width + col];
//    }
//  }

//  assert(sum > 0.0f);

//  unsigned int numberOfPixels = region.GetNumberOfPixels();
//  float areaOfPatch = static_cast<float>(numberOfPixels);

//  float confidence = sum/areaOfPatch;

//  return confidence;
//}

//// Manual smarter buffer loop, assuming (correctly) that the confidence values are zero inside the masked region. This is only marginally faster than the single iterator method with the mask check (~5s total in 300 iterations)
//template <typename TNode>
//float PriorityConfidence::ComputeConfidenceTerm(const TNode& queryNode) const
//{
//  // Sum the confidence map values in the valid region
//  // This is called ~50x per inpainting iteration (for 21x21 patches).
//  itk::Index<2> queryPixel = ITKHelpers::CreateIndex(queryNode);

//  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius);

//  // Ensure that the patch to use to compute the confidence is entirely inside the image
//  itk::ImageRegion<2> fullRegion = this->ConfidenceMapImage->GetLargestPossibleRegion();
//  region.Crop(fullRegion);

//  ConfidenceImageType::PixelType* buffer = this->ConfidenceMapImage->GetBufferPointer();

//  // The confidence is computed as the sum of the confidences of patch pixels
//  // in the source region / area of the patch

//  float sum = 0.0f;

//  int width = fullRegion.GetSize()[0];
////  int height = fullRegion.GetSize()[1];

//  // Move the pointer to the corner of the region
//  buffer += region.GetIndex()[1] * width + region.GetIndex()[0];

//  for(unsigned int rowId = 0; rowId < region.GetSize()[1]; ++rowId)
//  {
//    buffer += width;
//    for(unsigned int colId = 0; colId < region.GetSize()[0]; ++colId)
//    {
//      sum += *buffer;
//      buffer++;
//    }
//  }

//  assert(sum > 0.0f);

//  unsigned int numberOfPixels = region.GetNumberOfPixels();
//  float areaOfPatch = static_cast<float>(numberOfPixels);

//  float confidence = sum/areaOfPatch;

//  return confidence;
//}


//// Manual loop without indices, assuming (correctly) that the confidence values are zero inside the masked region. This is only marginally faster than the single iterator method with the mask check (~5s total in 300 iterations)
//template <typename TNode>
//float PriorityConfidence::ComputeConfidenceTerm(const TNode& queryNode) const
//{
//  // Sum the confidence map values in the valid region
//  // This is called ~50x per inpainting iteration (for 21x21 patches).
//  itk::Index<2> queryPixel = ITKHelpers::CreateIndex(queryNode);

//  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius);

//  // Ensure that the patch to use to compute the confidence is entirely inside the image
//  itk::ImageRegion<2> fullRegion = this->ConfidenceMapImage->GetLargestPossibleRegion();
//  region.Crop(fullRegion);

//  ConfidenceImageType::PixelType* buffer = this->ConfidenceMapImage->GetBufferPointer();

//  // The confidence is computed as the sum of the confidences of patch pixels
//  // in the source region / area of the patch

//  float sum = 0.0f;

//  int width = fullRegion.GetSize()[0];
////  int height = fullRegion.GetSize()[1];

//  // Move the pointer to the corner of the region
//  buffer += region.GetIndex()[1] * width + region.GetIndex()[0];

//  for(unsigned int rowId = 0; rowId < region.GetSize()[1]; ++rowId)
//  {
//    buffer += width;
//    for(unsigned int colId = 0; colId < region.GetSize()[0]; ++colId)
//    {
//      sum += *buffer;
//      ++buffer;
//    }
//  }

////  for(unsigned int rowId = 0; rowId < region.GetSize()[1]; ++rowId, buffer += width)
////  {
////    for(unsigned int colId = 0; colId < region.GetSize()[0]; ++colId, ++buffer)
////    {
////      sum += *buffer;
////    }
////  }

//  assert(sum > 0.0f);

//  unsigned int numberOfPixels = region.GetNumberOfPixels();
//  float areaOfPatch = static_cast<float>(numberOfPixels);

//  float confidence = sum/areaOfPatch;

//  return confidence;
//}

#include <xmmintrin.h>

// Manual loop with prefetching, assuming (correctly) that the confidence values are zero inside the masked region. This is only marginally faster than the single iterator method with the mask check (~5s total in 300 iterations)
template <typename TNode>
float PriorityConfidence::ComputeConfidenceTerm(const TNode& queryNode) const
{
  // Sum the confidence map values in the valid region
  // This is called ~50x per inpainting iteration (for 21x21 patches).
  itk::Index<2> queryPixel = ITKHelpers::CreateIndex(queryNode);

  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius);

  // Ensure that the patch to use to compute the confidence is entirely inside the image
  itk::ImageRegion<2> fullRegion = this->ConfidenceMapImage->GetLargestPossibleRegion();
  region.Crop(fullRegion);

  ConfidenceImageType::PixelType* buffer = this->ConfidenceMapImage->GetBufferPointer();

  // The confidence is computed as the sum of the confidences of patch pixels
  // in the source region / area of the patch

  float sum = 0.0f;

  int width = fullRegion.GetSize()[0];
//  int height = fullRegion.GetSize()[1];

  // Move the pointer to the corner of the region
  buffer += region.GetIndex()[1] * width + region.GetIndex()[0];

//  #pragma omp parallel for
//  for(unsigned int rowId = 0; rowId < region.GetSize()[1]; ++rowId)
//  {
//    _mm_prefetch(buffer, _MM_HINT_T0);
//    _mm_prefetch(buffer + 5, _MM_HINT_T0);
//    _mm_prefetch(buffer + 20, _MM_HINT_T0);

////    __builtin_prefetch(buffer);
////    __builtin_prefetch(buffer + 16); // There are probably 64 bytes in a cache line, and the prefetch instruction caches a cacheline. This is not enough to get us all the way across the row, so we prefetch the first half of the row, and then also the rest of the row.
//    buffer += width;
//  }

  // Move the pointer to the corner of the region
  buffer = this->ConfidenceMapImage->GetBufferPointer() + region.GetIndex()[1] * width + region.GetIndex()[0];

//  #pragma omp parallel for
  for(unsigned int rowId = 0; rowId < region.GetSize()[1]; ++rowId)
  {
    buffer += width;
    for(unsigned int colId = 0; colId < region.GetSize()[0]; ++colId)
    {
      sum += *buffer;
      ++buffer;
    }
  }

//  for(unsigned int rowId = 0; rowId < region.GetSize()[1]; ++rowId, buffer += width)
//  {
//    for(unsigned int colId = 0; colId < region.GetSize()[0]; ++colId, ++buffer)
//    {
//      sum += *buffer;
//    }
//  }

  assert(sum > 0.0f);

  unsigned int numberOfPixels = region.GetNumberOfPixels();
  float areaOfPatch = static_cast<float>(numberOfPixels);

  float confidence = sum/areaOfPatch;

  return confidence;
}
