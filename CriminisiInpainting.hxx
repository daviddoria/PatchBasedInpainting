#include "Derivatives.h"

template <typename TImage>
float CriminisiInpainting::ComputeAverageGradientChange(const typename TImage::Pointer patch, FloatVector2ImageType::Pointer preFillGradientImage, FloatVector2ImageType::Pointer postFillGradientImage, 
							const Mask::Pointer mask, const Mask::Pointer noMask, const std::vector<itk::Index<2> >& boundaryPixels)
{
  MaskedGradient<TImage>(patch, mask, preFillGradientImage);
  MaskedGradient<TImage>(patch, noMask, postFillGradientImage);
  
  float totalError = 0.0f;
  for(unsigned int boundaryPixelId = 0; boundaryPixelId < boundaryPixels.size(); ++boundaryPixelId)
    {
    FloatVector2ImageType::PixelType preFillGradient = preFillGradientImage->GetPixel(boundaryPixels[boundaryPixelId]);
    FloatVector2ImageType::PixelType postFillGradient = postFillGradientImage->GetPixel(boundaryPixels[boundaryPixelId]);
    //std::cout << "Prefill gradient: " << preFillGradient << std::endl;
    //std::cout << "Postfill gradient: " << postFillGradient << std::endl;
    //totalError += (preFillGradient - postFillGradient).GetNorm();
    totalError += (preFillGradient - postFillGradient).GetSquaredNorm();
    }

  float averageError = totalError / static_cast<float>(boundaryPixels.size());
  
  return averageError;
}