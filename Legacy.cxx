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