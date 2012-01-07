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
#include "PatchBasedInpainting.h"

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/HelpersOutput.h"
#include "Helpers/ITKHelpers.h"

// STL
#include <iomanip> // setfill, setw

// ITK
#include "itkMaskImageFilter.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

void PatchBasedInpainting::DebugWriteAllImages()
{
  //HelpersOutput::WriteSequentialImage<FloatScalarImageType>(this->ConfidenceImage, "Debug/ConfidenceImage", this->NumberOfCompletedIterations);
  //HelpersOutput::WriteSequentialImage<FloatScalarImageType>(this->DataImage, "Debug/DataImage", this->NumberOfCompletedIterations);
  //HelpersOutput::WriteSequentialImage<FloatScalarImageType>(this->PriorityImage, "Debug/PriorityImage", this->NumberOfCompletedIterations);

  //Helpers::DebugWriteSequentialImage<FloatVector2ImageType>(this->IsophoteImage, "IsophoteImage", this->NumberOfCompletedIterations);
  //HelpersOutput::Write2DVectorImage(this->IsophoteImage, Helpers::GetSequentialFileName("Debug/IsophoteImage", this->NumberOfCompletedIterations, "mha"));

  //HelpersOutput::WriteSequentialImage<UnsignedCharScalarImageType>(this->BoundaryImage, "Debug/BoundaryImage", this->NumberOfCompletedIterations);

  // Boundary isophotes
  /*
  typedef itk::MaskImageFilter< FloatVector2ImageType, UnsignedCharScalarImageType, FloatVector2ImageType > MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput(this->IsophoteImage);
  maskFilter->SetMaskImage(this->BoundaryImage);
  maskFilter->Update();
  //Helpers::Write2DVectorImage(maskFilter->GetOutput(), Helpers::GetSequentialFileName("BoundaryIsophotes", this->NumberOfCompletedIterations, "mha"));
  vtkSmartPointer<vtkPolyData> boundaryIsophotes = vtkSmartPointer<vtkPolyData>::New();
  Helpers::ConvertNonZeroPixelsToVectors(maskFilter->GetOutput(), boundaryIsophotes);
  HelpersOutput::WritePolyData(boundaryIsophotes, Helpers::GetSequentialFileName("Debug/BoundaryIsophotes", this->NumberOfCompletedIterations, "vtp"));
  */

  // Boundary normals
  //HelpersOutput::Write2DVectorImage(this->BoundaryNormals, Helpers::GetSequentialFileName("Debug/BoundaryNormals", this->NumberOfCompletedIterations, "mha"));
  //Helpers::DebugWriteSequentialImage<FloatVector2ImageType>(this->BoundaryNormals, "BoundaryNormals", this->NumberOfCompletedIterations);

  HelpersOutput::WriteSequentialImage<Mask>(this->MaskImage, "Debug/MaskImage", this->NumberOfCompletedIterations);
  HelpersOutput::WriteSequentialImage<FloatVectorImageType>(this->CurrentInpaintedImage, "Debug/CurrentImage", this->NumberOfCompletedIterations);

  RGBImageType::Pointer rgbImage = RGBImageType::New();
  ITKHelpers::VectorImageToRGBImage(this->CurrentInpaintedImage, rgbImage);
  HelpersOutput::WriteSequentialImage<RGBImageType>(rgbImage, "Debug/CurrentImage_RGB", this->NumberOfCompletedIterations);
}

void PatchBasedInpainting::DebugWriteAllImages(const itk::Index<2>& pixelToFill, const itk::Index<2>& bestMatchPixel, const unsigned int iteration)
{
  std::cout << "Writing debug images for iteration " << iteration << std::endl;

  DebugWritePatch(bestMatchPixel, "BestMatchPatch", iteration);
  std::cout << "Wrote BestMatchPatch.";

  DebugWritePatchToFillLocation(pixelToFill, iteration);
  std::cout << "Wrote pixelToFillLocation." << std::endl;

  DebugWritePixelToFill(pixelToFill, iteration);
  std::cout << "Wrote pixelToFill." << std::endl;

  //HelpersOutput::WriteSequentialImage<FloatVector2ImageType>(this->IsophoteImage,"Debug/Isophotes", iteration);
  //HelpersOutput::WriteSequentialImage<FloatScalarImageType>(this->ConfidenceImage,"Debug/Confidence", iteration);
  //HelpersOutput::WriteSequentialImage<UnsignedCharScalarImageType>(this->BoundaryImage,"Debug/Boundary", iteration);
  //HelpersOutput::WriteSequentialImage<FloatVector2ImageType>(this->BoundaryNormals,"Debug/BoundaryNormals", iteration);
  //HelpersOutput::WriteSequentialImage<FloatScalarImageType>(this->PriorityImage,"Debug/Priorities", iteration);
  HelpersOutput::WriteSequentialImage<Mask>(this->MaskImage,"Debug/Mask", iteration);
  HelpersOutput::WriteSequentialImage<FloatVectorImageType>(this->CurrentInpaintedImage,"Debug/FilledImage", iteration);

}

void PatchBasedInpainting::DebugWritePatch(const itk::Index<2>& pixel, const std::string& filePrefix, const unsigned int iteration)
{
  std::stringstream padded;
  padded << filePrefix << "_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  DebugWritePatch(pixel, padded.str());
}

void PatchBasedInpainting::DebugWritePatch(const itk::ImageRegion<2>& inputRegion, const std::string& filename)
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
    region.Crop(this->CurrentInpaintedImage->GetLargestPossibleRegion());

    ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
    extractFilter->SetRegionOfInterest(region);
    extractFilter->SetInput(this->CurrentInpaintedImage);
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

void PatchBasedInpainting::DebugWritePatch(const itk::Index<2>& pixel, const std::string& filename)
{
  try
  {
    typedef itk::RegionOfInterestImageFilter< FloatVectorImageType,
                                              FloatVectorImageType> ExtractFilterType;

    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(pixel, this->PatchRadius[0]);
    region.Crop(this->CurrentInpaintedImage->GetLargestPossibleRegion());

    ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
    extractFilter->SetRegionOfInterest(region);
    extractFilter->SetInput(this->CurrentInpaintedImage);
    extractFilter->Update();

    HelpersOutput::WriteImage<FloatVectorImageType>(extractFilter->GetOutput(), filename);
  }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in DebugWritePatch!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void PatchBasedInpainting::DebugWritePixelToFill(const itk::Index<2>& pixelToFill, const unsigned int iteration)
{
  // Create a blank image with the pixel to fill colored white
  UnsignedCharScalarImageType::Pointer pixelImage = UnsignedCharScalarImageType::New();
  pixelImage->SetRegions(this->MaskImage->GetLargestPossibleRegion());
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

  HelpersOutput::WriteImage<UnsignedCharScalarImageType>(pixelImage, padded.str());
}

void PatchBasedInpainting::DebugWritePatchToFillLocation(const itk::Index<2>& pixelToFill, const unsigned int iteration)
{
  // Create a blank image with the patch that has been filled colored white
  UnsignedCharScalarImageType::Pointer patchImage = UnsignedCharScalarImageType::New();
  patchImage->SetRegions(this->MaskImage->GetLargestPossibleRegion());
  patchImage->Allocate();
  // Make image black
  itk::ImageRegionIterator<UnsignedCharScalarImageType> blackIterator(patchImage, patchImage->GetLargestPossibleRegion());

  while(!blackIterator.IsAtEnd())
    {
    blackIterator.Set(0);
    ++blackIterator;
    }

  UnsignedCharScalarImageType::Pointer patch = UnsignedCharScalarImageType::New();
  ITKHelpers::CreateConstantPatch<UnsignedCharScalarImageType>(patch, 255, this->PatchRadius[0]);

  ITKHelpers::CopyPatchIntoImage<UnsignedCharScalarImageType>(patch, patchImage, pixelToFill);

  std::stringstream padded;
  padded << "PatchToFillLocation_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  HelpersOutput::WriteImage<UnsignedCharScalarImageType>(patchImage, padded.str());
}
/*
void WriteImageOfScores(const CandidatePairs& candidatePairs, const itk::ImageRegion<2>& imageRegion, const std::string& fileName)
{
  // Create the score-colored image
  FloatScalarImageType::Pointer scoreColoredImage = FloatScalarImageType::New();
  Helpers::InitializeImage<FloatScalarImageType>(scoreColoredImage, imageRegion);

  // Find max value (worst score)
  float worstScore = 0.0f;
  for(unsigned int pairId = 0; pairId < candidatePairs.GetNumberOfSourcePatches(); ++pairId)
    {
    float depthDifference = candidatePairs.GetPair(pairId).GetDifferenceMap.find(PatchPair::DepthDifference)->second;
    if(depthDifference > worstScore)
      {
      worstScore = candidatePairs[pairId].DifferenceMap.find(PatchPair::DepthDifference)->second;
      }
    }
  Helpers::SetImageToConstant<FloatScalarImageType>(scoreColoredImage, worstScore);
  Helpers::SetRegionToConstant<FloatScalarImageType>(scoreColoredImage, candidatePairs.GetTargetPatch().Region, 0.0f);

  for(unsigned int pairId = 0; pairId < candidatePairs.GetNumberOfSourcePatches(); ++pairId)
    {
    scoreColoredImage->SetPixel(Helpers::GetRegionCenter(candidatePairs[pairId].SourcePatch.Region),
                                candidatePairs[pairId].DifferenceMap.find(PatchPair::DepthDifference)->second);
    }

  HelpersOutput::WriteImage<FloatScalarImageType>(scoreColoredImage, fileName);
}*/
