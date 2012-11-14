// STL
#include <stdexcept>

// Custom
#include "ITKHelpers/ITKHelpers.h"
#include "ImageProcessing/Mask.h"
#include "Utilities/Statistics.h"

// ITK
#include "itkGaussianOperator.h"
#include "itkLaplacianOperator.h"
#include "itkImageRegionIterator.h"

namespace MaskOperations
{
  
template <class TImage>
void CopySelfPatchIntoHoleOfTargetRegion(TImage* const image, const Mask* const mask,
                                  const itk::ImageRegion<2>& sourceRegionInput,
                                  const itk::ImageRegion<2>& destinationRegionInput)
{
  CopySourcePatchIntoHoleOfTargetRegion(image, image, mask, sourceRegionInput, destinationRegionInput);
}

template <class TImage>
void CopySourcePatchIntoHoleOfTargetRegion(const TImage* const sourceImage, TImage* const targetImage, const Mask* const mask,
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

template <typename TImage>
void MaskedLaplacian(const TImage* const inputImage, const Mask* const mask, itk::Image<float, 2>* const laplacianImage)
{
  // Create a Gaussian kernel
  typedef itk::LaplacianOperator<float, 2> LaplacianOperatorType;

  itk::Size<2> radius;
  radius.Fill(1);

  LaplacianOperatorType laplacianOperator;
//   laplacianOperator.SetDirection(0);
//   laplacianOperator.SetVariance(blurVariance);
  laplacianOperator.CreateToRadius(radius);

  // Create the output image
//   typename TImage::Pointer laplacianImage = TImage::New();
//   ITKHelpers::DeepCopy(inputImage, laplacianImage.GetPointer());
  ITKHelpers::InitializeImage(laplacianImage, inputImage->GetLargestPossibleRegion());

  itk::ImageRegionConstIteratorWithIndex<TImage> imageIterator(inputImage, inputImage->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    itk::Index<2> centerPixelIndex = imageIterator.GetIndex();

    // We should not compute derivatives for pixels in the hole.
    if(mask->IsHole(centerPixelIndex))
      {
      //std::cout << "Skipping hole pixel..." << std::endl;
      laplacianImage->SetPixel(centerPixelIndex, 0.0f);
      ++imageIterator;
      continue;
      }

    // itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(centerPixelIndex, 1);
    // unsigned int numberOfValidPixelsInRegion = mask->CountValidPixels(region);
    // unsigned int numberOfNonCenterValidPixels = numberOfValidPixelsInRegion - 1;
    
    // Loop over all of the pixels in the kernel and use the ones that are not masked
    std::vector<float> values;
    std::vector<float> weights;
    for(unsigned int i = 0; i < laplacianOperator.Size(); ++i)
      {
      itk::Offset<2> offset = laplacianOperator.GetOffset(i);
      itk::Index<2> pixelIndex = centerPixelIndex + offset;

      if(offset[0] == 0 && offset[1] == 0)
      {
	continue;
      }
//       if(offset[0] == 0 && offset[1] == 0)
//       {
// 	weights.push_back(numberOfNonCenterValidPixels);
// 	values.push_back(inputImage->GetPixel(pixel));
//       }

// This is for a center 8 and 8 -1's kernel
//       if(inputImage->GetLargestPossibleRegion().IsInside(pixel) && mask->IsValid(pixel))
// 	{
// 	//weights.push_back(laplacianOperator.GetElement(i));
// 	weights.push_back(-1.0f);
// 	values.push_back(inputImage->GetPixel(pixel));
// 	}

      // This is for a center 4 and 4 -1's kernel
      if(inputImage->GetLargestPossibleRegion().IsInside(pixelIndex) && mask->IsValid(pixelIndex) && laplacianOperator.GetElement(i) != 0)
	{
	weights.push_back(1.0f);
	values.push_back(inputImage->GetPixel(pixelIndex));
	}
      }

    typename TImage::PixelType centerPixel = imageIterator.Get();

    // Determine the new pixel value
    float newPixelValue =  -1.0f * centerPixel * static_cast<float>(values.size());
    for(unsigned int i = 0; i < weights.size(); i++)
      {
//       std::cout << "Weights: ";
//       OutputHelpers::OutputVector(weights);
// 
//       std::cout << "Values: ";
//       OutputHelpers::OutputVector(values);

      newPixelValue += weights[i] * values[i];
      }

    laplacianImage->SetPixel(centerPixelIndex, newPixelValue);
    ++imageIterator;
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
void MaskedBlur(const TImage* const inputImage, const Mask* const mask, const float blurVariance, TImage* const output)
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
        // Since we use 1D kernels, we must manually construct a 2D offset with 0 in all
        // dimensions except the dimension of the current pass
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
void CreatePatchImage(const TImage* const image, const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion,
                      const Mask* const mask, TImage* const result)
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
itk::Index<2> FindHighestValueInNonZeroRegion(const TImage* const image, float& maxValue,
                                              const TRegionIndicatorImage* const indicatorImage)
{
  // Create a mask from the indicator image
  Mask::Pointer mask = Mask::New();
  mask->CreateFromImage(image, itk::NumericTraits<typename TRegionIndicatorImage::PixelType>::Zero);
  return FindHighestValueInMaskedRegion(image, maxValue, mask);
}


template<typename TImage>
typename TImage::PixelType AverageNonMaskedNeighborValue(const TImage* const image, const Mask* const mask,
                                                         const itk::Index<2>& pixel)
{
  std::vector<itk::Index<2> > validNeighbors = mask->GetValidNeighbors(pixel);
  std::vector<typename TImage::PixelType> validValues;
  for(unsigned int i = 0; i < validNeighbors.size(); ++i)
    {
    validValues.push_back(image->GetPixel(validNeighbors[i]));
    }
    
  if(validNeighbors.size() == 0)
    {
    throw std::runtime_error("Cannot compute the average value of the non-masked neighbors because there are 0 of them!");
    }

  using Statistics::Average;
  //using ITKHelpers::Average;
  return Average(validValues);
}

template<typename TImage>
typename TImage::PixelType AverageMaskedNeighborValue(const TImage* const image, const Mask* const mask,
                                                      const itk::Index<2>& pixel)
{
  std::vector<itk::Index<2> > holeNeighbors = mask->GetHoleNeighbors(pixel);
  std::vector<typename TImage::PixelType> holeValues;
  for(unsigned int i = 0; i < holeNeighbors.size(); ++i)
    {
    holeValues.push_back(image->GetPixel(holeNeighbors[i]));
    }

  if(holeNeighbors.size() == 0)
  {
    throw std::runtime_error("Cannot compute the average value of the non-masked neighbors because there are 0 of them!");
  }
  using Statistics::Average;
  //using ITKHelpers::Average;
  return Average(holeValues);
}

template<typename TImage>
std::vector<typename TImage::PixelType> GetValidPixelsInRegion(const TImage* const image, const Mask* const mask,
                                                               const itk::ImageRegion<2>& region)
{
  std::vector<typename TImage::PixelType> validPixels;
  
  itk::ImageRegionConstIteratorWithIndex<TImage> imageIterator(image, region);

  while(!imageIterator.IsAtEnd())
    {
    if(mask->IsValid(imageIterator.GetIndex()))
      {
      validPixels.push_back(imageIterator.Get());
      }
    ++imageIterator;
    }

  return validPixels;
}

} // end namespace
