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

#include "RotateVectors.h"
#include "SelfPatchMatch.h"
#include "Helpers.h"

#include <iostream>
#include <iomanip> // setfill and setw

#include <vnl/vnl_double_2.h>

CriminisiInpainting::CriminisiInpainting()
{
  this->PatchRadius.Fill(3);

  this->BoundaryImage = UnsignedCharScalarImageType::New();
  this->BoundaryNormals = FloatVector2ImageType::New();
  this->IsophoteImage = FloatVector2ImageType::New();
  this->PriorityImage = FloatScalarImageType::New();
  this->OriginalMask = MaskImageType::New();
  this->CurrentMask = MaskImageType::New();
  this->OriginalImage = FloatVectorImageType::New();
  this->CurrentImage = FloatVectorImageType::New();
  //this->Patch = ColorImageType::New();
  this->ConfidenceImage = FloatScalarImageType::New();
  
  this->Debug = true;
}

FloatVectorImageType::Pointer CriminisiInpainting::GetResult()
{
  return this->CurrentImage;
}

FloatScalarImageType::Pointer CriminisiInpainting::GetPriorityImage()
{
    return this->PriorityImage;
}

FloatScalarImageType::Pointer CriminisiInpainting::GetConfidenceImage()
{
  return this->ConfidenceImage;
}

UnsignedCharScalarImageType::Pointer CriminisiInpainting::GetBoundaryImage()
{
  return this->BoundaryImage;
}

void CriminisiInpainting::ComputeSourcePatches()
{
  //std::cout << "ComputeSourcePatches(): OriginalImage: " << this->OriginalImage->GetLargestPossibleRegion() << std::endl;
  try
  {
  itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(this->OriginalImage, this->OriginalImage->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    itk::ImageRegion<2> region = Helpers::GetRegionInRadiusAroundPixel(imageIterator.GetIndex(), this->PatchRadius[0]);
    if(IsValidPatch(region))
      {
      this->SourcePatches.push_back(region);
      }

    ++imageIterator;
    }
  std::cout << "There are " << this->SourcePatches.size() << " source patches." << std::endl;
  }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeSourcePatches!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }

}

void CriminisiInpainting::SetPatchRadius(unsigned int radius)
{
  // Since this is the radius of the patch, there are no restrictions for the radius to be odd or even.
  this->PatchRadius.Fill(radius);
}

void CriminisiInpainting::SetImage(FloatVectorImageType::Pointer image)
{
  // Store the original image
  Helpers::DeepCopyVectorImage<FloatVectorImageType>(image, this->OriginalImage);
  
  // Initialize the result to the original image
  Helpers::DeepCopyVectorImage<FloatVectorImageType>(image, this->CurrentImage);
}

void CriminisiInpainting::SetMask(MaskImageType::Pointer mask)
{
  // Initialize the CurrentMask to the OriginalMask
  Helpers::DeepCopy<UnsignedCharScalarImageType>(mask, this->CurrentMask);
  
  // Save the OriginalMask.
  Helpers::DeepCopy<UnsignedCharScalarImageType>(mask, this->OriginalMask);
}

void CriminisiInpainting::SetDebug(bool flag)
{
  this->Debug = flag;
}

void CriminisiInpainting::ExpandMask()
{
  // Ensure the mask is actually binary by forcing values in the range (122, 255) to 255 and
  // values in the range (0, 122) to 0.
  typedef itk::BinaryThresholdImageFilter <MaskImageType, MaskImageType> BinaryThresholdImageFilterType;

  BinaryThresholdImageFilterType::Pointer thresholdFilter = BinaryThresholdImageFilterType::New();
  thresholdFilter->SetInput(this->OriginalMask);
  thresholdFilter->SetLowerThreshold(122);
  thresholdFilter->SetUpperThreshold(255);
  thresholdFilter->SetInsideValue(255);
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->InPlaceOn();
  thresholdFilter->Update();

  // Expand the mask - this is necessary to prevent the isophotes from being undefined in the target region
  typedef itk::FlatStructuringElement<2> StructuringElementType;
  StructuringElementType::RadiusType radius;
  radius.Fill(this->PatchRadius[0]);
  //radius.Fill(2.0* this->PatchRadius[0]);

  StructuringElementType structuringElement = StructuringElementType::Box(radius);
  typedef itk::BinaryDilateImageFilter<MaskImageType, MaskImageType, StructuringElementType> BinaryDilateImageFilterType;

  BinaryDilateImageFilterType::Pointer expandMaskFilter = BinaryDilateImageFilterType::New();
  expandMaskFilter->SetInput(thresholdFilter->GetOutput());
  expandMaskFilter->SetKernel(structuringElement);
  expandMaskFilter->Update();
  
  Helpers::DeepCopy<MaskImageType>(expandMaskFilter->GetOutput(), this->CurrentMask);

  //WriteScaledImage<UnsignedCharImageType>(this->Mask, "expandedMask.mhd");
}

void CriminisiInpainting::Initialize()
{
  try
  {
  
  ExpandMask();
  
  // Do this before we mask the image with the expanded mask
  ComputeIsophotes();

  // Create a blank priority image
  this->PriorityImage->SetRegions(this->OriginalImage->GetLargestPossibleRegion());
  this->PriorityImage->Allocate();

  // Clone the mask - we need to invert the mask to actually perform the masking, but we don't want to disturb the original mask
  UnsignedCharScalarImageType::Pointer maskClone = UnsignedCharScalarImageType::New();
  Helpers::DeepCopy<UnsignedCharScalarImageType>(this->CurrentMask, maskClone);
  
  // Invert the mask
  typedef itk::InvertIntensityImageFilter <MaskImageType> InvertIntensityImageFilterType;

  InvertIntensityImageFilterType::Pointer invertIntensityFilter = InvertIntensityImageFilterType::New();
  invertIntensityFilter->SetInput(maskClone);
  //invertIntensityFilter->InPlaceOn();
  invertIntensityFilter->Update();

  // Convert the inverted mask to floats and scale them to between 0 and 1
  // to serve as the initial confidence image
  typedef itk::RescaleIntensityImageFilter< MaskImageType, FloatScalarImageType > RescaleFilterType;
  RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetInput(invertIntensityFilter->GetOutput());
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(1);
  rescaleFilter->Update();

  Helpers::DeepCopy<FloatScalarImageType>(rescaleFilter->GetOutput(), this->ConfidenceImage);
  //WriteImage<FloatImageType>(this->ConfidenceImage, "InitialConfidence.mhd");

  // Mask the input image with the inverted mask (blank the region in the input image that we will fill in)
  typedef itk::MaskImageFilter< FloatVectorImageType, MaskImageType, FloatVectorImageType> MaskFilterType;
  typename MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput1(this->OriginalImage);
  maskFilter->SetInput2(invertIntensityFilter->GetOutput());

  // We set non-masked pixels to green so we can visually ensure these pixels are not being copied during the inpainting
  FloatVectorImageType::PixelType green;
  green.SetSize(this->OriginalImage->GetNumberOfComponentsPerPixel());
  green.Fill(0);
  green[1] = 255;
  
  maskFilter->SetOutsideValue(green);
  maskFilter->Update();

  //this->Image->Graft(maskFilter->GetOutput());
  Helpers::DeepCopyVectorImage<FloatVectorImageType>(maskFilter->GetOutput(), this->CurrentImage);

  // Debugging outputs
  //WriteImage<TImage>(this->Image, "InitialImage.mhd");
  //WriteImage<UnsignedCharImageType>(this->Mask, "InitialMask.mhd");
  //WriteImage<FloatImageType>(this->ConfidenceImage, "InitialConfidence.mhd");
  //WriteImage<VectorImageType>(this->IsophoteImage, "InitialIsophotes.mhd");
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in Initialize!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void CriminisiInpainting::Inpaint()
{
  std::cout << "CriminisiInpainting::Inpaint()" << std::endl;
  try
  {
  Initialize();
  ComputeSourcePatches();

  int iteration = 0;
  while(HasMoreToInpaint(this->CurrentMask))
    {
    std::cout << "Iteration: " << iteration << std::endl;

    FindBoundary();
    DebugMessage("Found boundary.");
  
    ComputeBoundaryNormals();
    DebugMessage("Computed boundary normals.");
  
    ComputeAllPriorities();
    DebugMessage("Computed priorities.");
    
    itk::Index<2> pixelToFill = FindHighestPriority(this->PriorityImage);
    DebugMessage<itk::Index<2> >("Highest priority found to be ", pixelToFill);
    //std::cout << "Filling: " << pixelToFill << std::endl;

    this->DebugWritePatch(pixelToFill, "PatchToFill", iteration); // this must be done here because it is before the filling

    //itk::Index<2> bestMatchPixel = SelfPatchMatch<TImage, UnsignedCharImageType>(this->Image, this->Mask, pixelToFill, this->PatchRadius[0], iteration, this->Weights);
    //std::cout << "Best match pixel: " << bestMatchPixel << std::endl;

    itk::ImageRegion<2> targetRegion = Helpers::GetRegionInRadiusAroundPixel(pixelToFill, this->PatchRadius[0]);

    DebugMessage("Finding best patch...");
    unsigned int bestMatchSourcePatchId = BestPatch<FloatVectorImageType, MaskImageType>(this->CurrentImage, this->CurrentMask, this->SourcePatches, targetRegion);
    DebugMessage<unsigned int>("Found best patch to be ", bestMatchSourcePatchId);
    
    this->DebugWritePatch(this->SourcePatches[bestMatchSourcePatchId], "SourcePatch.png");
    this->DebugWritePatch(targetRegion, "TargetPatch.png");
    //std::cout << "Best match source patch id: " << bestMatchSourcePatchId << std::endl;

    //Helpers::CopySelfPatchIntoValidRegion<TImage>(this->Image, this->Mask, bestMatchPixel, pixelToFill, this->PatchRadius[0]);
    //Helpers::CopySelfPatchIntoValidRegion<FloatImageType>(this->ConfidenceImage, this->Mask, bestMatchPixel, pixelToFill, this->PatchRadius[0]);
    //Helpers::CopySelfPatchIntoValidRegion<VectorImageType>(this->IsophoteImage, this->Mask, bestMatchPixel, pixelToFill, this->PatchRadius[0]);

    // Copy the patch. This is the actual inpainting step.
    Helpers::CopySelfPatchIntoValidRegion<FloatVectorImageType>(this->CurrentImage, this->CurrentMask, this->SourcePatches[bestMatchSourcePatchId], targetRegion);
    
    // TODO: I think this is wrong - this will result in only 0's and 1's. Instead we should set the confidence for all of the pixels that the confidnce changed in the last iteration to the values that they changed to.
    //Helpers::CopySelfPatchIntoValidRegion<FloatScalarImageType>(this->ConfidenceImage, this->CurrentMask, this->SourcePatches[bestMatchSourcePatchId], targetRegion);
    UpdateConfidences(targetRegion);

    // The isophotes can be copied because they would only change slightly if recomputed.
    Helpers::CopySelfPatchIntoValidRegion<FloatVector2ImageType>(this->IsophoteImage, this->CurrentMask, this->SourcePatches[bestMatchSourcePatchId], targetRegion);

    // Update the mask
    this->UpdateMask(pixelToFill);
    DebugMessage("Updated mask.");
    
    // Sanity check everything
    if(this->Debug)
      {
      //DebugWriteAllImages(pixelToFill, bestMatchPixel, iteration);
      //DebugWriteImage<TImage>(this->Image,"FilledImage", iteration);
      std::stringstream ssImage;
      ssImage << "FilledImage_" << std::setfill('0') << std::setw(4) << iteration << ".png";
      Helpers::WriteRGBImage<FloatVectorImageType>(this->CurrentImage, ssImage.str());

      std::stringstream ssConfidence;
      ssConfidence << "Confidence_" << std::setfill('0') << std::setw(4) << iteration << ".mha";
      Helpers::WriteImage<FloatScalarImageType>(this->ConfidenceImage, ssConfidence.str());
      }

    iteration++;
    //this->Thrower->Throw(EventThrower::RefreshEvent);
    emit RefreshSignal();
    } // end main while loop

  
  }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in Inpaint!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void CriminisiInpainting::ComputeIsophotes()
{
  
  try
  {
  // Convert the color input image to a grayscale image
  UnsignedCharScalarImageType::Pointer grayscaleImage = UnsignedCharScalarImageType::New();
  Helpers::ColorToGrayscale<FloatVectorImageType>(this->CurrentImage, grayscaleImage);

  //WriteImage<UnsignedCharImageType>(grayscaleImage, "greyscale.mhd");

  // Blur the image to compute better gradient estimates
  typedef itk::DiscreteGaussianImageFilter<UnsignedCharScalarImageType, FloatScalarImageType >  filterType;

  // Create and setup a Gaussian filter
  filterType::Pointer gaussianFilter = filterType::New();
  gaussianFilter->SetInput(grayscaleImage);
  gaussianFilter->SetVariance(2);
  gaussianFilter->Update();

  //WriteImage<FloatImageType>(gaussianFilter->GetOutput(), "gaussianBlur.mhd");

  // Compute the gradient
  // TInputImage, TOperatorValueType, TOutputValueType
  typedef itk::GradientImageFilter<FloatScalarImageType, float, float>  GradientFilterType;
  GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
  gradientFilter->SetInput(gaussianFilter->GetOutput());
  gradientFilter->Update();

  //WriteImage<VectorImageType>(gradientFilter->GetOutput(), "gradient.mhd");

  // Rotate the gradient 90 degrees to obtain isophotes from gradient
  typedef itk::UnaryFunctorImageFilter<FloatVector2ImageType, FloatVector2ImageType,
  RotateVectors<
    FloatVector2ImageType::PixelType,
    FloatVector2ImageType::PixelType> > FilterType;

  FilterType::Pointer rotateFilter = FilterType::New();
  rotateFilter->SetInput(gradientFilter->GetOutput());
  rotateFilter->Update();

  //WriteImage<VectorImageType>(rotateFilter->GetOutput(), "originalIsophotes.mhd");

  // Mask the isophote image with the expanded version of the inpainting mask.
  // That is, keep only the values outside of the expanded mask. To do this, we have to first invert the mask.

  // Invert the mask
  typedef itk::InvertIntensityImageFilter <MaskImageType> InvertIntensityImageFilterType;

  InvertIntensityImageFilterType::Pointer invertMaskFilter = InvertIntensityImageFilterType::New();
  invertMaskFilter->SetInput(this->CurrentMask);
  invertMaskFilter->Update();

  //WriteScaledImage<UnsignedCharImageType>(invertMaskFilter->GetOutput(), "invertedExpandedMask.mhd");

  std::cout << "rotateFilter: " << rotateFilter->GetOutput()->GetLargestPossibleRegion() << std::endl;
  std::cout << "invertMaskFilter: " << invertMaskFilter->GetOutput()->GetLargestPossibleRegion() << std::endl;
  // Keep only values outside the masked region
  typedef itk::MaskImageFilter< FloatVector2ImageType, MaskImageType, FloatVector2ImageType > MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput1(rotateFilter->GetOutput());
  maskFilter->SetInput2(invertMaskFilter->GetOutput());
  maskFilter->Update();

  //this->IsophoteImage->Graft(maskFilter->GetOutput());
  Helpers::DeepCopy<FloatVector2ImageType>(maskFilter->GetOutput(), this->IsophoteImage);
  //WriteImage<VectorImageType>(this->IsophoteImage, "validIsophotes.mhd");
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeIsophotes!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

bool CriminisiInpainting::HasMoreToInpaint(MaskImageType::Pointer mask)
{
  itk::ImageRegionIterator<MaskImageType> imageIterator(mask,mask->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.Get() != 0)
      {
      return true;
      }

    ++imageIterator;
    }

  return false;
}

void CriminisiInpainting::FindBoundary()
{
  try
  {
  /*
  // If we simply find the boundary of the mask, the isophotes will not be copied into these pixels because they are 1 pixel
  // away from the filled region
  typedef itk::BinaryContourImageFilter <UnsignedCharImageType, UnsignedCharImageType >
          binaryContourImageFilterType;

  binaryContourImageFilterType::Pointer binaryContourFilter
          = binaryContourImageFilterType::New ();
  binaryContourFilter->SetInput(this->Mask);
  binaryContourFilter->Update();

  this->BoundaryImage = binaryContourFilter->GetOutput();
  */

  // Instead, we have to invert the mask before finding the boundary

  // Invert the mask
  typedef itk::InvertIntensityImageFilter <MaskImageType> InvertIntensityImageFilterType;
  InvertIntensityImageFilterType::Pointer invertIntensityFilter = InvertIntensityImageFilterType::New();
  invertIntensityFilter->SetInput(this->CurrentMask);
  invertIntensityFilter->Update();

  // Find the boundary
  typedef itk::BinaryContourImageFilter <MaskImageType, MaskImageType > binaryContourImageFilterType;
  binaryContourImageFilterType::Pointer binaryContourFilter = binaryContourImageFilterType::New ();
  binaryContourFilter->SetInput(invertIntensityFilter->GetOutput());
  binaryContourFilter->Update();

  //this->BoundaryImage = binaryContourFilter->GetOutput();
  //this->BoundaryImage->Graft(binaryContourFilter->GetOutput());
  Helpers::DeepCopy<UnsignedCharScalarImageType>(binaryContourFilter->GetOutput(), this->BoundaryImage);
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in FindBoundary!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void CriminisiInpainting::UpdateMask(itk::Index<2> pixel)
{
  try
  {
  // Create a black patch
  UnsignedCharScalarImageType::Pointer blackPatch = UnsignedCharScalarImageType::New();
  Helpers::CreateBlankPatch<UnsignedCharScalarImageType>(blackPatch, this->PatchRadius[0]);

  pixel[0] -= blackPatch->GetLargestPossibleRegion().GetSize()[0]/2;
  pixel[1] -= blackPatch->GetLargestPossibleRegion().GetSize()[1]/2;

  // Paste it into the mask
  typedef itk::PasteImageFilter <UnsignedCharScalarImageType, UnsignedCharScalarImageType > PasteImageFilterType;
  PasteImageFilterType::Pointer pasteFilter = PasteImageFilterType::New();
  pasteFilter->SetInput(0, this->CurrentMask);
  pasteFilter->SetInput(1, blackPatch);
  pasteFilter->SetSourceRegion(blackPatch->GetLargestPossibleRegion());
  pasteFilter->SetDestinationIndex(pixel);
  pasteFilter->InPlaceOn();
  pasteFilter->Update();

  //this->Mask = pasteFilter->GetOutput();
  //this->Mask->Graft(pasteFilter->GetOutput());
  Helpers::DeepCopy<UnsignedCharScalarImageType>(pasteFilter->GetOutput(), this->CurrentMask);
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in UpdateMask!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void CriminisiInpainting::ComputeBoundaryNormals()
{
  try
  {
  // Blur the mask, compute the gradient, then keep the normals only at the original mask boundary

  typedef itk::DiscreteGaussianImageFilter< UnsignedCharScalarImageType, FloatScalarImageType >  filterType;

  // Create and setup a Gaussian filter
  filterType::Pointer gaussianFilter = filterType::New();
  gaussianFilter->SetInput(this->CurrentMask);
  gaussianFilter->SetVariance(2);
  gaussianFilter->Update();

  typedef itk::GradientImageFilter< FloatScalarImageType, float, float>  GradientFilterType;
  GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
  gradientFilter->SetInput(gaussianFilter->GetOutput());
  gradientFilter->Update();

  // Only keep the normals at the boundary
  typedef itk::MaskImageFilter< FloatVector2ImageType, UnsignedCharScalarImageType, FloatVector2ImageType > MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput1(gradientFilter->GetOutput());
  maskFilter->SetInput2(this->BoundaryImage);
  maskFilter->Update();

  //this->BoundaryNormals = maskFilter->GetOutput();
  //this->BoundaryNormals->Graft(maskFilter->GetOutput());
  Helpers::DeepCopy<FloatVector2ImageType>(maskFilter->GetOutput(), this->BoundaryNormals);

  // Normalize
  itk::ImageRegionIterator<FloatVector2ImageType> imageIterator(this->BoundaryNormals,this->BoundaryNormals->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<UnsignedCharScalarImageType> boundaryIterator(this->BoundaryImage,this->BoundaryImage->GetLargestPossibleRegion());
  imageIterator.GoToBegin();
  boundaryIterator.GoToBegin();

  while(!imageIterator.IsAtEnd())
    {
    if(boundaryIterator.Get()) // The pixel is on the boundary
      {
      FloatVector2ImageType::PixelType p = imageIterator.Get();
      p.Normalize();
      imageIterator.Set(p);
      }
    ++imageIterator;
    ++boundaryIterator;
    }

  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeBoundaryNormals!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

itk::Index<2> CriminisiInpainting::FindHighestPriority(FloatScalarImageType::Pointer priorityImage)
{
  try
  {
  typedef itk::MinimumMaximumImageCalculator <FloatScalarImageType> ImageCalculatorFilterType;

  ImageCalculatorFilterType::Pointer imageCalculatorFilter = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(priorityImage);
  imageCalculatorFilter->Compute();
  
  DebugMessage<float>("Highest priority: ", imageCalculatorFilter->GetMaximum());
  DebugMessage<float>("Lowest priority: ", imageCalculatorFilter->GetMinimum());

  return imageCalculatorFilter->GetIndexOfMaximum();
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in FindHighestPriority!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void CriminisiInpainting::ComputeAllPriorities()
{
  try
  {
  // Only compute priorities for pixels on the boundary
  itk::ImageRegionConstIterator<UnsignedCharScalarImageType> boundaryIterator(this->BoundaryImage, this->BoundaryImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<FloatScalarImageType> priorityIterator(this->PriorityImage, this->PriorityImage->GetLargestPossibleRegion());

  boundaryIterator.GoToBegin();
  priorityIterator.GoToBegin();

  // The main loop is over the boundary image. We only want to compute priorities at boundary pixels
  unsigned int boundaryPixelCounter = 0;
  while(!boundaryIterator.IsAtEnd())
    {
    // If the pixel is not on the boundary, skip it and set its priority to -1.
    // -1 is used instead of 0 because if the priorities on the boundary get down to 0, if the non-boundary pixels priorities were set to 0, then we would end up choosing a random point from anywhere in the image,
    // rather than a point on the boundary.
    if(boundaryIterator.Get() == 0)
      {
      priorityIterator.Set(-1);
      ++boundaryIterator;
      ++priorityIterator;
      continue;
      }

    float priority = ComputePriority(boundaryIterator.GetIndex());
    //DebugMessage<float>("Priority: ", priority);
    priorityIterator.Set(priority);
    boundaryPixelCounter++;
    
    ++boundaryIterator;
    ++priorityIterator;
    }
    std::cout << "There were " << boundaryPixelCounter << " boundary pixels." << std::endl;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeAllPriorities!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

float CriminisiInpainting::ComputePriority(itk::Index<2> queryPixel)
{
  double confidence = ComputeConfidenceTerm(queryPixel);
  double data = ComputeDataTerm(queryPixel);

  return confidence * data;
}

float CriminisiInpainting::ComputeConfidenceTerm(itk::Index<2> queryPixel)
{
  std::cout << "Computing confidence for " << queryPixel << std::endl;
  try
  {
  if(!this->CurrentMask->GetLargestPossibleRegion().IsInside(Helpers::GetRegionInRadiusAroundPixel(queryPixel,this->PatchRadius[0])))
    {
    return 0;
    }

    // DONT allow for regions on/near the image border
    //itk::ImageRegionConstIterator<UnsignedCharImageType> maskIterator(this->Mask, GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0]));
    //itk::ImageRegionConstIterator<FloatImageType> confidenceIterator(this->ConfidenceImage, GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0]));

    // allow for regions on/near the image border

    itk::ImageRegion<2> region = this->CurrentMask->GetLargestPossibleRegion();
    region.Crop(Helpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0]));
    itk::ImageRegionConstIterator<MaskImageType> maskIterator(this->CurrentMask, region);
    itk::ImageRegionConstIterator<FloatScalarImageType> confidenceIterator(this->ConfidenceImage, region);

    maskIterator.GoToBegin();
    confidenceIterator.GoToBegin();
    // confidence = sum of the confidences of patch pixels in the source region / area of the patch

    float sum = 0;

    while(!maskIterator.IsAtEnd())
      {
      if(maskIterator.Get() == 0) // Pixel is in the source region
        {
        sum += confidenceIterator.Get();
        }
      ++confidenceIterator;
      ++maskIterator;
      }


    unsigned int numberOfPixels = GetNumberOfPixelsInPatch();
    float areaOfPatch = static_cast<float>(numberOfPixels);
    
    float confidence = sum/areaOfPatch;
    //DebugMessage<float>("Confidence: ", confidence);

    return confidence;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeConfidenceTerm!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

float CriminisiInpainting::ComputeDataTerm(itk::Index<2> queryPixel)
{
  try
  {
  itk::CovariantVector<float, 2> isophote = this->IsophoteImage->GetPixel(queryPixel);
  itk::CovariantVector<float, 2> boundaryNormal = this->BoundaryNormals->GetPixel(queryPixel);

  double alpha = 255; // for grey scale images
  // D(p) = |dot(isophote direction at p, normal of the front at p)|/alpha

  vnl_double_2 vnlIsophote(isophote[0], isophote[1]);

  vnl_double_2 vnlNormal(boundaryNormal[0], boundaryNormal[1]);

  double dot = std::abs(dot_product(vnlIsophote,vnlNormal));

  return dot/alpha;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeDataTerm!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

struct UpdatePixel
{
  UpdatePixel(itk::Index<2> i, float v) : index(i), value(v) {}
  itk::Index<2> index;
  float value;
};

void CriminisiInpainting::UpdateConfidences(itk::ImageRegion<2> region)
{
  itk::ImageRegionConstIterator<MaskImageType> maskIterator(this->CurrentMask, region);
  itk::ImageRegionIterator<FloatScalarImageType> confidenceIterator(this->ConfidenceImage, region);


  std::vector<UpdatePixel> pixelsToUpdate;
  while(!confidenceIterator.IsAtEnd())
    {
    if(maskIterator.Get() != 0)
      {
      pixelsToUpdate.push_back(UpdatePixel(confidenceIterator.GetIndex(), ComputeConfidenceTerm(confidenceIterator.GetIndex())));
      }

    ++confidenceIterator;
    ++maskIterator;
    }
  for(unsigned int i = 0; i < pixelsToUpdate.size(); ++i)
    {
    this->ConfidenceImage->SetPixel(pixelsToUpdate[i].index, pixelsToUpdate[i].value);
    }
}

bool CriminisiInpainting::IsValidPatch(itk::ImageRegion<2> region)
{
  // Check if the patch is inside the image
  if(!this->CurrentMask->GetLargestPossibleRegion().IsInside(region))
    {
    return false;
    }

  // Check if all the pixels in the patch are in the valid region of the image
  itk::ImageRegionConstIterator<MaskImageType> iterator(this->CurrentMask, region);
  while(!iterator.IsAtEnd())
  {
  if(iterator.Get()) // valid(unmasked) pixels are 0, masked pixels are 1 (non-zero)
    {
    return false;
    }

  ++iterator;
  }

  return true;

}

bool CriminisiInpainting::IsValidPatch(itk::Index<2> queryPixel, unsigned int radius)
{
  // This function checks if a patch is completely inside the image and not intersecting the mask

  itk::ImageRegion<2> region = Helpers::GetRegionInRadiusAroundPixel(queryPixel, radius);
  return IsValidPatch(region);
}

itk::CovariantVector<float, 2> CriminisiInpainting::GetAverageIsophote(itk::Index<2> queryPixel)
{
  try
  {
  if(!this->CurrentMask->GetLargestPossibleRegion().IsInside(Helpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0])))
    {
    itk::CovariantVector<float, 2> v;
    v[0] = 0; v[1] = 0;
    return v;
    }

  itk::ImageRegionConstIterator<MaskImageType> iterator(this->CurrentMask,Helpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0]));

  std::vector<itk::CovariantVector<float, 2> > vectors;

  while(!iterator.IsAtEnd())
    {
    if(IsValidPatch(iterator.GetIndex(), 3))
      {
      vectors.push_back(this->IsophoteImage->GetPixel(iterator.GetIndex()));
      }

    ++iterator;
    }

  itk::CovariantVector<float, 2> averageVector;
  averageVector[0] = 0;
  averageVector[1] = 0;

  if(vectors.size() == 0)
    {
    return averageVector;
    }

  for(unsigned int i = 0; i < vectors.size(); i++)
    {
    averageVector[0] += vectors[i][0];
    averageVector[1] += vectors[i][1];
    }
  averageVector[0] /= vectors.size();
  averageVector[1] /= vectors.size();

  return averageVector;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in GetAverageIsophote!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

unsigned int CriminisiInpainting::GetNumberOfPixelsInPatch()
{
  return this->GetPatchSize()[0]*this->GetPatchSize()[1];
}

itk::Size<2> CriminisiInpainting::GetPatchSize()
{
  itk::Size<2> patchSize;

  patchSize[0] = (this->PatchRadius[0]*2)+1;
  patchSize[1] = (this->PatchRadius[1]*2)+1;

  return patchSize;
}
