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

ColorImageType::IndexType CriminisiInpainting::FindBestPatchMatch(itk::Index<2> queryPixel)
{
  // CIELab matching
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



void CriminisiInpainting::UpdateIsophoteImage(FloatImageType::IndexType sourcePixel, itk::Index<2> targetPixel)
{
  // Copy isophotes from best patch
  typedef itk::RegionOfInterestImageFilter< VectorImageType,
                                            VectorImageType > ExtractFilterType;

  ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
  extractFilter->SetRegionOfInterest(GetRegionInRadiusAroundPixel(sourcePixel, this->PatchRadius[0]));
  extractFilter->SetInput(this->IsophoteImage);
  extractFilter->Update();

  VectorImageType::Pointer isophoteImagePatch = extractFilter->GetOutput();

  itk::ImageRegionIterator<VectorImageType> imageIterator(isophoteImagePatch, isophoteImagePatch->GetLargestPossibleRegion());
  itk::ImageRegionIterator<UnsignedCharImageType> maskIterator(this->Mask, GetRegionInRadiusAroundPixel(sourcePixel, this->PatchRadius[0]));
  imageIterator.GoToBegin();
  maskIterator.GoToBegin();

  // "clear" the pixels which are in the target region.
  VectorImageType::PixelType blankPixel;
  blankPixel.Fill(0);

  while(!imageIterator.IsAtEnd())
  {
    if(maskIterator.Get() != 0) // we are in the target region
      {
      imageIterator.Set(blankPixel);
      }

    ++imageIterator;
    ++maskIterator;
  }

  CopyPatchIntoImage<VectorImageType>(isophoteImagePatch, this->IsophoteImage, this->Mask, targetPixel);
}

void CriminisiInpainting::UpdateConfidenceImage(FloatImageType::IndexType sourcePixel, FloatImageType::IndexType targetPixel)
{
  // Loop over all pixels in the target region which were just filled and update their confidence.
  itk::ImageRegionIterator<FloatImageType> confidenceIterator(this->ConfidenceImage, GetRegionInRadiusAroundPixel(targetPixel, this->PatchRadius[0]));
  itk::ImageRegionIterator<UnsignedCharImageType> maskIterator(this->Mask, GetRegionInRadiusAroundPixel(targetPixel, this->PatchRadius[0]));
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

void something()
{
  // Extract the best patch
  typedef itk::RegionOfInterestImageFilter< ColorImageType,
                                            ColorImageType > ExtractFilterType;

  ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
  extractFilter->SetRegionOfInterest(GetRegionInRadiusAroundPixel(bestMatchPixel, this->PatchRadius[0]));
  extractFilter->SetInput(this->Image);
  extractFilter->Update();
  this->Patch->Graft(extractFilter->GetOutput());

  CopyPatchIntoImage<ColorImageType>(this->Patch, this->Image, this->Mask, pixelToFill);

}


template <class T>
void CopySelfPatchIntoValidRegion(typename T::Pointer image, UnsignedCharImageType::Pointer mask,
                                  itk::Index<2> sourcePixel, itk::Index<2> destinationPixel, unsigned int radius);

template <class T>
void CopySelfPatchIntoValidRegion(typename T::Pointer image, UnsignedCharImageType::Pointer mask, itk::Index<2> sourcePixel, itk::Index<2> destinationPixel, unsigned int patchRadius)
{
  try
  {
  // This function copies a patch of radius 'radius' into itself only where the 'mask' is non-zero.
  // 'mask' must be the same size as 'image'.

  itk::Size<2> radius;
  radius.Fill(patchRadius);

  itk::Size<2> onePixelSize;
  onePixelSize.Fill(1);

  itk::ImageRegion<2> sourceRegion(sourcePixel, onePixelSize); // This is a 1 pixel region indicating the center of the patch. The neighborhood iterator takes care of creating the actual region

  itk::ImageRegion<2> destinationRegion(destinationPixel, onePixelSize); // This is a 1 pixel region indicating the center of the patch. The neighborhood iterator takes care of creating the actual region

  itk::ConstNeighborhoodIterator<T> sourcePatchIterator(radius, image, sourceRegion);
  itk::NeighborhoodIterator<T> destinationPatchIterator(radius, image, destinationRegion);

  itk::ConstNeighborhoodIterator<UnsignedCharImageType> destinationMaskPatchIterator(radius, mask, destinationRegion);

  typename T::PixelType sourcePixelValue;
  typename T::PixelType destinationPixelValue;
  typename UnsignedCharImageType::PixelType maskPixelValue;

  while(!destinationPatchIterator.IsAtEnd())
    {
    for(unsigned int i = 0; i < (patchRadius*2 + 1)*(patchRadius*2 + 1); i++) // the number of pixels in the neighborhood
      {
      bool sourceIsInBounds;
      bool destinationIsInBounds;
      sourcePixelValue = sourcePatchIterator.GetPixel(i, sourceIsInBounds);
      destinationPixelValue = destinationPatchIterator.GetPixel(i, destinationIsInBounds);
      if(!(sourceIsInBounds && destinationIsInBounds))
        {
        // do nothing
        }
      else
        {
        bool inbounds; // this is not used
        maskPixelValue = destinationMaskPatchIterator.GetPixel(i, inbounds);

        if(maskPixelValue) // we are in the target region
          {
          destinationPatchIterator.SetPixel(i, sourcePixelValue);
          }
        }// end else
      } // end for loop over neighborhood pixels
      ++sourcePatchIterator;
      ++destinationMaskPatchIterator;
      ++destinationPatchIterator;
    } // end while loop (should only be one iteration)

  } // end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in CopySelfPatchIntoValidRegion!" << std::endl;
    std::cerr << err << std::endl;
    std::cout << "sourcePixel: " << sourcePixel
              << "destinationPixel: " << destinationPixel
              << "patchRadius: " << patchRadius << std::endl;
    exit(-1);
  }
}


// This version requires both patches to be entirely within the image region
template <class T>
void CopySelfPatchIntoValidRegion(typename T::Pointer image, UnsignedCharImageType::Pointer mask, itk::Index<2> sourcePixel, itk::Index<2> destinationPixel, unsigned int radius)
{
  try
  {
  // This function copies a patch of radius 'radius' into itself only where the 'mask' is non-zero.
  // 'mask' must be the same size as 'image'.

  itk::ImageRegion<2> sourceRegion = GetRegionInRadiusAroundPixel(sourcePixel, radius);
  itk::ImageRegion<2> destinationRegion = GetRegionInRadiusAroundPixel(destinationPixel, radius);

  itk::ImageRegionConstIterator<T> sourceIterator(image, sourceRegion);
  itk::ImageRegionConstIterator<UnsignedCharImageType> maskIterator(mask,destinationRegion);
  itk::ImageRegionIterator<T> destinationIterator(image, destinationRegion);

  while(!destinationIterator.IsAtEnd())
    {
    if(maskIterator.Get()) // we are in the target region
      {
      destinationIterator.Set(sourceIterator.Get());
      }
    ++sourceIterator;
    ++maskIterator;
    ++destinationIterator;
    }

  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in CopySelfPatchIntoValidRegion!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}


// This is the main driver function. It calls PatchImageDifference and then finds the best match in the area we want to match from and returns it.
// This was written for an inpainting algorithm - so the 'mask' should be interpreted as the region to inpaint
// (i.e. no data was defined in the 'image' in the 'mask != 0' region.)
// We want the best matching patch to come from a region that has no 'missing' pixels - i.e. it does not overlap the non-zero region of the mask.
template <typename TImage, typename TMask>
itk::Index<2> SelfPatchMatch(TImage* image, TMask* mask, itk::Index<2> queryPixel, unsigned int patchRadius, unsigned int iteration, std::vector<float> weights)
{
  try
  {
  FloatImageType::Pointer differenceImage = FloatImageType::New();

  ComputeDifferenceImage(image, mask, queryPixel, patchRadius, differenceImage, weights);

  // Compute the maximum value of the difference image. We will use this to fill the constant blank patch
  //float highestValue = MaxValue<FloatImageType>(differenceImage);
  float highestValue = itk::NumericTraits< typename FloatImageType::PixelType >::max();

  /*
  // ** Don't need this because we require the source patches to be complete - so if they are near the target region they will never get tried.
  // Paste a blank patch into the correlation image so that we don't match a region within the current patch or anywhere near it
  FloatImageType::Pointer blankPatch = FloatImageType::New();
  Helpers::CreateConstantPatch<FloatImageType>(blankPatch, highestValue, patchRadius * 2);
  Helpers::CopyPatchIntoImage<FloatImageType>(blankPatch, differenceImage, queryPixel);
  */

  // Set the differenceImage pixels in the masked region to a very high value (we don't want to match here)
  itk::ImageRegionIterator<FloatImageType> differenceImageIterator(differenceImage, differenceImage->GetLargestPossibleRegion());
  differenceImageIterator.GoToBegin();

  itk::ImageRegionConstIterator<TMask> maskIterator(mask, mask->GetLargestPossibleRegion());
  maskIterator.GoToBegin();

  while(!differenceImageIterator.IsAtEnd())
    {
    if(maskIterator.Get() > 0)
      {
      differenceImageIterator.Set(highestValue);
      }
    ++differenceImageIterator;
    ++maskIterator;
    }

  std::stringstream padded;
  padded << "Difference_" << std::setfill('0') << std::setw(4) << iteration << ".png";
  Helpers::WriteScaledScalarImage<FloatImageType>(differenceImage, padded.str());

  itk::Index<2> bestMatchIndex = Helpers::MinValueLocation<FloatImageType>(differenceImage);
  //std::cout << "BestMatchIndex: " << bestMatchIndex << std::endl;
  return bestMatchIndex;

  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in SelfPatchMatch!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <typename TImage, typename TMask>
itk::Index<2> SelfPatchMatch(TImage* image, TMask* mask, itk::Index<2> queryPixel, unsigned int patchRadius)
{
  unsigned int iteration = 0;
  std::vector<float> weights(TImage::PixelType::Dimension, 1.0/static_cast<float>(TImage::PixelType::Dimension));
  return SelfPatchMatch(image, mask, queryPixel, patchRadius, iteration, weights);
}


template <typename TImage, typename TMask>
itk::Index<2> SelfPatchMatch(TImage* image, TMask* mask, itk::Index<2> queryPixel, unsigned int patchRadius, unsigned int iteration, std::vector<float> weights);

template <typename TImage, typename TMask>
itk::Index<2> SelfPatchMatch(TImage* image, TMask* mask, itk::Index<2> queryPixel, unsigned int patchRadius);


template< typename TImage, typename TMask>
void ComputeDifferenceImage(const TImage* image, const TMask* mask,
                            itk::Index<2> queryPixel, unsigned int patchRadius, FloatImageType* output, std::vector<float> weights);



template< typename TImage, typename TMask>
void ComputeDifferenceImage(const TImage* image, const TMask* mask,
                            itk::Index<2> queryPixel, unsigned int patchRadius, FloatImageType* output, std::vector<float> weights)
{
  try
  {
  // This function moves a patch from 'image' centered at 'queryPixel' over the entire 'image' and computes the mean difference of the valid pixels at each location in the sweep.
  // Only patch with zero invalid (masked) pixels are compared.

  // Create an output image the same size as the input image
  output->SetRegions(image->GetLargestPossibleRegion());
  output->Allocate();

  typedef itk::ImageRegionConstIterator<TImage> ConstIteratorType;
  typedef itk::ImageRegionIterator<TImage> IteratorType;
  typedef itk::ImageRegionIterator<FloatImageType> OutputIteratorType;

  typedef itk::NeighborhoodAlgorithm
    ::ImageBoundaryFacesCalculator< TImage> FaceCalculatorType;

  FaceCalculatorType faceCalculator;

  typename FaceCalculatorType::FaceListType faceList;
  itk::Size<2> radiusSize;
  radiusSize.Fill(patchRadius);
  faceList = faceCalculator(image, image->GetLargestPossibleRegion(), radiusSize);

  typename FaceCalculatorType::FaceListType::iterator faceListIterator = faceList.begin();

  itk::ImageRegionConstIteratorWithIndex<TImage> centralIterator(image,*faceListIterator);
  OutputIteratorType outputIterator(output,*faceListIterator);
  centralIterator.GoToBegin();
  outputIterator.GoToBegin();

  while(!centralIterator.IsAtEnd())
    {
    float difference = PatchDifference(image, mask, queryPixel, centralIterator.GetIndex(), patchRadius, weights);
    outputIterator.Set(difference);

    ++centralIterator;
    ++outputIterator;
    }

  ++faceListIterator;
  // Iterate over all of the boundary regions setting their distance to a very high value

  while(faceListIterator != faceList.end())
    {
    OutputIteratorType outputFaceIterator(output,*faceListIterator);
    outputFaceIterator.GoToBegin();
    while(!outputFaceIterator.IsAtEnd())
      {
      outputFaceIterator.Set(itk::NumericTraits< typename FloatImageType::PixelType >::max());
      ++outputFaceIterator;
      }
    ++faceListIterator;
    }

  Helpers::WriteScaledScalarImage<FloatImageType>(output, "DifferenceImage.png");

  }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeDifferenceImage!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}


float CriminisiInpainting::HistogramDifference1D(const Patch& patch1, const Patch& patch2)
{
  // N 1D histograms
  std::vector<HistogramType::Pointer> sourceHistograms = Helpers::ComputeHistogramsOfRegionManual(this->CurrentOutputImage, patch1.Region);
  std::cout << "Source histograms: " << std::endl;
  for(unsigned int i = 0; i < sourceHistograms.size(); ++i)
    {
    Helpers::OutputHistogram(sourceHistograms[i]);
    }

  std::vector<HistogramType::Pointer> targetHistograms = Helpers::ComputeHistogramsOfMaskedRegion(this->CurrentOutputImage, this->CurrentMask, patch2.Region);
  std::cout << "Target histograms: " << std::endl;
  for(unsigned int i = 0; i < targetHistograms.size(); ++i)
    {
    Helpers::OutputHistogram(targetHistograms[i]);
    }

  float totalDifference = 0;
  for(unsigned int i = 0; i < targetHistograms.size(); ++i)
    {
    totalDifference += Helpers::HistogramDifference(sourceHistograms[i], targetHistograms[i]);
    }

  std::cout << "Histogram difference: " << totalDifference << std::endl;
  return totalDifference;
}


void CriminisiInpainting::UpdateMask(const itk::ImageRegion<2> region)
{
  try
  {
    /*
    // Create a black patch
    Mask::Pointer blackPatch = Mask::New();
    //Helpers::CreateBlankPatch<UnsignedCharScalarImageType>(blackPatch, this->PatchRadius[0]);
    Helpers::CreateConstantPatch<Mask>(blackPatch, this->CurrentMask->GetValidValue(), this->PatchRadius[0]);

    // Paste it into the mask
    typedef itk::PasteImageFilter <Mask, Mask> PasteImageFilterType;
    PasteImageFilterType::Pointer pasteFilter = PasteImageFilterType::New();
    //pasteFilter->SetInput(0, this->CurrentMask);
    //pasteFilter->SetInput(1, blackPatch);
    pasteFilter->SetSourceImage(blackPatch);
    pasteFilter->SetDestinationImage(this->CurrentMask);
    pasteFilter->SetSourceRegion(blackPatch->GetLargestPossibleRegion());
    pasteFilter->SetDestinationIndex(region.GetIndex());
    pasteFilter->Update();

    // Not sure how Mask::DeepCopyFrom would work on the output of a filter, so do this manually.
    Helpers::DeepCopy<Mask>(pasteFilter->GetOutput(), this->CurrentMask);
    this->CurrentMask->SetHoleValue(this->OriginalMask->GetHoleValue());
    this->CurrentMask->SetValidValue(this->OriginalMask->GetValidValue());
    */

    itk::ImageRegionIterator<Mask> maskIterator(this->CurrentMask, region);

    while(!maskIterator.IsAtEnd())
      {
      if(this->CurrentMask->IsHole(maskIterator.GetIndex()))
        {
        maskIterator.Set(this->CurrentMask->GetValidValue());
        }

      ++maskIterator;
      }
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in UpdateMask!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}


itk::Index<2> CriminisiInpainting::FindHighestValueOnBoundary(FloatScalarImageType::Pointer image, float& maxPriority)
{
  // Return the highest priority pixel. Return the value of that priority by reference.
  try
  {
    // We would rather explicity find the maximum on the boundary
    /*
    typedef itk::MinimumMaximumImageCalculator <FloatScalarImageType> ImageCalculatorFilterType;
    ImageCalculatorFilterType::Pointer imageCalculatorFilter = ImageCalculatorFilterType::New ();
    imageCalculatorFilter->SetImage(priorityImage);
    imageCalculatorFilter->Compute();

    DebugMessage<float>("Highest priority: ", imageCalculatorFilter->GetMaximum());
    DebugMessage<float>("Lowest priority: ", imageCalculatorFilter->GetMinimum());

    return imageCalculatorFilter->GetIndexOfMaximum();
    */
    maxPriority = 0; // priorities are non-negative, so anything better than 0 will win
    itk::Index<2> locationOfMaxPriority;
    itk::ImageRegionConstIteratorWithIndex<UnsignedCharScalarImageType> boundaryIterator(this->BoundaryImage, this->BoundaryImage->GetLargestPossibleRegion());

    while(!boundaryIterator.IsAtEnd())
      {
      if(boundaryIterator.Get())
        {
        if(image->GetPixel(boundaryIterator.GetIndex()) > maxPriority)
          {
          maxPriority = image->GetPixel(boundaryIterator.GetIndex());
          locationOfMaxPriority = boundaryIterator.GetIndex();
          }
        }
      ++boundaryIterator;
      }
    DebugMessage<float>("Highest priority: ", maxPriority);
    return locationOfMaxPriority;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in FindHighestPriority!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}


FloatVector2Type CriminisiInpainting::GetAverageIsophote(const itk::Index<2>& queryPixel)
{
  try
  {
    if(!this->CurrentMask->GetLargestPossibleRegion().IsInside(Helpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0])))
      {
      FloatVector2Type v;
      v[0] = 0; v[1] = 0;
      return v;
      }

    itk::ImageRegionConstIterator<Mask> iterator(this->CurrentMask,Helpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0]));

    std::vector<FloatVector2Type> vectors;

    unsigned int radius = 3; // Why 3?
    while(!iterator.IsAtEnd())
      {
      if(IsValidPatch(iterator.GetIndex(), radius))
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


    /*
    // Compute histograms and histogram difference
    //HistogramType::Pointer sourceHistogram = Helpers::ComputeNDHistogramOfRegionManual(this->CurrentOutputImage, sourcePatch.Region, this->HistogramBinsPerDimension);
    HistogramType::Pointer sourceHistogram = Helpers::ComputeNDHistogramOfMaskedRegionManual(this->CurrentOutputImage, this->CurrentMask, sourcePatch.Region, this->HistogramBinsPerDimension);
    HistogramType::Pointer targetHistogram = Helpers::ComputeNDHistogramOfMaskedRegionManual(this->CurrentOutputImage, this->CurrentMask, targetPatch.Region, this->HistogramBinsPerDimension);

    float histogramDifference = Helpers::NDHistogramDifference(sourceHistogram, targetHistogram);
    patchPair.HistogramDifference = histogramDifference;
    */



#if 0
unsigned int SelfPatchCompare::FindBestPatch(float& minDistance)
{
  // This function returns the Id of the best source patch, as well as returns the minDistance by reference

  try
  {
    minDistance = std::numeric_limits<float>::infinity();
    unsigned int bestMatchId = 0;
    if(!this->Image->GetLargestPossibleRegion().IsInside(this->Pairs.TargetPatch.Region))
      {
      // Force the target region to be entirely inside the image
      this->Pairs.TargetPatch.Region.Crop(this->Image->GetLargestPossibleRegion());

      ComputeOffsets();

      for(unsigned int i = 0; i < this->Pairs.size(); ++i)
	{
	float distance = PatchDifferenceBoundary(this->Pairs[i].SourcePatch);
	//std::cout << "Patch " << i << " distance " << distance << std::endl;
	if(distance < minDistance)
	  {
	  minDistance = distance;
	  bestMatchId = i;
	  }
	}
      }
    else // The target patch is entirely inside the image
      {
      ComputeOffsets();
      for(unsigned int i = 0; i < this->Pairs.size(); ++i)
	{
	float distance = PatchAverageSquaredDifference(this->Pairs[i].SourcePatch);
	//float distance = PatchDifferenceManual(this->SourceRegions[i]);
	//std::cout << "Patch " << i << " distance " << distance << std::endl;
	if(distance < minDistance)
	  {
	  minDistance = distance;
	  bestMatchId = i;
	  }
	}
      }

    return bestMatchId;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in FindBestPatch!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}
#endif


/*
float CriminisiInpainting::ComputeTotalContinuationDifference(PatchPair& patchPair)
{
  float totalContinuationDifference = 0.0f;

  // Identify border pixels on the source side of the boundary. Of course the boundary is defined in the target patch. The isophote extension is later check in the source patch.
  std::vector<itk::Index<2> > borderPixels = Helpers::GetNonZeroPixels<UnsignedCharScalarImageType>(this->BoundaryImage, patchPair.TargetPatch.Region);

  for(unsigned int i = 0; i < borderPixels.size(); ++i)
    {
    float difference = ComputePixelContinuationDifference(borderPixels[i], patchPair);
    totalContinuationDifference += difference;
    }

  // We don't watch patches to be penalized for having longer boundaries
  float averageContinuationDifference = totalContinuationDifference / borderPixels.size();

  patchPair.SetContinuationDifference(averageContinuationDifference);

  return averageContinuationDifference;
}
*/

/*
unsigned int CriminisiInpainting::FindPatchWithBestContinuationDifference(const Patch& targetPatch)
{
  // This is a naive method that recomputes the boundary for every pair! This is way too slow to actually use.

  std::vector<PatchPair> patchPairs;
  std::vector<float> differences;

  for(unsigned int i = 0; i < this->SourcePatches.size(); ++i)
    {
    PatchPair patchPair;
    patchPair.SourcePatch = this->SourcePatches[i];
    patchPair.TargetPatch = targetPatch;

    float continuationDifference = ContinuationDifference(patchPair);
    differences.push_back(continuationDifference);
    }

  unsigned int bestPatchId = Helpers::argmin<float>(differences);
  return bestPatchId;
}
*/


float CriminisiInpainting::ComputeAverageIsophoteDifference(const itk::Index<2>& targetRegionSourceSideBoundaryPixel,
                                                            const itk::Index<2>& sourceRegionTargetSideBoundaryPixel,
                                                            const PatchPair& patchPair)
{
  //std::cout << "ComputeAverageIsophoteDifference()" << std::endl;
  // Get the isophote on the hole side of the boundary
  /*
  FloatVector2Type isophote1 = this->IsophoteImage->GetPixel(targetRegionSourceSideBoundaryPixel);
  FloatVector2Type isophote2 = this->IsophoteImage->GetPixel(sourceRegionTargetSideBoundaryPixel);

  return ComputeIsophoteDifference(isophote1, isophote2);
*/

  itk::ImageRegion<2> smallTargetPatch = Helpers::GetRegionInRadiusAroundPixel(targetRegionSourceSideBoundaryPixel, 1);
  smallTargetPatch.Crop(patchPair.TargetPatch.Region);
  //itk::ImageRegion<2> smallSourcePatch = Helpers::GetRegionInRadiusAroundPixel(sourceRegionTargetSideBoundaryPixel, 1);

  // Get the pixels in the valid region and in the hole of the target patch.
  std::vector<itk::Index<2> > validTargetPixels = this->CurrentMask->GetValidPixelsInRegion(smallTargetPatch);
  std::vector<itk::Index<2> > holeTargetPixels = this->CurrentMask->GetHolePixelsInRegion(smallTargetPatch);

  // We actually want the hole pixels in the source region, not the target region, so shift them.
  std::vector<itk::Index<2> > holeSourcePixels;
  for(unsigned int i = 0; i < holeTargetPixels.size(); ++i)
    {
    itk::Index<2> shiftedPixel = holeTargetPixels[i] + patchPair.GetTargetToSourceOffset();

//     if(!this->CurrentMask->GetLargestPossibleRegion().IsInside(shiftedPixel))
//       {
//       std::cout << "holeTargetPixels[i]: " << holeTargetPixels[i] << std::endl;
//       std::cout << "patchPair.TargetPatch.Region: " << patchPair.TargetPatch.Region << std::endl;
//       std::cout << "patchPair.SourcePatch.Region: " << patchPair.SourcePatch.Region << std::endl;
//       std::cout << "targetRegionSourceSideBoundaryPixel: " << targetRegionSourceSideBoundaryPixel << std::endl;
//       std::cout << "sourceRegionTargetSideBoundaryPixel: " << sourceRegionTargetSideBoundaryPixel << std::endl;
//       std::cout << "patchPair.GetTargetToSourceOffset(): " << patchPair.GetTargetToSourceOffset() << std::endl;
//       std::cout << "shiftedPixel: " << shiftedPixel << std::endl;
//       std::cout << "this->CurrentMask->GetLargestPossibleRegion(): " << this->CurrentMask->GetLargestPossibleRegion() << std::endl;
//       exit(-1);
//       }

    holeSourcePixels.push_back(shiftedPixel);
    }

  std::vector<FloatVector2Type> sourceIsophotes;
  for(unsigned int i = 0; i < holeSourcePixels.size(); ++i)
    {
    sourceIsophotes.push_back(this->IsophoteImage->GetPixel(holeSourcePixels[i]));
    }

  std::vector<FloatVector2Type> targetIsophotes;
  for(unsigned int i = 0; i < validTargetPixels.size(); ++i)
    {
    targetIsophotes.push_back(this->IsophoteImage->GetPixel(validTargetPixels[i]));
    }

  FloatVector2Type averageSourceIsophote = Helpers::AverageVectors(sourceIsophotes);
  FloatVector2Type averageTargetIsophote = Helpers::AverageVectors(targetIsophotes);

  float isophoteDifference = ComputeIsophoteDifference(averageSourceIsophote, averageTargetIsophote);
  //std::cout << "Leave ComputeAverageIsophoteDifference()" << std::endl;
  return isophoteDifference;
}


void CriminisiInpainting::FindBestPatchForHighestPriority(PatchPair& bestPatchPair)
{
  // This function implements Criminisi's idea of "find the highest priority pixel and proceed to fill it".
  // We have replaced this idea with FindBestPatchLookAhead().

  // This function returns the best PatchPair by reference.
#if 0
  float highestPriority = 0;
  itk::Index<2> pixelToFill = FindHighestValueOnBoundary(this->PriorityImage, highestPriority);
  DebugMessage<itk::Index<2> >("Highest priority found to be ", pixelToFill);

  itk::ImageRegion<2> targetRegion = Helpers::GetRegionInRadiusAroundPixel(pixelToFill, this->PatchRadius[0]);
  Patch targetPatch;
  targetPatch.Region = targetRegion;

  DebugMessage("Finding best patch...");

  CandidatePairs candidatePairs;
  SelfPatchCompare* patchCompare;
  patchCompare = new SelfPatchCompareColor(this->CompareImage->GetNumberOfComponentsPerPixel(), candidatePairs);
  patchCompare->SetImage(this->CompareImage);
  patchCompare->SetMask(this->CurrentMask);

  float distance = 0;
  unsigned int bestMatchSourcePatchId = patchCompare->FindBestPatch(distance);
  //DebugMessage<unsigned int>("Found best patch to be ", bestMatchSourcePatchId);
  //std::cout << "Found best patch to be " << bestMatchSourcePatchId << std::endl;

  //this->DebugWritePatch(this->SourcePatches[bestMatchSourcePatchId], "SourcePatch.png");
  //this->DebugWritePatch(targetRegion, "TargetPatch.png");
  Patch sourcePatch;
  sourcePatch = this->SourcePatches[bestMatchSourcePatchId];

  bestPatchPair.TargetPatch = targetPatch;
  bestPatchPair.SourcePatch = sourcePatch;
#endif
}


float SelfPatchCompare::PatchDifferenceManual(const Patch& sourcePatch)
{
  // This function assumes that all pixels in the source region are unmasked.
  try
  {
    assert(this->Image->GetLargestPossibleRegion().IsInside(sourcePatch.Region));

    float totalDifference = 0;

    unsigned int componentsPerPixel = this->Image->GetNumberOfComponentsPerPixel();

    FloatVectorImageType::InternalPixelType *buffptr = this->Image->GetBufferPointer();
    unsigned int offsetDifference = (this->Image->ComputeOffset(this->Pairs->TargetPatch.Region.GetIndex())
                                    - this->Image->ComputeOffset(sourcePatch.Region.GetIndex())) * componentsPerPixel;

    float difference = 0;
    for(unsigned int pixelId = 0; pixelId < this->ValidOffsets.size(); ++pixelId)
      {
      /*
      for(unsigned int i = 0; i < componentsPerPixel; ++i)
        {
        targetPixel[i] = buffptr[this->ValidOffsets[pixelId] + i];
        sourcePixel[i] = buffptr[this->ValidOffsets[pixelId] - offsetDifference + i];
        }
      //std::cout << "Source pixel: " << sourcePixel << " target pixel: " << targetPixel << std::endl;
      float difference = Helpers::PixelSquaredDifference(sourcePixel, targetPixel);
      */
      difference = 0;
      for(unsigned int i = 0; i < componentsPerPixel; ++i)
        {
	//std::cout << "component " << i << ": " << buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i] << std::endl;
        //difference += fabs(buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i]);

	difference += (buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i]) *
		      (buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i]);
        }
      //std::cout << "difference: " << difference << std::endl;
      totalDifference += difference;
      }
    //std::cout << "totalDifference: " << totalDifference << std::endl;
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
    totalDifference *= totalDifference;
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



float SelfPatchCompare::SlowDifference(const Patch& sourcePatch)
{
  // This function assumes that all pixels in the source region are unmasked.

  // This method uses 3 iterators - one for the mask, and one for each image patch.
  // The entire mask is traversed looking for valid pixels, and then comparing the image pixels.
  // This is very inefficient because, since the target region stays constant for many thousands of patch
  // comparisons, the mask need only be traversed once. This method is performed by ComputeOffsets()
  // and PatchDifference*(). This function is only here for comparison purposes (to ensure the result of the other functions
  // is correct).

  try
  {
    //assert(this->Image->GetLargestPossibleRegion().IsInside(sourceRegion));

    itk::ImageRegion<2> newSourceRegion = sourcePatch.Region;
    itk::ImageRegion<2> newTargetRegion = this->Pairs->TargetPatch.Region;

    if(!this->Image->GetLargestPossibleRegion().IsInside(this->Pairs->TargetPatch.Region))
      {
      // Move the source region to the target region. We move this way because we want to iterate over the mask in the target region.
      itk::Offset<2> sourceTargetOffset = this->Pairs->TargetPatch.Region.GetIndex() - sourcePatch.Region.GetIndex();

      newSourceRegion.SetIndex(sourcePatch.Region.GetIndex() + sourceTargetOffset);

      // Force both regions to be entirely inside the image
      newTargetRegion.Crop(this->Image->GetLargestPossibleRegion());
      newSourceRegion.Crop(this->Image->GetLargestPossibleRegion());

      // Move the source region back to its original position
      newSourceRegion.SetIndex(newSourceRegion.GetIndex() - sourceTargetOffset);
      }

    //std::cout << "New source region: " << newSourceRegion << std::endl;
    //std::cout << "New target region: " << newTargetRegion << std::endl;
    itk::ImageRegionConstIterator<FloatVectorImageType> sourcePatchIterator(this->Image, newSourceRegion);
    itk::ImageRegionConstIterator<FloatVectorImageType> targetPatchIterator(this->Image, newTargetRegion);
    itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, newTargetRegion);

    float sum = 0;
    unsigned int validPixelCounter = 0;
    //unsigned int componentsPerPixel = this->Image->GetNumberOfComponentsPerPixel();
    FullSquaredPixelDifference differenceFunction(this->Image->GetNumberOfComponentsPerPixel());

    while(!sourcePatchIterator.IsAtEnd())
      {
      itk::Index<2> currentPixel = maskIterator.GetIndex();
      if(this->MaskImage->IsValid(currentPixel))
        {
        //std::cout << "Offset from iterator: " << this->Image->ComputeOffset(maskIterator.GetIndex()) * componentsPerPixel;
        FloatVectorImageType::PixelType sourcePixel = sourcePatchIterator.Get();
        FloatVectorImageType::PixelType targetPixel = targetPatchIterator.Get();
        //std::cout << "Source pixel: " << sourcePixel << " target pixel: " << targetPixel << std::endl;
        //float difference = Helpers::PixelSquaredDifference(sourcePixel, targetPixel);
	float difference = differenceFunction.Difference(sourcePixel, targetPixel);
        sum +=  difference;
        validPixelCounter++;
        }

      ++sourcePatchIterator;
      ++targetPatchIterator;
      ++maskIterator;
      } // end while iterate over sourcePatch

    //std::cout << "totalDifference: " << sum << std::endl;
    //std::cout << "Valid pixels: " << validPixelCounter << std::endl;

    if(validPixelCounter == 0)
      {
      std::cerr << "Zero valid pixels in PatchDifference." << std::endl;
      std::cerr << "Source region: " << sourcePatch.Region << std::endl;
      std::cerr << "Target region: " << this->Pairs->TargetPatch.Region << std::endl;
      std::cerr << "New source region: " << newSourceRegion << std::endl;
      std::cerr << "New target region: " << newTargetRegion << std::endl;
      exit(-1);
      }
    float averageDifference = sum/static_cast<float>(validPixelCounter);
    return averageDifference;
  } //end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in PatchDifference!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}


float SelfPatchCompare::PatchAverageSquaredDifference(const Patch& sourcePatch)
{
  // This function assumes that all pixels in the source region are unmasked.
  try
  {
    //assert(this->Image->GetLargestPossibleRegion().IsInside(sourceRegion));

    float totalDifference = 0.0f;

    FloatVectorImageType::InternalPixelType *buffptr = this->Image->GetBufferPointer();
    unsigned int offsetDifference = (this->Image->ComputeOffset(this->Pairs->TargetPatch.Region.GetIndex())
                                    - this->Image->ComputeOffset(sourcePatch.Region.GetIndex())) * this->NumberOfComponentsPerPixel;

    float squaredDifference = 0;

    FloatVectorImageType::PixelType sourcePixel;
    sourcePixel.SetSize(this->NumberOfComponentsPerPixel);

    FloatVectorImageType::PixelType targetPixel;
    targetPixel.SetSize(this->NumberOfComponentsPerPixel);

    FloatVectorImageType::PixelType differencePixel;
    differencePixel.SetSize(this->NumberOfComponentsPerPixel);

    FullSquaredPixelDifference differenceFunction(sourcePixel);

    for(unsigned int pixelId = 0; pixelId < this->ValidOffsets.size(); ++pixelId)
      {

      for(unsigned int i = 0; i < this->NumberOfComponentsPerPixel; ++i)
        {
	sourcePixel[i] = buffptr[this->ValidOffsets[pixelId] + i];
        targetPixel[i] = buffptr[this->ValidOffsets[pixelId] - offsetDifference + i];
        }


      squaredDifference = differenceFunction.Difference(sourcePixel, targetPixel);
      //difference = NonVirtualPixelDifference(sourcePixel, targetPixel); // This call seems to make it very slow?
      //difference = (sourcePixel-targetPixel).GetSquaredNorm(); // horribly slow

      //differencePixel = sourcePixel-targetPixel;
      //difference = differencePixel.GetSquaredNorm();

//       difference = 0;
//       for(unsigned int i = 0; i < componentsPerPixel; ++i)
//         {
// 	difference += (sourcePixel[i] - targetPixel[i]) *
// 		      (sourcePixel[i] - targetPixel[i]);
// 	}

      //totalDifference += difference;
      totalDifference += squaredDifference;
      }

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



float SelfPatchCompare::PatchAverageAbsoluteSourceDifference(const Patch& sourcePatch)
{
  // This function assumes that all pixels in the source region are unmasked/valid.
  try
  {
    //assert(this->Image->GetLargestPossibleRegion().IsInside(sourceRegion));

    float totalAbsoluteDifference = 0.0f;

    FloatVectorImageType::InternalPixelType *buffptr = this->Image->GetBufferPointer();

    // This is the vector from the target patch to the source patch.
    int targetToSourceOffsetPixels = this->Image->ComputeOffset(sourcePatch.Region.GetIndex()) - this->Image->ComputeOffset(this->Pairs->TargetPatch.Region.GetIndex());
    int targetToSourceOffset = targetToSourceOffsetPixels * this->NumberOfComponentsPerPixel;

    float absoluteDifference = 0.0f;

    FloatVectorImageType::PixelType sourcePixel(this->NumberOfComponentsPerPixel);

    FloatVectorImageType::PixelType targetPixel(this->NumberOfComponentsPerPixel);

    FullPixelDifference differenceFunction(this->NumberOfComponentsPerPixel);
    for(unsigned int pixelId = 0; pixelId < this->ValidTargetPatchOffsets.size(); ++pixelId)
      {

      for(unsigned int component = 0; component < this->NumberOfComponentsPerPixel; ++component)
        {
        sourcePixel[component] = buffptr[this->ValidTargetPatchOffsets[pixelId] + targetToSourceOffset + component];
        targetPixel[component] = buffptr[this->ValidTargetPatchOffsets[pixelId] + component];
        }

      absoluteDifference = differenceFunction.Difference(sourcePixel, targetPixel);

      totalAbsoluteDifference += absoluteDifference;
      }

    float averageAbsoluteDifference = totalAbsoluteDifference / static_cast<float>(this->ValidTargetPatchOffsets.size());
    return averageAbsoluteDifference;
  } //end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in PatchDifference!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}


void PatchBasedInpaintingGUI::DisplayIsophotes()
{
  if(this->IntermediateImages[this->IterationToDisplay].Isophotes->GetLargestPossibleRegion().GetSize()[0] != 0)
    {
    // Mask the isophotes image with the current boundary, because we only want to display the isophotes we are interested in.
    //FloatVector2ImageType::Pointer normalizedIsophotes = FloatVector2ImageType::New();
    //Helpers::DeepCopy<FloatVector2ImageType>(this->IntermediateImages[this->IterationToDisplay].Isophotes, normalizedIsophotes);
    //Helpers::NormalizeVectorImage(normalizedIsophotes);

    typedef itk::MaskImageFilter< FloatVector2ImageType, UnsignedCharScalarImageType, FloatVector2ImageType> MaskFilterType;
    typename MaskFilterType::Pointer maskFilter = MaskFilterType::New();
    //maskFilter->SetInput(normalizedIsophotes);
    maskFilter->SetInput(this->IntermediateImages[this->IterationToDisplay].Isophotes);
    maskFilter->SetMaskImage(this->IntermediateImages[this->IterationToDisplay].Boundary);
    FloatVector2ImageType::PixelType zero;
    zero.Fill(0);
    maskFilter->SetOutsideValue(zero);
    maskFilter->Update();

    HelpersOutput::WriteImageConditional<FloatVector2ImageType>(maskFilter->GetOutput(), "Debug/ShowIsophotes.BoundaryIsophotes.mha", this->DebugImages);
    HelpersOutput::WriteImageConditional<UnsignedCharScalarImageType>(this->IntermediateImages[this->IterationToDisplay].Boundary, "Debug/ShowIsophotes.Boundary.mha", this->DebugImages);

    Helpers::ConvertNonZeroPixelsToVectors(maskFilter->GetOutput(), this->IsophoteLayer.Vectors);

    if(this->DebugImages)
      {
      vtkSmartPointer<vtkXMLImageDataWriter> writer = vtkSmartPointer<vtkXMLImageDataWriter>::New();
      writer->SetFileName("Debug/VTKIsophotes.vti");
      writer->SetInputConnection(this->IsophoteLayer.ImageData->GetProducerPort());
      writer->Write();

      vtkSmartPointer<vtkXMLPolyDataWriter> polyDataWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
      polyDataWriter->SetFileName("Debug/VTKIsophotes.vtp");
      polyDataWriter->SetInputConnection(this->IsophoteLayer.Vectors->GetProducerPort());
      polyDataWriter->Write();
      }

    }
  else
    {
    std::cerr << "Isophotes are not defined!" << std::endl;
    }
}


void PatchBasedInpaintingGUI::DisplayData()
{
  vtkSmartPointer<vtkImageData> temp = vtkSmartPointer<vtkImageData>::New();
  Helpers::ITKScalarImageToScaledVTKImage<FloatScalarImageType>(this->IntermediateImages[this->IterationToDisplay].Data, temp);
  Helpers::MakeValueTransparent(temp, this->DataLayer.ImageData, 0); // Set the zero pixels to transparent
  this->qvtkWidget->GetRenderWindow()->Render();
}



void PatchBasedInpaintingGUI::DisplayBoundaryNormals()
{
//   if(this->Inpainting.GetBoundaryNormalsImage()->GetLargestPossibleRegion().GetSize()[0] != 0)
//     {
//     Helpers::ConvertNonZeroPixelsToVectors(this->IntermediateImages[this->IterationToDisplay].BoundaryNormals, this->BoundaryNormalsLayer.Vectors);
//     this->qvtkWidget->GetRenderWindow()->Render();
//
//     if(this->DebugImages)
//       {
//       std::cout << "Writing boundary normals..." << std::endl;
//
//       HelpersOutput::WriteImage<FloatVector2ImageType>(this->Inpainting.GetBoundaryNormalsImage(), "Debug/RefreshSlot.BoundaryNormals.mha");
//
//       HelpersOutput::WriteImageData(this->BoundaryNormalsLayer.ImageData, "Debug/RefreshSlot.VTKBoundaryNormals.vti");
//
//       HelpersOutput::WritePolyData(this->BoundaryNormalsLayer.Vectors, "Debug/RefreshSlot.VTKBoundaryNormals.vtp");
//       }
//     }
}



void PatchBasedInpaintingGUI::DisplayConfidence()
{
  vtkSmartPointer<vtkImageData> temp = vtkSmartPointer<vtkImageData>::New();
  Helpers::ITKScalarImageToScaledVTKImage<FloatScalarImageType>(this->IntermediateImages[this->IterationToDisplay].Confidence, temp);
  Helpers::MakeValueTransparent(temp, this->ConfidenceLayer.ImageData, 0); // Set the zero pixels to transparent
  this->qvtkWidget->GetRenderWindow()->Render();
}

void PatchBasedInpaintingGUI::DisplayConfidenceMap()
{
  Helpers::ITKScalarImageToScaledVTKImage<FloatScalarImageType>(this->IntermediateImages[this->IterationToDisplay].ConfidenceMap, this->ConfidenceMapLayer.ImageData);
  this->qvtkWidget->GetRenderWindow()->Render();
}


void PatchBasedInpaintingGUI::CreatePotentialTargetPatchesImage()
{
  DebugMessage("CreatePotentialTargetPatchesImage()");
  // Draw potential patch pairs

//   std::stringstream ssPatchPairsFile;
//   ssPatchPairsFile << "Debug/PatchPairs_" << Helpers::ZeroPad(this->Inpainting.GetIteration(), 3) << ".txt";
//   OutputPairs(potentialPatchPairs, ssPatchPairsFile.str());

  if(this->IterationToDisplay < 1)
    {
    return;
    }

  if(!this->Recorded[this->IterationToDisplay - 1])
    {
    return;
    }

  this->PotentialTargetPatchesImage->SetRegions(this->Inpainting.GetFullRegion());
  this->PotentialTargetPatchesImage->Allocate();
  this->PotentialTargetPatchesImage->FillBuffer(0);

  const std::vector<CandidatePairs>& potentialCandidatePairs = this->AllPotentialCandidatePairs[this->IterationToDisplay - 1];

  for(unsigned int i = 0; i < potentialCandidatePairs.size(); ++i)
    {
    Helpers::BlankAndOutlineRegion<UnsignedCharScalarImageType>(this->PotentialTargetPatchesImage,
                                                                potentialCandidatePairs[i].TargetPatch.Region, static_cast<unsigned char>(0),
                                                                static_cast<unsigned char>(255));
    }

  vtkSmartPointer<vtkImageData> temp = vtkSmartPointer<vtkImageData>::New();
  Helpers::ITKScalarImageToScaledVTKImage<UnsignedCharScalarImageType>(this->PotentialTargetPatchesImage, temp);
  Helpers::MakeValueTransparent(temp, this->PotentialPatchesLayer.ImageData, 0);

  Refresh();
}
