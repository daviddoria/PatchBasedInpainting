#include "itkVectorImage.h"

#include "Mask.h"
#include "PatchBasedInpainting.h"

int main()
{
  Mask::Pointer mask = Mask::New();

  typedef itk::VectorImage<float, 2> ImageType;
  ImageType::Pointer image = ImageType::New();

  PatchBasedInpainting<ImageType> patchBasedInpainting(image, mask);

  return 0;
}
