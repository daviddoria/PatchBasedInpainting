#include "BoundaryEnergy.h" // Appease syntax parser

#include "Helpers/ITKHelpers.h"
#include "ImageProcessing/MaskOperations.h"
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

  // We are unsure about the values of the boundary pixels, but they are definitely greater than .5
  // (we'd hope non-boundary=0 and boundary = 1 or 255)
  GreaterThanOrEqualFunctor<Mask::BoundaryImageType::PixelType> greaterThanOrEqualFunctor(1);
  std::vector<itk::Index<2> > pixelsSatisfyingFunctor = PixelsSatisfyingFunctor(boundaryImage.GetPointer(),
                                                        region, greaterThanOrEqualFunctor);

  if(pixelsSatisfyingFunctor.size() == 0)
  {
    std::stringstream ss;
    ss << "Cannot compute boundary energy - there are no boundary pixels in the specified region: " << region;
    throw std::runtime_error(ss.str());
  }
  
  float totalDifference = 0.0f;
  for(unsigned int i = 0; i < pixelsSatisfyingFunctor.size(); ++i)
  {
    typename TImage::PixelType averageMaskedNeighborValue =
      MaskOperations::AverageMaskedNeighborValue(Image, MaskImage, pixelsSatisfyingFunctor[i]);

    typename TImage::PixelType averageValidNeighborValue =
      MaskOperations::AverageNonMaskedNeighborValue(Image, MaskImage, pixelsSatisfyingFunctor[i]);

    totalDifference += Difference(averageMaskedNeighborValue, averageValidNeighborValue);
  }

  float averageDifference = totalDifference / static_cast<float>(pixelsSatisfyingFunctor.size());
  return averageDifference;
}

template<typename TImage>
float BoundaryEnergy<TImage>::operator()(const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion)
{
  // The boundary must be in the target region.
  Mask::BoundaryImageType::Pointer boundaryImage = Mask::BoundaryImageType::New();
  this->MaskImage->FindBoundaryInRegion(targetRegion, boundaryImage);

  // We are unsure about the values of the boundary pixels, but they are definitely greater than .5
  // (we'd hope non-boundary=0 and boundary = 1 or 255)
  GreaterThanOrEqualFunctor<Mask::BoundaryImageType::PixelType> greaterThanOrEqualFunctor(1);
  std::vector<itk::Index<2> > boundaryPixels = PixelsSatisfyingFunctor(boundaryImage.GetPointer(),
                                                        targetRegion, greaterThanOrEqualFunctor);

  if(boundaryPixels.size() == 0)
  {
    std::stringstream ss;
    ss << "Cannot compute boundary energy - there are no boundary pixels in the specified target region: " << targetRegion;
    throw std::runtime_error(ss.str());
  }

  float totalDifference = 0.0f;
  for(unsigned int i = 0; i < boundaryPixels.size(); ++i)
  {
    typename TImage::PixelType averageMaskedNeighborValue =
      MaskOperations::AverageMaskedNeighborValue(Image, MaskImage, boundaryPixels[i]);

    itk::Offset<2> boundaryPixelOffsetFromTargetCorner = boundaryPixels[i] - targetRegion.GetIndex();

    std::vector<itk::Offset<2> > validPixelOffsets = MaskImage->GetValidNeighborOffsets(boundaryPixels[i]);

    std::vector<itk::Index<2> > sourceRegionValidPixelIndices =
       ITKHelpers::OffsetsToIndices(validPixelOffsets, sourceRegion.GetIndex() + boundaryPixelOffsetFromTargetCorner);

    typename TImage::PixelType averageValidNeighborValue = ITKHelpers::AverageOfPixelsAtIndices(Image,
                                                                                                sourceRegionValidPixelIndices);
//     typename TImage::PixelType averageValidNeighborValue =
//       MaskOperations::AverageNonMaskedNeighborValue(Image, MaskImage, sourceRegion.GetIndex() + targetOffset);

    totalDifference += Difference(averageMaskedNeighborValue, averageValidNeighborValue);
  }

  float averageDifference = totalDifference / static_cast<float>(boundaryPixels.size());
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
