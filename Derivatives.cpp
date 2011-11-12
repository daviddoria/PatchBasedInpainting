#include "Derivatives.h"

#include "RotateVectors.h"

namespace Derivatives
{

void ComputeMaskedIsophotesInRegion(FloatScalarImageType::Pointer image, Mask::Pointer mask, const itk::ImageRegion<2>& region, FloatVector2ImageType::Pointer outputIsophotes)
{
  try
  {
    //Helpers::WriteImageConditional<FloatScalarImageType>(image, "Debug/ComputeMaskedIsophotes.luminance.mha", this->DebugImages);

    FloatVector2ImageType::Pointer gradient = FloatVector2ImageType::New();
    Helpers::InitializeImage<FloatVector2ImageType>(gradient, image->GetLargestPossibleRegion());
    MaskedGradientInRegion<FloatScalarImageType>(image, mask, region, gradient);

    //Helpers::DebugWriteImageConditional<FloatVector2ImageType>(gradient, "Debug/ComputeMaskedIsophotes.gradient.mha", this->DebugImages);
    //Helpers::Write2DVectorImage(gradient, "Debug/ComputeMaskedIsophotes.gradient.mha");
    
    // Rotate the gradient 90 degrees to obtain isophotes from gradient
    typedef itk::UnaryFunctorImageFilter<FloatVector2ImageType, FloatVector2ImageType,
    RotateVectors<FloatVector2ImageType::PixelType,
                  FloatVector2ImageType::PixelType> > FilterType;

    FilterType::Pointer rotateFilter = FilterType::New();
    rotateFilter->SetInput(gradient);
    rotateFilter->Update();

    //Helpers::DebugWriteImageConditional<FloatVector2ImageType>(rotateFilter->GetOutput(), "Debug/ComputeMaskedIsophotes.Isophotes.mha", this->DebugImages);
    //Helpers::Write2DVectorImage(rotateFilter->GetOutput(), "Debug/ComputeMaskedIsophotes.Isophotes.mha");

    Helpers::CopyPatch<FloatVector2ImageType>(rotateFilter->GetOutput(), outputIsophotes, region, region);
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeMaskedIsophotes!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

} // end namespace
