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

#include "Histograms.h"


std::vector<HistogramType::Pointer> ComputeHistogramsOfRegion(const FloatVectorImageType::Pointer image, const itk::ImageRegion<2>& region)
{
  
  std::vector<HistogramType::Pointer> histograms;
  
  typedef itk::RegionOfInterestImageFilter< FloatVectorImageType, FloatVectorImageType > RegionOfInterestImageFilterType;
  RegionOfInterestImageFilterType::Pointer regionOfInterestImageFilter = RegionOfInterestImageFilterType::New();
  regionOfInterestImageFilter->SetRegionOfInterest(region);
  regionOfInterestImageFilter->SetInput(image);
  regionOfInterestImageFilter->Update();

  // Compute the histogram of each channel
  for(unsigned int i = 0; i < image->GetNumberOfComponentsPerPixel(); ++i)
    {
    typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, FloatScalarImageType> IndexSelectionType;
    IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
    indexSelectionFilter->SetIndex(i);
    indexSelectionFilter->SetInput(image);
    indexSelectionFilter->GetOutput()->SetRequestedRegion(region);
    indexSelectionFilter->Update();
  
    const unsigned int MeasurementVectorSize = 1;
    const unsigned int binsPerDimension = 30;
    
    HistogramType::MeasurementVectorType lowerBound(MeasurementVectorSize);
    lowerBound.Fill(0);
    
    HistogramType::MeasurementVectorType upperBound(MeasurementVectorSize);
    upperBound.Fill(255) ;
    
    HistogramType::SizeType size(MeasurementVectorSize);
    size.Fill(binsPerDimension);
    
    ImageToHistogramFilterType::Pointer imageToHistogramFilter = ImageToHistogramFilterType::New();
    imageToHistogramFilter->SetInput(indexSelectionFilter->GetOutput());
    imageToHistogramFilter->SetHistogramBinMinimum(lowerBound);
    imageToHistogramFilter->SetHistogramBinMaximum(upperBound);
    imageToHistogramFilter->SetHistogramSize(size);
    imageToHistogramFilter->SetAutoMinimumMaximum(false);
    imageToHistogramFilter->Update();

    histograms.push_back(imageToHistogramFilter->GetOutput());
    }
    
  return histograms;
}


std::vector<HistogramType::Pointer> ComputeHistogramsOfMaskedRegion(const FloatVectorImageType::Pointer image, const Mask::Pointer mask, const itk::ImageRegion<2>& region)
{
  
  std::vector<HistogramType::Pointer> histograms;

  // Compute the histogram of each channel
  for(unsigned int i = 0; i < image->GetNumberOfComponentsPerPixel(); ++i)
    {
      
    typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, FloatScalarImageType> IndexSelectionType;
    IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
    indexSelectionFilter->SetIndex(i);
    indexSelectionFilter->SetInput(image);
    indexSelectionFilter->GetOutput()->SetRequestedRegion(region);
    indexSelectionFilter->Update();
  
    const unsigned int MeasurementVectorSize = 1;
    const unsigned int binsPerDimension = 30;
    
    HistogramType::MeasurementVectorType lowerBound(MeasurementVectorSize);
    lowerBound.Fill(0);
    
    HistogramType::MeasurementVectorType upperBound(MeasurementVectorSize);
    upperBound.Fill(255) ;
    
    HistogramType::SizeType size(MeasurementVectorSize);
    size.Fill(binsPerDimension);
    
    
    itk::ImageRegionConstIterator<FloatScalarImageType> imageIterator(indexSelectionFilter->GetOutput(), region);
    
    HistogramType::Pointer histogram = HistogramType::New();

    histogram->SetMeasurementVectorSize(MeasurementVectorSize);
 
    histogram->Initialize(size, lowerBound, upperBound );
  
    while(!imageIterator.IsAtEnd())
      {
      if(mask->IsHole(imageIterator.GetIndex()))
	{
	++imageIterator;
	continue;
	}
      float pixel = imageIterator.Get();
      HistogramType::MeasurementVectorType mv(MeasurementVectorSize);
      mv[0] = pixel;

      histogram->IncreaseFrequencyOfMeasurement(mv, 1);
      
      ++imageIterator;
      }
      
    histograms.push_back(histogram);
    }
    
  return histograms;
}

std::vector<HistogramType::Pointer> ComputeHistogramsOfRegionManual(const FloatVectorImageType::Pointer image, const itk::ImageRegion<2>& region)
{
  
  std::vector<HistogramType::Pointer> histograms;

  // Compute the histogram of each channel
  for(unsigned int i = 0; i < image->GetNumberOfComponentsPerPixel(); ++i)
    {
      
    typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, FloatScalarImageType> IndexSelectionType;
    IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
    indexSelectionFilter->SetIndex(i);
    indexSelectionFilter->SetInput(image);
    indexSelectionFilter->GetOutput()->SetRequestedRegion(region);
    indexSelectionFilter->Update();
  
    const unsigned int MeasurementVectorSize = 1;
    const unsigned int binsPerDimension = 30;
    
    HistogramType::MeasurementVectorType lowerBound(MeasurementVectorSize);
    lowerBound.Fill(0);
    
    HistogramType::MeasurementVectorType upperBound(MeasurementVectorSize);
    upperBound.Fill(255) ;
    
    HistogramType::SizeType size(MeasurementVectorSize);
    size.Fill(binsPerDimension);
    
    
    itk::ImageRegionConstIterator<FloatScalarImageType> imageIterator(indexSelectionFilter->GetOutput(), region);
    
    HistogramType::Pointer histogram = HistogramType::New();

    histogram->SetMeasurementVectorSize(MeasurementVectorSize);
 
    histogram->Initialize(size, lowerBound, upperBound );
  
    while(!imageIterator.IsAtEnd())
      {

      float pixel = imageIterator.Get();
      HistogramType::MeasurementVectorType mv(MeasurementVectorSize);
      mv[0] = pixel;

      histogram->IncreaseFrequencyOfMeasurement(mv, 1);
      
      ++imageIterator;
      }
      
    histograms.push_back(histogram);
    }
    
  return histograms;
}


HistogramType::Pointer ComputeNDHistogramOfRegionManual(const FloatVectorImageType::Pointer image, const itk::ImageRegion<2>& region, const unsigned int binsPerDimension)
{
  const unsigned int MeasurementVectorSize = image->GetNumberOfComponentsPerPixel();
  
  HistogramType::MeasurementVectorType lowerBound(MeasurementVectorSize);
  lowerBound.Fill(0);

  HistogramType::MeasurementVectorType upperBound(MeasurementVectorSize);
  upperBound.Fill(255) ;

  HistogramType::SizeType size(MeasurementVectorSize);
  size.Fill(binsPerDimension);

  itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(image, region);

  HistogramType::Pointer histogram = HistogramType::New();

  histogram->SetMeasurementVectorSize(MeasurementVectorSize);

  histogram->Initialize(size, lowerBound, upperBound );

  while(!imageIterator.IsAtEnd())
    {
    FloatVectorImageType::PixelType pixel = imageIterator.Get();
    HistogramType::MeasurementVectorType mv(MeasurementVectorSize);
    for(unsigned int i = 0; i < MeasurementVectorSize; ++i)
      {
      mv[i] = pixel[i];
      }

    histogram->IncreaseFrequencyOfMeasurement(mv, 1);

    ++imageIterator;
    }

  return histogram;
}


HistogramType::Pointer ComputeNDHistogramOfMaskedRegionManual(const FloatVectorImageType::Pointer image, const Mask::Pointer mask, const itk::ImageRegion<2>& region, const unsigned int binsPerDimension)
{
  const unsigned int MeasurementVectorSize = image->GetNumberOfComponentsPerPixel();

  HistogramType::MeasurementVectorType lowerBound(MeasurementVectorSize);
  lowerBound.Fill(0);

  HistogramType::MeasurementVectorType upperBound(MeasurementVectorSize);
  upperBound.Fill(255) ;

  HistogramType::SizeType size(MeasurementVectorSize);
  size.Fill(binsPerDimension);

  itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(image, region);

  HistogramType::Pointer histogram = HistogramType::New();

  histogram->SetMeasurementVectorSize(MeasurementVectorSize);

  histogram->Initialize(size, lowerBound, upperBound );

  while(!imageIterator.IsAtEnd())
    {
    if(mask->IsHole(imageIterator.GetIndex()))
      {
      ++imageIterator;
      continue;
      }
    FloatVectorImageType::PixelType pixel = imageIterator.Get();
    HistogramType::MeasurementVectorType mv(MeasurementVectorSize);
    for(unsigned int i = 0; i < MeasurementVectorSize; ++i)
      {
      mv[i] = pixel[i];
      }

    histogram->IncreaseFrequencyOfMeasurement(mv, 1);

    ++imageIterator;
    }

  return histogram;
}

void OutputHistogram(const HistogramType::Pointer histogram)
{
  for(unsigned int i = 0; i < histogram->GetSize(0); ++i)
    {
    std::cout << histogram->GetFrequency(i) << " ";
    }
  std::cout << std::endl;
}

float NDHistogramDifference(const HistogramType::Pointer histogram1, const HistogramType::Pointer histogram2)
{
  unsigned int totalBins = 1;
  for(unsigned int i = 0; i < histogram1->GetSize().GetNumberOfElements(); ++i)
    {
    totalBins *= histogram1->GetSize(i);
    if(histogram1->GetSize(i) != histogram2->GetSize(i))
      {
      std::cerr << "Histograms must be the same size!" << std::endl;
      return 0;
      }
    }

  float totalDifference = 0;
  
  for(unsigned int i = 0; i < totalBins; ++i)
    {
    // The casts to float are necessary other wise the integer division always ends up = 0 !
    float normalized1 = static_cast<float>(histogram1->GetFrequency(i))/static_cast<float>(histogram1->GetTotalFrequency());
    float normalized2 = static_cast<float>(histogram2->GetFrequency(i))/static_cast<float>(histogram2->GetTotalFrequency());
    float difference = fabs(normalized1 - normalized2);
    //std::cout << "difference: " << difference << std::endl;
    totalDifference += difference;
    }

  //std::cout << "totalDifference: " << totalDifference << std::endl;
  return totalDifference;
}

float HistogramDifference(const HistogramType::Pointer histogram1, const HistogramType::Pointer histogram2)
{
  if(histogram1->GetSize(0) != histogram2->GetSize(0))
    {
    std::cerr << "Histograms must be the same size!" << std::endl;
    return 0;
    }

  float totalDifference = 0;
  for(unsigned int i = 0; i < histogram1->GetSize(0); ++i)
    {
//     std::cout << "h1 " << i << " " << histogram1->GetFrequency(i) << std::endl;
//     std::cout << "h1 total " << histogram1->GetTotalFrequency() << std::endl;
// 
//     std::cout << "h2 " << i << " " << histogram2->GetFrequency(i) << std::endl;
//     std::cout << "h2 total " << histogram2->GetTotalFrequency() << std::endl;

    // The casts to float are necessary other wise the integer division always ends up = 0 !
    float normalized1 = static_cast<float>(histogram1->GetFrequency(i))/static_cast<float>(histogram1->GetTotalFrequency());
    float normalized2 = static_cast<float>(histogram2->GetFrequency(i))/static_cast<float>(histogram2->GetTotalFrequency());
    //float difference = fabs(normalized1 - normalized2);
    float difference = (normalized1 - normalized2)*(normalized1 - normalized2);
    //std::cout << "difference: " << difference << std::endl;
    totalDifference += difference;
    }

  //std::cout << "totalDifference: " << totalDifference << std::endl;
  return totalDifference;
}
