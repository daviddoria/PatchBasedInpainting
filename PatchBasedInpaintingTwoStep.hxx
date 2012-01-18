#include "PatchBasedInpaintingTwoStep.h" // Appease syntax parser

template <typename TImage>
PatchBasedInpaintingTwoStep<TImage>::PatchBasedInpaintingTwoStep(const TImage* const image, const Mask* mask) : PatchBasedInpainting<TImage>(image, mask)
{

}

template <typename TImage>
itk::Index<2> PatchBasedInpainting<TImage>::DetermineBestSourcePixel(itk::Index<2> targetPixel)
{
  // Compare the FeatureVectorPixelDescriptor's at every pixel to the FeatureVectorPixelDescriptor at the 'targetPixel'

  // Sort the values of the differences of the FeatureVectorPixelDescriptor's

  // For the pixels corresponding to the top N FeatureVectorPixelDescriptor differences, compare the ImagePatchPixelDescriptors to the targetPixel's ImagePatchPixelDescriptor

  // Return the location of the item with the lowest ImagePatchPixelDescriptor difference
}
