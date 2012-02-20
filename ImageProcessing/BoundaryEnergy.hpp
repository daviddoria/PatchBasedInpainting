#include "BoundaryEnergy.h" // Appease syntax parser

#include "Helpers/ITKHelpers.h"
#include "ImageProcessing/PixelFilterFunctors.hpp"

template<typename TImage>
BoundaryEnergy<TImage>::BoundaryEnergy(const TImage* const image, const Mask* const mask) : Image(image), MaskImage(mask)
{

}

template<typename TImage>
float BoundaryEnergy<TImage>::operator()(const itk::ImageRegion<2>& region)
{
  Mask::BoundaryImageType::Pointer boundaryImage = Mask::BoundaryImageType::New();
  this->MaskImage->FindBoundaryInRegion(region, boundaryImage);

  GreaterThanFunctor<Mask::BoundaryImageType::PixelType> greaterThanFunctor(.5); // We are unsure about the values of the boundary pixels, but they are definitely greater than .5 (we'd hope non-boundary=0 and boundary = 1 or 255)
  std::vector<itk::Index<2> > pixelsSatisfyingFunctor = PixelsSatisfyingFunctor(boundaryImage.GetPointer(),
                                                      boundaryImage->GetLargestPossibleRegion(), greaterThanFunctor);

  float totalDifference = 0.0f;
  for(unsigned int i = 0; i < pixelsSatisfyingFunctor.size(); ++i)
  {
    typename TImage::PixelType averageMaskedNeighborValue =
      ITKHelpers::AverageMaskedNeighborValue(Image, MaskImage, pixelsSatisfyingFunctor[i]);

    typename TImage::PixelType averageValidNeighborValue =
      ITKHelpers::AverageNonMaskedNeighborValue(Image, MaskImage, pixelsSatisfyingFunctor[i]);

    totalDifference += Difference(averageMaskedNeighborValue, averageValidNeighborValue);
  }

  float averageDifference = totalDifference / static_cast<float>(pixelsSatisfyingFunctor.size());
  return averageDifference;
}

template<typename TImage>
template <typename T>
float BoundaryEnergy<TImage>::Difference(const T& a, const T& b)
{
  return a - b;
}

template<typename TImage>
float BoundaryEnergy<TImage>::Difference(const VectorPixelType& a, const VectorPixelType& b)
{
  return (a-b).GetNorm();
}
