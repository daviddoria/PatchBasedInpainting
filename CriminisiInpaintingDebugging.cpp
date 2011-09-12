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

#include "CriminisiInpainting.h"

#include "Helpers.h"

#include <iomanip> // setfill, setw

void CriminisiInpainting::DebugWriteAllImages()
{
  Helpers::DebugWriteImage<FloatScalarImageType>(this->ConfidenceImage, "ConfidenceImage", this->Iteration);
  Helpers::DebugWriteImage<FloatScalarImageType>(this->DataImage, "DataImage", this->Iteration);
  Helpers::DebugWriteImage<FloatScalarImageType>(this->PriorityImage, "PriorityImage", this->Iteration);
  
  Helpers::DebugWriteImage<FloatVector2ImageType>(this->IsophoteImage, "IsophoteImage", this->Iteration);
  
  Helpers::DebugWriteImage<UnsignedCharScalarImageType>(this->BoundaryImage, "BoundaryImage", this->Iteration);
  Helpers::DebugWriteImage<FloatVector2ImageType>(this->BoundaryNormals, "BoundaryNormals", this->Iteration);
  
  Helpers::DebugWriteImage<Mask>(this->CurrentMask, "CurrentMask", this->Iteration);
  Helpers::DebugWriteImage<FloatVectorImageType>(this->CurrentImage, "CurrentImage", this->Iteration);
}

void CriminisiInpainting::DebugWriteAllImages(const itk::Index<2> pixelToFill, const itk::Index<2> bestMatchPixel, const unsigned int iteration)
{
  std::cout << "Writing debug images for iteration " << iteration << std::endl;

  DebugWritePatch(bestMatchPixel, "BestMatchPatch", iteration);
  std::cout << "Wrote BestMatchPatch.";

  DebugWritePatchToFillLocation(pixelToFill, iteration);
  std::cout << "Wrote pixelToFillLocation." << std::endl;

  DebugWritePixelToFill(pixelToFill, iteration);
  std::cout << "Wrote pixelToFill." << std::endl;

  Helpers::DebugWriteImage<FloatVector2ImageType>(this->IsophoteImage,"Isophotes", iteration);
  Helpers::DebugWriteImage<FloatScalarImageType>(this->ConfidenceImage,"Confidence", iteration);
  Helpers::DebugWriteImage<UnsignedCharScalarImageType>(this->BoundaryImage,"Boundary", iteration);
  Helpers::DebugWriteImage<FloatVector2ImageType>(this->BoundaryNormals,"BoundaryNormals", iteration);
  Helpers::DebugWriteImage<FloatScalarImageType>(this->PriorityImage,"Priorities", iteration);
  Helpers::DebugWriteImage<Mask>(this->CurrentMask,"Mask", iteration);
  Helpers::DebugWriteImage<FloatVectorImageType>(this->CurrentImage,"FilledImage", iteration);

}

void CriminisiInpainting::DebugWritePatch(const itk::Index<2> pixel, const std::string filePrefix, const unsigned int iteration)
{
  std::stringstream padded;
  padded << filePrefix << "_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  DebugWritePatch(pixel, padded.str());
}

void CriminisiInpainting::DebugWritePatch(const itk::ImageRegion<2> inputRegion, const std::string filename)
{
  if(!this->Debug)
    {
    return;
    }
  try
  {
    typedef itk::RegionOfInterestImageFilter< FloatVectorImageType,
					      FloatVectorImageType> ExtractFilterType;
    itk::ImageRegion<2> region = inputRegion;
    region.Crop(this->CurrentImage->GetLargestPossibleRegion());

    ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
    extractFilter->SetRegionOfInterest(region);
    extractFilter->SetInput(this->CurrentImage);
    extractFilter->Update();
    /*
    typedef itk::Image<itk::CovariantVector<unsigned char, TImage::PixelType::Dimension>, 2> OutputImageType;

    typedef itk::CastImageFilter< TImage, OutputImageType > CastFilterType;
    typename CastFilterType::Pointer castFilter = CastFilterType::New();
    castFilter->SetInput(extractFilter->GetOutput());
    castFilter->Update();

    Helpers::WriteImage<OutputImageType>(castFilter->GetOutput(), filename);
    */
  }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in DebugWritePatch!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void CriminisiInpainting::DebugWritePatch(const itk::Index<2> pixel, const std::string filename)
{
  try
  {
    typedef itk::RegionOfInterestImageFilter< FloatVectorImageType,
					      FloatVectorImageType> ExtractFilterType;

    itk::ImageRegion<2> region = Helpers::GetRegionInRadiusAroundPixel(pixel, this->PatchRadius[0]);
    region.Crop(this->CurrentImage->GetLargestPossibleRegion());

    ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
    extractFilter->SetRegionOfInterest(region);
    extractFilter->SetInput(this->CurrentImage);
    extractFilter->Update();

    Helpers::WriteImage<FloatVectorImageType>(extractFilter->GetOutput(), filename);
  }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in DebugWritePatch!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void CriminisiInpainting::DebugWritePixelToFill(const itk::Index<2> pixelToFill, const unsigned int iteration)
{
  // Create a blank image with the pixel to fill colored white
  UnsignedCharScalarImageType::Pointer pixelImage = UnsignedCharScalarImageType::New();
  pixelImage->SetRegions(this->CurrentMask->GetLargestPossibleRegion());
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

void CriminisiInpainting::DebugWritePatchToFillLocation(const itk::Index<2> pixelToFill, const unsigned int iteration)
{
  // Create a blank image with the patch that has been filled colored white
  UnsignedCharScalarImageType::Pointer patchImage = UnsignedCharScalarImageType::New();
  patchImage->SetRegions(this->CurrentMask->GetLargestPossibleRegion());
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


void CriminisiInpainting::DebugMessage(const std::string& message)
{
  if(this->Debug)
    {
    std::cout << message << std::endl;
    }
}
