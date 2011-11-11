/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "itkCastImageFilter.h"

// STL
#include <iomanip> // for setfill()

// VTK
#include <vtkImageData.h>

// ITK
#include "itkBilateralImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkComposeImageFilter.h"
#include "itkGaussianOperator.h"
#include "itkImageFileWriter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkPasteImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkVectorMagnitudeImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

// Qt
#include <QColor>
#include <QDir>

// Custom
#include "Mask.h"

namespace Helpers
{

/** Copy the input to the output*/
template<typename TImage>
void DeepCopy(const typename TImage::Pointer input, typename TImage::Pointer output)
{
  //std::cout << "DeepCopy()" << std::endl;
  if(output->GetLargestPossibleRegion() != input->GetLargestPossibleRegion())
    {
    output->SetRegions(input->GetLargestPossibleRegion());
    output->Allocate();
    }
  DeepCopyInRegion<TImage>(input, input->GetLargestPossibleRegion(), output);
}

template<typename TImage>
void DeepCopyInRegion(const typename TImage::Pointer input, const itk::ImageRegion<2>& region, typename TImage::Pointer output)
{
  // This function assumes that the size of input and output are the same.
  
  itk::ImageRegionConstIterator<TImage> inputIterator(input, region);
  itk::ImageRegionIterator<TImage> outputIterator(output, region);

  while(!inputIterator.IsAtEnd())
    {
    outputIterator.Set(inputIterator.Get());
    ++inputIterator;
    ++outputIterator;
    }
}

template<typename T>
void ReplaceValue(typename T::Pointer image, const typename T::PixelType queryValue, const typename T::PixelType replacementValue)
{
  // This function replaces all pixels in 'image' equal to 'queryValue' with 'replacementValue'
  itk::ImageRegionIterator<T> imageIterator(image, image->GetLargestPossibleRegion());
  imageIterator.GoToBegin();
  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.Get() == queryValue)
      {
      imageIterator.Set(replacementValue);
      }
    ++imageIterator;
    }
}

template <class T>
void CreateBlankPatch(typename T::Pointer patch, const unsigned int radius)
{
  CreateConstantPatch<T>(patch, itk::NumericTraits< typename T::PixelType >::Zero, radius);
}

template <class T>
void CreateConstantPatch(typename T::Pointer patch, const typename T::PixelType value, const unsigned int radius)
{
  try
  {
  typename T::IndexType start;
  start.Fill(0);

  typename T::SizeType size;
  size.Fill(radius*2 + 1);

  typename T::RegionType region(start, size);

  patch->SetRegions(region);
  patch->Allocate();

  itk::ImageRegionIterator<T> imageIterator(patch, patch->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    imageIterator.Set(value);
    ++imageIterator;
    }

  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in CreateConstantPatch!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <class T>
std::vector<T> MaxValuesVectorImage(const typename itk::VectorImage<T, 2>::Pointer image)
{
  typedef itk::VectorImage<T, 2> VectorImageType;
  typedef itk::Image<T, 2> ScalarImageType;
  
  typedef itk::VectorIndexSelectionCastImageFilter<VectorImageType, ScalarImageType > IndexSelectionType;
  typename IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
  indexSelectionFilter->SetInput(image);

  std::vector<T> maxValues;
  for(unsigned int channel = 0; channel < image->GetNumberOfComponentsPerPixel(); ++channel)
    {
    indexSelectionFilter->SetIndex(channel);
    indexSelectionFilter->Update();
    maxValues.push_back(MaxValue<ScalarImageType>(indexSelectionFilter->GetOutput()));
    }
  return maxValues;
}


template <class T>
float MaxValue(const typename T::Pointer image)
{
  typedef typename itk::MinimumMaximumImageCalculator<T>
          ImageCalculatorFilterType;

  typename ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(image);
  imageCalculatorFilter->Compute();

  return imageCalculatorFilter->GetMaximum();
}

template <class T>
float MaxValueLocation(const typename T::Pointer image)
{
  typedef typename itk::MinimumMaximumImageCalculator<T>
          ImageCalculatorFilterType;

  typename ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(image);
  imageCalculatorFilter->Compute();

  return imageCalculatorFilter->GetIndexOfMaximum();
}

template <class T>
float MinValue(const typename T::Pointer image)
{
  typedef typename itk::MinimumMaximumImageCalculator<T>
          ImageCalculatorFilterType;

  typename ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(image);
  imageCalculatorFilter->Compute();

  return imageCalculatorFilter->GetMinimum();
}

template <class T>
itk::Index<2> MinValueLocation(const typename T::Pointer image)
{
  typedef typename itk::MinimumMaximumImageCalculator<T>
          ImageCalculatorFilterType;

  typename ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(image);
  imageCalculatorFilter->Compute();

  return imageCalculatorFilter->GetIndexOfMinimum();
}

template <class T>
void CopyPatchIntoImage(const typename T::Pointer patch, typename T::Pointer image, const Mask::Pointer mask, const itk::Index<2>& position)
{
  try
  {
  // This function copies 'patch' into 'image' centered at 'position' only where the 'mask' is non-zero

  // 'Mask' must be the same size as 'image'
  if(mask->GetLargestPossibleRegion().GetSize() != image->GetLargestPossibleRegion().GetSize())
    {
    std::cerr << "mask and image must be the same size!" << std::endl;
    exit(-1);
    }

  // The PasteFilter expects the lower left corner of the destination position, but we have passed the center pixel.
  position[0] -= patch->GetLargestPossibleRegion().GetSize()[0]/2;
  position[1] -= patch->GetLargestPossibleRegion().GetSize()[1]/2;

  itk::ImageRegion<2> region = GetRegionInRadiusAroundPixel(position, patch->GetLargestPossibleRegion().GetSize()[0]/2);

  itk::ImageRegionConstIterator<T> patchIterator(patch,patch->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<Mask> maskIterator(mask,region);
  itk::ImageRegionIterator<T> imageIterator(image, region);

  while(!patchIterator.IsAtEnd())
    {
    if(mask->IsHole(maskIterator.GetIndex())) // we are in the target region
      {
      imageIterator.Set(patchIterator.Get());
      }
    ++imageIterator;
    ++maskIterator;
    ++patchIterator;
    }

  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in CopyPatchIntoImage(patch, image, mask, position)!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <class T>
void CopySourcePatchIntoHoleOfTargetRegion(typename T::Pointer sourceImage, typename T::Pointer targetImage, const Mask::Pointer mask,
                             const itk::ImageRegion<2>& sourceRegionInput, const itk::ImageRegion<2>& destinationRegionInput)
{
  try
  {
    itk::ImageRegion<2> fullImageRegion = sourceImage->GetLargestPossibleRegion();
//     if(targetImage->GetLargestPossibleRegion() != fullImageRegion)
//       {
//       std::cerr << "CopyPatchIntoValidRegion::Images must be the same size!" << std::endl;
//       exit(-1);
//       }

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

    itk::ImageRegionConstIterator<T> sourceIterator(sourceImage, sourceRegion);
    itk::ImageRegionIterator<T> destinationIterator(targetImage, destinationRegion);
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
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in CopySelfPatchIntoValidRegion!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <class T>
void CopySelfPatchIntoHoleOfTargetRegion(typename T::Pointer image, const Mask::Pointer mask,
                                  const itk::ImageRegion<2>& sourceRegionInput, const itk::ImageRegion<2>& destinationRegionInput)
{
  CopySourcePatchIntoHoleOfTargetRegion<T>(image, image, mask, sourceRegionInput, destinationRegionInput);
}

template <class T>
void CopyPatchIntoImage(const typename T::Pointer patch, typename T::Pointer image, const itk::Index<2>& centerPixel)
{
  try
  {
    // This function copies 'patch' into 'image' centered at 'position'.

    // The PasteFilter expects the lower left corner of the destination position, but we have passed the center pixel.
    itk::Index<2> cornerPixel;
    cornerPixel[0] = centerPixel[0] - patch->GetLargestPossibleRegion().GetSize()[0]/2;
    cornerPixel[1] = centerPixel[1] - patch->GetLargestPossibleRegion().GetSize()[1]/2;

    typedef itk::PasteImageFilter <T, T> PasteImageFilterType;

    typename PasteImageFilterType::Pointer pasteFilter = PasteImageFilterType::New();
    pasteFilter->SetInput(0, image);
    pasteFilter->SetInput(1, patch);
    pasteFilter->SetSourceRegion(patch->GetLargestPossibleRegion());
    pasteFilter->SetDestinationIndex(cornerPixel);
    pasteFilter->InPlaceOn();
    pasteFilter->Update();

    image->Graft(pasteFilter->GetOutput());

  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in CopyPatchIntoImage(patch, image, centerPixel)!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <class TImage>
void CopyPatch(const typename TImage::Pointer sourceImage, typename TImage::Pointer targetImage, const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion)
{
  if(targetRegion.GetSize() != sourceRegion.GetSize())
    {
    std::cerr << "Can't copy regions that aren't the same size!" << std::endl;
    return;
    }

  itk::ImageRegionConstIterator<TImage> sourceIterator(sourceImage, sourceRegion);
  itk::ImageRegionIterator<TImage> targetIterator(targetImage, targetRegion);

  while(!sourceIterator.IsAtEnd())
    {
    targetIterator.Set(sourceIterator.Get());

    ++sourceIterator;
    ++targetIterator;
    }
}

template <class T>
void CopyPatch(const typename T::Pointer sourceImage, typename T::Pointer targetImage,
               const itk::Index<2>& sourcePosition, const itk::Index<2>& targetPosition, const unsigned int radius)
{
  try
  {
  // Copy a patch of radius 'radius' centered at 'sourcePosition' from 'sourceImage' to 'targetImage' centered at 'targetPosition'
  typedef itk::RegionOfInterestImageFilter<T,T> ExtractFilterType;

  typename ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
  extractFilter->SetRegionOfInterest(GetRegionInRadiusAroundPixel(sourcePosition, radius));
  extractFilter->SetInput(sourceImage);
  extractFilter->Update();

  CopyPatchIntoImage<T>(extractFilter->GetOutput(), targetImage, targetPosition);
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in CopyPatch!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}




template <typename TImage>
void ColorToGrayscale(const typename TImage::Pointer colorImage, UnsignedCharScalarImageType::Pointer grayscaleImage)
{
  grayscaleImage->SetRegions(colorImage->GetLargestPossibleRegion());
  grayscaleImage->Allocate();

  itk::ImageRegionConstIterator<TImage> colorImageIterator(colorImage, colorImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<UnsignedCharScalarImageType> grayscaleImageIterator(grayscaleImage, grayscaleImage->GetLargestPossibleRegion());

  typename TImage::PixelType largestPixel;
  largestPixel.Fill(255);

  float largestNorm = largestPixel.GetNorm();

  while(!colorImageIterator.IsAtEnd())
    {
    grayscaleImageIterator.Set(colorImageIterator.Get().GetNorm()*(255./largestNorm));

    ++colorImageIterator;
    ++grayscaleImageIterator;
    }
}



template <typename TImage>
void ITKScalarImageToScaledVTKImage(const typename TImage::Pointer image, vtkImageData* outputImage)
{
  //std::cout << "ITKScalarImagetoVTKImage()" << std::endl;
  
  // Rescale and cast for display
  typedef itk::RescaleIntensityImageFilter<TImage, UnsignedCharScalarImageType > RescaleFilterType;
  typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(255);
  rescaleFilter->SetInput(image);
  rescaleFilter->Update();

  // Setup and allocate the VTK image
  outputImage->SetNumberOfScalarComponents(1);
  outputImage->SetScalarTypeToUnsignedChar();
  outputImage->SetDimensions(image->GetLargestPossibleRegion().GetSize()[0],
                             image->GetLargestPossibleRegion().GetSize()[1],
                             1);

  outputImage->AllocateScalars();

  // Copy all of the scaled magnitudes to the output image
  itk::ImageRegionConstIteratorWithIndex<UnsignedCharScalarImageType> imageIterator(rescaleFilter->GetOutput(), rescaleFilter->GetOutput()->GetLargestPossibleRegion());
  imageIterator.GoToBegin();

  while(!imageIterator.IsAtEnd())
    {
    unsigned char* pixel = static_cast<unsigned char*>(outputImage->GetScalarPointer(imageIterator.GetIndex()[0],
                                                                                     imageIterator.GetIndex()[1],0));
    pixel[0] = imageIterator.Get();

    ++imageIterator;
    }
    
  outputImage->Modified();
}

template<typename TImage>
void SetRegionToConstant(typename TImage::Pointer image, const itk::ImageRegion<2>& region, const typename TImage::PixelType& value)
{
  typename itk::ImageRegionIterator<TImage> imageIterator(image, region);

  while(!imageIterator.IsAtEnd())
    {
    imageIterator.Set(value);

    ++imageIterator;
    }
}

template<typename TImage>
void SetImageToConstant(typename TImage::Pointer image, const typename TImage::PixelType& constant)
{
  SetRegionToConstant<TImage>(image, image->GetLargestPossibleRegion(), constant);
}

template<typename TImage>
unsigned int CountNonZeroPixels(const typename TImage::Pointer image)
{
  typename itk::ImageRegionIterator<TImage> imageIterator(image, image->GetLargestPossibleRegion());

  unsigned int numberOfNonZeroPixels = 0;
  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.Get())
      {
      numberOfNonZeroPixels++;
      }

    ++imageIterator;
    }
  return numberOfNonZeroPixels;
}

template<typename TImage>
std::vector<itk::Index<2> > GetNonZeroPixels(const typename TImage::Pointer image)
{
  return GetNonZeroPixels<TImage>(image, image->GetLargestPossibleRegion());
}

template<typename TImage>
std::vector<itk::Index<2> > GetNonZeroPixels(const typename TImage::Pointer image, const itk::ImageRegion<2>& region)
{
  std::vector<itk::Index<2> > nonZeroPixels;
  
  typename itk::ImageRegionIterator<TImage> imageIterator(image, region);

  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.Get())
      {
      nonZeroPixels.push_back(imageIterator.GetIndex());
      }

    ++imageIterator;
    }
  return nonZeroPixels;
}


template <class T>
unsigned int argmin(const typename std::vector<T>& vec)
{
  T minValue = std::numeric_limits<T>::max();
  unsigned int minLocation = 0;
  for(unsigned int i = 0; i < vec.size(); ++i)
    {
    if(vec[i] < minValue)
      {
      minValue = vec[i];
      minLocation = i;
      }
    }
    
  return minLocation;
}


template<typename TImage>
void WriteRegionAsImage(const typename TImage::Pointer image, const itk::ImageRegion<2>& region, const std::string& filename)
{
  // This function varies from WriteRegion() in that the Origin of the output image is (0,0).
  // Because of this, the region cannot be overlayed on the original image, but can be easily compared to other regions.
  //std::cout << "WriteRegion() " << filename << std::endl;
  //std::cout << "region " << region << std::endl;
  
  typedef itk::RegionOfInterestImageFilter<TImage, TImage> RegionOfInterestImageFilterType;

  typename RegionOfInterestImageFilterType::Pointer regionOfInterestImageFilter = RegionOfInterestImageFilterType::New();
  regionOfInterestImageFilter->SetRegionOfInterest(region);
  regionOfInterestImageFilter->SetInput(image);
  regionOfInterestImageFilter->Update();
  
  itk::Point<float, 2> origin;
  origin.Fill(0);
  regionOfInterestImageFilter->GetOutput()->SetOrigin(origin);

  //std::cout << "regionOfInterestImageFilter " << regionOfInterestImageFilter->GetOutput()->GetLargestPossibleRegion() << std::endl;
  
  typename itk::ImageFileWriter<TImage>::Pointer writer = itk::ImageFileWriter<TImage>::New();
  writer->SetFileName(filename);
  writer->SetInput(regionOfInterestImageFilter->GetOutput());
  writer->Update();
}



template<typename TImage>
void WriteRegionUnsignedChar(const typename TImage::Pointer image, const itk::ImageRegion<2>& region, const std::string& filename)
{
  // The file that is output has Origin = (0,0) because of how VectorImageToRGBImage copies the image.
  
  //std::cout << "WriteRegionUnsignedChar() " << filename << std::endl;
  typedef itk::RegionOfInterestImageFilter<TImage, TImage> RegionOfInterestImageFilterType;

  typename RegionOfInterestImageFilterType::Pointer regionOfInterestImageFilter = RegionOfInterestImageFilterType::New();
  regionOfInterestImageFilter->SetRegionOfInterest(region);
  regionOfInterestImageFilter->SetInput(image);
  regionOfInterestImageFilter->Update();

  //std::cout << "regionOfInterestImageFilter " << regionOfInterestImageFilter->GetOutput()->GetLargestPossibleRegion() << std::endl;
  
  RGBImageType::Pointer rgbImage = RGBImageType::New();
  VectorImageToRGBImage(regionOfInterestImageFilter->GetOutput(), rgbImage);
  
  //std::cout << "rgbImage " << rgbImage->GetLargestPossibleRegion() << std::endl;
  
  typename itk::ImageFileWriter<RGBImageType>::Pointer writer = itk::ImageFileWriter<RGBImageType>::New();
  writer->SetFileName(filename);
  writer->SetInput(rgbImage);
  writer->Update();
}





template<typename TImage>
void BlankAndOutlineRegion(typename TImage::Pointer image, const itk::ImageRegion<2>& region, const typename TImage::PixelType& blankValue, const typename TImage::PixelType& outlineValue)
{
  SetRegionToConstant<TImage>(image, region, blankValue);
  OutlineRegion<TImage>(image, region, outlineValue);

}

template<typename TImage>
void OutlineRegion(typename TImage::Pointer image, const itk::ImageRegion<2>& region, const typename TImage::PixelType& value)
{
  itk::ImageRegionIterator<TImage> iterator(image, region);

  for(unsigned int i = region.GetIndex()[0]; i < region.GetIndex()[0] + region.GetSize()[0]; ++i)
    {
    itk::Index<2> index;
    index[0] = i;
    index[1] = region.GetIndex()[1];
    image->SetPixel(index, value);

    index[0] = i;
    index[1] = region.GetIndex()[1] + region.GetSize()[1] - 1;
    image->SetPixel(index, value);
    }

  for(unsigned int j = region.GetIndex()[1]; j < region.GetIndex()[1] + region.GetSize()[1]; ++j)
    {
    itk::Index<2> index;
    index[0] = region.GetIndex()[0];
    index[1] = j;
    image->SetPixel(index, value);

    index[0] = region.GetIndex()[0] + region.GetSize()[0] - 1;
    index[1] = j;
    image->SetPixel(index, value);
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
void MaskedBlur(const typename TImage::Pointer inputImage, const Mask::Pointer mask, const float blurVariance, typename TImage::Pointer output)
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
  InitializeImage<TImage>(blurredImage, inputImage->GetLargestPossibleRegion());
  
  // Initialize
  typename TImage::Pointer operatingImage = TImage::New();
  DeepCopy<TImage>(inputImage, operatingImage);
  
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
	itk::Offset<2> offset = OffsetFrom1DOffset(gaussianOperator.GetOffset(i), dimensionPass);
      
	itk::Index<2> pixel = centerPixel + offset;
	if(blurredImage->GetLargestPossibleRegion().IsInside(pixel) && mask->IsValid(pixel))
	  {
	  Contribution contribution;
	  contribution.weight = gaussianOperator.GetElement(i);
	  contribution.value = operatingImage->GetPixel(pixel);
	  contribution.offset = OffsetFrom1DOffset(gaussianOperator.GetOffset(i), dimensionPass);
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
    DeepCopy<TImage>(blurredImage, operatingImage);
    }
  
  // Copy the final image to the output.
  DeepCopy<TImage>(blurredImage, output);
}



template<typename TImage>
void InitializeImage(typename TImage::Pointer image, const itk::ImageRegion<2>& region)
{
  image->SetRegions(region);
  image->Allocate();
  //image->FillBuffer(0);
}

template<typename TImage>
void CreatePatchImage(typename TImage::Pointer image, const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion, Mask::Pointer mask, typename TImage::Pointer result)
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

template<typename TVectorImage>
void BlurAllChannels(const typename TVectorImage::Pointer image, typename TVectorImage::Pointer output, const float sigma)
{
  typedef itk::Image<typename TVectorImage::InternalPixelType, 2> ScalarImageType;
  
  // Disassembler
  typedef itk::VectorIndexSelectionCastImageFilter<TVectorImage, ScalarImageType> IndexSelectionType;
  typename IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
  indexSelectionFilter->SetInput(image);
  
  // Reassembler
  typedef itk::ComposeImageFilter<ScalarImageType> ImageToVectorImageFilterType;
  typename ImageToVectorImageFilterType::Pointer imageToVectorImageFilter = ImageToVectorImageFilterType::New();
  
  std::vector< typename ScalarImageType::Pointer > filteredImages;
  
  for(unsigned int i = 0; i < image->GetNumberOfComponentsPerPixel(); ++i)
    {
    indexSelectionFilter->SetIndex(i);
    indexSelectionFilter->Update();
  
    typename ScalarImageType::Pointer imageChannel = ScalarImageType::New();
    DeepCopy<ScalarImageType>(indexSelectionFilter->GetOutput(), imageChannel);
  
    typedef itk::BilateralImageFilter<ScalarImageType, ScalarImageType>  BilateralFilterType;
    typename BilateralFilterType::Pointer bilateralFilter = BilateralFilterType::New();
    bilateralFilter->SetInput(imageChannel);
    bilateralFilter->SetDomainSigma(sigma);
    bilateralFilter->SetRangeSigma(sigma);
    bilateralFilter->Update();
    
    typename ScalarImageType::Pointer blurred = ScalarImageType::New();
    DeepCopy<ScalarImageType>(bilateralFilter->GetOutput(), blurred);
    
    filteredImages.push_back(blurred);
    imageToVectorImageFilter->SetInput(i, filteredImages[i]);
    }

  imageToVectorImageFilter->Update();
 
  DeepCopy<TVectorImage>(imageToVectorImageFilter->GetOutput(), output);
}

template<typename T>
void NormalizeVector(std::vector<T>& v)
{
  T total = static_cast<T>(0);
  for(unsigned int i = 0; i < v.size(); ++i)
    {
    total += v[i];
    }
    
  for(unsigned int i = 0; i < v.size(); ++i)
    {
    v[i] /= total;
    }
}

template<typename TImage>
void DilateImage(const typename TImage::Pointer image, typename TImage::Pointer dilatedImage, const unsigned int radius)
{
  typedef itk::BinaryBallStructuringElement<typename TImage::PixelType, 2> StructuringElementType;
  StructuringElementType structuringElement;
  structuringElement.SetRadius(radius);
  structuringElement.CreateStructuringElement();
 
  typedef itk::BinaryDilateImageFilter <TImage, TImage, StructuringElementType> BinaryDilateImageFilterType;
 
  typename BinaryDilateImageFilterType::Pointer dilateFilter = BinaryDilateImageFilterType::New();
  dilateFilter->SetInput(image);
  dilateFilter->SetKernel(structuringElement);
  dilateFilter->Update();
  
  DeepCopy<TImage>(dilateFilter->GetOutput(), dilatedImage);
    
}

template<typename TImage>
void ChangeValue(const typename TImage::Pointer image, const typename TImage::PixelType& oldValue, const typename TImage::PixelType& newValue)
{
  itk::ImageRegionIterator<TImage> iterator(image, image->GetLargestPossibleRegion());

  while(!iterator.IsAtEnd())
    {
    if(iterator.Get() == oldValue)
      {
      iterator.Set(newValue);
      }
    ++iterator;
    }  
}

template<typename TPixel>
void ScaleChannel(const typename itk::VectorImage<TPixel, 2>::Pointer image, const unsigned int channel, const TPixel channelMax, typename itk::VectorImage<TPixel, 2>::Pointer output)
{
  typedef itk::VectorImage<TPixel, 2> VectorImageType;
  typedef itk::Image<TPixel, 2> ScalarImageType;
  
  typedef itk::VectorIndexSelectionCastImageFilter<VectorImageType, ScalarImageType > IndexSelectionType;
  typename IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
  indexSelectionFilter->SetIndex(channel);
  indexSelectionFilter->SetInput(image);
  indexSelectionFilter->Update();
  
  typedef itk::RescaleIntensityImageFilter< ScalarImageType, ScalarImageType > RescaleFilterType;
  typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetInput(indexSelectionFilter->GetOutput());
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(channelMax);
  rescaleFilter->Update();
  
  DeepCopy<itk::VectorImage<TPixel, 2> >(image, output);
  
  ReplaceChannel<TPixel>(output, channel, rescaleFilter->GetOutput(), output);
}

template<typename TPixel>
void ReplaceChannel(const typename itk::VectorImage<TPixel, 2>::Pointer image, const unsigned int channel, typename itk::Image<TPixel, 2>::Pointer replacement, typename itk::VectorImage<TPixel, 2>::Pointer output)
{
  if(image->GetLargestPossibleRegion() != replacement->GetLargestPossibleRegion())
    {
    std::cerr << "Cannot replace channel!" << std::endl;
    exit(-1);
    }

  DeepCopy<typename itk::VectorImage<TPixel, 2> >(image, output);
  
  itk::ImageRegionConstIterator<itk::VectorImage<TPixel, 2> > iterator(image, image->GetLargestPossibleRegion());
  
  while(!iterator.IsAtEnd())
    {
    typename itk::VectorImage<TPixel, 2>::PixelType pixel = iterator.Get();
    pixel[channel] = replacement->GetPixel(iterator.GetIndex());
    output->SetPixel(iterator.GetIndex(), pixel);
    ++iterator;
    }
}

template<typename TImage>
typename TImage::TPixel ComputeMaxPixelDifference(const typename TImage::Pointer image)
{
  
//   // We assume all values of all channels are positive, so the max difference can be computed as max(p_i - 0) because (0,0,0,...) is the minimum pixel.
//   
//   itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(this->OriginalImage, this->OriginalImage->GetLargestPossibleRegion());
// 
//   FloatVectorImageType::PixelType zeroPixel;
//   zeroPixel.SetSize(this->OriginalImage->GetNumberOfComponentsPerPixel());
//   zeroPixel.Fill(0);
//   
//   FloatVectorImageType::PixelType maxPixel = zeroPixel;
//   while(!imageIterator.IsAtEnd())
//     {
//     FloatVectorImageType::PixelType pixel = this->OriginalImage->GetPixel(imageIterator.GetIndex());
//     //std::cout << "pixel: " << pixel << " Norm: " << pixel.GetNorm() << std::endl;
//     if(pixel.GetNorm() > maxPixel.GetNorm())
//       {
//       maxPixel = pixel;
//       }
//     ++imageIterator;
//     }
// 
//   /*
//   this->MaxPixelDifference = 0.0f;
//   for(unsigned int i = 0; i < this->OriginalImage->GetNumberOfComponentsPerPixel(); ++i)
//     {
//     this->MaxPixelDifference += maxPixel[i];
//     }
//   */
//   this->MaxPixelDifferenceSquared = FullPixelDifference::Difference(maxPixel, zeroPixel, zeroPixel.GetNumberOfElements());
//   
//   std::cout << "MaxPixelDifference computed to be: " << this->MaxPixelDifferenceSquared << std::endl;
//   
}

}// end namespace
