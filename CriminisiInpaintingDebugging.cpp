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

// Class declaration
#include "CriminisiInpainting.h"

// Custom
#include "Helpers.h"

// STL
#include <iomanip> // setfill, setw

// ITK
#include "itkMaskImageFilter.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

void CriminisiInpainting::DebugWriteAllImages()
{
  Helpers::DebugWriteSequentialImage<FloatScalarImageType>(this->ConfidenceImage, "ConfidenceImage", this->NumberOfCompletedIterations);
  Helpers::DebugWriteSequentialImage<FloatScalarImageType>(this->DataImage, "DataImage", this->NumberOfCompletedIterations);
  Helpers::DebugWriteSequentialImage<FloatScalarImageType>(this->PriorityImage, "PriorityImage", this->NumberOfCompletedIterations);
  
  //Helpers::DebugWriteSequentialImage<FloatVector2ImageType>(this->IsophoteImage, "IsophoteImage", this->NumberOfCompletedIterations);
  Helpers::Write2DVectorImage(this->IsophoteImage, Helpers::GetSequentialFileName("IsophoteImage", this->NumberOfCompletedIterations, "mha"));
  
  Helpers::DebugWriteSequentialImage<UnsignedCharScalarImageType>(this->BoundaryImage, "BoundaryImage", this->NumberOfCompletedIterations);

  // Boundary isophotes
  typedef itk::MaskImageFilter< FloatVector2ImageType, UnsignedCharScalarImageType, FloatVector2ImageType > MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput(this->IsophoteImage);
  maskFilter->SetMaskImage(this->BoundaryImage);
  maskFilter->Update();
  //Helpers::Write2DVectorImage(maskFilter->GetOutput(), Helpers::GetSequentialFileName("BoundaryIsophotes", this->NumberOfCompletedIterations, "mha"));
  vtkSmartPointer<vtkPolyData> boundaryIsophotes = vtkSmartPointer<vtkPolyData>::New();
  Helpers::ConvertNonZeroPixelsToVectors(maskFilter->GetOutput(), boundaryIsophotes);
  Helpers::WritePolyData(boundaryIsophotes, Helpers::GetSequentialFileName("BoundaryIsophotes", this->NumberOfCompletedIterations, "vtp"));

  // Boundary normals
  Helpers::Write2DVectorImage(this->BoundaryNormals, Helpers::GetSequentialFileName("BoundaryNormals", this->NumberOfCompletedIterations, "mha"));
  //Helpers::DebugWriteSequentialImage<FloatVector2ImageType>(this->BoundaryNormals, "BoundaryNormals", this->NumberOfCompletedIterations);
  
  Helpers::DebugWriteSequentialImage<Mask>(this->CurrentMask, "CurrentMask", this->NumberOfCompletedIterations);
  Helpers::DebugWriteSequentialImage<FloatVectorImageType>(this->CurrentOutputImage, "CurrentImage", this->NumberOfCompletedIterations);

  RGBImageType::Pointer rgbImage = RGBImageType::New();
  Helpers::VectorImageToRGBImage(this->CurrentOutputImage, rgbImage);
  Helpers::DebugWriteSequentialImage<RGBImageType>(rgbImage, "CurrentImage_RGB", this->NumberOfCompletedIterations);
}

void CriminisiInpainting::DebugWriteAllImages(const itk::Index<2>& pixelToFill, const itk::Index<2>& bestMatchPixel, const unsigned int iteration)
{
  std::cout << "Writing debug images for iteration " << iteration << std::endl;

  DebugWritePatch(bestMatchPixel, "BestMatchPatch", iteration);
  std::cout << "Wrote BestMatchPatch.";

  DebugWritePatchToFillLocation(pixelToFill, iteration);
  std::cout << "Wrote pixelToFillLocation." << std::endl;

  DebugWritePixelToFill(pixelToFill, iteration);
  std::cout << "Wrote pixelToFill." << std::endl;

  Helpers::DebugWriteSequentialImage<FloatVector2ImageType>(this->IsophoteImage,"Isophotes", iteration);
  Helpers::DebugWriteSequentialImage<FloatScalarImageType>(this->ConfidenceImage,"Confidence", iteration);
  Helpers::DebugWriteSequentialImage<UnsignedCharScalarImageType>(this->BoundaryImage,"Boundary", iteration);
  Helpers::DebugWriteSequentialImage<FloatVector2ImageType>(this->BoundaryNormals,"BoundaryNormals", iteration);
  Helpers::DebugWriteSequentialImage<FloatScalarImageType>(this->PriorityImage,"Priorities", iteration);
  Helpers::DebugWriteSequentialImage<Mask>(this->CurrentMask,"Mask", iteration);
  Helpers::DebugWriteSequentialImage<FloatVectorImageType>(this->CurrentOutputImage,"FilledImage", iteration);

}

void CriminisiInpainting::DebugWritePatch(const itk::Index<2>& pixel, const std::string& filePrefix, const unsigned int iteration)
{
  std::stringstream padded;
  padded << filePrefix << "_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  DebugWritePatch(pixel, padded.str());
}

void CriminisiInpainting::DebugWritePatch(const itk::ImageRegion<2>& inputRegion, const std::string& filename)
{
  if(!this->DebugImages)
    {
    return;
    }
  try
  {
    typedef itk::RegionOfInterestImageFilter< FloatVectorImageType,
					      FloatVectorImageType> ExtractFilterType;
    itk::ImageRegion<2> region = inputRegion;
    region.Crop(this->CurrentOutputImage->GetLargestPossibleRegion());

    ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
    extractFilter->SetRegionOfInterest(region);
    extractFilter->SetInput(this->CurrentOutputImage);
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

void CriminisiInpainting::DebugWritePatch(const itk::Index<2>& pixel, const std::string& filename)
{
  try
  {
    typedef itk::RegionOfInterestImageFilter< FloatVectorImageType,
					      FloatVectorImageType> ExtractFilterType;

    itk::ImageRegion<2> region = Helpers::GetRegionInRadiusAroundPixel(pixel, this->PatchRadius[0]);
    region.Crop(this->CurrentOutputImage->GetLargestPossibleRegion());

    ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
    extractFilter->SetRegionOfInterest(region);
    extractFilter->SetInput(this->CurrentOutputImage);
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

void CriminisiInpainting::DebugWritePixelToFill(const itk::Index<2>& pixelToFill, const unsigned int iteration)
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

void CriminisiInpainting::DebugWritePatchToFillLocation(const itk::Index<2>& pixelToFill, const unsigned int iteration)
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
  if(this->DebugMessages)
    {
    std::cout << message << std::endl;
    }
}
