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

#include "Helpers.h"

// VTK
#include <vtkPolyData.h>

// ITK
#include "itkRescaleIntensityImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkImageFileWriter.h"

namespace HelpersOutput
{

template <typename TImage>
void WriteSequentialImage(const TImage* const image, const std::string& filePrefix, const unsigned int iteration)
{
  std::string fileName = Helpers::GetSequentialFileName(filePrefix, iteration, "mha");

  HelpersOutput::WriteImage<TImage>(image, fileName);
}

template <typename TImage>
void WriteImageConditional(const TImage* const image, const std::string& fileName, const bool condition)
{
  if(condition)
    {
    WriteImage<TImage>(image, fileName);
    }
}


template <class TImage>
void WriteScaledScalarImage(const TImage* const image, const std::string& filename)
{
//   if(T::PixelType::Dimension > 1)
//     {
//     std::cerr << "Cannot write scaled scalar image with vector image input!" << std::endl;
//     return;
//     }
  typedef itk::RescaleIntensityImageFilter<TImage, UnsignedCharScalarImageType> RescaleFilterType; // expected ';' before rescaleFilter

  typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetInput(image);
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(255);
  rescaleFilter->Update();

  typedef itk::ImageFileWriter<UnsignedCharScalarImageType> WriterType;
  typename WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(filename);
  writer->SetInput(rescaleFilter->GetOutput());
  writer->Update();
}


template<typename TImage>
void WriteImage(const TImage* const image, const std::string& filename)
{
  // This is a convenience function so that images can be written in 1 line instead of 4.
  typename itk::ImageFileWriter<TImage>::Pointer writer = itk::ImageFileWriter<TImage>::New();
  writer->SetFileName(filename);
  writer->SetInput(image);
  writer->Update();
}


template<typename TImage>
void WriteRGBImage(const TImage* const input, const std::string& filename)
{
  typedef itk::Image<itk::CovariantVector<unsigned char, 3>, 2> RGBImageType;

  RGBImageType::Pointer output = RGBImageType::New();
  output->SetRegions(input->GetLargestPossibleRegion());
  output->Allocate();

  itk::ImageRegionConstIterator<TImage> inputIterator(input, input->GetLargestPossibleRegion());
  itk::ImageRegionIterator<RGBImageType> outputIterator(output, output->GetLargestPossibleRegion());

  while(!inputIterator.IsAtEnd())
    {
    itk::CovariantVector<unsigned char, 3> pixel;
    for(unsigned int i = 0; i < 3; ++i)
      {
      pixel[i] = inputIterator.Get()[i];
      }
    outputIterator.Set(pixel);
    ++inputIterator;
    ++outputIterator;
    }

  typename itk::ImageFileWriter<RGBImageType>::Pointer writer = itk::ImageFileWriter<RGBImageType>::New();
  writer->SetFileName(filename);
  writer->SetInput(output);
  writer->Update();
}

// template<typename TImage>
// void WritePatch(const ImagePatch<TImage>& patch, const std::string& filename)
// {
//   WriteRegion<TImage>(patch.Image, patch.Region, filename);
// }
// 
// template<typename TImage>
// void WriteMaskedPatch(const Mask* mask, const ImagePatch<TImage>& patch, const std::string& filename, const typename TImage::PixelType& holeColor)
// {
//   WriteMaskedRegion<TImage>(patch.Image, mask, patch.Region, filename);
// }

template<typename TImage>
void WriteMaskedRegion(const TImage* const image, const Mask* mask, const itk::ImageRegion<2>& region, const std::string& filename, const typename TImage::PixelType& holeColor)
{
  typedef itk::RegionOfInterestImageFilter<TImage, TImage> RegionOfInterestImageFilterType;
  typename RegionOfInterestImageFilterType::Pointer regionOfInterestImageFilter = RegionOfInterestImageFilterType::New();
  regionOfInterestImageFilter->SetRegionOfInterest(region);
  regionOfInterestImageFilter->SetInput(image);
  regionOfInterestImageFilter->Update();

  typedef itk::RegionOfInterestImageFilter<Mask, Mask> RegionOfInterestMaskFilterType;
  typename RegionOfInterestMaskFilterType::Pointer regionOfInterestMaskFilter = RegionOfInterestMaskFilterType::New();
  regionOfInterestMaskFilter->SetRegionOfInterest(region);
  regionOfInterestMaskFilter->SetInput(mask);
  regionOfInterestMaskFilter->Update();

  itk::ImageRegionIterator<TImage> imageIterator(regionOfInterestImageFilter->GetOutput(), regionOfInterestImageFilter->GetOutput()->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    typename TImage::PixelType pixel = imageIterator.Get();

    itk::Index<2> index = imageIterator.GetIndex();

    if(regionOfInterestMaskFilter->GetOutput()->IsHole(imageIterator.GetIndex()))
      {
      regionOfInterestImageFilter->GetOutput()->SetPixel(index, holeColor);
      }

    ++imageIterator;
    }

  typename itk::ImageFileWriter<TImage>::Pointer writer = itk::ImageFileWriter<TImage>::New();
  writer->SetFileName(filename);
  writer->SetInput(regionOfInterestImageFilter->GetOutput());
  writer->Update();
}


template<typename TImage>
void WriteRegion(const TImage* const image, const itk::ImageRegion<2>& region, const std::string& filename)
{
  //std::cout << "WriteRegion() " << filename << std::endl;
  //std::cout << "region " << region << std::endl;
  typedef itk::RegionOfInterestImageFilter<TImage, TImage> RegionOfInterestImageFilterType;

  typename RegionOfInterestImageFilterType::Pointer regionOfInterestImageFilter = RegionOfInterestImageFilterType::New();
  regionOfInterestImageFilter->SetRegionOfInterest(region);
  regionOfInterestImageFilter->SetInput(image);
  regionOfInterestImageFilter->Update();

  //std::cout << "regionOfInterestImageFilter " << regionOfInterestImageFilter->GetOutput()->GetLargestPossibleRegion() << std::endl;

  typename itk::ImageFileWriter<TImage>::Pointer writer = itk::ImageFileWriter<TImage>::New();
  writer->SetFileName(filename);
  writer->SetInput(regionOfInterestImageFilter->GetOutput());
  writer->Update();
}

// Why is RegionOfInterestImageFilter used (above) instead of ExtractFilterType ?
// template <typename TImage>
// void PatchBasedInpainting<TImage>::DebugWritePatch(const itk::ImageRegion<2>& inputRegion, const std::string& filename)
// {
//   if(!this->DebugImages)
//     {
//     return;
//     }
// 
//   typedef itk::RegionOfInterestImageFilter< FloatVectorImageType,
//                                             FloatVectorImageType> ExtractFilterType;
//   itk::ImageRegion<2> region = inputRegion;
//   region.Crop(this->CurrentInpaintedImage->GetLargestPossibleRegion());
// 
//   ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
//   extractFilter->SetRegionOfInterest(region);
//   extractFilter->SetInput(this->CurrentInpaintedImage);
//   extractFilter->Update();
//   /*
//   typedef itk::Image<itk::CovariantVector<unsigned char, TImage::PixelType::Dimension>, 2> OutputImageType;
// 
//   typedef itk::CastImageFilter< TImage, OutputImageType > CastFilterType;
//   typename CastFilterType::Pointer castFilter = CastFilterType::New();
//   castFilter->SetInput(extractFilter->GetOutput());
//   castFilter->Update();
// 
//   Helpers::WriteImage<OutputImageType>(castFilter->GetOutput(), filename);
//   */
// 
// }

} // end namespace
