
#include "PriorityRandom.h"

template <typename TImage>
PriorityRandom<TImage>::PriorityRandom(const FloatVectorImageType* const image, const Mask* const maskImage, const unsigned int patchRadius) :
Priority<TImage>(image, maskImage, patchRadius)
{

}

template <typename TImage>
std::vector<std::string> PriorityRandom<TImage>::GetImageNames()
{
  std::vector<std::string> imageNames = Priority<TImage>::GetImageNames();
  return imageNames;
}

template <typename TImage>
float PriorityRandom<TImage>::ComputePriority(const itk::Index<2>& queryPixel)
{
  return drand48();
}
