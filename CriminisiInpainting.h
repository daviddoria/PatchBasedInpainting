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

#ifndef CriminisiInpainting_h
#define CriminisiInpainting_h

// Custom
#include "Helpers.h"
#include "Patch.h"
#include "Types.h"

// ITK
#include "itkCovariantVector.h"
#include "itkImage.h"
  
class CriminisiInpainting
{

public:

  enum DifferenceTypeEnum {DIFFERENCE_ALL, DIFFERENCE_ALL255, DIFFERENCE_DEPTH};
  
  /////////////////// CriminisiInpaintingInterface.cpp //////////////////
  
  void SetDifferenceType(const int);
  
    // Specify the image to inpaint.
  void SetImage(const FloatVectorImageType::Pointer image);
  
  // Specify the region to inpaint.
  void SetMask(const Mask::Pointer mask);
  
  // Specify the size of the patches to copy.
  void SetPatchRadius(const unsigned int);

  // Specify if you want to write debuging images.
  void SetDebugImages(const bool);
  
  // Specify if you want to see debugging messages.
  void SetDebugMessages(const bool);
  
  // Get the output of the inpainting.
  FloatVectorImageType::Pointer GetResult();
  
  // Get the current confidence image (confidences computed on the current boundary)
  FloatScalarImageType::Pointer GetConfidenceImage();

  // Get the current confidence map image
  FloatScalarImageType::Pointer GetConfidenceMapImage();
  
  // Get the current confidence image
  FloatScalarImageType::Pointer GetPriorityImage();
  
  // Get the current boundary image
  UnsignedCharScalarImageType::Pointer GetBoundaryImage();

  // Get the current boundary image
  FloatVector2ImageType::Pointer GetBoundaryNormalsImage();
  
  // Get the current isophote image
  FloatVector2ImageType::Pointer GetIsophoteImage();

  // Get the current data image
  FloatScalarImageType::Pointer GetDataImage();
  
  // Get the current mask image
  Mask::Pointer GetMaskImage();
  
  /////////////////// CriminisiInpainting.cpp //////////////////
  
  // Constructor
  CriminisiInpainting();

  // A single step of the algorithm. The real work is done here.
  void Iterate();

  // A loop that calls Iterate() until the inpainting is complete.
  void Inpaint();
  
  // Compute the confidence values for pixels that were just inpainted.
  void UpdateConfidences(const itk::ImageRegion<2>& targetRegion, const float value);
  
  // Initialize everything.
  void Initialize();

  // Determine whether or not the inpainting is completed by seeing if there are any pixels in the mask that still need to be filled.
  bool HasMoreToInpaint();
  
  bool GetUsedPatchPair(const unsigned int id, PatchPair& patchPair);
  
  bool GetPotentialPatchPairs(const unsigned int iteration, std::vector<PatchPair>& patchPairs);
  
  // Return the number of completed iterations
  unsigned int GetNumberOfCompletedIterations();
  
  // When an image is loaded, it's size is taken as the size that everything else should be. We don't want to keep referring to Image->GetLargestPossibleRegion,
  // so we store the region in a member variable. For the same reason, if we want to know the size of the images that this class is operating on, the user should
  // not have to query a specific image, but rather access this more global region definition.
  itk::ImageRegion<2> GetFullRegion();
  
private:

  // This is the original Criminisi idea of filling the highest priority first
  void FindBestPatchForHighestPriority(Patch& sourcePatch, Patch& targetPatch);
  
  // This is a new idea to try to fill several patches and return the best pair
  void FindBestPatchLookAhead(PatchPair& bestPatchPair);

  // Image to inpaint. This should not be modified throughout the algorithm.
  FloatVectorImageType::Pointer OriginalImage;
  
  // The CIELab conversion of the input RGB image
  FloatVectorImageType::Pointer CIELabImage;
  
  // The intermediate steps and eventually the result of the inpainting.
  FloatVectorImageType::Pointer CurrentOutputImage;
  
  // This image will be used for all patch to patch comparisons. It should point at either OriginalImage or CIELabImage.
  FloatVectorImageType::Pointer CompareImage;
  
  // The mask specifying the region to inpaint. This does not change throughout the algorithm - it is the original mask.
  Mask::Pointer OriginalMask;
  
  // This mask is updated as patches are copied.
  Mask::Pointer CurrentMask;
  
  // Keep track of the confidence of each pixel
  FloatScalarImageType::Pointer ConfidenceMapImage;
  
  // Store the computed confidences on the boundary
  FloatScalarImageType::Pointer ConfidenceImage;

  // Keep track of the data term of each pixel
  FloatScalarImageType::Pointer DataImage;
  
  // The patch radius.
  itk::Size<2> PatchRadius;

  // Store the computed isophotes.
  FloatVector2ImageType::Pointer IsophoteImage;
  
  // Keep track of the edge of the region to inpaint.
  UnsignedCharScalarImageType::Pointer BoundaryImage;
  
  // Store the computed boundary normals.
  FloatVector2ImageType::Pointer BoundaryNormals;

  // Keep track of the priority of each pixel.
  FloatScalarImageType::Pointer PriorityImage;

  // Compute the data term at each pixel on the curren boundary.
  void ComputeAllDataTerms();
  
  // Compute the confidence term at each pixel on the curren boundary.
  void ComputeAllConfidenceTerms();
  
  // Set the region to the full region and allocate an image
  template<typename TImage>
  void InitializeImage(typename TImage::Pointer);
  
  // Initialization functions
  
  // This function expands the mask a little bit. We must do this because the isophotes may not be well defined 
  // on the original mask boundary (if the segmentation is very tight), but they will be better defined a few pixels away.
  void InitializeMask();
  
  // The initial confidence is 0 in the hole and 1 elsewhere.
  void InitializeConfidenceMap();
  
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

  // Find the boundary of a region.
  void FindBoundary();
  
  // Compute the isophotes.
  void ComputeIsophotes();

  // Compute the normals of a region boundary.
  void ComputeBoundaryNormals();

  // Enlarge the mask so that isophotes are not computed over the mask/image boundary
  void ExpandMask();

  // Criminisi specific functions
  // Compute the priorities at all boundary pixels.
  void ComputeAllPriorities();
  
  // Compute the priority of a specific pixel.
  float ComputePriority(const itk::Index<2>& queryPixel);
  
  // Compute the Confidence at a pixel.
  float ComputeConfidenceTerm(const itk::Index<2>& queryPixel);
  
  // Compute the Data at a pixel.
  float ComputeDataTerm(const itk::Index<2>& queryPixel);

  // Return the highest value of the specified image out of the pixels under the current BoundaryImage.
  itk::Index<2> FindHighestValueOnBoundary(const FloatScalarImageType::Pointer image, float& maxValue);

  // Update the mask so that the pixels in the region that was filled are marked as filled.
  void UpdateMask(const itk::ImageRegion<2>& region);

  // Locate all patches centered at pixels in 'region' that are completely inside of the image and completely inside of the source region
  void ComputeSourcePatches(const itk::ImageRegion<2>& region);
  
  // Store the list of source patches computed with ComputeSourcePatches()
  std::vector<Patch> SourcePatches;
  
  // This tracks the number of iterations that have been completed.
  unsigned int NumberOfCompletedIterations;
  
  // This is set when the image is loaded so that the region of all of the images can be addressed without referencing any specific image.
  itk::ImageRegion<2> FullImageRegion;
  
  // Store the pairs of patches that were actually used. These are tracked for visualization purposes only.
  std::vector<PatchPair> UsedPatchPairs;
  
  // Store the pairs of patches that were examined at every iteration. These are tracked for visualization purposes only.
  std::vector<std::vector<PatchPair> > PotentialPatchPairs;
  
  // The number of bins to use per dimension in the histogram computations.
  unsigned int HistogramBinsPerDimension;
  
  // The maximum number of patch pairs to examine in deciding which one to actually fill.
  // The number compared could actually be less than this near the end of the inpainting because there may not be enough non-zero priority values outside of one patch region.
  unsigned int MaxPotentialPatches;
  
  ///////////// CriminisiInpaintingDebugging.cpp /////////////
  void DebugWriteAllImages();
  void DebugWriteAllImages(const itk::Index<2>& pixelToFill, const itk::Index<2>& bestMatchPixel, const unsigned int iteration);
  void DebugWritePatch(const itk::Index<2>& pixel, const std::string& filePrefix, const unsigned int iteration);
  void DebugWritePatch(const itk::ImageRegion<2>& region, const std::string& filename);

  void DebugWritePatch(const itk::Index<2>& pixel, const std::string& filename);
  void DebugWritePixelToFill(const itk::Index<2>& pixelToFill, const unsigned int iteration);
  void DebugWritePatchToFillLocation(const itk::Index<2>& pixelToFill, const unsigned int iteration);

  // Should we output images at every iteration?
  bool DebugImages;
  
  // Should we output verbose information about what is happenening at every iteration?
  bool DebugMessages;
  
  // Output a message if DebugMessages is set to true.
  void DebugMessage(const std::string&);
  
  // Output a message and a value if DebugMessages is set to true.
  template <typename T>
  void DebugMessage(const std::string& message, const T value);

};

#include "CriminisiInpainting.hxx"

#endif
