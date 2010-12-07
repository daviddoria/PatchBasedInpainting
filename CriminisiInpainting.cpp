#include "CriminisiInpainting.h"

#include "RotateVectors.h"

#include <iostream>
#include <iomanip> // setfill and setw

#include <vnl/vnl_double_2.h>

CriminisiInpainting::CriminisiInpainting()
{
  this->PatchRadius[0] = 5;
  this->PatchRadius[1] = 5;

  this->BoundaryImage = UnsignedCharImageType::New();
  this->BoundaryNormals = VectorImageType::New();
  this->MeanDifferenceImage = FloatImageType::New();
  this->IsophoteImage = VectorImageType::New();
  this->PriorityImage = FloatImageType::New();
  this->Mask = UnsignedCharImageType::New();
  this->InputMask = UnsignedCharImageType::New();
  this->Image = ColorImageType::New();
  this->Patch = ColorImageType::New();
  this->ConfidenceImage = FloatImageType::New();

  this->WriteIntermediateImages = false;
}
    
void CriminisiInpainting::SetWriteIntermediateImages(bool flag)
{
  this->WriteIntermediateImages = flag;
}

void CriminisiInpainting::ExpandMask()
{
  // Expand the mask - this is necessary to prevent the isophotes from being undefined in the target region
  typedef itk::FlatStructuringElement<2> StructuringElementType;
  StructuringElementType::RadiusType radius;
  radius.Fill(this->PatchRadius[0]);
  //radius.Fill(2.0* this->PatchRadius[0]);

  StructuringElementType structuringElement = StructuringElementType::Box(radius);
  typedef itk::BinaryDilateImageFilter<UnsignedCharImageType, UnsignedCharImageType, StructuringElementType>
          BinaryDilateImageFilterType;

  BinaryDilateImageFilterType::Pointer expandMaskFilter
          = BinaryDilateImageFilterType::New();
  expandMaskFilter->SetInput(this->InputMask);
  expandMaskFilter->SetKernel(structuringElement);
  expandMaskFilter->Update();
  this->Mask->Graft(expandMaskFilter->GetOutput());

  WriteScaledImage<UnsignedCharImageType>(this->Mask, "expandedMask.mhd");
}

void CriminisiInpainting::Inpaint()
{
  //CreateCIELABImage();

  ExpandMask();

  // Do this before we expand the mask the image with the expanded mask
  ComputeIsophotes();

  // Create a blank priority image
  this->PriorityImage->SetRegions(this->Image->GetLargestPossibleRegion());
  this->PriorityImage->Allocate();

  // Create a blank mean difference image
  this->MeanDifferenceImage->SetRegions(this->Image->GetLargestPossibleRegion());
  this->MeanDifferenceImage->Allocate();

  // Clone the mask - we need to invert the mask to actually perform the masking, but we don't want to disturb the original mask
  typedef itk::ImageDuplicator< UnsignedCharImageType > DuplicatorType;
  DuplicatorType::Pointer duplicator = DuplicatorType::New();
  duplicator->SetInputImage(this->Mask);
  duplicator->Update();

  // Invert the mask
  typedef itk::InvertIntensityImageFilter <UnsignedCharImageType>
          InvertIntensityImageFilterType;

  InvertIntensityImageFilterType::Pointer invertIntensityFilter
          = InvertIntensityImageFilterType::New();
  invertIntensityFilter->SetInput(duplicator->GetOutput());
  invertIntensityFilter->InPlaceOn();
  invertIntensityFilter->Update();

  // Convert the inverted mask to floats and scale them to between 0 and 1
  // to serve as the initial confidence image
  typedef itk::RescaleIntensityImageFilter< UnsignedCharImageType, FloatImageType > RescaleFilterType;
  RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetInput(invertIntensityFilter->GetOutput());
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(1);
  rescaleFilter->Update();

  this->ConfidenceImage->Graft(rescaleFilter->GetOutput());
  WriteImage<FloatImageType>(this->ConfidenceImage, "InitialConfidence.mhd");

  // Mask the input image with the inverted mask (blank the region in the input image that we will fill in)
  typedef itk::MaskImageFilter< ColorImageType, UnsignedCharImageType, ColorImageType > MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput1(this->Image);
  maskFilter->SetInput2(invertIntensityFilter->GetOutput());
  maskFilter->InPlaceOn();
  ColorImageType::PixelType green;
  green[0] = 0;
  green[1] = 255;
  green[2] = 0;
  maskFilter->SetOutsideValue(green);
  maskFilter->Update();

  this->Image->Graft(maskFilter->GetOutput());

  int iteration = 0;
  while(HasMoreToInpaint(this->Mask))
    {
    std::cout << "Iteration: " << iteration << std::endl;

    FindBoundary();

    ComputeBoundaryNormals();

    ComputeAllPriorities();

    itk::Index<2> pixelToFill = FindHighestPriority(this->PriorityImage);
    std::cout << "Filling: " << pixelToFill << std::endl;

    itk::Index<2> bestMatchPixel = FindBestPatchMatch(pixelToFill);

    std::cout << "Best match found at: " << bestMatchPixel << std::endl;

    // Extract the best patch
    typedef itk::RegionOfInterestImageFilter< ColorImageType,
                                                ColorImageType > ExtractFilterType;

    ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
    extractFilter->SetRegionOfInterest(GetRegionAroundPixel(bestMatchPixel));
    extractFilter->SetInput(this->Image);
    extractFilter->Update();
    //this->Patch = extractFilter->GetOutput();
    this->Patch->Graft(extractFilter->GetOutput());

    //CopyPatchIntoImage(this->Patch, this->Image, pixelToFill);
    CopyPatchIntoTargetRegion<ColorImageType>(this->Patch, this->Image, pixelToFill);

    WriteDebugImages(pixelToFill, iteration);

    UpdateConfidenceImage(bestMatchPixel, pixelToFill);
    UpdateIsophoteImage(bestMatchPixel, pixelToFill);

    // Update the mask
    UpdateMask(pixelToFill);

    // Sanity check everything
    //DebugTests();


    iteration++;
    } // end main while loop

  WriteImage<ColorImageType>(this->Image, "result.jpg");

}

void CriminisiInpainting::WriteDebugImages(FloatImageType::IndexType pixelToFill, unsigned int iteration)
{
  std::cout << "Writing debug images for pixelToFill: " << pixelToFill << std::endl;
  {
  typedef itk::RegionOfInterestImageFilter< ColorImageType,
                                            ColorImageType > ExtractFilterType;

  ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
  extractFilter->SetRegionOfInterest(GetRegionAroundPixel(pixelToFill));
  extractFilter->SetInput(this->Image);
  extractFilter->Update();

  std::stringstream padded;
  padded << "PatchToFill_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  WriteImage<ColorImageType>(extractFilter->GetOutput(), padded.str());
  }

  /*
  {
  std::stringstream padded;
  padded << "CIELAB_correlation_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  WriteImage<FloatImageType>(this->MeanDifferenceImage, padded.str());
  }
  */
  
  // Create a blank image with the pixel to fill colored white
  {
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

  // Create a blank image with the patch that has been filled colored white
  {
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
  CreateConstantPatch<UnsignedCharImageType>(patch, 255);

  UnsignedCharImageType::IndexType lowerLeft;
  lowerLeft[0] = pixelToFill[0] - this->Patch->GetLargestPossibleRegion().GetSize()[0]/2;
  lowerLeft[1] = pixelToFill[1] - this->Patch->GetLargestPossibleRegion().GetSize()[1]/2;
  typedef itk::PasteImageFilter <UnsignedCharImageType, UnsignedCharImageType >
          PasteImageFilterType;

  PasteImageFilterType::Pointer pasteFilter
          = PasteImageFilterType::New ();
  pasteFilter->SetInput(0, patchImage);
  pasteFilter->SetInput(1, patch);
  pasteFilter->SetSourceRegion(patch->GetLargestPossibleRegion());
  pasteFilter->SetDestinationIndex(lowerLeft);
  pasteFilter->InPlaceOn();
  pasteFilter->Update();

  std::stringstream padded;
  padded << "PatchToFillLocation_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  WriteImage<UnsignedCharImageType>(pasteFilter->GetOutput(), padded.str());
  }

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
  //padded << "Boundary_" << std::setfill('0') << std::setw(4) << iteration << ".jpg";
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
  padded << "FilledImage_" << std::setfill('0') << std::setw(4) << iteration << ".jpg";
  WriteImage<ColorImageType>(this->Image, padded.str());
  }
}


void CriminisiInpainting::ComputeIsophotes()
{
  // Convert the color input image to a grayscale image
  UnsignedCharImageType::Pointer grayscaleImage = UnsignedCharImageType::New();
  ColorToGrayscale(this->Image, grayscaleImage);
  WriteImage<UnsignedCharImageType>(grayscaleImage, "greyscale.mhd");

  // Blur the image to compute better gradient estimates
  typedef itk::DiscreteGaussianImageFilter<
          UnsignedCharImageType, FloatImageType >  filterType;

  // Create and setup a Gaussian filter
  filterType::Pointer gaussianFilter = filterType::New();
  gaussianFilter->SetInput(grayscaleImage);
  gaussianFilter->SetVariance(2);
  gaussianFilter->Update();

  WriteImage<FloatImageType>(gaussianFilter->GetOutput(), "gaussianBlur.mhd");

  // Compute the gradient
  // TInputImage, TOperatorValueType, TOutputValueType
  typedef itk::GradientImageFilter<
      FloatImageType, float, float>  GradientFilterType;
  GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
  gradientFilter->SetInput(gaussianFilter->GetOutput());
  gradientFilter->Update();

  WriteImage<VectorImageType>(gradientFilter->GetOutput(), "gradient.mhd");

  // Rotate the gradient 90 degrees to obtain isophotes from gradient
  typedef itk::UnaryFunctorImageFilter<VectorImageType, VectorImageType,
                                  RotateVectors<
    VectorImageType::PixelType,
    VectorImageType::PixelType> > FilterType;

  FilterType::Pointer rotateFilter = FilterType::New();
  rotateFilter->SetInput(gradientFilter->GetOutput());
  rotateFilter->Update();

  WriteImage<VectorImageType>(rotateFilter->GetOutput(), "originalIsophotes.mhd");

  // Mask the isophote image with the expanded version of the inpainting mask.
  // That is, keep only the values outside of the expanded mask. To do this, we have to first invert the mask.

  // Invert the mask
  typedef itk::InvertIntensityImageFilter <UnsignedCharImageType>
          InvertIntensityImageFilterType;

  InvertIntensityImageFilterType::Pointer invertMaskFilter
          = InvertIntensityImageFilterType::New();
  invertMaskFilter->SetInput(this->Mask);
  invertMaskFilter->Update();

  WriteScaledImage<UnsignedCharImageType>(invertMaskFilter->GetOutput(), "invertedExpandedMask.mhd");

  // Keep only values outside the masked region
  typedef itk::MaskImageFilter< VectorImageType, UnsignedCharImageType, VectorImageType > MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput1(rotateFilter->GetOutput());
  maskFilter->SetInput2(invertMaskFilter->GetOutput());
  maskFilter->Update();

  this->IsophoteImage->Graft(maskFilter->GetOutput());
  WriteImage<VectorImageType>(this->IsophoteImage, "validIsophotes.mhd");
}

bool CriminisiInpainting::HasMoreToInpaint(UnsignedCharImageType::Pointer mask)
{
  itk::ImageRegionIterator<UnsignedCharImageType> imageIterator(mask,mask->GetLargestPossibleRegion());

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
  typedef itk::InvertIntensityImageFilter <UnsignedCharImageType>
          InvertIntensityImageFilterType;

  InvertIntensityImageFilterType::Pointer invertIntensityFilter
          = InvertIntensityImageFilterType::New();
  invertIntensityFilter->SetInput(this->Mask);
  invertIntensityFilter->Update();

  // Find the boundary
  typedef itk::BinaryContourImageFilter <UnsignedCharImageType, UnsignedCharImageType >
          binaryContourImageFilterType;

  binaryContourImageFilterType::Pointer binaryContourFilter
          = binaryContourImageFilterType::New ();
  binaryContourFilter->SetInput(invertIntensityFilter->GetOutput());
  binaryContourFilter->Update();

  //this->BoundaryImage = binaryContourFilter->GetOutput();
  this->BoundaryImage->Graft(binaryContourFilter->GetOutput());

  WriteImage<UnsignedCharImageType>(this->BoundaryImage, "Boundary.mhd");
}

template <class T>
void CriminisiInpainting::CreateBlankPatch(typename T::Pointer patch)
{
  typename T::IndexType start;
  start.Fill(0);

  typename T::RegionType region(start, this->GetPatchSize());

  patch->SetRegions(region);
  patch->Allocate();

  itk::ImageRegionIterator<T> imageIterator(patch, patch->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    imageIterator.Set(0);
    ++imageIterator;
    }
}

template <class T>
void CriminisiInpainting::CreateConstantPatch(typename T::Pointer patch, unsigned char value)
{
  typename T::IndexType start;
  start[0] = 0;
  start[1] = 0;

  typename T::RegionType region(start, this->GetPatchSize());

  patch->SetRegions(region);
  patch->Allocate();

  itk::ImageRegionIterator<T> imageIterator(patch, patch->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    imageIterator.Set(value);
    ++imageIterator;
    }
}


void CriminisiInpainting::CreateConstantFloatBlockingPatch(FloatImageType::Pointer patch, float value)
{
  // Create a 3-patch x 3-patch patch
  FloatImageType::IndexType start;
  start[0] = 0;
  start[1] = 0;

  FloatImageType::SizeType size;
  size[0] = 3*this->GetPatchSize()[0];
  size[1] = 3*this->GetPatchSize()[1];

  FloatImageType::RegionType region(start, size);

  patch->SetRegions(region);
  patch->Allocate();

  itk::ImageRegionIterator<FloatImageType> imageIterator(patch, patch->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    imageIterator.Set(value);
    ++imageIterator;
    }
}

void CriminisiInpainting::UpdateMask(UnsignedCharImageType::IndexType pixel)
{
  // Create a black patch
  UnsignedCharImageType::Pointer blackPatch = UnsignedCharImageType::New();
  CreateBlankPatch<UnsignedCharImageType>(blackPatch);

  pixel[0] -= blackPatch->GetLargestPossibleRegion().GetSize()[0]/2;
  pixel[1] -= blackPatch->GetLargestPossibleRegion().GetSize()[1]/2;

  // Paste it into the mask
  typedef itk::PasteImageFilter <UnsignedCharImageType, UnsignedCharImageType >
          PasteImageFilterType;

  PasteImageFilterType::Pointer pasteFilter
          = PasteImageFilterType::New();
  pasteFilter->SetInput(0, this->Mask);
  pasteFilter->SetInput(1, blackPatch);
  pasteFilter->SetSourceRegion(blackPatch->GetLargestPossibleRegion());
  pasteFilter->SetDestinationIndex(pixel);
  pasteFilter->InPlaceOn();
  pasteFilter->Update();

  this->Mask = pasteFilter->GetOutput();
}

void CriminisiInpainting::ComputeBoundaryNormals()
{
  // Blur the mask, compute the gradient, then keep the normals only at the original mask boundary

  typedef itk::DiscreteGaussianImageFilter<
          UnsignedCharImageType, FloatImageType >  filterType;

  // Create and setup a Gaussian filter
  filterType::Pointer gaussianFilter = filterType::New();
  gaussianFilter->SetInput(this->Mask);
  gaussianFilter->SetVariance(2);
  gaussianFilter->Update();

  typedef itk::GradientImageFilter<
      FloatImageType, float, float>  GradientFilterType;
  GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
  gradientFilter->SetInput(gaussianFilter->GetOutput());
  gradientFilter->Update();

  // Only keep the normals at the boundary
  typedef itk::MaskImageFilter< VectorImageType, UnsignedCharImageType, VectorImageType > MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput1(gradientFilter->GetOutput());
  maskFilter->SetInput2(this->BoundaryImage);
  maskFilter->Update();

  //this->BoundaryNormals = maskFilter->GetOutput();
  this->BoundaryNormals->Graft(maskFilter->GetOutput());

  // Normalize
  itk::ImageRegionIterator<VectorImageType> imageIterator(this->BoundaryNormals,this->BoundaryNormals->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<UnsignedCharImageType> boundaryIterator(this->BoundaryImage,this->BoundaryImage->GetLargestPossibleRegion());
  imageIterator.GoToBegin();
  boundaryIterator.GoToBegin();

  while(!imageIterator.IsAtEnd())
    {
    if(boundaryIterator.Get()) // The pixel is on the boundary
      {
      VectorImageType::PixelType p = imageIterator.Get();
      p.Normalize();
      imageIterator.Set(p);
      }
    ++imageIterator;
    ++boundaryIterator;
    }
}

itk::Index<2> CriminisiInpainting::FindHighestPriority(FloatImageType::Pointer priorityImage)
{
  typedef itk::MinimumMaximumImageCalculator <FloatImageType>
          ImageCalculatorFilterType;

  ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(priorityImage);
  imageCalculatorFilter->Compute();

  return imageCalculatorFilter->GetIndexOfMaximum();
}


#if 0 // CIELab matching
ColorImageType::IndexType CriminisiInpainting::FindBestPatchMatch(itk::Index<2> queryPixel)
{
  if(!this->Image->GetLargestPossibleRegion().IsInside(GetRegionAroundPixel(queryPixel)))
    {
    std::cerr << "queryPixel is too close to the border!" << std::endl;
    exit(-1);
    }

  // Setup a neighborhood iterator over the whole CIELAB image
  itk::ConstNeighborhoodIterator<CIELABImageType, CIELABBoundaryConditionType> movingCIELABPatchIterator(this->PatchRadius, this->CIELABImage, this->CIELABImage->GetLargestPossibleRegion());
  movingCIELABPatchIterator.GoToBegin();

  // Setup a neighborhood iterator over the whole Mask image
  itk::ConstNeighborhoodIterator<UnsignedCharImageType, UnsignedCharBoundaryConditionType> movingMaskPatchIterator(this->PatchRadius, this->Mask, this->Mask->GetLargestPossibleRegion());
  movingMaskPatchIterator.GoToBegin();

  itk::ImageRegionIterator<FloatImageType> meanDifferenceIterator(this->MeanDifferenceImage, this->MeanDifferenceImage->GetLargestPossibleRegion());
  meanDifferenceIterator.GoToBegin();

  // Setup an iterator over the fixed cielab image patch
  itk::ImageRegionConstIterator<CIELABImageType> fixedCIELABPatchIterator(this->CIELABImage, GetRegionAroundPixel(queryPixel));
  fixedCIELABPatchIterator.GoToBegin();

  // Setup an iterator over the fixed mask patch
  itk::ImageRegionConstIterator<UnsignedCharImageType> fixedMaskPatchIterator(this->Mask, GetRegionAroundPixel(queryPixel));
  fixedMaskPatchIterator.GoToBegin();

  unsigned int numberOfPixelsInNeighborhood = this->GetPatchSize()[0] * this->GetPatchSize()[1];
  unsigned int minValidPixels = numberOfPixelsInNeighborhood/4;
  // Loop over every pixel in the image with a neighborhood
  while(!movingCIELABPatchIterator.IsAtEnd())
    {
    double sumDifference = 0;
    fixedCIELABPatchIterator.GoToBegin();
    fixedMaskPatchIterator.GoToBegin();

    // Only consider pixels which have a neighborhood entirely inside of the image
    if(!movingCIELABPatchIterator.InBounds())
      {
      meanDifferenceIterator.Set(-1);
      ++movingCIELABPatchIterator;
      ++movingMaskPatchIterator;
      ++meanDifferenceIterator;
      continue;
      }

    unsigned int valid = 0;
    unsigned int invalid = 0;
    // Loop over all pixels in the neighborhood of the current pixel
    for(unsigned int i = 0; i < numberOfPixelsInNeighborhood; i++)
      {
      // Only compare pixels in the source region
      if(fixedMaskPatchIterator.Get() == 0)
        {
        if(movingMaskPatchIterator.GetPixel(i) != 0)
          {
          invalid++;
          continue;
          }

        valid++;
        CIELABImageType::PixelType patchPixel = fixedCIELABPatchIterator.Get();
        CIELABImageType::PixelType cielabPixel = movingCIELABPatchIterator.GetPixel(i);

        for(unsigned int p = 0; p < 3; p++)
          {
          // CIELAB difference
          sumDifference += std::abs(static_cast<float>(cielabPixel[p]) - static_cast<float>(patchPixel[p]));
          }
        }

      ++fixedCIELABPatchIterator;
      ++fixedMaskPatchIterator;
      }

    if(valid > minValidPixels && invalid == 0)
      {
      meanDifferenceIterator.Set(sumDifference/static_cast<double>(valid));
      }
    else
      {
      meanDifferenceIterator.Set(-1);
      }

    ++meanDifferenceIterator;
    ++movingCIELABPatchIterator;
    ++movingMaskPatchIterator;
    }

  // Compute the maximum value of the difference image. We will use this to fill the constant blank patch
  typedef itk::MinimumMaximumImageCalculator <FloatImageType>
          ImageCalculatorFilterType;

  ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(this->MeanDifferenceImage);
  imageCalculatorFilter->Compute();

  float highestValue = imageCalculatorFilter->GetMaximum();

  // Paste a blank patch into the correlation image so that we don't match a region within the current patch or anywhere near it
  FloatImageType::Pointer blankPatch = FloatImageType::New();
  CreateConstantFloatBlockingPatch(blankPatch, highestValue);
  CopyPatchIntoImage<FloatImageType>(blankPatch, this->MeanDifferenceImage, queryPixel);

  // Blank the masked region in the difference image
  //meanDifferenceIterator.GoToBegin(); // Don't know why we can't reuse this iterator - Arithmetic exception is thrown from itkImageHelper
  itk::ImageRegionIterator<FloatImageType> correlationIterator(this->MeanDifferenceImage,this->MeanDifferenceImage->GetLargestPossibleRegion());
  correlationIterator.GoToBegin();

  itk::ImageRegionConstIterator<UnsignedCharImageType> maskIterator(this->Mask,this->Mask->GetLargestPossibleRegion());
  maskIterator.GoToBegin();

  while(!maskIterator.IsAtEnd())
    {
    if(maskIterator.Get() != 0)
      {
      correlationIterator.Set(highestValue);
      }

    ++maskIterator;
    ++correlationIterator;
    }

  // Replace -1's with the highest value (we do NOT want to choose these points)
  ReplaceValue(this->MeanDifferenceImage, -1, highestValue);
  WriteImage<FloatImageType>(this->MeanDifferenceImage, "BlankedCorrelation.mhd");

  imageCalculatorFilter->SetImage(this->MeanDifferenceImage);
  imageCalculatorFilter->Compute();

  UnsignedCharImageType::IndexType bestMatchIndex = imageCalculatorFilter->GetIndexOfMinimum();
  std::cout << "BestMatchIndex: " << bestMatchIndex << std::endl;
  return bestMatchIndex;
}
#endif

template <class T>
void CriminisiInpainting::CopyPatchIntoImage(typename T::Pointer patch, typename T::Pointer &image, itk::Index<2> position)
{
  typedef itk::PasteImageFilter<T, T> PasteImageFilterType;

  // We have passed the center of the patch, but the paste filter wants the lower left corner of the patch
  position[0] -= patch->GetLargestPossibleRegion().GetSize()[0]/2;
  position[1] -= patch->GetLargestPossibleRegion().GetSize()[1]/2;

  typename PasteImageFilterType::Pointer pasteFilter
          = PasteImageFilterType::New();
  pasteFilter->SetInput(0, image);
  pasteFilter->SetInput(1, patch);
  pasteFilter->SetSourceRegion(patch->GetLargestPossibleRegion());
  pasteFilter->SetDestinationIndex(position);
  pasteFilter->InPlaceOn();
  pasteFilter->Update();

  image = pasteFilter->GetOutput();
}

template <class T>
void CriminisiInpainting::CopyPatchIntoTargetRegion(typename T::Pointer patch, typename T::Pointer image, itk::Index<2> position)
{
  itk::ImageRegionConstIterator<T> patchIterator(patch,patch->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<UnsignedCharImageType> maskIterator(this->Mask,GetRegionAroundPixel(position));
  itk::ImageRegionIterator<T> imageIterator(image, GetRegionAroundPixel(position));

  while(!patchIterator.IsAtEnd())
    {
    if(maskIterator.Get()) // we are in the target region
      {
      imageIterator.Set(patchIterator.Get());
      }
    ++imageIterator;
    ++maskIterator;
    ++patchIterator;
    }
}


void CriminisiInpainting::ComputeAllPriorities()
{
  // Only compute priorities for pixels on the boundary
  itk::ImageRegionConstIterator<UnsignedCharImageType> boundaryIterator(this->BoundaryImage, this->BoundaryImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<FloatImageType> priorityIterator(this->PriorityImage, this->PriorityImage->GetLargestPossibleRegion());

  boundaryIterator.GoToBegin();
  priorityIterator.GoToBegin();

  // The main loop is over the boundary image. We only want to compute priorities at boundary pixels
  while(!boundaryIterator.IsAtEnd())
    {
    // If the pixel is not on the boundary, skip it and set its priority to -1.
    // -1 is used instead of 0 because if the priorities on the boundary get to 0, we still want to choose a boundary
    // point rather than a random image point.
    if(boundaryIterator.Get() == 0)
      {
      priorityIterator.Set(-1);
      ++boundaryIterator;
      ++priorityIterator;
      continue;
      }

    float priority = ComputePriority(boundaryIterator.GetIndex());
    //std::cout << "Priority: " << priority << std::endl;
    priorityIterator.Set(priority);

    ++boundaryIterator;
    ++priorityIterator;
    }
}

float CriminisiInpainting::ComputePriority(UnsignedCharImageType::IndexType queryPixel)
{
  double confidence = ComputeConfidenceTerm(queryPixel);
  double data = ComputeDataTerm(queryPixel);

  return confidence * data;
}


float CriminisiInpainting::ComputeConfidenceTerm(itk::Index<2> queryPixel)
{
  if(!this->Mask->GetLargestPossibleRegion().IsInside(GetRegionAroundPixel(queryPixel,this->PatchRadius[0])))
    {
    return 0;
    }

  itk::ImageRegionConstIterator<UnsignedCharImageType> maskIterator(this->Mask, GetRegionAroundPixel(queryPixel));
  itk::ImageRegionConstIterator<FloatImageType> confidenceIterator(this->ConfidenceImage, GetRegionAroundPixel(queryPixel));
  maskIterator.GoToBegin();
  confidenceIterator.GoToBegin();
  // confidence = sum of the confidences of patch pixels in the source region / area of the patch

  unsigned int numberOfPixels = GetNumberOfPixelsInPatch();

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

  float areaOfPatch = static_cast<float>(numberOfPixels);
  return sum/areaOfPatch;
}

float CriminisiInpainting::ComputeDataTerm(UnsignedCharImageType::IndexType queryPixel)
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

bool CriminisiInpainting::IsValidPatch(itk::Index<2> queryPixel, unsigned int radius)
{
  // Check if the patch is inside the image
  if(!this->Mask->GetLargestPossibleRegion().IsInside(GetRegionAroundPixel(queryPixel,radius)))
    {
    return false;
    }

  // Check if all the pixels in the patch are in the valid region of the image
  itk::ImageRegionConstIterator<UnsignedCharImageType> iterator(this->Mask,GetRegionAroundPixel(queryPixel, radius));
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

itk::CovariantVector<float, 2> CriminisiInpainting::GetAverageIsophote(UnsignedCharImageType::IndexType queryPixel)
{
  if(!this->Mask->GetLargestPossibleRegion().IsInside(GetRegionAroundPixel(queryPixel)))
    {
    itk::CovariantVector<float, 2> v;
    v[0] = 0; v[1] = 0;
    return v;
    }

  itk::ImageRegionConstIterator<UnsignedCharImageType> iterator(this->Mask,GetRegionAroundPixel(queryPixel));

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

UnsignedCharImageType::RegionType CriminisiInpainting::GetRegionAroundPixel(UnsignedCharImageType::IndexType pixel)
{
  return GetRegionAroundPixel(pixel, this->GetPatchSize()[0]/2);
}

UnsignedCharImageType::RegionType CriminisiInpainting::GetRegionAroundPixel(UnsignedCharImageType::IndexType pixel, unsigned int radius)
{
  pixel[0] -= radius;
  pixel[1] -= radius;

  UnsignedCharImageType::RegionType region;
  region.SetIndex(pixel);
  UnsignedCharImageType::SizeType size;
  size[0] = radius*2 + 1;
  size[1] = radius*2 + 1;
  region.SetSize(size);

  return region;
}

void CriminisiInpainting::UpdateConfidenceImage(FloatImageType::IndexType sourcePixel, FloatImageType::IndexType targetPixel)
{
  // Loop over all pixels in the target region which were just filled and update their
  itk::ImageRegionIterator<FloatImageType> confidenceIterator(this->ConfidenceImage, GetRegionAroundPixel(targetPixel));
  itk::ImageRegionIterator<UnsignedCharImageType> maskIterator(this->Mask, GetRegionAroundPixel(targetPixel));
  confidenceIterator.GoToBegin();
  maskIterator.GoToBegin();

  while(!maskIterator.IsAtEnd())
    {
    if(maskIterator.Get()) // This was a target pixel, compute its confidence
      {
      confidenceIterator.Set(ComputeConfidenceTerm(maskIterator.GetIndex()));
      }
    ++confidenceIterator;
    ++maskIterator;
    }
}

void CriminisiInpainting::UpdateIsophoteImage(FloatImageType::IndexType sourcePixel, itk::Index<2> targetPixel)
{
  // Copy isophotes from best patch
  typedef itk::RegionOfInterestImageFilter< VectorImageType,
                                            VectorImageType > ExtractFilterType;

  ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
  extractFilter->SetRegionOfInterest(GetRegionAroundPixel(sourcePixel));
  extractFilter->SetInput(this->IsophoteImage);
  extractFilter->Update();

  VectorImageType::Pointer image = extractFilter->GetOutput();

  itk::ImageRegionIterator<VectorImageType> imageIterator(image,image->GetLargestPossibleRegion());
  itk::ImageRegionIterator<UnsignedCharImageType> maskIterator(this->Mask, GetRegionAroundPixel(sourcePixel));
  imageIterator.GoToBegin();
  maskIterator.GoToBegin();

  // "clear" the pixels which are in the target region.
  VectorImageType::PixelType blankPixel;
  blankPixel[0] = 0;
  blankPixel[1] = 0;

  while(!imageIterator.IsAtEnd())
  {
    if(maskIterator.Get() != 0) // we are in the target region
      {
      imageIterator.Set(blankPixel);
      }

    ++imageIterator;
    ++maskIterator;
  }

  CopyPatchIntoTargetRegion<VectorImageType>(image, this->IsophoteImage, targetPixel);
}

void CriminisiInpainting::ColorToGrayscale(ColorImageType::Pointer colorImage, UnsignedCharImageType::Pointer grayscaleImage)
{
  grayscaleImage->SetRegions(colorImage->GetLargestPossibleRegion());
  grayscaleImage->Allocate();

  itk::ImageRegionConstIterator<ColorImageType> colorImageIterator(colorImage,colorImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<UnsignedCharImageType> grayscaleImageIterator(grayscaleImage,grayscaleImage->GetLargestPossibleRegion());

  ColorImageType::PixelType largestPixel;
  largestPixel[0] = 255;
  largestPixel[1] = 255;
  largestPixel[2] = 255;

  float largestNorm = largestPixel.GetNorm();

  while(!colorImageIterator.IsAtEnd())
  {
    grayscaleImageIterator.Set(colorImageIterator.Get().GetNorm()*(255./largestNorm));

    ++colorImageIterator;
    ++grayscaleImageIterator;
  }
}

unsigned int CriminisiInpainting::GetNumberOfPixelsInPatch()
{
  return this->GetPatchSize()[0]*this->GetPatchSize()[1];
}

void CriminisiInpainting::CreateBorderMask(FloatImageType::Pointer image)
{
  itk::NeighborhoodIterator<FloatImageType, FloatBoundaryConditionType> imageIterator(this->PatchRadius, image, image->GetLargestPossibleRegion());
  imageIterator.GoToBegin();
  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.InBounds())
      {
      imageIterator.SetCenterPixel(1);
      }
    else
      {
      imageIterator.SetCenterPixel(0);
      }
    ++imageIterator;
    }
}

void CriminisiInpainting::ReplaceValue(FloatImageType::Pointer image, float oldValue, float newValue)
{
  itk::ImageRegionIterator<FloatImageType> imageIterator(image, image->GetLargestPossibleRegion());
  imageIterator.GoToBegin();
  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.Get() == oldValue)
      {
      imageIterator.Set(newValue);
      }
    ++imageIterator;
    }
}

itk::Size<2> CriminisiInpainting::GetPatchSize()
{
  itk::Size<2> patchSize;

  patchSize[0] = (this->PatchRadius[0]*2)+1;
  patchSize[1] = (this->PatchRadius[1]*2)+1;

  return patchSize;
}

void CriminisiInpainting::DebugTests()
{
  // Check to make sure patch doesn't have any masked pixels
  itk::ImageRegionIterator<ColorImageType> patchIterator(this->Patch,this->Patch->GetLargestPossibleRegion());

  while(!patchIterator.IsAtEnd())
    {
    ColorImageType::PixelType val = patchIterator.Get();
    if(val[0] == 0 && val[1] == 255 && val[2] == 0)
      {
      std::cerr << "Patch has a blank pixel!" << std::endl;
      exit(-1);
      }
    ++patchIterator;
    }
}

template <class T>
void CriminisiInpainting::WriteScaledImage(typename T::Pointer image, std::string filename)
{
  typedef itk::RescaleIntensityImageFilter<T, UnsignedCharImageType> RescaleFilterType; // expected ';' before rescaleFilter

  typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetInput(image);
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(255);
  rescaleFilter->Update();

  typename itk::ImageFileWriter<T>::Pointer writer = itk::ImageFileWriter<T>::New();
  writer->SetFileName(filename);
  writer->SetInput(image);
  writer->Update();
}

template <class T>
void CriminisiInpainting::WriteImage(typename T::Pointer image, std::string filename)
{
  typename itk::ImageFileWriter<T>::Pointer writer = itk::ImageFileWriter<T>::New();
  writer->SetFileName(filename);
  writer->SetInput(image);
  writer->Update();
}

void CriminisiInpainting::CreateCIELABImage()
{
  this->CIELABImage->SetRegions(this->Image->GetLargestPossibleRegion());
  this->CIELABImage->Allocate();

  itk::ImageRegionConstIterator<ColorImageType> rgbIterator(this->Image,this->Image->GetLargestPossibleRegion());
  itk::ImageRegionIterator<CIELABImageType> cielabIterator(this->CIELABImage,this->CIELABImage->GetLargestPossibleRegion());

  rgbIterator.GoToBegin();
  cielabIterator.GoToBegin();

  while(!rgbIterator.IsAtEnd())
    {
    itk::CovariantVector<int,3> cielab = RGBtoCIELAB(rgbIterator.Get());
    cielabIterator.Set(cielab);

    ++rgbIterator;
    ++cielabIterator;
    }

}

itk::CovariantVector<int ,3> CriminisiInpainting::RGBtoCIELAB(itk::CovariantVector<unsigned char, 3> rgb)
{
  unsigned char R,G,B;
  R = rgb[0];
  G = rgb[1];
  B = rgb[2];

  float X, Y, Z, fX, fY, fZ;
  int a, b, L;

  X = 0.412453*R + 0.357580*G + 0.180423*B;
  Y = 0.212671*R + 0.715160*G + 0.072169*B;
  Z = 0.019334*R + 0.119193*G + 0.950227*B;

  X /= (255 * 0.950456);
  Y /=  255;
  Z /= (255 * 1.088754);

  if (Y > 0.008856)
    {
    fY = pow(Y, 1.0/3.0);
    L = (int)(116.0*fY - 16.0 + 0.5);
    }
  else
    {
    fY = 7.787*Y + 16.0/116.0;
    L = (int)(903.3*Y + 0.5);
    }

  if (X > 0.008856)
    {
    fX = pow(X, 1.0/3.0);
    }
  else
    {
    fX = 7.787*X + 16.0/116.0;
    }

  if (Z > 0.008856)
    {
    fZ = pow(Z, 1.0/3.0);
    }
  else
    {
    fZ = 7.787*Z + 16.0/116.0;
    }

  a = (int)(500.0*(fX - fY) + 0.5);
  b = (int)(200.0*(fY - fZ) + 0.5);

  itk::CovariantVector<int, 3> cielab;
  cielab[0] = L;
  cielab[1] = a;
  cielab[2] = b;

  return cielab;

//printf("RGB=(%d,%d,%d) ==> Lab(%d,%d,%d)\n",R,G,B,*L,*a,*b);
}
