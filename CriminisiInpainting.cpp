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

// Custom
#include "RotateVectors.h"
#include "SelfPatchMatch.h"
#include "Helpers.h"

// STL
#include <iostream>
#include <iomanip> // setfill and setw

// VXL
#include <vnl/vnl_double_2.h>

// ITK
#include "itkVectorMagnitudeImageFilter.h"

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
  this->DataImage = FloatScalarImageType::New();
  
  this->Debug = true;
  this->Iteration = 0;
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

FloatVector2ImageType::Pointer CriminisiInpainting::GetBoundaryNormalsImage()
{
  return this->BoundaryNormals;
}

FloatVector2ImageType::Pointer CriminisiInpainting::GetIsophoteImage()
{
  return this->IsophoteImage;
}

FloatScalarImageType::Pointer CriminisiInpainting::GetDataImage()
{
  return this->DataImage;
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
  
  if(this->Debug)
    {
    Helpers::WriteImage<MaskImageType>(this->OriginalMask, "ExpandMask.input.mha");
    }
    
  typedef itk::BinaryThresholdImageFilter <MaskImageType, MaskImageType> BinaryThresholdImageFilterType;
  BinaryThresholdImageFilterType::Pointer thresholdFilter = BinaryThresholdImageFilterType::New();
  thresholdFilter->SetInput(this->OriginalMask);
  thresholdFilter->SetLowerThreshold(122);
  thresholdFilter->SetUpperThreshold(255);
  thresholdFilter->SetInsideValue(255);
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->InPlaceOn();
  thresholdFilter->Update();
  
  if(this->Debug)
    {
    Helpers::WriteImage<MaskImageType>(thresholdFilter->GetOutput(), "ExpandMask.thresholdedMask.mha");
    }

  // Expand the mask - this is necessary to prevent the isophotes from being undefined in the target region
  typedef itk::FlatStructuringElement<2> StructuringElementType;
  StructuringElementType::RadiusType radius;
  radius.Fill(2); // Just a little bit of expansion
  //radius.Fill(this->PatchRadius[0]); // This was working, but huge expansion
  //radius.Fill(2.0* this->PatchRadius[0]);

  StructuringElementType structuringElement = StructuringElementType::Box(radius);
  typedef itk::BinaryDilateImageFilter<MaskImageType, MaskImageType, StructuringElementType> BinaryDilateImageFilterType;
  BinaryDilateImageFilterType::Pointer expandMaskFilter = BinaryDilateImageFilterType::New();
  expandMaskFilter->SetInput(thresholdFilter->GetOutput());
  expandMaskFilter->SetKernel(structuringElement);
  expandMaskFilter->Update();
  
  if(this->Debug)
    {
    Helpers::WriteImage<MaskImageType>(expandMaskFilter->GetOutput(), "ExpandMask.expandedMask.mha");
    }
    
  Helpers::DeepCopy<MaskImageType>(expandMaskFilter->GetOutput(), this->CurrentMask);

  //WriteScaledImage<UnsignedCharImageType>(this->Mask, "expandedMask.mhd");
}

void CriminisiInpainting::InitializeMask()
{
  ExpandMask();
}

void CriminisiInpainting::InitializeConfidence()
{

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
}

void CriminisiInpainting::InitializeData()
{
  // Create a blank priority image
  this->DataImage->SetRegions(this->OriginalImage->GetLargestPossibleRegion());
  this->DataImage->Allocate();
  this->DataImage->FillBuffer(0);
}

void CriminisiInpainting::InitializeImage()
{
  // Invert the mask
  typedef itk::InvertIntensityImageFilter <MaskImageType> InvertIntensityImageFilterType;
  InvertIntensityImageFilterType::Pointer invertIntensityFilter = InvertIntensityImageFilterType::New();
  invertIntensityFilter->SetInput(this->CurrentMask);
  invertIntensityFilter->Update();
  
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
}

void CriminisiInpainting::InitializePriority()
{
  // Create a blank priority image
  this->PriorityImage->SetRegions(this->OriginalImage->GetLargestPossibleRegion());
  this->PriorityImage->Allocate();
  this->PriorityImage->FillBuffer(0);
}

void CriminisiInpainting::Initialize()
{
  try
  {
    InitializeMask();
    if(this->Debug)
      {
      Helpers::WriteImage<MaskImageType>(this->CurrentMask, "OriginalMask.mha");
      }
      
    // Do this before we mask the image with the expanded mask
    ComputeIsophotes();
    if(this->Debug)
      {
      Helpers::WriteImage<FloatVector2ImageType>(this->IsophoteImage, "OriginalIsophotes.mha");
      }
    
    InitializeData();
    if(this->Debug)
      {
      Helpers::WriteImage<FloatScalarImageType>(this->DataImage, "OriginalData.mha");
      }
      
    InitializePriority();
    if(this->Debug)
      {
      Helpers::WriteImage<FloatScalarImageType>(this->PriorityImage, "OriginalPriority.mha");
      }
      
    InitializeConfidence();
    if(this->Debug)
      {
      Helpers::WriteImage<FloatScalarImageType>(this->ConfidenceImage, "OriginalConfidence.mha");
      }
      
    InitializeImage();
    if(this->Debug)
      {
      Helpers::WriteImage<FloatVectorImageType>(this->CurrentImage, "OriginalImage.mha");
      }
      
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
  //std::cout << "CriminisiInpainting::Inpaint()" << std::endl;
  try
  {
  Initialize();
  ComputeSourcePatches();

  this->Iteration = 0;
  while(HasMoreToInpaint())
    {
    std::cout << "Iteration: " << this->Iteration << std::endl;

    FindBoundary();
    if(this->Debug)
      {
      Helpers::WriteImage<UnsignedCharScalarImageType>(this->BoundaryImage, "BoundaryImage.mha");
      }
    DebugMessage("Found boundary.");
  
    ComputeBoundaryNormals();
    if(this->Debug)
      {
      Helpers::WriteImage<FloatVector2ImageType>(this->BoundaryNormals, "BoundaryNormals.mha");
      }
    DebugMessage("Computed boundary normals.");

    ComputeAllDataTerms();
  
    ComputeAllPriorities();
    DebugMessage("Computed priorities.");
    
    itk::Index<2> pixelToFill = FindHighestPriority(this->PriorityImage);
    DebugMessage<itk::Index<2> >("Highest priority found to be ", pixelToFill);
    //std::cout << "Filling: " << pixelToFill << std::endl;

    this->DebugWritePatch(pixelToFill, "PatchToFill", this->Iteration); // this must be done here because it is before the filling

    //itk::Index<2> bestMatchPixel = SelfPatchMatch<TImage, UnsignedCharImageType>(this->Image, this->Mask, pixelToFill, this->PatchRadius[0], iteration, this->Weights);
    //std::cout << "Best match pixel: " << bestMatchPixel << std::endl;

    itk::ImageRegion<2> targetRegion = Helpers::GetRegionInRadiusAroundPixel(pixelToFill, this->PatchRadius[0]);

    DebugMessage("Finding best patch...");
    unsigned int bestMatchSourcePatchId = BestPatch<FloatVectorImageType, MaskImageType>(this->CurrentImage, this->CurrentMask, this->SourcePatches, targetRegion);
    DebugMessage<unsigned int>("Found best patch to be ", bestMatchSourcePatchId);
    
    //this->DebugWritePatch(this->SourcePatches[bestMatchSourcePatchId], "SourcePatch.png");
    //this->DebugWritePatch(targetRegion, "TargetPatch.png");
    //std::cout << "Best match source patch id: " << bestMatchSourcePatchId << std::endl;

    // Copy the patch. This is the actual inpainting step.
    Helpers::CopySelfPatchIntoValidRegion<FloatVectorImageType>(this->CurrentImage, this->CurrentMask, this->SourcePatches[bestMatchSourcePatchId], targetRegion);

    // Copy the new confidences into the confidence image
    UpdateConfidences(targetRegion);

    // The isophotes can be copied because they would only change slightly if recomputed.
    Helpers::CopySelfPatchIntoValidRegion<FloatVector2ImageType>(this->IsophoteImage, this->CurrentMask, this->SourcePatches[bestMatchSourcePatchId], targetRegion);

    // Update the mask
    this->UpdateMask(pixelToFill);
    DebugMessage("Updated mask.");
    
    // Sanity check everything
    if(this->Debug)
      {
      DebugWriteAllImages();
      }

    this->Iteration++;
    
    emit RefreshSignal();
    } // end while(HasMoreToInpaint) loop

  
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
    if(this->Debug)
      {
      Helpers::WriteImage<FloatVectorImageType>(this->CurrentImage, "ComputeIsophotes.input.mha");
      }
    
    typedef itk::VectorMagnitudeImageFilter<FloatVectorImageType, UnsignedCharScalarImageType>  VectorMagnitudeFilterType;
    VectorMagnitudeFilterType::Pointer magnitudeFilter = VectorMagnitudeFilterType::New();
    magnitudeFilter->SetInput(this->CurrentImage);
    magnitudeFilter->Update();
    
    if(this->Debug)
      {
      Helpers::WriteImage<UnsignedCharScalarImageType>(magnitudeFilter->GetOutput(), "ComputeIsophotes.magnitude.mha");
      }
    /*
    // Convert the color input image to a grayscale image
    UnsignedCharScalarImageType::Pointer grayscaleImage = UnsignedCharScalarImageType::New();
    Helpers::ColorToGrayscale<FloatVectorImageType>(this->CurrentImage, grayscaleImage);

    if(this->Debug)
      {
      Helpers::WriteImage<UnsignedCharScalarImageType>(grayscaleImage, "ComputeIsophotes.greyscale.mha");
      }
    */

    // Blur the image to compute better gradient estimates
    typedef itk::DiscreteGaussianImageFilter<UnsignedCharScalarImageType, FloatScalarImageType >  BlurFilterType;
    BlurFilterType::Pointer blurFilter = BlurFilterType::New();
    blurFilter->SetInput(magnitudeFilter->GetOutput());
    blurFilter->SetVariance(2);
    blurFilter->Update();

    if(this->Debug)
      {
      Helpers::WriteImage<FloatScalarImageType>(blurFilter->GetOutput(), "ComputeIsophotes.blurred.mha");
      }

    // Compute the gradient
    // Template parameters are <TInputImage, TOperatorValueType, TOutputValueType>
    typedef itk::GradientImageFilter<FloatScalarImageType, float, float>  GradientFilterType;
    GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
    gradientFilter->SetInput(blurFilter->GetOutput());
    gradientFilter->Update();

    if(this->Debug)
      {
      Helpers::WriteImage<FloatVector2ImageType>(gradientFilter->GetOutput(), "ComputeIsophotes.gradient.mha");
      }

    // Rotate the gradient 90 degrees to obtain isophotes from gradient
    typedef itk::UnaryFunctorImageFilter<FloatVector2ImageType, FloatVector2ImageType,
    RotateVectors<
      FloatVector2ImageType::PixelType,
      FloatVector2ImageType::PixelType> > FilterType;

    FilterType::Pointer rotateFilter = FilterType::New();
    rotateFilter->SetInput(gradientFilter->GetOutput());
    rotateFilter->Update();

    if(this->Debug)
      {
      Helpers::WriteImage<FloatVector2ImageType>(rotateFilter->GetOutput(), "ComputeIsophotes.rotatedGradient.mha");
      }
      
    // Mask the isophote image with the expanded version of the inpainting mask.
    // That is, keep only the values outside of the expanded mask. To do this, we have to first invert the mask.

    // Invert the mask
    typedef itk::InvertIntensityImageFilter <MaskImageType> InvertIntensityImageFilterType;
    InvertIntensityImageFilterType::Pointer invertMaskFilter = InvertIntensityImageFilterType::New();
    invertMaskFilter->SetInput(this->CurrentMask);
    invertMaskFilter->Update();

    if(this->Debug)
      {
      Helpers::WriteImage<MaskImageType>(invertMaskFilter->GetOutput(), "ComputeIsophotes.invertedMask.mha");
      }

    //std::cout << "rotateFilter: " << rotateFilter->GetOutput()->GetLargestPossibleRegion() << std::endl;
    //std::cout << "invertMaskFilter: " << invertMaskFilter->GetOutput()->GetLargestPossibleRegion() << std::endl;
    
    // Keep only values outside the masked region
    typedef itk::MaskImageFilter< FloatVector2ImageType, MaskImageType, FloatVector2ImageType > MaskFilterType;
    MaskFilterType::Pointer maskFilter = MaskFilterType::New();
    maskFilter->SetInput1(rotateFilter->GetOutput());
    maskFilter->SetInput2(invertMaskFilter->GetOutput());
    maskFilter->Update();

    if(this->Debug)
      {
      Helpers::WriteImage<FloatVector2ImageType>(maskFilter->GetOutput(), "ComputeIsophotes.maskedIsophotes.mha");
      }
      
    Helpers::DeepCopy<FloatVector2ImageType>(maskFilter->GetOutput(), this->IsophoteImage);
   
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeIsophotes!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

bool CriminisiInpainting::HasMoreToInpaint()
{
  itk::ImageRegionIterator<MaskImageType> imageIterator(this->CurrentMask, this->CurrentMask->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.Get() != 0) // Pixel still needs to be filled
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

    if(Debug)
      {
      Helpers::WriteImage<MaskImageType>(this->CurrentMask, "FindBoundary.CurrentMask.mha");
      Helpers::WriteImage<MaskImageType>(this->CurrentMask, "FindBoundary.CurrentMask.png");
      }
      
    // Invert the mask
    typedef itk::InvertIntensityImageFilter <MaskImageType> InvertIntensityImageFilterType;
    InvertIntensityImageFilterType::Pointer invertIntensityFilter = InvertIntensityImageFilterType::New();
    invertIntensityFilter->SetInput(this->CurrentMask);
    invertIntensityFilter->Update();

    if(Debug)
      {
      Helpers::WriteImage<MaskImageType>(invertIntensityFilter->GetOutput(), "FindBoundary.InvertedMask.mha");
      }

    // Find the boundary
    typedef itk::BinaryContourImageFilter <MaskImageType, MaskImageType > binaryContourImageFilterType;
    binaryContourImageFilterType::Pointer binaryContourFilter = binaryContourImageFilterType::New();
    binaryContourFilter->SetInput(invertIntensityFilter->GetOutput());
    binaryContourFilter->Update();

    if(Debug)
      {
      Helpers::WriteImage<MaskImageType>(binaryContourFilter->GetOutput(), "FindBoundary.Boundary.mha");
      }

    //this->BoundaryImage = binaryContourFilter->GetOutput();
    //this->BoundaryImage->Graft(binaryContourFilter->GetOutput());
    Helpers::DeepCopy<UnsignedCharScalarImageType>(binaryContourFilter->GetOutput(), this->BoundaryImage);

    if(Debug)
      {
      Helpers::WriteImage<UnsignedCharScalarImageType>(this->BoundaryImage, "FindBoundary.BoundaryImage.mha");
      }
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in FindBoundary!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void CriminisiInpainting::UpdateMask(const itk::Index<2> inputPixel)
{
  try
  {
    // Create a black patch
    UnsignedCharScalarImageType::Pointer blackPatch = UnsignedCharScalarImageType::New();
    Helpers::CreateBlankPatch<UnsignedCharScalarImageType>(blackPatch, this->PatchRadius[0]);
    itk::Index<2> pixel;
    pixel[0] = inputPixel[0] - blackPatch->GetLargestPossibleRegion().GetSize()[0]/2;
    pixel[1] = inputPixel[1] - blackPatch->GetLargestPossibleRegion().GetSize()[1]/2;

    // Paste it into the mask
    typedef itk::PasteImageFilter <UnsignedCharScalarImageType, UnsignedCharScalarImageType > PasteImageFilterType;
    PasteImageFilterType::Pointer pasteFilter = PasteImageFilterType::New();
    pasteFilter->SetInput(0, this->CurrentMask);
    pasteFilter->SetInput(1, blackPatch);
    pasteFilter->SetSourceRegion(blackPatch->GetLargestPossibleRegion());
    pasteFilter->SetDestinationIndex(pixel);
    pasteFilter->Update();

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
    
    if(Debug)
      {
      Helpers::WriteImage<UnsignedCharScalarImageType>(this->BoundaryImage, "ComputeBoundaryNormals.BoundaryImage.mha");
      Helpers::WriteImage<UnsignedCharScalarImageType>(this->BoundaryImage, "ComputeBoundaryNormals.BoundaryImage.png");
      Helpers::WriteImage<UnsignedCharScalarImageType>(this->CurrentMask, "ComputeBoundaryNormals.CurrentMask.mha");
      }
      
    // Blur the mask
    typedef itk::DiscreteGaussianImageFilter< UnsignedCharScalarImageType, FloatScalarImageType >  BlurFilterType;
    BlurFilterType::Pointer gaussianFilter = BlurFilterType::New();
    gaussianFilter->SetInput(this->CurrentMask);
    gaussianFilter->SetVariance(2);
    gaussianFilter->Update();

    if(Debug)
      {
      Helpers::WriteImage<FloatScalarImageType>(gaussianFilter->GetOutput(), "ComputeBoundaryNormals.BlurredMask.mha");
      }

    // Compute the gradient of the blurred mask
    typedef itk::GradientImageFilter< FloatScalarImageType, float, float>  GradientFilterType;
    GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
    gradientFilter->SetInput(gaussianFilter->GetOutput());
    gradientFilter->Update();

    if(Debug)
      {
      Helpers::WriteImage<FloatVector2ImageType>(gradientFilter->GetOutput(), "ComputeBoundaryNormals.BlurredMaskGradient.mha");
      }

    // Only keep the normals at the boundary
    typedef itk::MaskImageFilter< FloatVector2ImageType, UnsignedCharScalarImageType, FloatVector2ImageType > MaskFilterType;
    MaskFilterType::Pointer maskFilter = MaskFilterType::New();
    maskFilter->SetInput1(gradientFilter->GetOutput());
    maskFilter->SetInput2(this->BoundaryImage);
    maskFilter->Update();

    if(Debug)
      {
      Helpers::WriteImage<FloatVector2ImageType>(maskFilter->GetOutput(), "ComputeBoundaryNormals.BoundaryNormalsUnnormalized.mha");
      }
      
    //this->BoundaryNormals = maskFilter->GetOutput();
    //this->BoundaryNormals->Graft(maskFilter->GetOutput());
    Helpers::DeepCopy<FloatVector2ImageType>(maskFilter->GetOutput(), this->BoundaryNormals);

    // Normalize the vectors because we just care about their direction (the Data term computation calls for the normalized boundary normal)
    itk::ImageRegionIterator<FloatVector2ImageType> imageIterator(this->BoundaryNormals, this->BoundaryNormals->GetLargestPossibleRegion());
    itk::ImageRegionConstIterator<UnsignedCharScalarImageType> boundaryIterator(this->BoundaryImage, this->BoundaryImage->GetLargestPossibleRegion());
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

    if(Debug)
      {
      Helpers::WriteImage<FloatVector2ImageType>(this->BoundaryNormals, "ComputeBoundaryNormals.BoundaryNormals.mha");
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

    // Blank the priority image.
    // -1 is used instead of 0 because if the priorities on the boundary get down to 0, if the non-boundary pixels priorities were set to 0, then we would end up choosing a random point from anywhere in the image,
    this->PriorityImage->FillBuffer(-1);

    // The main loop is over the boundary image. We only want to compute priorities at boundary pixels
    unsigned int boundaryPixelCounter = 0;
    while(!boundaryIterator.IsAtEnd())
      {

      if(boundaryIterator.Get() != 0) // Pixel is on the boundary
	{
	float priority = ComputePriority(boundaryIterator.GetIndex());
	//DebugMessage<float>("Priority: ", priority);
	priorityIterator.Set(priority);
	boundaryPixelCounter++;
	}    
      ++boundaryIterator;
      ++priorityIterator;
      }
    DebugMessage<unsigned int>("Number of boundary pixels: ", boundaryPixelCounter);
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeAllPriorities!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

float CriminisiInpainting::ComputePriority(const itk::Index<2> queryPixel)
{
  //double confidence = ComputeConfidenceTerm(queryPixel);
  //double data = ComputeDataTerm(queryPixel);

  double confidence = this->ConfidenceImage->GetPixel(queryPixel);
  double data = this->DataImage->GetPixel(queryPixel);

  return confidence * data;
}

float CriminisiInpainting::ComputeConfidenceTerm(const itk::Index<2> queryPixel)
{
  //DebugMessage<itk::Index<2>>("Computing confidence for ", queryPixel);
  try
  {
    if(!this->CurrentMask->GetLargestPossibleRegion().IsInside(Helpers::GetRegionInRadiusAroundPixel(queryPixel,this->PatchRadius[0])))
      {
      return 0;
      }

    // Allow for regions on/near the image border

    itk::ImageRegion<2> region = this->CurrentMask->GetLargestPossibleRegion();
    region.Crop(Helpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0]));
    itk::ImageRegionConstIterator<MaskImageType> maskIterator(this->CurrentMask, region);
    itk::ImageRegionConstIterator<FloatScalarImageType> confidenceIterator(this->ConfidenceImage, region);

    maskIterator.GoToBegin();
    confidenceIterator.GoToBegin();
    // The confidence is computed as the sum of the confidences of patch pixels in the source region / area of the patch

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

float CriminisiInpainting::ComputeDataTerm(const itk::Index<2> queryPixel)
{
  try
  {
    FloatVector2Type isophote = this->IsophoteImage->GetPixel(queryPixel);
    FloatVector2Type boundaryNormal = this->BoundaryNormals->GetPixel(queryPixel);

    if(this->Debug)
      {
      //std::cout << "Isophote: " << isophote << std::endl;
      //std::cout << "Boundary normal: " << boundaryNormal << std::endl;
      }
    double alpha = 255; // for grey scale images
    // D(p) = |dot(isophote direction at p, normal of the front at p)|/alpha

    vnl_double_2 vnlIsophote(isophote[0], isophote[1]);

    vnl_double_2 vnlNormal(boundaryNormal[0], boundaryNormal[1]);

    double dot = std::abs(dot_product(vnlIsophote,vnlNormal));

    float dataTerm = dot/alpha;

    return dataTerm;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeDataTerm!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

// This struct is used by UpdateConfidences to keep track of the pixels and their values that should be replaced.
// We have to do this in a two pass fashion because the old neighboring values cannot be changed until all new values
// have been computed.
struct UpdatePixel
{
  UpdatePixel(itk::Index<2> i, float v) : index(i), value(v) {}
  itk::Index<2> index;
  float value;
};

void CriminisiInpainting::UpdateConfidences(const itk::ImageRegion<2> inputRegion)
{
  try
  {
    // Force the region to update to be entirely inside the image
    itk::ImageRegion<2> region = CropToValidRegion(inputRegion);
    
    // Use an iterator to find masked pixels. Compute their new value, and save it in a vector of pixels and their new values.
    // Do not update the pixels until after all new values have been computed, because we want to use the old values in all of
    // the computations.
    itk::ImageRegionConstIterator<MaskImageType> maskIterator(this->CurrentMask, region);

    std::vector<UpdatePixel> pixelsToUpdate;
    while(!maskIterator.IsAtEnd())
      {
      if(maskIterator.Get() != 0) // Pixel is masked
	{
	itk::Index<2> currentPixel = maskIterator.GetIndex();
	pixelsToUpdate.push_back(UpdatePixel(currentPixel, ComputeConfidenceTerm(currentPixel)));
	}

      ++maskIterator;
      } // end while looop with iterator

    // Actually update the pixels
    for(unsigned int i = 0; i < pixelsToUpdate.size(); ++i)
      {
      this->ConfidenceImage->SetPixel(pixelsToUpdate[i].index, pixelsToUpdate[i].value);
      }
  } // end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in UpdateConfidences!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}


void CriminisiInpainting::ComputeAllDataTerms()
{
  try
  {
    itk::ImageRegionConstIteratorWithIndex<MaskImageType> maskIterator(this->BoundaryImage, this->BoundaryImage->GetLargestPossibleRegion());

    // Blank the data term image
    this->DataImage->FillBuffer(0);

    while(!maskIterator.IsAtEnd())
      {
      if(maskIterator.Get() != 0) // This is a pixel on the current boundary
	{
	itk::Index<2> currentPixel = maskIterator.GetIndex();
	float dataTerm = ComputeDataTerm(currentPixel);
	this->DataImage->SetPixel(currentPixel, dataTerm);
	//DebugMessage<float>("Set DataTerm to ", dataTerm);
	}

      ++maskIterator;
      }
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeAllDataTerms!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

bool CriminisiInpainting::IsValidPatch(const itk::ImageRegion<2> region)
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

bool CriminisiInpainting::IsValidPatch(const itk::Index<2> queryPixel, const unsigned int radius)
{
  // This function checks if a patch is completely inside the image and not intersecting the mask

  itk::ImageRegion<2> region = Helpers::GetRegionInRadiusAroundPixel(queryPixel, radius);
  return IsValidPatch(region);
}

FloatVector2Type CriminisiInpainting::GetAverageIsophote(const itk::Index<2> queryPixel)
{
  try
  {
    if(!this->CurrentMask->GetLargestPossibleRegion().IsInside(Helpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0])))
      {
      FloatVector2Type v;
      v[0] = 0; v[1] = 0;
      return v;
      }

    itk::ImageRegionConstIterator<MaskImageType> iterator(this->CurrentMask,Helpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0]));

    std::vector<FloatVector2Type> vectors;

    while(!iterator.IsAtEnd())
      {
      if(IsValidPatch(iterator.GetIndex(), 3))
	{
	vectors.push_back(this->IsophoteImage->GetPixel(iterator.GetIndex()));
	}

      ++iterator;
      }

    FloatVector2Type averageVector;
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

itk::ImageRegion<2> CriminisiInpainting::CropToValidRegion(const itk::ImageRegion<2> inputRegion)
{
  itk::ImageRegion<2> outputRegion = inputRegion; // Initialize
  itk::ImageRegion<2> region = this->CurrentMask->GetLargestPossibleRegion(); // This could have been CurrentImage, or any of the other images - they should all be the same size
  region.Crop(outputRegion);
  
  return outputRegion;
}
