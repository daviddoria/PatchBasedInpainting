/*=========================================================================
 *
 *  Copyright David Doria 2010 daviddoria@gmail.com
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

#include "CriminisiInpainting.h"

#include "RotateVectors.h"
#include "SelfPatchMatch.h"
#include "Helpers.h"

#include <iostream>
#include <iomanip> // setfill and setw

#include <vnl/vnl_double_2.h>

template <class TImage>
CriminisiInpainting<TImage>::CriminisiInpainting()
{
  this->PatchRadius.Fill(3);

  this->BoundaryImage = UnsignedCharImageType::New();
  this->BoundaryNormals = VectorImageType::New();
  this->IsophoteImage = VectorImageType::New();
  this->PriorityImage = FloatImageType::New();
  this->Mask = UnsignedCharImageType::New();
  this->InputMask = UnsignedCharImageType::New();
  this->Image = TImage::New();
  //this->Patch = ColorImageType::New();
  this->ConfidenceImage = FloatImageType::New();

  this->WriteIntermediateImages = false;
  this->Weights.resize(TImage::PixelType::GetNumberOfComponents());
  for(unsigned int i = 0; i < this->Weights.size(); i++)
    {
    this->Weights[i] = 1.0/this->Weights.size();
    }
}

template <class TImage>
void CriminisiInpainting<TImage>::SetWeights(std::vector<float> weights)
{
  this->Weights = weights;
}

template <class TImage>
void CriminisiInpainting<TImage>::SetPatchRadius(unsigned int radius)
{
  // Since this is the radius of the patch, there are no restrictions for the radius to be odd or even.
  this->PatchRadius.Fill(radius);
}

template <class TImage>
void CriminisiInpainting<TImage>::SetImage(typename TImage::Pointer image)
{
  this->Image->Graft(image);
}

template <class TImage>
void CriminisiInpainting<TImage>::SetInputMask(UnsignedCharImageType::Pointer mask)
{
  this->InputMask->Graft(mask);
}

template <class TImage>
void CriminisiInpainting<TImage>::SetWriteIntermediateImages(bool flag)
{
  this->WriteIntermediateImages = flag;
}

template <class TImage>
void CriminisiInpainting<TImage>::ExpandMask()
{
  // Ensure the mask is actually binary
  typedef itk::BinaryThresholdImageFilter <UnsignedCharImageType, UnsignedCharImageType>
          BinaryThresholdImageFilterType;

  BinaryThresholdImageFilterType::Pointer thresholdFilter
          = BinaryThresholdImageFilterType::New();
  thresholdFilter->SetInput(this->InputMask);
  thresholdFilter->SetLowerThreshold(122);
  thresholdFilter->SetUpperThreshold(255);
  thresholdFilter->SetInsideValue(255);
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->InPlaceOn();
  thresholdFilter->Update();

  // Expand the mask - this is necessary to prevent the isophotes from being undefined in the target region
  typedef itk::FlatStructuringElement<2> StructuringElementType;
  StructuringElementType::RadiusType radius;
  radius.Fill(this->PatchRadius[0]);
  //radius.Fill(2.0* this->PatchRadius[0]);

  StructuringElementType structuringElement = StructuringElementType::Box(radius);
  typedef itk::BinaryDilateImageFilter<UnsignedCharImageType, UnsignedCharImageType, StructuringElementType>
          BinaryDilateImageFilterType;

  BinaryDilateImageFilterType::Pointer expandMaskFilter
          = BinaryDilateImageFilterType::New();
  expandMaskFilter->SetInput(thresholdFilter->GetOutput());
  expandMaskFilter->SetKernel(structuringElement);
  expandMaskFilter->Update();
  this->Mask->Graft(expandMaskFilter->GetOutput());

  //WriteScaledImage<UnsignedCharImageType>(this->Mask, "expandedMask.mhd");
}

template <class TImage>
void CriminisiInpainting<TImage>::Initialize()
{
  ExpandMask();

  // Do this before we mask the image with the expanded mask
  ComputeIsophotes();

  // Create a blank priority image
  this->PriorityImage->SetRegions(this->Image->GetLargestPossibleRegion());
  this->PriorityImage->Allocate();

  // Clone the mask - we need to invert the mask to actually perform the masking, but we don't want to disturb the original mask
  typedef itk::ImageDuplicator< UnsignedCharImageType > DuplicatorType;
  DuplicatorType::Pointer duplicator = DuplicatorType::New();
  duplicator->SetInputImage(this->Mask);
  duplicator->Update();

  // Invert the mask
  typedef itk::InvertIntensityImageFilter <UnsignedCharImageType>
          InvertIntensityImageFilterType;

  InvertIntensityImageFilterType::Pointer invertIntensityFilter
          = InvertIntensityImageFilterType::New();
  invertIntensityFilter->SetInput(duplicator->GetOutput());
  invertIntensityFilter->InPlaceOn();
  invertIntensityFilter->Update();

  // Convert the inverted mask to floats and scale them to between 0 and 1
  // to serve as the initial confidence image
  typedef itk::RescaleIntensityImageFilter< UnsignedCharImageType, FloatImageType > RescaleFilterType;
  RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetInput(invertIntensityFilter->GetOutput());
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(1);
  rescaleFilter->Update();

  this->ConfidenceImage->Graft(rescaleFilter->GetOutput());
  //WriteImage<FloatImageType>(this->ConfidenceImage, "InitialConfidence.mhd");

  // Mask the input image with the inverted mask (blank the region in the input image that we will fill in)
  typedef itk::MaskImageFilter< TImage, UnsignedCharImageType, TImage> MaskFilterType;
  typename MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput1(this->Image);
  maskFilter->SetInput2(invertIntensityFilter->GetOutput());
  maskFilter->InPlaceOn();

  // We set non-masked pixels to green so we can visually ensure these pixels are not being copied during the inpainting
  typename TImage::PixelType green;
  green[0] = 0;
  green[1] = 255;
  green[2] = 0;
  for(unsigned int i = 3; i < TImage::PixelType::GetNumberOfComponents(); i++)
    {
    green[i] = 0;
    }
  maskFilter->SetOutsideValue(green);
  maskFilter->Update();

  this->Image->Graft(maskFilter->GetOutput());

  // Debugging outputs
  //WriteImage<TImage>(this->Image, "InitialImage.mhd");
  //WriteImage<UnsignedCharImageType>(this->Mask, "InitialMask.mhd");
  //WriteImage<FloatImageType>(this->ConfidenceImage, "InitialConfidence.mhd");
  //WriteImage<VectorImageType>(this->IsophoteImage, "InitialIsophotes.mhd");
}

template <class TImage>
void CriminisiInpainting<TImage>::Inpaint()
{
  try
  {
  Initialize();

  int iteration = 0;
  while(HasMoreToInpaint(this->Mask))
    {
    std::cout << "Iteration: " << iteration << std::endl;

    FindBoundary();

    ComputeBoundaryNormals();

    ComputeAllPriorities();

    itk::Index<2> pixelToFill = FindHighestPriority(this->PriorityImage);
    std::cout << "Filling: " << pixelToFill << std::endl;

    if(this->WriteIntermediateImages)
      {
      DebugWritePatch(pixelToFill, "PatchToFill", iteration); // this must be done here because it is before the filling
      }

    itk::Index<2> bestMatchPixel = SelfPatchMatch<TImage, UnsignedCharImageType>(this->Image, this->Mask, pixelToFill, this->PatchRadius[0], iteration, this->Weights);
    std::cout << "Best match pixel: " << bestMatchPixel << std::endl;

    CopySelfPatchIntoValidRegion<TImage>(this->Image, this->Mask, bestMatchPixel, pixelToFill, this->PatchRadius[0]);
    CopySelfPatchIntoValidRegion<FloatImageType>(this->ConfidenceImage, this->Mask, bestMatchPixel, pixelToFill, this->PatchRadius[0]);
    CopySelfPatchIntoValidRegion<VectorImageType>(this->IsophoteImage, this->Mask, bestMatchPixel, pixelToFill, this->PatchRadius[0]);

    // Update the mask
    UpdateMask(pixelToFill);

    // Sanity check everything
    if(this->WriteIntermediateImages)
      {
      //DebugWriteAllImages(pixelToFill, bestMatchPixel, iteration);
      DebugWriteImage<TImage>(this->Image,"FilledImage", iteration);
      }

    iteration++;
    } // end main while loop

  WriteImage<TImage>(this->Image, "result.mhd");
  WriteImage<TImage>(this->Image, "result.png");

    }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in Inpaint!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <class TImage>
void CriminisiInpainting<TImage>::ComputeIsophotes()
{
  try
  {
  // Convert the color input image to a grayscale image
  UnsignedCharImageType::Pointer grayscaleImage = UnsignedCharImageType::New();
  ColorToGrayscale<TImage>(this->Image, grayscaleImage);

  //WriteImage<UnsignedCharImageType>(grayscaleImage, "greyscale.mhd");

  // Blur the image to compute better gradient estimates
  typedef itk::DiscreteGaussianImageFilter<
          UnsignedCharImageType, FloatImageType >  filterType;

  // Create and setup a Gaussian filter
  filterType::Pointer gaussianFilter = filterType::New();
  gaussianFilter->SetInput(grayscaleImage);
  gaussianFilter->SetVariance(2);
  gaussianFilter->Update();

  //WriteImage<FloatImageType>(gaussianFilter->GetOutput(), "gaussianBlur.mhd");

  // Compute the gradient
  // TInputImage, TOperatorValueType, TOutputValueType
  typedef itk::GradientImageFilter<
      FloatImageType, float, float>  GradientFilterType;
  GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
  gradientFilter->SetInput(gaussianFilter->GetOutput());
  gradientFilter->Update();

  //WriteImage<VectorImageType>(gradientFilter->GetOutput(), "gradient.mhd");

  // Rotate the gradient 90 degrees to obtain isophotes from gradient
  typedef itk::UnaryFunctorImageFilter<VectorImageType, VectorImageType,
                                  RotateVectors<
    VectorImageType::PixelType,
    VectorImageType::PixelType> > FilterType;

  FilterType::Pointer rotateFilter = FilterType::New();
  rotateFilter->SetInput(gradientFilter->GetOutput());
  rotateFilter->Update();

  //WriteImage<VectorImageType>(rotateFilter->GetOutput(), "originalIsophotes.mhd");

  // Mask the isophote image with the expanded version of the inpainting mask.
  // That is, keep only the values outside of the expanded mask. To do this, we have to first invert the mask.

  // Invert the mask
  typedef itk::InvertIntensityImageFilter <UnsignedCharImageType>
          InvertIntensityImageFilterType;

  InvertIntensityImageFilterType::Pointer invertMaskFilter
          = InvertIntensityImageFilterType::New();
  invertMaskFilter->SetInput(this->Mask);
  invertMaskFilter->Update();

  //WriteScaledImage<UnsignedCharImageType>(invertMaskFilter->GetOutput(), "invertedExpandedMask.mhd");

  // Keep only values outside the masked region
  typedef itk::MaskImageFilter< VectorImageType, UnsignedCharImageType, VectorImageType > MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput1(rotateFilter->GetOutput());
  maskFilter->SetInput2(invertMaskFilter->GetOutput());
  maskFilter->Update();

  this->IsophoteImage->Graft(maskFilter->GetOutput());
  //WriteImage<VectorImageType>(this->IsophoteImage, "validIsophotes.mhd");
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeIsophotes!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <class TImage>
bool CriminisiInpainting<TImage>::HasMoreToInpaint(UnsignedCharImageType::Pointer mask)
{
  itk::ImageRegionIterator<UnsignedCharImageType> imageIterator(mask,mask->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.Get() != 0)
      {
      return true;
      }

    ++imageIterator;
    }

  return false;
}

template <class TImage>
void CriminisiInpainting<TImage>::FindBoundary()
{
  try
  {
  /*
  // If we simply find the boundary of the mask, the isophotes will not be copied into these pixels because they are 1 pixel
  // away from the filled region
  typedef itk::BinaryContourImageFilter <UnsignedCharImageType, UnsignedCharImageType >
          binaryContourImageFilterType;

  binaryContourImageFilterType::Pointer binaryContourFilter
          = binaryContourImageFilterType::New ();
  binaryContourFilter->SetInput(this->Mask);
  binaryContourFilter->Update();

  this->BoundaryImage = binaryContourFilter->GetOutput();
  */

  // Instead, we have to invert the mask before finding the boundary

  // Invert the mask
  typedef itk::InvertIntensityImageFilter <UnsignedCharImageType>
          InvertIntensityImageFilterType;

  InvertIntensityImageFilterType::Pointer invertIntensityFilter
          = InvertIntensityImageFilterType::New();
  invertIntensityFilter->SetInput(this->Mask);
  invertIntensityFilter->Update();

  // Find the boundary
  typedef itk::BinaryContourImageFilter <UnsignedCharImageType, UnsignedCharImageType >
          binaryContourImageFilterType;

  binaryContourImageFilterType::Pointer binaryContourFilter
          = binaryContourImageFilterType::New ();
  binaryContourFilter->SetInput(invertIntensityFilter->GetOutput());
  binaryContourFilter->Update();

  //this->BoundaryImage = binaryContourFilter->GetOutput();
  this->BoundaryImage->Graft(binaryContourFilter->GetOutput());
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in FindBoundary!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <class TImage>
void CriminisiInpainting<TImage>::UpdateMask(itk::Index<2> pixel)
{
  try
  {
  // Create a black patch
  UnsignedCharImageType::Pointer blackPatch = UnsignedCharImageType::New();
  CreateBlankPatch<UnsignedCharImageType>(blackPatch, this->PatchRadius[0]);

  pixel[0] -= blackPatch->GetLargestPossibleRegion().GetSize()[0]/2;
  pixel[1] -= blackPatch->GetLargestPossibleRegion().GetSize()[1]/2;

  // Paste it into the mask
  typedef itk::PasteImageFilter <UnsignedCharImageType, UnsignedCharImageType >
          PasteImageFilterType;

  PasteImageFilterType::Pointer pasteFilter
          = PasteImageFilterType::New();
  pasteFilter->SetInput(0, this->Mask);
  pasteFilter->SetInput(1, blackPatch);
  pasteFilter->SetSourceRegion(blackPatch->GetLargestPossibleRegion());
  pasteFilter->SetDestinationIndex(pixel);
  pasteFilter->InPlaceOn();
  pasteFilter->Update();

  //this->Mask = pasteFilter->GetOutput();
  this->Mask->Graft(pasteFilter->GetOutput());
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in UpdateMask!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <class TImage>
void CriminisiInpainting<TImage>::ComputeBoundaryNormals()
{
  try
  {
  // Blur the mask, compute the gradient, then keep the normals only at the original mask boundary

  typedef itk::DiscreteGaussianImageFilter<
          UnsignedCharImageType, FloatImageType >  filterType;

  // Create and setup a Gaussian filter
  filterType::Pointer gaussianFilter = filterType::New();
  gaussianFilter->SetInput(this->Mask);
  gaussianFilter->SetVariance(2);
  gaussianFilter->Update();

  typedef itk::GradientImageFilter<
      FloatImageType, float, float>  GradientFilterType;
  GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
  gradientFilter->SetInput(gaussianFilter->GetOutput());
  gradientFilter->Update();

  // Only keep the normals at the boundary
  typedef itk::MaskImageFilter< VectorImageType, UnsignedCharImageType, VectorImageType > MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput1(gradientFilter->GetOutput());
  maskFilter->SetInput2(this->BoundaryImage);
  maskFilter->Update();

  //this->BoundaryNormals = maskFilter->GetOutput();
  this->BoundaryNormals->Graft(maskFilter->GetOutput());

  // Normalize
  itk::ImageRegionIterator<VectorImageType> imageIterator(this->BoundaryNormals,this->BoundaryNormals->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<UnsignedCharImageType> boundaryIterator(this->BoundaryImage,this->BoundaryImage->GetLargestPossibleRegion());
  imageIterator.GoToBegin();
  boundaryIterator.GoToBegin();

  while(!imageIterator.IsAtEnd())
    {
    if(boundaryIterator.Get()) // The pixel is on the boundary
      {
      VectorImageType::PixelType p = imageIterator.Get();
      p.Normalize();
      imageIterator.Set(p);
      }
    ++imageIterator;
    ++boundaryIterator;
    }

  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeBoundaryNormals!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <class TImage>
itk::Index<2> CriminisiInpainting<TImage>::FindHighestPriority(FloatImageType::Pointer priorityImage)
{
  typedef itk::MinimumMaximumImageCalculator <FloatImageType>
          ImageCalculatorFilterType;

  ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(priorityImage);
  imageCalculatorFilter->Compute();

  return imageCalculatorFilter->GetIndexOfMaximum();
}

template <class TImage>
void CriminisiInpainting<TImage>::ComputeAllPriorities()
{
  try
  {
  // Only compute priorities for pixels on the boundary
  itk::ImageRegionConstIterator<UnsignedCharImageType> boundaryIterator(this->BoundaryImage, this->BoundaryImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<FloatImageType> priorityIterator(this->PriorityImage, this->PriorityImage->GetLargestPossibleRegion());

  boundaryIterator.GoToBegin();
  priorityIterator.GoToBegin();

  // The main loop is over the boundary image. We only want to compute priorities at boundary pixels
  while(!boundaryIterator.IsAtEnd())
    {
    // If the pixel is not on the boundary, skip it and set its priority to -1.
    // -1 is used instead of 0 because if the priorities on the boundary get to 0, we still want to choose a boundary
    // point rather than a random image point.
    if(boundaryIterator.Get() == 0)
      {
      priorityIterator.Set(-1);
      ++boundaryIterator;
      ++priorityIterator;
      continue;
      }

    float priority = ComputePriority(boundaryIterator.GetIndex());
    //std::cout << "Priority: " << priority << std::endl;
    priorityIterator.Set(priority);

    ++boundaryIterator;
    ++priorityIterator;
    }

  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeAllPriorities!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <class TImage>
float CriminisiInpainting<TImage>::ComputePriority(UnsignedCharImageType::IndexType queryPixel)
{
  double confidence = ComputeConfidenceTerm(queryPixel);
  double data = ComputeDataTerm(queryPixel);

  return confidence * data;
}

template <class TImage>
float CriminisiInpainting<TImage>::ComputeConfidenceTerm(itk::Index<2> queryPixel)
{
  try
  {
  if(!this->Mask->GetLargestPossibleRegion().IsInside(GetRegionInRadiusAroundPixel(queryPixel,this->PatchRadius[0])))
    {
    return 0;
    }

  // DONT allow for regions on/near the image border
  //itk::ImageRegionConstIterator<UnsignedCharImageType> maskIterator(this->Mask, GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0]));
  //itk::ImageRegionConstIterator<FloatImageType> confidenceIterator(this->ConfidenceImage, GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0]));

  // allow for regions on/near the image border

    itk::ImageRegion<2> region = this->Mask->GetLargestPossibleRegion();
    region.Crop(GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0]));
    itk::ImageRegionConstIterator<UnsignedCharImageType> maskIterator(this->Mask, region);
    itk::ImageRegionConstIterator<FloatImageType> confidenceIterator(this->ConfidenceImage, region);


    maskIterator.GoToBegin();
    confidenceIterator.GoToBegin();
    // confidence = sum of the confidences of patch pixels in the source region / area of the patch

    unsigned int numberOfPixels = GetNumberOfPixelsInPatch();

    float sum = 0;

    while(!maskIterator.IsAtEnd())
      {
      if(maskIterator.Get() == 0) // Pixel is in the source region
        {
        sum += confidenceIterator.Get();
        }
      ++confidenceIterator;
      ++maskIterator;
      }

    float areaOfPatch = static_cast<float>(numberOfPixels);
    return sum/areaOfPatch;

  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeConfidenceTerm!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <class TImage>
float CriminisiInpainting<TImage>::ComputeDataTerm(UnsignedCharImageType::IndexType queryPixel)
{
  try
  {
  itk::CovariantVector<float, 2> isophote = this->IsophoteImage->GetPixel(queryPixel);
  itk::CovariantVector<float, 2> boundaryNormal = this->BoundaryNormals->GetPixel(queryPixel);

  double alpha = 255; // for grey scale images
  // D(p) = |dot(isophote direction at p, normal of the front at p)|/alpha

  vnl_double_2 vnlIsophote(isophote[0], isophote[1]);

  vnl_double_2 vnlNormal(boundaryNormal[0], boundaryNormal[1]);

  double dot = std::abs(dot_product(vnlIsophote,vnlNormal));

  return dot/alpha;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeDataTerm!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <class TImage>
bool CriminisiInpainting<TImage>::IsValidPatch(itk::Index<2> queryPixel, unsigned int radius)
{
  try
  {
  // Check if the patch is inside the image
  if(!this->Mask->GetLargestPossibleRegion().IsInside(GetRegionInRadiusAroundPixel(queryPixel,radius)))
    {
    return false;
    }

  // Check if all the pixels in the patch are in the valid region of the image
  itk::ImageRegionConstIterator<UnsignedCharImageType> iterator(this->Mask,GetRegionInRadiusAroundPixel(queryPixel, radius));
    while(!iterator.IsAtEnd())
    {
    if(iterator.Get()) // valid(unmasked) pixels are 0, masked pixels are 1 (non-zero)
      {
      return false;
      }

    ++iterator;
    }

  return true;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in IsValidPatch!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <class TImage>
itk::CovariantVector<float, 2> CriminisiInpainting<TImage>::GetAverageIsophote(UnsignedCharImageType::IndexType queryPixel)
{
  try
  {
  if(!this->Mask->GetLargestPossibleRegion().IsInside(GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0])))
    {
    itk::CovariantVector<float, 2> v;
    v[0] = 0; v[1] = 0;
    return v;
    }

  itk::ImageRegionConstIterator<UnsignedCharImageType> iterator(this->Mask,GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0]));

  std::vector<itk::CovariantVector<float, 2> > vectors;

  while(!iterator.IsAtEnd())
    {
    if(IsValidPatch(iterator.GetIndex(), 3))
      {
      vectors.push_back(this->IsophoteImage->GetPixel(iterator.GetIndex()));
      }

    ++iterator;
    }

  itk::CovariantVector<float, 2> averageVector;
  averageVector[0] = 0;
  averageVector[1] = 0;

  if(vectors.size() == 0)
    {
    return averageVector;
    }

  for(unsigned int i = 0; i < vectors.size(); i++)
    {
    averageVector[0] += vectors[i][0];
    averageVector[1] += vectors[i][1];
    }
  averageVector[0] /= vectors.size();
  averageVector[1] /= vectors.size();

  return averageVector;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in GetAverageIsophote!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <class TImage>
unsigned int CriminisiInpainting<TImage>::GetNumberOfPixelsInPatch()
{
  return this->GetPatchSize()[0]*this->GetPatchSize()[1];
}

template <class TImage>
itk::Size<2> CriminisiInpainting<TImage>::GetPatchSize()
{
  itk::Size<2> patchSize;

  patchSize[0] = (this->PatchRadius[0]*2)+1;
  patchSize[1] = (this->PatchRadius[1]*2)+1;

  return patchSize;
}

template class CriminisiInpainting<ColorImageType>;
template class CriminisiInpainting<RGBDIImageType>;
