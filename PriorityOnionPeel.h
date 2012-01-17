#ifndef PriorityOnionPeel_H
#define PriorityOnionPeel_H

#include "Priority.h"

#include "itkImage.h"

class Mask;

/**
\class PriorityOnionPeel
\brief This class computes the priority of a pixel based on its closeness to the original hole boundary.
       This example of a Priority subclass depends on inputs that we must already have.
*/
class PriorityOnionPeel : public Priority
{
public:
  typedef itk::Image<float, 2> ConfidenceMapImageType;
  ///////////////////////////////////////////
  // Functions reimplemented from Priority //
  ///////////////////////////////////////////

  PriorityOnionPeel(const Mask* const maskImage, const unsigned int patchRadius) : MaskImage(maskImage), PatchRadius(patchRadius){}

  float ComputePriority(const itk::Index<2>& queryPixel) const;

  void Update(const itk::ImageRegion<2>& filledRegion);

private:

  /** Compute the Confidence values for pixels that were just inpainted.*/
  void UpdateConfidences(const itk::ImageRegion<2>& targetRegion, const float value);

  /** Compute the Confidence at a pixel.*/
  float ComputeConfidenceTerm(const itk::Index<2>& queryPixel) const;

  /** Keep track of the Confidence of each pixel*/
  ConfidenceMapImageType::Pointer ConfidenceMapImage;

  /** The mask image. */
  const Mask* MaskImage;

    /** The patch radius to use when computing the confidence. */
  const unsigned int PatchRadius;
};

#endif
