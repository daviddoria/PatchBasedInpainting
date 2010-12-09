/*
Copyright (C) 2010 David Doria, daviddoria@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "CriminisiInpainting.h"

#include "Helpers.h"

#include <iomanip> // detfill, setw


  
void CriminisiInpainting::DebugWriteAllImages(itk::Index<2> pixelToFill, unsigned int iteration)
{
  std::cout << "Writing debug images for pixelToFill: " << pixelToFill << std::endl;

  DebugWritePatchToFill(pixelToFill, iteration);
  DebugWritePatchToFillLocation(pixelToFill, iteration);
  
  DebugWritePixelToFill(pixelToFill, iteration);

  {
  std::stringstream padded;
  padded << "BestPatch_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  WriteImage<ColorImageType>(this->Patch, padded.str());
  }

  {
  std::stringstream padded;
  padded << "Isophotes" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  WriteImage<VectorImageType>(this->IsophoteImage, padded.str());
  }

  {
  std::stringstream padded;
  padded << "Confidence_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  WriteImage<FloatImageType>(this->ConfidenceImage, padded.str());
  }

  {
  std::stringstream padded;
  padded << "Boundary_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  WriteImage<UnsignedCharImageType>(this->BoundaryImage, padded.str());
  }

  {
  std::stringstream padded;
  padded << "BoundaryNormals_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  WriteImage<VectorImageType>(this->BoundaryNormals, padded.str());
  }

  {
  std::stringstream padded;
  padded << "Priorities_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  WriteImage<FloatImageType>(this->PriorityImage, padded.str());
  }

  {
  std::stringstream padded;
  padded << "Difference_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  WriteImage<FloatImageType>(this->MeanDifferenceImage, padded.str());
  }

  {
  std::stringstream padded;
  padded << "Mask_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  WriteImage<UnsignedCharImageType>(this->Mask, padded.str());
  }

  {
  std::stringstream padded;
  padded << "FilledImage_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  WriteImage<ColorImageType>(this->Image, padded.str());
  }
}

void CriminisiInpainting::DebugWritePatchToFill(itk::Index<2> pixelToFill, unsigned int iteration)
{
  typedef itk::RegionOfInterestImageFilter< ColorImageType,
                                            ColorImageType > ExtractFilterType;

  ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
  extractFilter->SetRegionOfInterest(GetRegionAroundPixel(pixelToFill, this->PatchRadius[0]));
  extractFilter->SetInput(this->Image);
  extractFilter->Update();

  std::stringstream padded;
  padded << "PatchToFill_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  WriteImage<ColorImageType>(extractFilter->GetOutput(), padded.str());
}

void CriminisiInpainting::DebugWritePixelToFill(itk::Index<2> pixelToFill, unsigned int iteration)
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

void CriminisiInpainting::DebugWritePatchToFillLocation(itk::Index<2> pixelToFill, unsigned int iteration)
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