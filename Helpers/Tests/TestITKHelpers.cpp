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

#include "ITKHelpers.h"

#include "Testing.h"

#include <iostream>
#include <stdexcept>

static void TestGetIndexString();
static void TestGetSizeString();
static void TestCropToRegion();
static void TestOutputImageType();
static void TestAverageVectors();
static void TestOffsetFrom1DOffset();
static void TestSizeFromRadius();
static void TestGetNextPixelAlongVector();
static void TestGetOffsetAlongVector();
static void TestNormalizeVectorImage();
static void TestAngleBetween();
static void TestGetRegionCenter();
static void TestGetRegionInRadiusAroundPixel();
static void TestGet8NeighborOffsets();

//////////////////////////////////////////////////////////////////////////////////////////
////////////////// Function template tests (defined in ITKHelpers.hxx) ///////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void TestHasNeighborWithValue();
static void TestDeepCopy_Scalar();
static void TestDeepCopy_Vector();

int main()
{
  TestGetIndexString();
  TestGetSizeString();
  TestCropToRegion();
  TestOutputImageType();
  TestAverageVectors();
  TestOffsetFrom1DOffset();
  TestSizeFromRadius();
  TestGetNextPixelAlongVector();
  TestGetOffsetAlongVector();
  TestNormalizeVectorImage();
  TestAngleBetween();
  TestGetRegionCenter();
  TestGetRegionInRadiusAroundPixel();
  TestGet8NeighborOffsets();

  // Function template tests (defined in ITKHelpers.hxx)
  TestHasNeighborWithValue();
  TestDeepCopy_Scalar();
  TestDeepCopy_Vector();

  ////////////////// Not yet tested //////////////////////
#if 0
  // The return value MUST be a smart pointer
itk::ImageBase<2>::Pointer CreateImageWithSameType(const itk::ImageBase<2>* input);

// Convert the first 3 channels of a float vector image to an unsigned char/color/rgb image.
void VectorImageToRGBImage(const FloatVectorImageType* image, RGBImageType* rgbImage);

// Convert an RGB image to the CIELAB colorspace.
void RGBImageToCIELabImage(RGBImageType* const rgbImage, FloatVectorImageType* cielabImage);

// Convert the first 3 channels of an ITK image to the CIELAB colorspace.
void ITKImageToCIELabImage(const FloatVectorImageType* rgbImage, FloatVectorImageType* cielabImage);


// Apply the MaskedBlur function to every channel of a VectorImage separately.
void VectorMaskedBlur(const FloatVectorImageType* inputImage, const Mask* mask, const float blurVariance, FloatVectorImageType* output);

template<typename TVectorImage>
void AnisotropicBlurAllChannels(const TVectorImage* image, TVectorImage* output, const float sigma);

template<typename TImage>
void OutlineRegion(TImage* image, const itk::ImageRegion<2>& region, const typename TImage::PixelType& value);

template<typename TImage>
void DeepCopyInRegion(const TImage* input, const itk::ImageRegion<2>& region, TImage* output);

template <class TImage>
void CopyPatch(const TImage* sourceImage, TImage* targetImage, const itk::Index<2>& sourcePosition, const itk::Index<2>& targetPosition, const unsigned int radius);

template <class TImage>
void CopyPatch(const TImage* sourceImage, TImage* targetImage, const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion);

template <class TImage>
void CreateConstantPatch(TImage* patch, const typename TImage::PixelType& value, const unsigned int radius);

template<typename TImage>
void ReplaceValue(TImage* image, const typename TImage::PixelType& queryValue, const typename TImage::PixelType& replacementValue);

template <class TImage>
void CopyPatchIntoImage(const TImage* patch, TImage* image, const itk::Index<2>& position);

template <class TImage>
void CopySelfPatchIntoHoleOfTargetRegion(TImage* image, const Mask* mask,
                                   const itk::ImageRegion<2>& sourceRegionInput, const itk::ImageRegion<2>& destinationRegionInput);

template <class TImage>
void CopySourcePatchIntoHoleOfTargetRegion(const TImage* sourceImage, TImage* targetImage, const Mask* mask,
                                           const itk::ImageRegion<2>& sourceRegionInput, const itk::ImageRegion<2>& destinationRegionInput);

template <class TImage>
float MaxValue(const TImage* image);

template <class T>
std::vector<T> MaxValuesVectorImage(const itk::VectorImage<T, 2>* const image);

template <class TImage>
float MaxValueLocation(const TImage* image);

template <class TImage>
float MinValue(const TImage* image);

template <class TImage>
itk::Index<2> MinValueLocation(const TImage* image);

template <typename TImage>
void ColorToGrayscale(const TImage* colorImage, UnsignedCharScalarImageType* grayscaleImage);

template<typename TImage>
void BlankAndOutlineRegion(TImage* image, const itk::ImageRegion<2>& region, const typename TImage::PixelType& blankValue, const typename TImage::PixelType& outlineValue);

template<typename TImage>
void SetRegionToConstant(TImage* image, const itk::ImageRegion<2>& region,const typename TImage::PixelType& constant);

template<typename TImage>
void SetImageToConstant(TImage* image, const typename TImage::PixelType& constant);

template<typename TImage>
unsigned int CountNonZeroPixels(const TImage* image);

template<typename TImage>
std::vector<itk::Index<2> > GetNonZeroPixels(const TImage* image);

template<typename TImage>
std::vector<itk::Index<2> > GetNonZeroPixels(const TImage* image, const itk::ImageRegion<2>& region);

template <typename TImage>
void MaskedBlur(const TImage* inputImage, const Mask* mask, const float blurVariance, TImage* output);

template<typename TImage>
void InitializeImage(TImage* image, const itk::ImageRegion<2>& region);

template<typename TImage>
void CreatePatchImage(TImage* image, const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion, const Mask* mask, TImage* result);

template<typename TImage>
void DilateImage(const TImage* image, TImage* dilatedImage, const unsigned int radius);

template<typename TImage>
void ChangeValue(const TImage* image, const typename TImage::PixelType& oldValue, const typename TImage::PixelType& newValue);

template<typename TPixel>
void ExtractChannel(const itk::VectorImage<TPixel, 2>* image, const unsigned int channel, typename itk::Image<TPixel, 2>* output);

template<typename TPixel>
void ScaleChannel(const itk::VectorImage<TPixel, 2>* image, const unsigned int channel, const TPixel channelMax, typename itk::VectorImage<TPixel, 2>* output);

template<typename TPixel>
void ReplaceChannel(const itk::VectorImage<TPixel, 2>* image, const unsigned int channel, typename itk::Image<TPixel, 2>* replacement, typename itk::VectorImage<TPixel, 2>* output);

template<typename TImage>
typename TImage::TPixel ComputeMaxPixelDifference(const TImage* image);

template<typename TImage>
void ReadImage(const std::string&, TImage* image);

template<typename TInputImage, typename TOutputImage, typename TFilter>
void FilterImage(const TInputImage* input, TOutputImage* output);

// Median filter an image ignoring a masked region.
template<typename TImage>
void MaskedMedianFilter(const TImage* inputImage, const Mask* mask, const unsigned int kernelRadius, TImage* output);*/




#endif
  return EXIT_SUCCESS;
}

void TestDeepCopyGeneric()
{
  // Test the DeepCopy that uses downcasting to check for the image type.
// TODO: Fix this
//   itk::ImageBase<2>::Pointer image1 = FloatScalarImageType::New();
//   Testing::GetBlankImage(image1.GetPointer());
//   FloatScalarImageType::Pointer image2 = FloatScalarImageType::New();
//   ITKHelpers::DeepCopy(image1, image2);
//   if(!Testing::ImagesEqual(image1.GetPointer(), image2.GetPointer()))
//     {
//     throw std::runtime_error("Images are not equal!");
//     }
}

void TestGetIndexString()
{
  itk::Index<2> index = {{1,2}};
  std::string indexString = ITKHelpers::GetIndexString(index);
  std::string correct_indexString = "(1, 2)";
  if(indexString != correct_indexString)
    {
    std::stringstream ss;
    ss << "indexString is " << indexString << " but should be " << correct_indexString;
    throw std::runtime_error(ss.str());
    }
}

void TestGetSizeString()
{
  itk::Size<2> size = {{1,2}};
  std::string sizeString = ITKHelpers::GetSizeString(size);
  std::string correct_sizeString = "(1, 2)";
  if(sizeString != correct_sizeString)
    {
    std::stringstream ss;
    ss << "sizeString is " << sizeString << " but should be " << correct_sizeString;
    throw std::runtime_error(ss.str());
    }
}

void TestCropToRegion()
{
  itk::Index<2> indexA = {{0,0}};
  itk::Size<2> sizeA = {{100,100}};
  itk::ImageRegion<2> regionA(indexA, sizeA);

  itk::Index<2> indexB = {{-5,-5}};
  itk::Size<2> sizeB = {{10,10}};
  itk::ImageRegion<2> regionB(indexB, sizeB);

  itk::Index<2> correct_indexCropped = {{0,0}};
  itk::Size<2> correct_sizeCropped = {{5,5}};
  itk::ImageRegion<2> correct_regionCropped(correct_indexCropped, correct_sizeCropped);

  itk::ImageRegion<2> cropped = ITKHelpers::CropToRegion(regionA, regionB);

  if(cropped != correct_regionCropped)
    {
    std::stringstream ss;
    ss << "cropped is " << cropped << " but should be " << correct_regionCropped;
    throw std::runtime_error(ss.str());
    }
}

void TestOutputImageType()
{
  FloatScalarImageType::Pointer image = FloatScalarImageType::New();
  ITKHelpers::OutputImageType(image);
}

void TestAverageVectors()
{
  std::vector<FloatVector2Type> vectors;
  FloatVector2Type a;
  a[0] = 1;
  a[1] = 5;
  FloatVector2Type b;
  b[0] = 2;
  b[1] = 10;
  vectors.push_back(a);
  vectors.push_back(b);

  FloatVector2Type average = ITKHelpers::AverageVectors(vectors);
  FloatVector2Type correct_average;
  correct_average[0] = 1.5;
  correct_average[1] = 7.5;

  if(average != correct_average)
    {
    std::stringstream ss;
    ss << "average is " << average << " but should be " << correct_average;
    throw std::runtime_error(ss.str());
    }
}

void TestOffsetFrom1DOffset()
{
  // This function creates an Offset<2> by setting the 'dimension' index of the Offset<2> to the value of the Offset<1>, and filling the other position with a 0.
  itk::Offset<1> offset1D;
  offset1D[0] = 5;

  itk::Offset<2> offset2D = ITKHelpers::OffsetFrom1DOffset(offset1D, 1);
  itk::Offset<2> correct_offset2D = {{0, 5}};
  if(offset2D != correct_offset2D)
    {
    std::stringstream ss;
    ss << "offset2D is " << offset2D << " but should be " << correct_offset2D;
    throw std::runtime_error(ss.str());
    }
}

void TestSizeFromRadius()
{
  itk::Size<2> sizeFromRadius = ITKHelpers::SizeFromRadius(5);
  itk::Size<2> correct_sizeFromRadius = {{11,11}};
  if(sizeFromRadius != correct_sizeFromRadius)
    {
    std::stringstream ss;
    ss << "sizeFromRadius is " << sizeFromRadius << " but should be " << correct_sizeFromRadius;
    throw std::runtime_error(ss.str());
    }
}

void TestGetNextPixelAlongVector()
{
  itk::Index<2> pixel = {{0,0}};
  FloatVector2Type direction;
  direction[0] = 1;
  direction[1] = 0;
  itk::Index<2> nextPixel = ITKHelpers::GetNextPixelAlongVector(pixel, direction);
  itk::Index<2> correct_nextPixel = {{1,0}};
  if(nextPixel != correct_nextPixel)
    {
    std::stringstream ss;
    ss << "nextPixel is " << nextPixel << " but should be " << correct_nextPixel;
    throw std::runtime_error(ss.str());
    }
}

void TestGetOffsetAlongVector()
{
  // Get the direction in integer pixel coordinates of the 'vector'
  FloatVector2Type direction;
  direction[0] = 5;
  direction[1] = 5;
  itk::Offset<2> offset = ITKHelpers::GetOffsetAlongVector(direction);
  itk::Offset<2> correct_offset = {{1,1}};
  if(offset != correct_offset)
    {
    std::stringstream ss;
    ss << "nextPixel is " << offset << " but should be " << correct_offset;
    throw std::runtime_error(ss.str());
    }
}

void TestNormalizeVectorImage()
{
  FloatVector2ImageType::Pointer image = FloatVector2ImageType::New();
  Testing::GetBlankImage(image.GetPointer());
  itk::Index<2> corner = {{0,0}};
  FloatVector2Type v;
  v[0] = 5;
  v[1] = 1;
  image->SetPixel(corner, v);
  ITKHelpers::NormalizeVectorImage(image.GetPointer());
  FloatVector2Type correct_norm;
  correct_norm[0] = 0.98058;
  correct_norm[1] = 0.19612;
  FloatVector2Type norm = image->GetPixel(corner);
  if(!Testing::ValuesEqual(norm[0], correct_norm[0], 1e-4) || !Testing::ValuesEqual(norm[1], correct_norm[1], 1e-4))
    {
    std::stringstream ss;
    ss << "NormalizeVectorImage: norm is " << norm << " but should be " << correct_norm;
    throw std::runtime_error(ss.str());
    }
}

void TestAngleBetween()
{
  FloatVector2Type up;
  up[0] = 0;
  up[1] = 1;
  FloatVector2Type diagonal;
  diagonal[0] = 1;
  diagonal[1] = 1;
  float angleBetween = ITKHelpers::AngleBetween(up, diagonal);
  float correct_angleBetween = 0.78539816; // this is 45 degrees in radians
  if(!Testing::ValuesEqual(angleBetween, correct_angleBetween))
    {
    std::stringstream ss;
    ss << "angleBetween is " << angleBetween << " but should be " << correct_angleBetween;
    throw std::runtime_error(ss.str());
    }
}

void TestGetRegionCenter()
{
  itk::Index<2> index = {{0,0}};
  itk::Size<2> size = {{11,11}};
  itk::ImageRegion<2> region(index,size);
  itk::Index<2> center = ITKHelpers::GetRegionCenter(region);
  itk::Index<2> correct_center = {{5,5}};
  if(center != correct_center)
    {
    std::stringstream ss;
    ss << "center is " << center << " but should be " << correct_center;
    throw std::runtime_error(ss.str());
    }
}

void TestGetRegionInRadiusAroundPixel()
{
  itk::Index<2> index = {{30,30}};
  unsigned int radius = 5;
  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(index, radius);

  itk::Size<2> correct_size = {{11,11}};
  itk::Index<2> correct_index = {{index[0]-radius, index[1]-radius}};
  itk::ImageRegion<2> correct_region(correct_index, correct_size);
  if(region != correct_region)
    {
    std::stringstream ss;
    ss << "GetRegionInRadiusAroundPixel: region is " << region << " but should be " << correct_region;
    throw std::runtime_error(ss.str());
    }
}

void TestGet8NeighborOffsets()
{
  std::vector<itk::Offset<2> > offsets = ITKHelpers::Get8NeighborOffsets();
  if(offsets.size() != 8)
    {
    std::stringstream ss;
    ss << "There are only " << offsets.size() << " offsets (should be 8).";
    throw std::runtime_error(ss.str());
    }

  struct OffsetInserter
  {
    bool operator()(const itk::Offset<2>& item1, const itk::Offset<2>& item2) const
    {
      return true;
    }
  };

  std::set<itk::Offset<2>, OffsetInserter> offsetsSet;
  for(unsigned int i = 0; i < offsets.size(); ++i)
    {
    offsetsSet.insert(offsets[i]);
    }

  if(offsetsSet.size() != 8)
    {
    throw std::runtime_error("Offsets are not unique!");
    }

  for(unsigned int i = 0; i < offsets.size(); ++i)
    {
    itk::Offset<2> offset = offsets[i];
    if(((offset[0] != 0) && (offset[0] != 1) && (offset[0] != -1)) ||
       ((offset[1] != 0) && (offset[1] != 1) && (offset[1] != -1)))
      {
      std::stringstream ss;
      ss << "Neighbor offsets must be -1, 0, or 1! offset " << i << " is " << offsets[i];
      throw std::runtime_error(ss.str());
      }
    }
}

void TestHasNeighborWithValue()
{
itk::Index<2> pixel = {{5,5}};
FloatScalarImageType::Pointer image = FloatScalarImageType::New();
Testing::GetBlankImage(image.GetPointer());
float value = 2;
bool hasNeighbor = ITKHelpers::HasNeighborWithValue(pixel, image.GetPointer(), value);
if(hasNeighbor)
  {
  throw std::runtime_error("Image should not have a neighbor with this value!");
  }
itk::Index<2> neighborPixel = {{6,5}};

image->SetPixel(neighborPixel, value);
hasNeighbor = ITKHelpers::HasNeighborWithValue(pixel, image.GetPointer(), value);
if(!hasNeighbor)
  {
  throw std::runtime_error("Image should have a neighbor with this value!");
  }
}

void TestDeepCopy_Scalar()
{
  FloatScalarImageType::Pointer image1 = FloatScalarImageType::New();
  Testing::GetBlankImage(image1.GetPointer());
  FloatScalarImageType::Pointer image2 = FloatScalarImageType::New();
  ITKHelpers::DeepCopy<FloatScalarImageType>(image1, image2);
  if(!Testing::ImagesEqual(image1.GetPointer(), image2.GetPointer()))
    {
    throw std::runtime_error("DeepCopy FloatScalarImageType: Images are not equal!");
    }
}


void TestDeepCopy_Vector()
{
  FloatVectorImageType::Pointer image1 = FloatVectorImageType::New();
  Testing::GetBlankImage(image1.GetPointer(), 3);
  itk::Index<2> corner = {{0,0}};
  FloatVectorImageType::PixelType image1pixel = image1->GetPixel(corner);
  std::cout << "Image1 pixel: " << image1pixel << std::endl;

  FloatVectorImageType::Pointer image2 = FloatVectorImageType::New();
  //ITKHelpers::DeepCopy<FloatScalarImageType>(image1.GetPointer(), image2.GetPointer());
  ITKHelpers::DeepCopy(image1.GetPointer(), image2.GetPointer());
  FloatVectorImageType::PixelType image2pixel = image2->GetPixel(corner);
  std::cout << "Image2 pixel: " << image2pixel << std::endl;
  if(!Testing::ImagesEqual(image1.GetPointer(), image2.GetPointer()))
    {
    throw std::runtime_error("DeepCopy FloatVectorImageType: Images are not equal!");
    }
}
