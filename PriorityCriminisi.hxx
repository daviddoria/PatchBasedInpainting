
#include "PriorityCriminisi.h"

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/HelpersOutput.h"
#include "Isophotes.h"
#include "Helpers/ITKHelpers.h"
#include "Helpers/ITKVTKHelpers.h"

// VXL
#include <vnl/vnl_double_2.h>

// ITK
#include "itkDiscreteGaussianImageFilter.h"
#include "itkGradientImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkInvertIntensityImageFilter.h"

// VTK
#include <vtkSmartPointer.h>

template <typename TImage>
PriorityCriminisi<TImage>::PriorityCriminisi(const TImage* const image, const Mask* const maskImage, unsigned int patchRadius) :
                                     PriorityOnionPeel<TImage>(image, maskImage, patchRadius)
{
  this->BoundaryNormalsImage = FloatVector2ImageType::New();
  ITKHelpers::InitializeImage<FloatVector2ImageType>(this->BoundaryNormalsImage, image->GetLargestPossibleRegion());

  unsigned int blurVariance = 2;
  ComputeBoundaryNormals(blurVariance);

  this->IsophoteImage = FloatVector2ImageType::New();
  ITKHelpers::InitializeImage<FloatVector2ImageType>(this->IsophoteImage, image->GetLargestPossibleRegion());
  Isophotes::ComputeColorIsophotesInRegion(image, maskImage, image->GetLargestPossibleRegion(), this->IsophoteImage);

}

template <typename TImage>
std::vector<std::string> PriorityCriminisi<TImage>::GetImageNames()
{
  std::vector<std::string> imageNames = PriorityOnionPeel<TImage>::GetImageNames();
  imageNames.push_back("Isophotes");
  imageNames.push_back("BoundaryNormals");
  return imageNames;
}

template <typename TImage>
std::vector<NamedVTKImage> PriorityCriminisi<TImage>::GetNamedImages()
{
  std::vector<NamedVTKImage> namedImages = PriorityOnionPeel<TImage>::GetNamedImages();

  NamedVTKImage isophoteNamedImage;
  isophoteNamedImage.Name = "Isophotes";
  vtkSmartPointer<vtkImageData> isophoteImageVTK = vtkSmartPointer<vtkImageData>::New();
  ITKVTKHelpers::ITKImageToVTKVectorFieldImage(this->IsophoteImage, isophoteImageVTK);
  isophoteNamedImage.ImageData = isophoteImageVTK;
  isophoteNamedImage.Vectors = true;
  namedImages.push_back(isophoteNamedImage);

  NamedVTKImage boundaryNormalsNamedImage;
  boundaryNormalsNamedImage.Name = "BoundaryNormals";
  vtkSmartPointer<vtkImageData> boundaryNormalsImageVTK = vtkSmartPointer<vtkImageData>::New();
  ITKVTKHelpers::ITKImageToVTKVectorFieldImage(this->BoundaryNormalsImage, boundaryNormalsImageVTK);
  boundaryNormalsNamedImage.ImageData = isophoteImageVTK;
  boundaryNormalsNamedImage.Vectors = true;
  namedImages.push_back(boundaryNormalsNamedImage);

  return namedImages;
}

template <typename TImage>
void PriorityCriminisi<TImage>::Update(const itk::ImageRegion<2>& filledRegion)
{
  PriorityOnionPeel<TImage>::Update(filledRegion);

  Isophotes::ComputeColorIsophotesInRegion(this->Image, this->MaskImage, filledRegion, this->IsophoteImage);

  unsigned int blurVariance = 2;
  ComputeBoundaryNormals(blurVariance);
}

template <typename TImage>
float PriorityCriminisi<TImage>::ComputePriority(const itk::Index<2>& queryPixel)
{
  //std::cout << "PriorityCriminisi::ComputePriority()" << std::endl;
  float confidenceTerm = ComputeConfidenceTerm(queryPixel);
  float dataTerm = ComputeDataTerm(queryPixel);

  float priority = confidenceTerm * dataTerm;

  return priority;
}

/*
template <typename TImage>
float PriorityCriminisi<TImage>::ComputeDataTerm(const itk::Index<2>& queryPixel)
{
  // The difference between this funciton and Criminisi's original data term computation (ComputeDataTermCriminisi)
  // is that we claim there is no reason to penalize the priority of linear structures that don't have a perpendicular incident
  // angle with the boundary. Of course, we don't want to continue structures that are almost parallel with the boundary, but above
  // a threshold, the strength of the isophote should be more important than the angle of incidence.
  try
  {
    FloatVector2Type isophote = this->IsophoteImage->GetPixel(queryPixel);
    FloatVector2Type boundaryNormal = this->BoundaryNormalsImage->GetPixel(queryPixel);

    DebugMessage<FloatVector2Type>("Isophote: ", isophote);
    DebugMessage<FloatVector2Type>("Boundary normal: ", boundaryNormal);
    // D(p) = |dot(isophote at p, normalized normal of the front at p)|/alpha

    vnl_double_2 vnlIsophote(isophote[0], isophote[1]);

    vnl_double_2 vnlNormal(boundaryNormal[0], boundaryNormal[1]);

    float dataTerm = 0.0f;

    float angleBetween = Helpers::AngleBetween(isophote, boundaryNormal);
    if(angleBetween < 20)
      {
      float projectionMagnitude = isophote.GetNorm() * cos(angleBetween);

      dataTerm = projectionMagnitude;
      }
    else
    {
      dataTerm = isophote.GetNorm();
    }

    return dataTerm;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeDataTerm!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}
*/

template <typename TImage>
float PriorityCriminisi<TImage>::ComputeDataTerm(const itk::Index<2>& queryPixel)
{
  try
  {
    FloatVector2Type isophote = this->IsophoteImage->GetPixel(queryPixel);
    FloatVector2Type boundaryNormal = this->BoundaryNormalsImage->GetPixel(queryPixel);

    //DebugMessage<FloatVector2Type>("Isophote: ", isophote);
    //DebugMessage<FloatVector2Type>("Boundary normal: ", boundaryNormal);
    // D(p) = |dot(isophote at p, normalized normal of the front at p)|/alpha

    vnl_double_2 vnlIsophote(isophote[0], isophote[1]);

    vnl_double_2 vnlNormal(boundaryNormal[0], boundaryNormal[1]);

    float dot = std::abs(dot_product(vnlIsophote,vnlNormal));

    float alpha = 255; // This doesn't actually contribue anything, since the argmax of the priority is all that is used, and alpha ends up just being a scaling factor since the proiority is purely multiplicative.
    float dataTerm = dot/alpha;

    return dataTerm;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeDataTerm!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <typename TImage>
void PriorityCriminisi<TImage>::ComputeBoundaryNormals(const float blurVariance)
{
  try
  {
    // Blur the mask, compute the gradient, then keep the normals only at the original mask boundary

    //HelpersOutput::WriteImageConditional<UnsignedCharScalarImageType>(this->BoundaryImage, "Debug/ComputeBoundaryNormals.BoundaryImage.mha", this->DebugImages);
    //HelpersOutput::WriteImageConditional<Mask>(this->MaskImage, "Debug/ComputeBoundaryNormals.CurrentMask.mha", this->DebugImages);

    // Blur the mask
    typedef itk::DiscreteGaussianImageFilter< Mask, FloatScalarImageType >  BlurFilterType;
    BlurFilterType::Pointer gaussianFilter = BlurFilterType::New();
    gaussianFilter->SetInput(this->MaskImage);
    gaussianFilter->SetVariance(blurVariance);
    gaussianFilter->Update();

    //HelpersOutput::WriteImageConditional<FloatScalarImageType>(gaussianFilter->GetOutput(), "Debug/ComputeBoundaryNormals.BlurredMask.mha", this->DebugImages);

    // Compute the gradient of the blurred mask
    typedef itk::GradientImageFilter< FloatScalarImageType, float, float>  GradientFilterType;
    GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
    gradientFilter->SetInput(gaussianFilter->GetOutput());
    gradientFilter->Update();

    //HelpersOutput::WriteImageConditional<FloatVector2ImageType>(gradientFilter->GetOutput(), "Debug/ComputeBoundaryNormals.BlurredMaskGradient.mha", this->DebugImages);

    // Only keep the normals at the boundary
    typedef itk::MaskImageFilter< FloatVector2ImageType, UnsignedCharScalarImageType, FloatVector2ImageType > MaskFilterType;
    MaskFilterType::Pointer maskFilter = MaskFilterType::New();
    maskFilter->SetInput(gradientFilter->GetOutput());
    maskFilter->SetMaskImage(this->BoundaryImage);
    maskFilter->Update();

    //HelpersOutput::WriteImageConditional<FloatVector2ImageType>(maskFilter->GetOutput(),
      //                                                          "Debug/ComputeBoundaryNormals.BoundaryNormalsUnnormalized.mha", this->DebugImages);

    ITKHelpers::DeepCopy<FloatVector2ImageType>(maskFilter->GetOutput(), this->BoundaryNormalsImage);

    // Normalize the vectors because we just care about their direction (the Data term computation calls for the normalized boundary normal)
    itk::ImageRegionIterator<FloatVector2ImageType> boundaryNormalsIterator(this->BoundaryNormalsImage, this->BoundaryNormalsImage->GetLargestPossibleRegion());
    itk::ImageRegionConstIterator<UnsignedCharScalarImageType> boundaryIterator(this->BoundaryImage, this->BoundaryImage->GetLargestPossibleRegion());

    while(!boundaryNormalsIterator.IsAtEnd())
      {
      if(boundaryIterator.Get()) // The pixel is on the boundary
        {
        FloatVector2ImageType::PixelType p = boundaryNormalsIterator.Get();
        p.Normalize();
        boundaryNormalsIterator.Set(p);
        }
      ++boundaryNormalsIterator;
      ++boundaryIterator;
      }

    //HelpersOutput::WriteImageConditional<FloatVector2ImageType>(this->BoundaryNormalsImage, "Debug/ComputeBoundaryNormals.BoundaryNormals.mha", this->DebugImages);
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeBoundaryNormals!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <typename TImage>
FloatScalarImageType* PriorityCriminisi<TImage>::GetDataImage()
{
  // TODO: Actually create the data image. This is not used for the algorithm, but just for debugging output.
  FloatScalarImageType::Pointer dataImage = FloatScalarImageType::New();
  return dataImage;
}
