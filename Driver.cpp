#include "itkVectorImage.h"

#include "Mask.h"
#include "PatchBasedInpainting.h"
#include "PriorityOnionPeel.h"
#include "PriorityManual.h"

int main()
{
  Mask::Pointer mask = Mask::New();

  typedef itk::VectorImage<float, 2> ImageType;
  ImageType::Pointer image = ImageType::New();

  PatchBasedInpainting<ImageType> patchBasedInpainting(image, mask);

  // Demonstrate the burden put on the user for setting up the priority functions.
  // Load images necessary for PriorityManual (we just create them here rather than load them in this demo)
//   PriorityManual::ManualPriorityImageType::Pointer manualPriorityImage = PriorityManual::ManualPriorityImageType::New();
//   PriorityOnionPeel priorityOnionPeel(mask, 5);
//   PriorityManual priorityManual(manualPriorityImage, &priorityOnionPeel);
//   patchBasedInpainting->SetPriorityFunction(&priorityManual);

  PriorityOnionPeel priorityOnionPeel(mask, 5);
  patchBasedInpainting.SetPriorityFunction(&priorityOnionPeel);

  return 0;
}
