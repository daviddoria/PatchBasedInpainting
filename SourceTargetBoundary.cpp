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

#include <stdexcept>

bool GetAdjacentBoundaryPixel(const itk::Index<2>& boundaryPixel, const PatchPair& patchPair, itk::Index<2>& adjacentBoundaryPixel);

bool PatchBasedInpainting::GetAdjacentBoundaryPixel(const itk::Index<2>& targetPatchSourceSideBoundaryPixel, const PatchPair& patchPair,
                                                   itk::Index<2>& sourcePatchTargetSideBoundaryPixel)
{
  if(this->CurrentMask->IsHole(targetPatchSourceSideBoundaryPixel) || !patchPair.TargetPatch.Region.IsInside(targetPatchSourceSideBoundaryPixel))
    {
    throw std::runtime_error("Error: The input boundary pixel must be on the valid side of the boundary (not in the hole)!");
    }

  FloatVector2Type sourceSideIsophote = this->IsophoteImage->GetPixel(targetPatchSourceSideBoundaryPixel);

  itk::Index<2> pixelAcrossBoundary = Helpers::GetNextPixelAlongVector(targetPatchSourceSideBoundaryPixel, sourceSideIsophote);

  // Some pixels might not have a valid pixel on the other side of the boundary.
  bool valid = false;

  // If the next pixel along the isophote is in bounds and in the hole region of the patch, procede.
  if(patchPair.TargetPatch.Region.IsInside(pixelAcrossBoundary) && this->CurrentMask->IsHole(pixelAcrossBoundary))
    {
    valid = true;
    }
  else
    {
    // There is no requirement for the isophote to be pointing a particular orientation, so try to step along the negative isophote.
    sourceSideIsophote *= -1.0;
    pixelAcrossBoundary = Helpers::GetNextPixelAlongVector(targetPatchSourceSideBoundaryPixel, sourceSideIsophote);
    if(patchPair.TargetPatch.Region.IsInside(pixelAcrossBoundary) && this->CurrentMask->IsHole(pixelAcrossBoundary))
      {
      valid = true;
      }
    }

  if(!valid)
    {
    return valid;
    }

  // Determine the position of the pixel relative to the patch corner.
  itk::Offset<2> intraPatchOffset = pixelAcrossBoundary - patchPair.TargetPatch.Region.GetIndex();

  // Determine the position of the corresponding pixel in the source patch and return by reference.
  sourcePatchTargetSideBoundaryPixel = patchPair.SourcePatch.Region.GetIndex() + intraPatchOffset;

  return valid;
}

  
// Compute the continuation difference for every pair. Store the values in the PatchPair objects inside of the CandidatePairs object.
void PatchBasedInpainting::ComputeAllContinuationDifferences(CandidatePairs& candidatePairs)
{
  EnterFunction("ComputeAllContinuationDifferences()");

  // Naively, we could just call ComputeTotalContinuationDifference on each patch pair for a forward looking set. However, this
  // would recalculate the boundary for every pair. This is a lot of extra work because the boundary does not change from source patch to source patch.

  // Identify border pixels on the source side of the boundary.
  std::vector<itk::Index<2> > borderPixels = Helpers::GetNonZeroPixels<UnsignedCharScalarImageType>(this->BoundaryImage, candidatePairs.TargetPatch.Region);

  //for(unsigned int sourcePatchId = 0; sourcePatchId < this->SourcePatches.size(); ++sourcePatchId)
  for(unsigned int sourcePatchId = 0; sourcePatchId < candidatePairs.size(); ++sourcePatchId)
    {
    // Only compute if the values are not already computed.
    if(candidatePairs[sourcePatchId].IsValidBoundaryIsophoteAngleDifference() &&
       candidatePairs[sourcePatchId].IsValidBoundaryIsophoteStrengthDifference() &&
       candidatePairs[sourcePatchId].IsValidBoundaryPixelDifference())
       //candidatePairs[sourcePatchId].IsValidSSD()) // Don't check this, it is not related to the continuation difference
      {
      continue;
      }
    float totalPixelDifference = 0.0f;
    float totalIsophoteAngleDifference = 0.0f;
    float totalIsophoteStrengthDifference = 0.0f;
    unsigned int numberUsed = 0;
    for(unsigned int pixelId = 0; pixelId < borderPixels.size(); ++pixelId)
      {
      itk::Index<2> targetRegionSourceSideBoundaryPixel = borderPixels[pixelId];
      itk::Index<2> sourceRegionTargetSideBoundaryPixel;
      bool valid = GetAdjacentBoundaryPixel(targetRegionSourceSideBoundaryPixel, candidatePairs[sourcePatchId], sourceRegionTargetSideBoundaryPixel);
      if(!valid)
        {
        continue;
        }
      numberUsed++;

      // Pixel difference
      float normalizedSquaredPixelDifference = ComputeNormalizedSquaredPixelDifference(targetRegionSourceSideBoundaryPixel, sourceRegionTargetSideBoundaryPixel);
      totalPixelDifference += normalizedSquaredPixelDifference;
      DebugMessage<float>("ComputeAllContinuationDifferences::normalizedSquaredPixelDifference ", normalizedSquaredPixelDifference);

      // Isophote differences
      FloatVector2Type averageSourceIsophote = ComputeAverageIsophoteSourcePatch(sourceRegionTargetSideBoundaryPixel, candidatePairs[sourcePatchId]);
      FloatVector2Type averageTargetIsophote = ComputeAverageIsophoteTargetPatch(targetRegionSourceSideBoundaryPixel, candidatePairs[sourcePatchId]);

      float isophoteAngleDifference = ComputeIsophoteAngleDifference(averageSourceIsophote, averageTargetIsophote);
      totalIsophoteAngleDifference += isophoteAngleDifference;

      float isophoteStrengthDifference = ComputeIsophoteStrengthDifference(averageSourceIsophote, averageTargetIsophote);
      totalIsophoteStrengthDifference += isophoteStrengthDifference;

      } // end loop over pixels

    DebugMessage<unsigned int>("numberUsed ", numberUsed);
    DebugMessage<unsigned int>("Out of ", borderPixels.size());

    if(numberUsed == 0)
      {
      std::cout << "Warning: no pixels were used in ComputeAllContinuationDifferences()" << std::endl;
      numberUsed = 1; // Set this to 1 to avoid divide by zero.
      }

    float averagePixelDifference = totalPixelDifference / static_cast<float>(numberUsed);
    DebugMessage<float>("averagePixelDifference ", averagePixelDifference);

    float averageIsophoteAngleDifference = totalIsophoteAngleDifference / static_cast<float>(numberUsed);
    float averageIsophoteStrengthDifference = totalIsophoteStrengthDifference / static_cast<float>(numberUsed);
    candidatePairs[sourcePatchId].SetBoundaryPixelDifference(averagePixelDifference);
    candidatePairs[sourcePatchId].SetBoundaryIsophoteAngleDifference(averageIsophoteAngleDifference);
    candidatePairs[sourcePatchId].SetBoundaryIsophoteStrengthDifference(averageIsophoteStrengthDifference);

    } // end loop over pairs

  LeaveFunction("ComputeAllContinuationDifferences()");
}


FloatVector2Type PatchBasedInpainting::ComputeAverageIsophoteSourcePatch(const itk::Index<2>& sourcePatchPixel, const PatchPair& patchPair)
{
  // This function computes the average isophote of the pixels around 'pixel' in the target side of the source patch (pixels that will end up filling the hole).
  // The input 'pixel' is expected to be on the target side of the boundary in the source patch.

  // The target patch is the only patch in which the hole/boundary is actually defined, so computations must take place in that frame.
  itk::Index<2> targetPatchPixel = sourcePatchPixel + patchPair.GetSourceToTargetOffset();

  itk::ImageRegion<2> smallTargetPatch = Helpers::GetRegionInRadiusAroundPixel(targetPatchPixel, 1);
  smallTargetPatch.Crop(patchPair.TargetPatch.Region);

  // Get the pixels in the hole of the target patch.
  std::vector<itk::Index<2> > holeTargetPixels = this->CurrentMask->GetHolePixelsInRegion(smallTargetPatch);

  // We actually want the hole pixels in the source region, not the target region, so shift them.
  std::vector<itk::Index<2> > holeSourcePixels;
  itk::Offset<2> shiftAmount = patchPair.GetTargetToSourceOffset();
  for(unsigned int i = 0; i < holeTargetPixels.size(); ++i)
    {
    itk::Index<2> shiftedPixel = holeTargetPixels[i] + shiftAmount;

    holeSourcePixels.push_back(shiftedPixel);
    }

  std::vector<FloatVector2Type> sourceIsophotes;
  for(unsigned int i = 0; i < holeSourcePixels.size(); ++i)
    {
    sourceIsophotes.push_back(this->IsophoteImage->GetPixel(holeSourcePixels[i]));
    }

  FloatVector2Type averageSourceIsophote = Helpers::AverageVectors(sourceIsophotes);
  return averageSourceIsophote;
}

FloatVector2Type PatchBasedInpainting::ComputeAverageIsophoteTargetPatch(const itk::Index<2>& pixel, const PatchPair& patchPair)
{
  // This function computes the average isophote of the pixels around 'pixel' in the source side of the target patch.

  itk::ImageRegion<2> smallTargetPatch = Helpers::GetRegionInRadiusAroundPixel(pixel, 1);
  smallTargetPatch.Crop(patchPair.TargetPatch.Region);

  // Get the pixels in the valid region and in the hole of the target patch.
  std::vector<itk::Index<2> > validTargetPixels = this->CurrentMask->GetValidPixelsInRegion(smallTargetPatch);

  std::vector<FloatVector2Type> targetIsophotes;
  for(unsigned int i = 0; i < validTargetPixels.size(); ++i)
    {
    targetIsophotes.push_back(this->IsophoteImage->GetPixel(validTargetPixels[i]));
    }

  FloatVector2Type averageTargetIsophote = Helpers::AverageVectors(targetIsophotes);

  return averageTargetIsophote;
}


float PatchBasedInpainting::ComputeIsophoteAngleDifference(const FloatVector2Type& v1, const FloatVector2Type& v2)
{
  //std::cout << "ComputeIsophoteAngleDifference()" << std::endl;
  // Compute the isophote difference.
  float isophoteDifference = Helpers::AngleBetween(v1, v2);

  float isophoteDifferenceNormalized = isophoteDifference/3.14159; // The maximum angle between vectors is pi, so this produces a score between 0 and 1.
  DebugMessage<float>("isophoteDifferenceNormalized: ", isophoteDifferenceNormalized);

  //std::cout << "Leave ComputeIsophoteDifference()" << std::endl;
  return isophoteDifferenceNormalized;
}

float PatchBasedInpainting::ComputeIsophoteStrengthDifference(const FloatVector2Type& v1, const FloatVector2Type& v2)
{
  //std::cout << "ComputeIsophoteStrengthDifference()" << std::endl;
  // Compute the isophote difference.
  float isophoteDifference = fabs(v1.GetNorm() - v2.GetNorm());

  return isophoteDifference;
}

float PatchBasedInpainting::ComputeNormalizedSquaredPixelDifference(const itk::Index<2>& pixel1, const itk::Index<2>& pixel2)
{
  // Compute the pixel difference.

  FloatVectorImageType::PixelType value1 = this->CompareImage->GetPixel(pixel1);
  FloatVectorImageType::PixelType value2 = this->CompareImage->GetPixel(pixel2);
  DebugMessage<FloatVectorImageType::PixelType>("value1 ", value1);
  DebugMessage<FloatVectorImageType::PixelType>("value2 ", value2);

  float pixelSquaredDifference = FullSquaredPixelDifference::Difference(this->CompareImage->GetPixel(pixel1), this->CompareImage->GetPixel(pixel2), this->CompareImage->GetNumberOfComponentsPerPixel());
  DebugMessage<float>("ComputePixelDifference::pixelSquaredDifference", pixelSquaredDifference);

  //std::cout << "pixelDifference: " << pixelDifference << std::endl;
  //float pixelDifferenceNormalized = pixelDifference / MaxPixelDifference; // This produces a score between 0 and 1.
  DebugMessage<float>("ComputePixelDifference::normFactor ", MaxPixelDifferenceSquared);
  float pixelSquaredDifferenceNormalized = pixelSquaredDifference / MaxPixelDifferenceSquared; // This produces a score between 0 and 1.
  //DebugMessage<float>("pixelDifferenceNormalized: ", pixelDifferenceNormalized);

  return pixelSquaredDifferenceNormalized;
}

// Compute the difference between two isophotes
  float ComputeIsophoteAngleDifference(const FloatVector2Type& v1, const FloatVector2Type& v2);
  float ComputeIsophoteStrengthDifference(const FloatVector2Type& v1, const FloatVector2Type& v2);
  //float ComputeAverageIsophoteDifference(const itk::Index<2>& pixel1, const itk::Index<2>& pixel2, const PatchPair& patchPair);
  FloatVector2Type ComputeAverageIsophoteSourcePatch(const itk::Index<2>& pixel, const PatchPair& patchPair);
  FloatVector2Type ComputeAverageIsophoteTargetPatch(const itk::Index<2>& pixel, const PatchPair& patchPair);


  float ComputeNormalizedSquaredPixelDifference(const itk::Index<2>& pixel1, const itk::Index<2>& pixel2);



void PatchBasedInpainting::ComputeMinimumBoundaryGradientChange(unsigned int& bestForwardLookId, unsigned int& bestSourcePatchId)
{
  EnterFunction("ComputeMinimumBoundaryGradientChange()");
  // For the top N patches, compute the continuation difference by comparing the gradient at source side boundary pixels before and after filling.
  float lowestScore = std::numeric_limits< float >::max();

  itk::Index<2> zeroIndex;
  zeroIndex.Fill(0);
  itk::ImageRegion<2> outputRegion(zeroIndex, this->PotentialCandidatePairs[0][0].SourcePatch.Region.GetSize());

//   FloatScalarImageType::Pointer luminancePatch = FloatScalarImageType::New();
//   luminancePatch->SetRegions(outputRegion);
//   luminancePatch->SetNumberOfComponentsPerPixel(this->CurrentOutputImage->GetNumberOfComponentsPerPixel());
//   luminancePatch->Allocate();

  FloatVectorImageType::Pointer patch = FloatVectorImageType::New();
  patch->SetRegions(outputRegion);
  patch->SetNumberOfComponentsPerPixel(this->CurrentOutputImage->GetNumberOfComponentsPerPixel());
  patch->Allocate();

  FloatVector2ImageType::PixelType zeroVector;
  zeroVector.Fill(0);

  FloatVector2ImageType::Pointer preFillGradient = FloatVector2ImageType::New();
  preFillGradient->SetRegions(outputRegion);
  preFillGradient->Allocate();
  preFillGradient->FillBuffer(zeroVector);

  FloatVector2ImageType::Pointer postFillGradient = FloatVector2ImageType::New();
  postFillGradient->SetRegions(outputRegion);
  postFillGradient->Allocate();
  postFillGradient->FillBuffer(zeroVector);

  // Create an entirely unmasked Mask
  Mask::Pointer noMask = Mask::New();
  noMask->SetRegions(outputRegion);
  noMask->Allocate();

  itk::ImageRegionIterator<Mask> noMaskIterator(noMask, noMask->GetLargestPossibleRegion());

  while(!noMaskIterator.IsAtEnd())
    {
    noMaskIterator.Set(noMask->GetValidValue());
    ++noMaskIterator;
    }

  for(unsigned int forwardLookId = 0; forwardLookId < this->PotentialCandidatePairs.size(); ++forwardLookId)
    {
    std::cout << "Computing boundary gradient difference for forward look set " << forwardLookId << std::endl;
    // The boundary only need to be computed once for every forward look set
    std::vector<itk::Index<2> > boundaryPixels = Helpers::GetNonZeroPixels<UnsignedCharScalarImageType>(this->BoundaryImage,
                                                                                                        this->PotentialCandidatePairs[forwardLookId].TargetPatch.Region);
    if(boundaryPixels.size() < 1)
      {
      throw std::runtime_error("There must be at least 1 boundary pixel);
      }

    itk::Offset<2> patchOffset = this->CurrentMask->GetLargestPossibleRegion().GetIndex() - this->PotentialCandidatePairs[forwardLookId].TargetPatch.Region.GetIndex();
    for(unsigned int boundaryPixelId = 0; boundaryPixelId < boundaryPixels.size(); ++boundaryPixelId)
      {
      boundaryPixels[boundaryPixelId] += patchOffset;
      }

    // Get the current mask
    typedef itk::RegionOfInterestImageFilter<Mask,Mask> ExtractFilterType;
    typename ExtractFilterType::Pointer extractMaskFilter = ExtractFilterType::New();
    extractMaskFilter->SetRegionOfInterest(this->PotentialCandidatePairs[forwardLookId].TargetPatch.Region);
    extractMaskFilter->SetInput(this->CurrentMask);
    extractMaskFilter->Update();

    unsigned int maxNumberToInspect = 100u;
    unsigned int numberOfSourcePatchesToInspect = std::min(maxNumberToInspect, this->PotentialCandidatePairs[forwardLookId].size());
    for(unsigned int sourcePatchId = 0; sourcePatchId < numberOfSourcePatchesToInspect; ++sourcePatchId)
      {
      Helpers::CreatePatchImage<FloatVectorImageType>(this->CurrentOutputImage, this->PotentialCandidatePairs[forwardLookId][sourcePatchId].SourcePatch.Region, this->PotentialCandidatePairs[forwardLookId].TargetPatch.Region, this->CurrentMask, patch);

      float sumOfComponentErrors = 0.0f;
      for(unsigned int component = 0; component < this->CurrentOutputImage->GetNumberOfComponentsPerPixel(); ++component)
        {
//      typedef itk::VectorImageToImageAdaptor<float, 2> ImageAdaptorType;
//      ImageAdaptorType::Pointer adaptor = ImageAdaptorType::New();
//      adaptor->SetExtractComponentIndex(component);
//      adaptor->SetImage(patch);
//
        typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, FloatScalarImageType> IndexSelectionType;
        IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
        indexSelectionFilter->SetIndex(component);
        indexSelectionFilter->SetInput(patch);
        indexSelectionFilter->Update();

        Helpers::SetImageToConstant<FloatVector2ImageType>(preFillGradient, zeroVector);
        Helpers::SetImageToConstant<FloatVector2ImageType>(postFillGradient, zeroVector);
        //float averageError = ComputeAverageGradientChange<ImageAdaptorType>(adaptor, preFillGradient, postFillGradient, extractMaskFilter->GetOutput(), noMask, boundaryPixels);
        float averageError = ComputeAverageGradientChange<FloatScalarImageType>(indexSelectionFilter->GetOutput(), preFillGradient, postFillGradient, extractMaskFilter->GetOutput(), noMask, boundaryPixels);

        sumOfComponentErrors += averageError;

        } // end component loop

      this->PotentialCandidatePairs[forwardLookId][sourcePatchId].SetBoundaryGradientDifference(sumOfComponentErrors);
      if(sumOfComponentErrors < lowestScore)
        {
        lowestScore = sumOfComponentErrors;
        bestForwardLookId = forwardLookId;
        bestSourcePatchId = sourcePatchId;

        HelpersOutput::Write2DVectorImage(preFillGradient, "Debug/BestPrefill.mha");

        HelpersOutput::Write2DVectorImage(postFillGradient, "Debug/BestPostfill.mha");

        HelpersOutput::WriteVectorImageAsRGB(patch, "Debug/BestPatch.mha");
        }
      } // end source patch loop
    } // end forward look set loop
  LeaveFunction("ComputeMinimumBoundaryGradientChange()");
}


  template <typename TImage>
  float ComputeAverageGradientChange(const typename TImage::Pointer patch, FloatVector2ImageType::Pointer preFillGradient, FloatVector2ImageType::Pointer postFillGradient,
                                     const Mask::Pointer mask, const Mask::Pointer noMask, const std::vector<itk::Index<2> >& boundaryPixels);


template <typename TImage>
float PatchBasedInpainting::ComputeAverageGradientChange(const typename TImage::Pointer patch, FloatVector2ImageType::Pointer preFillGradientImage,
                                                         FloatVector2ImageType::Pointer postFillGradientImage,
                                                         const Mask::Pointer mask, const Mask::Pointer noMask, const std::vector<itk::Index<2> >& boundaryPixels)
{
  Derivatives::MaskedGradient<TImage>(patch, mask, preFillGradientImage);
  Derivatives::MaskedGradient<TImage>(patch, noMask, postFillGradientImage);

  float totalError = 0.0f;
  for(unsigned int boundaryPixelId = 0; boundaryPixelId < boundaryPixels.size(); ++boundaryPixelId)
    {
    FloatVector2ImageType::PixelType preFillGradient = preFillGradientImage->GetPixel(boundaryPixels[boundaryPixelId]);
    FloatVector2ImageType::PixelType postFillGradient = postFillGradientImage->GetPixel(boundaryPixels[boundaryPixelId]);
    //std::cout << "Prefill gradient: " << preFillGradient << std::endl;
    //std::cout << "Postfill gradient: " << postFillGradient << std::endl;
    //totalError += (preFillGradient - postFillGradient).GetNorm();
    totalError += (preFillGradient - postFillGradient).GetSquaredNorm();
    }

  float averageError = totalError / static_cast<float>(boundaryPixels.size());

  return averageError;
}

  void ComputeMinimumBoundaryGradientChange(unsigned int& bestForwardLookId, unsigned int& bestSourcePatchId);
