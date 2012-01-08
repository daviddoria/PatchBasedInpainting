// Custom
#include "Helpers/ITKHelpers.h"
#include "Mask.h"

// ITK
#include "itkGaussianOperator.h"
#include "itkImageRegionIterator.h"

namespace MaskOperations
{
  
template <class TImage>
void CopySelfPatchIntoHoleOfTargetRegion(TImage* image, const Mask* mask,
                                  const itk::ImageRegion<2>& sourceRegionInput, const itk::ImageRegion<2>& destinationRegionInput)
{
  CopySourcePatchIntoHoleOfTargetRegion<TImage>(image, image, mask, sourceRegionInput, destinationRegionInput);
}


template <class TImage>
void CopySourcePatchIntoHoleOfTargetRegion(const TImage* sourceImage, TImage* targetImage, const Mask* mask,
                             const itk::ImageRegion<2>& sourceRegionInput, const itk::ImageRegion<2>& destinationRegionInput)
{
  itk::ImageRegion<2> fullImageRegion = sourceImage->GetLargestPossibleRegion();

  // We pass the regions by const reference, so copy them here before they are mutated
  itk::ImageRegion<2> sourceRegion = sourceRegionInput;
  itk::ImageRegion<2> destinationRegion = destinationRegionInput;

  // Move the source region to the desintation region
  itk::Offset<2> offset = destinationRegion.GetIndex() - sourceRegion.GetIndex();
  sourceRegion.SetIndex(sourceRegion.GetIndex() + offset);

  // Make the destination be entirely inside the image
  destinationRegion.Crop(fullImageRegion);
  sourceRegion.Crop(fullImageRegion);

  // Move the source region back
  sourceRegion.SetIndex(sourceRegion.GetIndex() - offset);

  itk::ImageRegionConstIterator<TImage> sourceIterator(sourceImage, sourceRegion);
  itk::ImageRegionIterator<TImage> destinationIterator(targetImage, destinationRegion);
  itk::ImageRegionConstIterator<Mask> maskIterator(mask, destinationRegion);

  while(!sourceIterator.IsAtEnd())
    {
    if(mask->IsHole(maskIterator.GetIndex())) // we are in the target region
      {
      destinationIterator.Set(sourceIterator.Get());
      }
    ++sourceIterator;
    ++maskIterator;
    ++destinationIterator;
    }
}


// This struct is used inside MaskedBlur()
struct Contribution
{
  float weight;
  unsigned char value;
  itk::Offset<2> offset;
};

template <typename TImage>
void MaskedBlur(const TImage* inputImage, const Mask* mask, const float blurVariance, TImage* output)
{
  // Create a Gaussian kernel
  typedef itk::GaussianOperator<float, 1> GaussianOperatorType;

  // Make a (2*kernelRadius+1)x1 kernel
  itk::Size<1> radius;
  radius.Fill(20); // Make a length 41 kernel

  GaussianOperatorType gaussianOperator;
  gaussianOperator.SetDirection(0); // It doesn't matter which direction we set - we will be interpreting the kernel as 1D (no direction)
  gaussianOperator.SetVariance(blurVariance);
  gaussianOperator.CreateToRadius(radius);

//   {
//   // Debugging only
//   std::cout << "gaussianOperator: " << gaussianOperator << std::endl;
//   for(unsigned int i = 0; i < gaussianOperator.Size(); i++)
//     {
//     //std::cout << i << " : " << gaussianOperator.GetOffset(i) << std::endl;
//     std::cout << i << " : " << gaussianOperator.GetElement(i) << std::endl;
//     }
//   }

  // Create the output image - data will be deep copied into it
  typename TImage::Pointer blurredImage = TImage::New();
  ITKHelpers::InitializeImage<TImage>(blurredImage, inputImage->GetLargestPossibleRegion());

  // Initialize
  typename TImage::Pointer operatingImage = TImage::New();
  ITKHelpers::DeepCopy<TImage>(inputImage, operatingImage);

  for(unsigned int dimensionPass = 0; dimensionPass < 2; dimensionPass++) // The image is 2D
    {
    itk::ImageRegionIterator<TImage> imageIterator(operatingImage, operatingImage->GetLargestPossibleRegion());

    while(!imageIterator.IsAtEnd())
      {
      itk::Index<2> centerPixel = imageIterator.GetIndex();

      // We should not compute derivatives for pixels in the hole.
      if(mask->IsHole(centerPixel))
        {
        ++imageIterator;
        continue;
        }

      // Loop over all of the pixels in the kernel and use the ones that fit a criteria
      std::vector<Contribution> contributions;
      for(unsigned int i = 0; i < gaussianOperator.Size(); i++)
        {
        // Since we use 1D kernels, we must manually construct a 2D offset with 0 in all dimensions except the dimension of the current pass
        itk::Offset<2> offset = ITKHelpers::OffsetFrom1DOffset(gaussianOperator.GetOffset(i), dimensionPass);

        itk::Index<2> pixel = centerPixel + offset;
        if(blurredImage->GetLargestPossibleRegion().IsInside(pixel) && mask->IsValid(pixel))
          {
          Contribution contribution;
          contribution.weight = gaussianOperator.GetElement(i);
          contribution.value = operatingImage->GetPixel(pixel);
          contribution.offset = ITKHelpers::OffsetFrom1DOffset(gaussianOperator.GetOffset(i), dimensionPass);
          contributions.push_back(contribution);
          }
        }

      float total = 0.0f;
      for(unsigned int i = 0; i < contributions.size(); i++)
        {
        total += contributions[i].weight;
        }

      // Determine the new pixel value
      float newPixelValue = 0.0f;
      for(unsigned int i = 0; i < contributions.size(); i++)
        {
        itk::Index<2> pixel = centerPixel + contributions[i].offset;
        newPixelValue += contributions[i].weight/total * operatingImage->GetPixel(pixel);
        }

      blurredImage->SetPixel(centerPixel, newPixelValue);
      ++imageIterator;
      }

    // For the separable Gaussian filtering concept to work, the next pass must operate on the output of the current pass.
    ITKHelpers::DeepCopy<TImage>(blurredImage, operatingImage);
    }

  // Copy the final image to the output.
  ITKHelpers::DeepCopy<TImage>(blurredImage, output);
}


template<typename TImage>
void CreatePatchImage(TImage* image, const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion, const Mask* mask, TImage* result)
{
  // The input 'result' is expected to already be sized and initialized.

  itk::ImageRegionConstIterator<TImage> sourceRegionIterator(image, sourceRegion);
  itk::ImageRegionConstIterator<TImage> targetRegionIterator(image, targetRegion);

  itk::ImageRegionIterator<TImage> resultIterator(result, result->GetLargestPossibleRegion());

  while(!sourceRegionIterator.IsAtEnd())
    {

    if(mask->IsHole(targetRegionIterator.GetIndex()))
      {
      resultIterator.Set(sourceRegionIterator.Get());
      }
    else
      {
      resultIterator.Set(targetRegionIterator.Get());
      }

    ++sourceRegionIterator;
    ++targetRegionIterator;
    ++resultIterator;
    }
}

template<typename TImage>
itk::Index<2> FindHighestValueInMaskedRegion(const TImage* const image, float& maxValue, const Mask* const maskImage)
{
  //EnterFunction("FindHighestValueOnBoundary()");
  // Return the location of the highest pixel in 'image' out of the non-zero pixels in 'boundaryImage'. Return the value of that pixel by reference.

  // Explicity find the maximum on the boundary
  maxValue = 0.0f; // priorities are non-negative, so anything better than 0 will win

  std::vector<itk::Index<2> > boundaryPixels = ITKHelpers::GetNonZeroPixels<UnsignedCharScalarImageType>(maskImage);

  if(boundaryPixels.size() <= 0)
    {
    throw std::runtime_error("FindHighestValueOnBoundary(): No boundary pixels!");
    }

  itk::Index<2> locationOfMaxValue = boundaryPixels[0];

  for(unsigned int i = 0; i < boundaryPixels.size(); ++i)
    {
    if(image->GetPixel(boundaryPixels[i]) > maxValue)
      {
      maxValue = image->GetPixel(boundaryPixels[i]);
      locationOfMaxValue = boundaryPixels[i];
      }
    }
  //DebugMessage<float>("Highest value: ", maxValue);
  //LeaveFunction("FindHighestValueOnBoundary()");
  return locationOfMaxValue;
}

template<typename TImage, typename TRegionIndicatorImage>
itk::Index<2> FindHighestValueInNonZeroRegion(const TImage* const image, float& maxValue, const TRegionIndicatorImage* const indicatorImage)
{
  // Create a mask from the indicator image
  Mask::Pointer mask = Mask::New();
  mask->CreateFromImage(image, itk::NumericTraits<typename TRegionIndicatorImage::PixelType>::Zero);
  return FindHighestValueInMaskedRegion(image, maxValue, mask);
}

} // end namespace
