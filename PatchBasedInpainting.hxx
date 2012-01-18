#include "PatchBasedInpainting.h" // Appease syntax parser

template <typename TImage>
PatchBasedInpainting<TImage>::PatchBasedInpainting(const TImage* const image, const Mask* mask) : Inpainting<TImage>(image, mask)
{

}

template <typename TImage>
void PatchBasedInpainting<TImage>::Inpaint()
{
  while(!IsDone())
    {
    Iterate();
    }
}

template <typename TImage>
void PatchBasedInpainting<TImage>::Iterate()
{
  itk::Index<2> targetPixel = DetermineTargetPixel();
  itk::Index<2> sourcePixel = DetermineBestSourcePixel(targetPixel);

  // Fill the region around 'targetPixel' with the pixels in the region around 'sourcePixel'
}

template <typename TImage>
bool PatchBasedInpainting<TImage>::IsDone()
{
  // Determine if the hole is entirely filled.
  return false;
}

template <typename TImage>
itk::Index<2> PatchBasedInpainting<TImage>::DetermineTargetPixel()
{
//   for all boundary pixels
//   {
//     this->PriorityFunction->ComputePriority()
//   }
// 
//   return pixel with highest priority
}

template <typename TImage>
itk::Index<2> PatchBasedInpainting<TImage>::DetermineBestSourcePixel(itk::Index<2> targetPixel)
{
  // Compare the ImagePatchPixelDescriptor at every pixel with a non-NULL ImagePatchPixelDescriptor to the ImagePatchPixelDescriptor at the 'targetPixel'

  // Return the location of the item with the lowest ImagePatchPixelDescriptor difference
}
