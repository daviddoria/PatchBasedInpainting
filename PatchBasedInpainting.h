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
#include "ITKImageCollection.h"
#include "Mask.h"
#include "Patch.h"
#include "PatchPair.h"
#include "PatchSorting.h"
#include "SourcePatchCollection.h"
#include "Types.h"

class Priority;
class SelfPatchCompare;

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

  // Specify the size of the patches to copy.
  void SetPatchRadius(const unsigned int);
  unsigned int GetPatchRadius();

  // Get the result/output of the inpainting so far. When the algorithm is complete, this will be the final output.
  FloatVectorImageType::Pointer GetCurrentOutputImage();

  // Get the current mask image
  Mask* GetMaskImage();

  //////////////////////////////////////////////////////////////
  /////////////////// CriminisiInpainting.cpp //////////////////
  //////////////////////////////////////////////////////////////

  // Constructor
  //PatchBasedInpainting();
  PatchBasedInpainting(const FloatVectorImageType* image, const Mask* mask);

  // A single step of the algorithm. The real work is done here.
  PatchPair Iterate();

  // A loop that calls Iterate() until the inpainting is complete.
  void Inpaint();

  // Initialize everything.
  void Initialize();

  // Determine whether or not the inpainting is completed by seeing if there are any pixels in the mask that still need to be filled.
  bool HasMoreToInpaint();

  // Return the number of completed iterations
  unsigned int GetNumberOfCompletedIterations();

  // When an image is loaded, it's size is taken as the size that everything else should be. We don't want to keep referring to Image->GetLargestPossibleRegion,
  // so we store the region in a member variable. For the same reason, if we want to know the size of the images that this class is operating on, the user should
  // not have to query a specific image, but rather access this more global region definition.
  itk::ImageRegion<2> GetFullRegion();

  // Return a pointer to all forward look sets.
  std::vector<CandidatePairs>& GetPotentialCandidatePairsReference();

  //void SetPatchCompare(SelfPatchCompare* PatchCompare);
  SelfPatchCompare* GetPatchCompare() const;

  std::shared_ptr<PatchSortFunctor> PatchSortFunction;

//   template <typename T>
//   void SetPriorityFunction();
  void SetPriorityFunction(const std::string& priorityType);

  Priority* GetPriorityFunction();

  // We store the patch radius, so we need this function to compute the actual patch size from the radius.
  itk::Size<2> GetPatchSize();

  ITKImageCollection& GetImagesToUpdate();

private:

  void SetupHistograms();
  void ColorImageInsideHole();

  void BlurImage();

  void ComputeScores(CandidatePairs& candidatePairs);

  virtual PatchPair FindBestPatch();

  // The intermediate steps and eventually the result of the inpainting.
  FloatVectorImageType::Pointer CurrentInpaintedImage;

  // This image will be used for all patch to patch comparisons.
  //itk::ImageBase<2>* CompareImage; // Ideally we would be able to compare any type of image...
  FloatVectorImageType* CompareImage; // Currently we can only compare images of this type.

  // The images in this collection will have the selected patch copied into them at each iteration.
  // It should at least include the image and the mask.
  ITKImageCollection ImagesToUpdate;

  // The mask specifying the region to inpaint. It is updated as patches are copied.
  Mask::Pointer MaskImage;

  // The patch radius.
  itk::Size<2> PatchRadius;

  // Determine if a patch is completely valid (no hole pixels).
  bool IsValidPatch(const itk::Index<2>& queryPixel, const unsigned int radius);

  // Determine if a region is completely valid (no hole pixels).
  bool IsValidRegion(const itk::ImageRegion<2>& region);

  // Compute the number of pixels in a patch of the specified size.
  unsigned int GetNumberOfPixelsInPatch();

  bool PatchExists(const itk::ImageRegion<2>& region);

  SourcePatchCollection* SourcePatches;

  // This tracks the number of iterations that have been completed.
  unsigned int NumberOfCompletedIterations;

  // This is set when the image is loaded so that the region of all of the images can be addressed without referencing any specific image.
  itk::ImageRegion<2> FullImageRegion;

  // The number of bins to use per dimension in the histogram computations.
  unsigned int HistogramBinsPerDimension;

  // The maximum possible pixel squared difference in the image.
  float MaxPixelDifferenceSquared;

  // Set the member MaxPixelDifference;
  void ComputeMaxPixelDifference();

  ///////////// CriminisiInpaintingDebugging.cpp /////////////
  void DebugWriteAllImages();
  void DebugWriteAllImages(const itk::Index<2>& pixelToFill, const itk::Index<2>& bestMatchPixel, const unsigned int iteration);
  void DebugWritePatch(const itk::Index<2>& pixel, const std::string& filePrefix, const unsigned int iteration);
  void DebugWritePatch(const itk::ImageRegion<2>& region, const std::string& filename);

  void DebugWritePatch(const itk::Index<2>& pixel, const std::string& filename);
  void DebugWritePixelToFill(const itk::Index<2>& pixelToFill, const unsigned int iteration);
  void DebugWritePatchToFillLocation(const itk::Index<2>& pixelToFill, const unsigned int iteration);

  std::shared_ptr<SelfPatchCompare> PatchCompare;

  std::shared_ptr<Priority> PriorityFunction;

};

#include "PatchBasedInpainting.hxx"

#endif
