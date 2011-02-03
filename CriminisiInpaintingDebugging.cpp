/*=========================================================================
 *
 *  Copyright David Doria 2010 daviddoria@gmail.com
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

#include "CriminisiInpainting.h"

#include "Helpers.h"

#include <iomanip> // setfill, setw

template <class TImage>
void CriminisiInpainting<TImage>::DebugWriteAllImages(itk::Index<2> pixelToFill, itk::Index<2> bestMatchPixel, unsigned int iteration)
{
  std::cout << "Writing debug images for iteration " << iteration << std::endl;

  DebugWritePatch(bestMatchPixel, "BestMatchPatch", iteration);
  std::cout << "Wrote BestMatchPatch.";

  DebugWritePatchToFillLocation(pixelToFill, iteration);
  std::cout << "Wrote pixelToFillLocation." << std::endl;

  DebugWritePixelToFill(pixelToFill, iteration);
  std::cout << "Wrote pixelToFill." << std::endl;

  DebugWriteImage<VectorImageType>(this->IsophoteImage,"Isophotes", iteration);
  DebugWriteImage<FloatImageType>(this->ConfidenceImage,"Confidence", iteration);
  DebugWriteImage<UnsignedCharImageType>(this->BoundaryImage,"Boundary", iteration);
  DebugWriteImage<VectorImageType>(this->BoundaryNormals,"BoundaryNormals", iteration);
  DebugWriteImage<FloatImageType>(this->PriorityImage,"Priorities", iteration);
  DebugWriteImage<UnsignedCharImageType>(this->Mask,"Mask", iteration);
  DebugWriteImage<TImage>(this->Image,"FilledImage", iteration);

}



template <class TImage>
void CriminisiInpainting<TImage>::DebugWritePatch(itk::Index<2> pixel, std::string filePrefix, unsigned int iteration)
{
  std::stringstream padded;
  padded << filePrefix << "_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  DebugWritePatch(pixel, padded.str());
}

template <class TImage>
void CriminisiInpainting<TImage>::DebugWritePatch(itk::Index<2> pixel, std::string filename)
{
  typedef itk::RegionOfInterestImageFilter< TImage,
                                            TImage> ExtractFilterType;

  typename ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
  extractFilter->SetRegionOfInterest(GetRegionInRadiusAroundPixel(pixel, this->PatchRadius[0]));
  extractFilter->SetInput(this->Image);
  extractFilter->Update();

  WriteImage<TImage>(extractFilter->GetOutput(), filename);

}

template <class TImage>
void CriminisiInpainting<TImage>::DebugWritePixelToFill(itk::Index<2> pixelToFill, unsigned int iteration)
{
  // Create a blank image with the pixel to fill colored white
  UnsignedCharImageType::Pointer pixelImage = UnsignedCharImageType::New();
  pixelImage->SetRegions(this->Mask->GetLargestPossibleRegion());
  pixelImage->Allocate();

  itk::ImageRegionIterator<UnsignedCharImageType> iterator(pixelImage, pixelImage->GetLargestPossibleRegion());

  while(!iterator.IsAtEnd())
    {
    if(iterator.GetIndex()[0] == pixelToFill[0] && iterator.GetIndex()[1] == pixelToFill[1])
      {
      iterator.Set(255);
      }
    else
      {
      iterator.Set(0);
      }
    ++iterator;
    }


  std::stringstream padded;
  padded << "PixelToFill_" << std::setfill('0') << std::setw(4) << iteration << ".jpg";

  WriteImage<UnsignedCharImageType>(pixelImage, padded.str());
}

template <class TImage>
void CriminisiInpainting<TImage>::DebugWritePatchToFillLocation(itk::Index<2> pixelToFill, unsigned int iteration)
{
  // Create a blank image with the patch that has been filled colored white
  UnsignedCharImageType::Pointer patchImage = UnsignedCharImageType::New();
  patchImage->SetRegions(this->Mask->GetLargestPossibleRegion());
  patchImage->Allocate();
  // Make image black
  itk::ImageRegionIterator<UnsignedCharImageType> blackIterator(patchImage, patchImage->GetLargestPossibleRegion());

  while(!blackIterator.IsAtEnd())
    {
    blackIterator.Set(0);
    ++blackIterator;
    }

  UnsignedCharImageType::Pointer patch = UnsignedCharImageType::New();
  CreateConstantPatch<UnsignedCharImageType>(patch, 255, this->PatchRadius[0]);

  CopyPatchIntoImage<UnsignedCharImageType>(patch, patchImage, pixelToFill);

  std::stringstream padded;
  padded << "PatchToFillLocation_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  WriteImage<UnsignedCharImageType>(patchImage, padded.str());
}

template class CriminisiInpainting<ColorImageType>;
template class CriminisiInpainting<RGBDIImageType>;
