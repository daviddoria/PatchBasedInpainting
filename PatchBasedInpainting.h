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

#ifndef PatchBasedInpainting_h
#define PatchBasedInpainting_h

// Custom
#include "CandidatePairs.h"
#include "DebugOutputs.h"
#include "Helpers.h"
#include "Patch.h"
#include "PatchPair.h"
#include "Priority.h"
#include "SelfPatchCompare.h"
#include "Types.h"

// ITK
#include "itkCovariantVector.h"
#include "itkImage.h"

// Boost
#include <boost/function.hpp>

class PatchBasedInpainting : public DebugOutputs
{

public:
  ///////////////////////////////////////////////////////////////////////
  /////////////////// CriminisiInpaintingInterface.cpp //////////////////
  ///////////////////////////////////////////////////////////////////////
  
  void SetDifferenceType(const int);
  
  // Specify the image to inpaint.
  void SetImage(const FloatVectorImageType::Pointer image);
  
  // Specify the region to inpaint.
  void SetMask(const Mask::Pointer mask);
  
  // Specify the size of the patches to copy.
  void SetPatchRadius(const unsigned int);
  
  // Specify the maximum number of top candidate patches to consider. Near the end of the inpainting there may not be this many viable patches, that is why we set the max instead of the absolute number of patches.
  void SetMaxForwardLookPatches(const unsigned int);

  void SetNumberOfTopPatchesToSave(const unsigned int);

  // Get the current boundary image
  UnsignedCharScalarImageType::Pointer GetBoundaryImage();

  // Get the current boundary image
  FloatVector2ImageType::Pointer GetBoundaryNormalsImage();
  
  // Get the current isophote image
  FloatVector2ImageType::Pointer GetIsophoteImage();

  // Get the result/output of the inpainting so far. When the algorithm is complete, this will be the final output.
  FloatVectorImageType::Pointer GetCurrentOutputImage();
  
  // Get the current mask image
  Mask::Pointer GetMaskImage();
  
  //////////////////////////////////////////////////////////////
  /////////////////// CriminisiInpainting.cpp //////////////////
  //////////////////////////////////////////////////////////////
  
  // Constructor
  PatchBasedInpainting();

  // A single step of the algorithm. The real work is done here.
  PatchPair Iterate();

  // A loop that calls Iterate() until the inpainting is complete.
  void Inpaint();
  
  // Initialize everything.
  void Initialize();

  // Determine whether or not the inpainting is completed by seeing if there are any pixels in the mask that still need to be filled.
  bool HasMoreToInpaint();
  
  CandidatePairs& GetPotentialCandidatePairReference(const unsigned int forwardLookId);
  
  std::vector<CandidatePairs> GetPotentialCandidatePairs();
  
  // Return the number of completed iterations
  unsigned int GetNumberOfCompletedIterations();
  
  // When an image is loaded, it's size is taken as the size that everything else should be. We don't want to keep referring to Image->GetLargestPossibleRegion,
  // so we store the region in a member variable. For the same reason, if we want to know the size of the images that this class is operating on, the user should
  // not have to query a specific image, but rather access this more global region definition.
  itk::ImageRegion<2> GetFullRegion();

  // Return a pointer to all forward look sets.
  std::vector<CandidatePairs>& GetPotentialCandidatePairsReference();
  
  void SetCompareToOriginal();
  void SetCompareToBlurred();
  void SetCompareToCIELAB();
  //////////// These functions should be private, but we want to do some external testing for the moment.
  
  // Compute the isophotes.
  //void ComputeMaskedIsophotesInRegion(FloatScalarImageType::Pointer image, Mask::Pointer mask, const itk::ImageRegion<2>& region, FloatVector2ImageType::Pointer outputIsophotes);

  bool GetAdjacentBoundaryPixel(const itk::Index<2>& boundaryPixel, const PatchPair& patchPair, itk::Index<2>& adjacentBoundaryPixel);
  
  // Find the boundary of a region.
  void FindBoundary();

  // Compute the normals of the hole boundary.
  void ComputeBoundaryNormals(const float blurVariance);

  void SetPatchCompare(SelfPatchCompare* PatchCompare);
  
  boost::function<bool (const PatchPair& , const PatchPair& )> PatchSortFunction;
  
  boost::function<void (CandidatePairs& candidatePairs, PatchPair& bestPatchPair )> PatchSearchFunction;
  
  void SetPatchSearchFunctionToScaleConsistent();
  void SetPatchSearchFunctionToNormal();
  void SetPatchSearchFunctionToTwoStepDepth();
  
private:
  
  void RecomputeScoresWithNewPatches(std::vector<Patch>& newPatches, PatchPair& usedPatchPair);
  
  void BlurImage();
  void ComputeIsophotes();
  
  PatchPair PreviousIterationUsedPatchPair;
  // Compute the difference between two isophotes
  float ComputeIsophoteAngleDifference(const FloatVector2Type& v1, const FloatVector2Type& v2);
  float ComputeIsophoteStrengthDifference(const FloatVector2Type& v1, const FloatVector2Type& v2);
  //float ComputeAverageIsophoteDifference(const itk::Index<2>& pixel1, const itk::Index<2>& pixel2, const PatchPair& patchPair);
  FloatVector2Type ComputeAverageIsophoteSourcePatch(const itk::Index<2>& pixel, const PatchPair& patchPair);
  FloatVector2Type ComputeAverageIsophoteTargetPatch(const itk::Index<2>& pixel, const PatchPair& patchPair);
  
  // Compute the continuation difference for every pair. Store the values in the PatchPair objects inside of the CandidatePairs object.
  void ComputeAllContinuationDifferences(CandidatePairs& candidatePairs);
  
  
  float ComputeNormalizedSquaredPixelDifference(const itk::Index<2>& pixel1, const itk::Index<2>& pixel2);
  
  // Determine the difference along an extended isophote of the pixel that will be filled. The PatchPair is non-const because we store the score in the class.
  //float ComputeTotalContinuationDifference(PatchPair& patchPair);
  
  // This is a new idea to try to fill several patches and return the best pair. Note that if the number of look ahead patches is 1, this is exactly the same as not looking ahead.
  void FindBestPatchLookAhead(PatchPair& bestPatchPair);

  // One of these functions is called multiple times from FindBestPatchLookAhead (based on the value of PatchSearchFunction)
  void FindBestPatchScaleConsistent(CandidatePairs& candidatePairs, PatchPair& bestPatchPair);
  void FindBestPatchNormal(CandidatePairs& candidatePairs, PatchPair& bestPatchPair);
  void FindBestPatchTwoStepDepth(CandidatePairs& candidatePairs, PatchPair& bestPatchPair);
  

  
  // Image to inpaint. This should not be modified throughout the algorithm.
  FloatVectorImageType::Pointer OriginalImage;
  
  // The CIELab conversion of the input RGB image
  FloatVectorImageType::Pointer CIELabImage;
  
  // The blurred image which is useful for computing gradients as well as softening pixel to pixel comparisons.
  FloatVectorImageType::Pointer BlurredImage;
  
  // The intermediate steps and eventually the result of the inpainting.
  FloatVectorImageType::Pointer CurrentOutputImage;
  
  // This image will be used for all patch to patch comparisons. It should point at either OriginalImage or CIELabImage.
  FloatVectorImageType::Pointer CompareImage;
  
  // The mask specifying the region to inpaint. This does not change throughout the algorithm - it is the original mask.
  Mask::Pointer OriginalMask;
  
  // This mask is updated as patches are copied.
  Mask::Pointer CurrentMask;

  // The patch radius.
  itk::Size<2> PatchRadius;

  // Store the computed isophotes.
  FloatVector2ImageType::Pointer IsophoteImage;
  
  // Keep track of the edge of the region to inpaint.
  UnsignedCharScalarImageType::Pointer BoundaryImage;
  
  // Store the computed boundary normals.
  FloatVector2ImageType::Pointer BoundaryNormals;

  
  // Set the region to the full region and allocate an image
  template<typename TImage>
  void InitializeImage(typename TImage::Pointer);
  
  // Initialization functions
  
  // The target image is colored bright green inside the hole. This is helpful when watching the inpainting proceed.
  void InitializeTargetImage();
    
  //itk::CovariantVector<float, 2> GetAverageIsophote(const itk::Index<2>& queryPixel);
  
  // Determine if a patch is completely valid (no hole pixels).
  bool IsValidPatch(const itk::Index<2>& queryPixel, const unsigned int radius);
  
  // Determine if a region is completely valid (no hole pixels).
  bool IsValidRegion(const itk::ImageRegion<2>& region);
  
  // Crop 'region' to be entirely inside the full region of the images we are operating on.
  itk::ImageRegion<2> CropToValidRegion(const itk::ImageRegion<2>& region);

  // Compute the number of pixels in a patch of the specified size.
  unsigned int GetNumberOfPixelsInPatch();

  // We store the patch radius, so we need this function to compute the actual patch size from the radius.
  itk::Size<2> GetPatchSize();

  // Return the highest value of the specified image out of the pixels under a specified BoundaryImage.
  itk::Index<2> FindHighestValueOnBoundary(const FloatScalarImageType::Pointer image, float& maxValue, UnsignedCharScalarImageType::Pointer boundaryImage);

  // Update the mask so that the pixels in the region that was filled are marked as filled.
  void UpdateMask(const itk::ImageRegion<2>& region);

  // Locate all patches centered at pixels in 'region' that are completely inside of the image and completely inside of the source region and add them to the current list of source patches.
  void AddAllSourcePatchesInRegion(const itk::ImageRegion<2>& region);
  
  bool PatchExists(const itk::ImageRegion<2>& region);
  
  std::vector<Patch> AddNewSourcePatchesInRegion(const itk::ImageRegion<2>& region);
  
  // Store the list of source patches computed with ComputeSourcePatches()
  std::vector<Patch> SourcePatches;
  
  // This tracks the number of iterations that have been completed.
  unsigned int NumberOfCompletedIterations;
  
  // This is set when the image is loaded so that the region of all of the images can be addressed without referencing any specific image.
  itk::ImageRegion<2> FullImageRegion;
  
  // Store the current list of CandidatePatches.
  std::vector<CandidatePairs> PotentialCandidatePairs;
  
  // The number of bins to use per dimension in the histogram computations.
  unsigned int HistogramBinsPerDimension;
  
  // The maximum number of patch pairs to examine in deciding which one to actually fill.
  // The number compared could actually be less than this near the end of the inpainting because there may not be enough non-zero priority values outside of one patch region.
  unsigned int MaxForwardLookPatches;

  unsigned int NumberOfTopPatchesToSave;
  
  // The maximum possible pixel squared difference in the image.
  float MaxPixelDifferenceSquared;
  
  // Set the member MaxPixelDifference;
  void ComputeMaxPixelDifference();
  
  FloatScalarImageType::Pointer LuminanceImage;
  
  ///////////// CriminisiInpaintingDebugging.cpp /////////////
  void DebugWriteAllImages();
  void DebugWriteAllImages(const itk::Index<2>& pixelToFill, const itk::Index<2>& bestMatchPixel, const unsigned int iteration);
  void DebugWritePatch(const itk::Index<2>& pixel, const std::string& filePrefix, const unsigned int iteration);
  void DebugWritePatch(const itk::ImageRegion<2>& region, const std::string& filename);

  void DebugWritePatch(const itk::Index<2>& pixel, const std::string& filename);
  void DebugWritePixelToFill(const itk::Index<2>& pixelToFill, const unsigned int iteration);
  void DebugWritePatchToFillLocation(const itk::Index<2>& pixelToFill, const unsigned int iteration);

  template <typename TImage>
  float ComputeAverageGradientChange(const typename TImage::Pointer patch, FloatVector2ImageType::Pointer preFillGradient, FloatVector2ImageType::Pointer postFillGradient, const Mask::Pointer mask, const Mask::Pointer noMask, const std::vector<itk::Index<2> >& boundaryPixels);
  
  void ComputeMinimumBoundaryGradientChange(unsigned int& bestForwardLookId, unsigned int& bestSourcePatchId);
  
  unsigned int ComputeMinimumScoreLookAhead();

  SelfPatchCompare* PatchCompare;
  
  Priority* PriorityFunction;
  
};

#include "PatchBasedInpainting.hxx"

void WriteImageOfScores(const CandidatePairs& pairs, const itk::ImageRegion<2>& imageRegion, const std::string& fileName);

#endif
