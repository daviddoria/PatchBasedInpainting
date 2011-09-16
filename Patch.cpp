#include "Patch.h"

#include "itkRegionOfInterestImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

Patch::Patch(const FloatVectorImageType::Pointer image, const itk::ImageRegion<2>& region)
{
  this->Region = region;
  
  /*
  typedef itk::RegionOfInterestImageFilter< FloatVectorImageType, FloatVectorImageType > RegionOfInterestImageFilterType;
  RegionOfInterestImageFilterType::Pointer regionOfInterestImageFilter = RegionOfInterestImageFilterType::New();
  regionOfInterestImageFilter->SetRegionOfInterest(region);
  regionOfInterestImageFilter->SetInput(image);
  regionOfInterestImageFilter->Update();
  */
  /*
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
    
    typedef itk::Statistics::ImageToHistogramFilter< FloatScalarImageType > ImageToHistogramFilterType;
    
    ImageToHistogramFilterType::HistogramType::MeasurementVectorType lowerBound(MeasurementVectorSize);
    lowerBound.Fill(0);
    
    ImageToHistogramFilterType::HistogramType::MeasurementVectorType upperBound(MeasurementVectorSize);
    upperBound.Fill(255) ;
    
    ImageToHistogramFilterType::HistogramType::SizeType size(MeasurementVectorSize);
    size.Fill(binsPerDimension);
    
    ImageToHistogramFilterType::Pointer imageToHistogramFilter = ImageToHistogramFilterType::New();
    imageToHistogramFilter->SetInput(indexSelectionFilter->GetOutput());
    imageToHistogramFilter->SetHistogramBinMinimum(lowerBound);
    imageToHistogramFilter->SetHistogramBinMaximum(upperBound);
    imageToHistogramFilter->SetHistogramSize(size);
    imageToHistogramFilter->SetAutoMinimumMaximum(false);
    imageToHistogramFilter->Update();

    this->Histograms.push_back(imageToHistogramFilter->GetOutput());
    }
  */
}
