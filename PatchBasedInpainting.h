#ifndef PatchBasedInpainting_h
#define PatchBasedInpainting_h

#include "Inpainting.h"

#include "Priority.h"
#include "PixelDescriptor.h"

/**
\class PatchBasedInpainting
\brief This class perform image inpainting by copying patches from elsewhere in the image into the hole.
       The PatchBasedInpainting hierarchy implements the Template Method pattern.
*/
template <typename TImage>
class PatchBasedInpainting : public Inpainting<TImage>
{
public:

  /** Constructor. An image and a mask are always required.*/
  PatchBasedInpainting(const TImage* const image, const Mask* mask);

  /** Perform Iterate() until IsDone() is true. */
  void Inpaint();

  /** Set the priority function to use. The client must have configured this object with all of the data it will need.*/
  void SetPriorityFunction(Priority* priorityFunction)
  {
    this->PriorityFunction = priorityFunction;
  }

protected:
  Priority* PriorityFunction;

  /** Perform a single step of the inpainting procedure. */
  void Iterate();

  /** Determine if the hole is completely filled or if more inpainting is necessary. */
  bool IsDone();

  /** Use the PriorityFunction to determine which pixel to fill. */
  itk::Index<2> DetermineTargetPixel();

  /** Do the comparisons from the target pixel to all valid source pixels. */
  itk::Index<2> DetermineBestSourcePixel(itk::Index<2> targetPixel);

  /** This image contains a descriptor of every pixel. */
  itk::Image<PixelDescriptor*, 2> DescriptorImage;
};

#include "PatchBasedInpainting.hxx"

#endif
