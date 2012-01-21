#include "PatchBasedInpainting.h" // Appease syntax parser

template <typename TImage>
PatchBasedInpainting<TImage>::PatchBasedInpainting(const TImage* const image, const Mask* mask) : Inpainting<TImage>(image, mask)
{

}

template <typename TImage>
void PatchBasedInpainting<TImage>::Inpaint()
{
  // NOTE MP : Doesn't this basic scheme seem completely retarded from a performance point of view?
  //           Maybe these notes will help:
  while(!IsDone())  // Traverse all the pixels to figure out if a hole-pixel remains, then ...
    {
    Iterate();  // Traverse all the pixels to find the next hole-pixel to fill.. !!
    }
    
  // NOTE MP : Normally, an algorithm like this should be implemented by keeping a priority-queue 
  //           of the hole-pixels to visit, and run until the queue is empty.
}

template <typename TImage>
void PatchBasedInpainting<TImage>::Iterate()
{
  // NOTE MP : The following two lines should be externalized from this function, because the 
  //           priorities of the target pixels should drive the traversal. In other words, as 
  //           with a priority-queue, the top element should be the next to "visit", you shouldn't
  //           have to do a full search for that top element, the only way to coordinate this is 
  //           from the traversal algorithm, not here.
  itk::Index<2> targetPixel = DetermineTargetPixel();
  itk::Index<2> sourcePixel = DetermineBestSourcePixel(targetPixel);

  // Fill the region around 'targetPixel' with the pixels in the region around 'sourcePixel'
}

template <typename TImage>
bool PatchBasedInpainting<TImage>::IsDone()
{
  // NOTE MP : Shouldn't the algorithm terminate when it is done? 
  //           In any case, it will certainly be more efficient and practical for the algorithm
  //           itself to keep track of its incremental progress towards the solution than to have 
  //           some algorithm like "do a pass, check completion, and repeat".
  //           IMO, this function is useless.
  
  // Determine if the hole is entirely filled.
  return false;
}

template <typename TImage>
itk::Index<2> PatchBasedInpainting<TImage>::DetermineTargetPixel()
{
  // NOTE MP : This should be implemented with a priority-queue. 
  //           There are plenty such implementations available, namely, 
  //             - the standard std::priority_queue
  //             - the BGL's n_ary_heap_indirect  (recommended for its flexibility, and static 
  //               indirection), see the implementation of the breadth_first_search in the BGL.
  
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
