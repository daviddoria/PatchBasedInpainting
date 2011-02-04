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
template <typename TDebugImageType>
void CriminisiInpainting<TImage>::DebugWriteImage(typename TDebugImageType::Pointer image, std::string filePrefix, unsigned int iteration)
{
  std::stringstream padded;
  padded << filePrefix << "_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  Helpers::WriteImage<TDebugImageType>(image, padded.str());
}

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

  DebugWriteImage<FloatVector2ImageType>(this->IsophoteImage,"Isophotes", iteration);
  DebugWriteImage<FloatScalarImageType>(this->ConfidenceImage,"Confidence", iteration);
  DebugWriteImage<UnsignedCharScalarImageType>(this->BoundaryImage,"Boundary", iteration);
  DebugWriteImage<FloatVector2ImageType>(this->BoundaryNormals,"BoundaryNormals", iteration);
  DebugWriteImage<FloatScalarImageType>(this->PriorityImage,"Priorities", iteration);
  DebugWriteImage<MaskImageType>(this->Mask,"Mask", iteration);
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
void CriminisiInpainting<TImage>::DebugWritePatch(itk::ImageRegion<2> region, std::string filename)
{
 try
  {
  typedef itk::RegionOfInterestImageFilter< TImage,
                                            TImage> ExtractFilterType;

  region.Crop(this->Image->GetLargestPossibleRegion());

  typename ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
  extractFilter->SetRegionOfInterest(region);
  extractFilter->SetInput(this->Image);
  extractFilter->Update();

  typedef itk::Image<itk::CovariantVector<unsigned char, TImage::PixelType::Dimension>, 2> OutputImageType;

  typedef itk::CastImageFilter< TImage, OutputImageType > CastFilterType;
  typename CastFilterType::Pointer castFilter = CastFilterType::New();
  castFilter->SetInput(extractFilter->GetOutput());
  castFilter->Update();

  Helpers::WriteImage<OutputImageType>(castFilter->GetOutput(), filename);
  }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in DebugWritePatch!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <class TImage>
void CriminisiInpainting<TImage>::DebugWritePatch(itk::Index<2> pixel, std::string filename)
{
  try
  {
  typedef itk::RegionOfInterestImageFilter< TImage,
                                            TImage> ExtractFilterType;

  itk::ImageRegion<2> region = Helpers::GetRegionInRadiusAroundPixel(pixel, this->PatchRadius[0]);
  region.Crop(this->Image->GetLargestPossibleRegion());

  typename ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
  extractFilter->SetRegionOfInterest(region);
  extractFilter->SetInput(this->Image);
  extractFilter->Update();

  Helpers::WriteImage<TImage>(extractFilter->GetOutput(), filename);
  }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in DebugWritePatch!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <class TImage>
void CriminisiInpainting<TImage>::DebugWritePixelToFill(itk::Index<2> pixelToFill, unsigned int iteration)
{
  // Create a blank image with the pixel to fill colored white
  UnsignedCharScalarImageType::Pointer pixelImage = UnsignedCharScalarImageType::New();
  pixelImage->SetRegions(this->Mask->GetLargestPossibleRegion());
  pixelImage->Allocate();

  itk::ImageRegionIterator<UnsignedCharScalarImageType> iterator(pixelImage, pixelImage->GetLargestPossibleRegion());

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

  Helpers::WriteImage<UnsignedCharScalarImageType>(pixelImage, padded.str());
}

template <class TImage>
void CriminisiInpainting<TImage>::DebugWritePatchToFillLocation(itk::Index<2> pixelToFill, unsigned int iteration)
{
  // Create a blank image with the patch that has been filled colored white
  UnsignedCharScalarImageType::Pointer patchImage = UnsignedCharScalarImageType::New();
  patchImage->SetRegions(this->Mask->GetLargestPossibleRegion());
  patchImage->Allocate();
  // Make image black
  itk::ImageRegionIterator<UnsignedCharScalarImageType> blackIterator(patchImage, patchImage->GetLargestPossibleRegion());

  while(!blackIterator.IsAtEnd())
    {
    blackIterator.Set(0);
    ++blackIterator;
    }

  UnsignedCharScalarImageType::Pointer patch = UnsignedCharScalarImageType::New();
  Helpers::CreateConstantPatch<UnsignedCharScalarImageType>(patch, 255, this->PatchRadius[0]);

  Helpers::CopyPatchIntoImage<UnsignedCharScalarImageType>(patch, patchImage, pixelToFill);

  std::stringstream padded;
  padded << "PatchToFillLocation_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  Helpers::WriteImage<UnsignedCharScalarImageType>(patchImage, padded.str());
}
