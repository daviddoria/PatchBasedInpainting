/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
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

#ifndef DilatedVarianceDifferenceAcceptanceVisitor_HPP
#define DilatedVarianceDifferenceAcceptanceVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

// Custom
#include <Mask/Mask.h>
#include <ITKHelpers/ITKHelpers.h>

// ITK
#include "itkImage.h"
#include "itkImageRegion.h"

/**

 */
template <typename TGraph, typename TImage>
struct DilatedVarianceDifferenceAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;
  unsigned int NumberOfFinishedVertices = 0;

  float DifferenceThreshold;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  DilatedVarianceDifferenceAcceptanceVisitor(TImage* const image, Mask* const mask,
                                             const unsigned int halfWidth, const float differenceThreshold = 100) :
  Image(image), MaskImage(mask), HalfWidth(halfWidth), DifferenceThreshold(differenceThreshold)
  {
  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy) const override
  {
    //std::cout << "DilatedVarianceDifferenceAcceptanceVisitor::AcceptMatch" << std::endl;

    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, HalfWidth);

    itk::Index<2> sourcePixel = ITKHelpers::CreateIndex(source);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourcePixel, HalfWidth);

    //std::cout << "Extracting target region mask..." << std::endl;
    Mask::Pointer targetRegionMask = Mask::New();
    ITKHelpers::ExtractRegion(MaskImage, targetRegion, targetRegionMask.GetPointer());
    //std::cout << "There are " << ITKHelpers::CountPixelsWithValue(targetRegionMask.GetPointer(), targetRegionMask->GetHoleValue()) << " hole pixels in the target region." << std::endl;

    //std::cout << "Dilating target region mask..." << std::endl;
    Mask::Pointer dilatedTargetRegionMask = Mask::New();
    ITKHelpers::DilateImage(targetRegionMask.GetPointer(), dilatedTargetRegionMask.GetPointer(), 6);
    //std::cout << "There are " << ITKHelpers::CountPixelsWithValue(dilatedTargetRegionMask.GetPointer(), dilatedTargetRegionMask->GetHoleValue()) << " dilated hole pixels in the target region." << std::endl;

    // Separate so that only the newly dilated part of the hole remains
    //std::cout << "XORing dilated target mask with target mask..." << std::endl;
    typedef itk::Image<bool, 2> BoolImage;
    BoolImage::Pointer rindImage = BoolImage::New(); // "rind" like an "orange rind"
    ITKHelpers::XORRegions(targetRegionMask.GetPointer(), targetRegionMask->GetLargestPossibleRegion(),
                           dilatedTargetRegionMask.GetPointer(), dilatedTargetRegionMask->GetLargestPossibleRegion(), rindImage.GetPointer());

    //std::cout << "There are " << ITKHelpers::CountPixelsWithValue(rindImage.GetPointer(), true) << " XORed hole pixels in the target region." << std::endl;
    
    std::vector<itk::Index<2> > rindPixels = ITKHelpers::GetPixelsWithValue(rindImage.GetPointer(), rindImage->GetLargestPossibleRegion(), true);
    //std::cout << "There are " << rindPixels.size() << " rindPixels." << std::endl;

    std::vector<itk::Offset<2> > rindOffsets = ITKHelpers::IndicesToOffsets(rindPixels, ITKHelpers::ZeroIndex());
    //std::cout << "There are " << rindOffsets.size() << " rindOffsets." << std::endl;

    //std::cout << "Computing variances..." << std::endl;
    // Compute the variance of the rind pixels in the target region
    std::vector<itk::Index<2> > targetRindPixels = ITKHelpers::OffsetsToIndices(rindOffsets, targetRegion.GetIndex());
    //std::cout << "There are " << targetRindPixels.size() << " targetRindPixels." << std::endl;
    //std::cout << "Computing target variances..." << std::endl;
    typename TImage::PixelType targetRegionSourcePixelVariance = ITKHelpers::VarianceOfPixelsAtIndices(Image, targetRindPixels);
    //std::cout << "targetRegionSourcePixelVariance: " << targetRegionSourcePixelVariance << std::endl;
    
    // Compute the variance of the rind pixels in the source region
    std::vector<itk::Index<2> > sourceRindPixels = ITKHelpers::OffsetsToIndices(rindOffsets, sourceRegion.GetIndex());
    //std::cout << "There are " << sourceRindPixels.size() << " targetRindPixels." << std::endl;
    //std::cout << "Computing source variances..." << std::endl;
    typename TImage::PixelType sourceRegionTargetPixelVariance = ITKHelpers::VarianceOfPixelsAtIndices(Image, sourceRindPixels);
    //std::cout << "sourceRegionTargetPixelVariance: " << sourceRegionTargetPixelVariance << std::endl;
    
    // Compute the difference
    computedEnergy = (targetRegionSourcePixelVariance - sourceRegionTargetPixelVariance).GetNorm();
    std::cout << "DilatedVarianceDifferenceAcceptanceVisitor Energy: " << computedEnergy << std::endl;

    if(computedEnergy < DifferenceThreshold)
      {
      std::cout << "DilatedVarianceDifferenceAcceptanceVisitor: Match accepted (less than " << DifferenceThreshold << ")" << std::endl;
      return true;
      }
    else
      {
      std::cout << "DilatedVarianceDifferenceAcceptanceVisitor: Match rejected (greater than " << DifferenceThreshold << ")" << std::endl;
      return false;
      }
  };

};

#endif
