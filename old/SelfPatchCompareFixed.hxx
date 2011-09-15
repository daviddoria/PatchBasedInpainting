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

#include "Helpers.h"
#include "Types.h"

template< typename TImage>
void SelfPatchCompare<TImage>::SetImage(typename TImage::Pointer image)
{
  this->Image = image;
}

template< typename TImage>
void SelfPatchCompare<TImage>::SetMask(Mask::Pointer mask)
{
  this->MaskImage = mask;
}

template< typename TImage>
void SelfPatchCompare<TImage>::SetTargetRegion(const itk::ImageRegion<2> region)
{
  this->TargetRegion = region;
}

template< typename TImage>
void SelfPatchCompare<TImage>::SetSourceRegions(const std::vector<itk::ImageRegion<2> >& regions)
{
  this->SourceRegions = regions;
}

template< typename TImage>
void SelfPatchCompare<TImage>::ComputeOffsets()
{
  itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, this->TargetRegion);

  while(!maskIterator.IsAtEnd())
    {
    if(maskIterator.Get())
      {
      this->ValidOffsets.push_back(this->Image->ComputeOffset(maskIterator.GetIndex()));
      }

    ++maskIterator;
    }
}
  
template< typename TImage>
float SelfPatchCompare<TImage>::PatchDifference(const itk::ImageRegion<2> sourceRegion)
{
  // This function assumes that all pixels in the source region are unmasked.
  try
  {
    assert(this->Image->GetLargestPossibleRegion().IsInside(sourceRegion));

    itk::ImageRegion<2> newSourceRegion = sourceRegion;
    itk::ImageRegion<2> newTargetRegion = this->TargetRegion;
    
    if(!this->Image->GetLargestPossibleRegion().IsInside(this->TargetRegion))
      {
      // Move the source region to the target region. We move this way because we want to iterate over the mask in the target region.
      itk::Offset<2> sourceTargetOffset = this->TargetRegion.GetIndex() - sourceRegion.GetIndex();
      
      newSourceRegion.SetIndex(sourceRegion.GetIndex() + sourceTargetOffset);

      // Force both regions to be entirely inside the image
      newTargetRegion.Crop(this->Image->GetLargestPossibleRegion());
      newSourceRegion.Crop(this->Image->GetLargestPossibleRegion());

      // Move the source region back to its original position
      newSourceRegion.SetIndex(newSourceRegion.GetIndex() - sourceTargetOffset);
      }

    float totalDifference = 0;

    totalDifference = 0;
    typename TImage::PixelType *buffptr = this->Image->GetBufferPointer();
    unsigned int offsetDifference = this->Image->ComputeOffset(newTargetRegion.GetIndex()) - this->Image->ComputeOffset(newSourceRegion.GetIndex());
    
    for(unsigned int pixelId = 0; pixelId < this->ValidOffsets.size(); ++pixelId)
      {
      typename TImage::PixelType targetPixel = buffptr[this->ValidOffsets[pixelId]];
      typename TImage::PixelType sourcePixel = buffptr[this->ValidOffsets[pixelId] - offsetDifference];
      double difference = Helpers::PixelSquaredDifference(sourcePixel, targetPixel);
      totalDifference += difference;
      }
    
    /*
    if(validPixelCounter == 0)
      {
      std::cerr << "Zero valid pixels in PatchDifference." << std::endl;
      std::cerr << "Source region: " << sourceRegion << std::endl;
      std::cerr << "Target region: " << targetRegion << std::endl;
      std::cerr << "New source region: " << newSourceRegion << std::endl;
      std::cerr << "New target region: " << newTargetRegion << std::endl;
      exit(-1);
      }
    */
    float averageDifference = totalDifference/static_cast<float>(this->ValidOffsets.size());
    return averageDifference;
  } //end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in PatchDifference!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}


template< typename TImage>
unsigned int SelfPatchCompare<TImage>::FindBestPatch()
{
  float minDistance = std::numeric_limits<float>::infinity();
  unsigned int bestMatchId = 0;
  for(unsigned int i = 0; i < this->SourceRegions.size(); i++)
    {
    float distance = PatchDifference(this->SourceRegions[i]);
    //std::cout << "Patch " << i << " distance " << distance << std::endl;
    if(distance < minDistance)
      {
      minDistance = distance;
      bestMatchId = i;
      }
    }

  return bestMatchId;
}
