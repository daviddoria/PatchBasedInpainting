#include "Histogram.h"

#include "itkVectorImage.h"
#include "itkImageRegionConstIterator.h"

#include "Mask.h"

namespace Histogram
{
typedef itk::VectorImage<float, 2> FloatVectorImageType;

std::vector<float> HistogramRegion(const FloatVectorImageType* image, const itk::ImageRegion<2>& imageRegion,
                                   const Mask* mask, const itk::ImageRegion<2>& maskRegion, const bool invertMask)
{
  std::vector<float> histogram(this->Colors.size(), 0.0f);

  itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(image, imageRegion);
  itk::ImageRegionConstIterator<Mask> maskIterator(mask, maskRegion);

  while(!imageIterator.IsAtEnd())
    {
    if(mask->IsHole(maskIterator.GetIndex()))
      {
      ++imageIterator;
      ++maskIterator;
      continue;
      }
    FloatVectorImageType::PixelType pixel = imageIterator.Get();
    ColorMeasurementVectorType measurement;
    measurement[0] = pixel[0];
    measurement[1] = pixel[1];
    measurement[2] = pixel[2];

    TreeType::InstanceIdentifierVectorType neighbors;
    this->KDTree->Search( measurement, 1u, neighbors );

    histogram[neighbors[0]] += 1.0f;

    ++imageIterator;
    ++maskIterator;
    }

  return histogram;
}

}
