#include "HelpersDisplay.h"

#include "Helpers.h"

#include <vtkImageData.h>

namespace HelpersDisplay
{

// Convert a vector ITK image to a VTK image for display
void ITKVectorImageToVTKImage(const FloatVectorImageType::Pointer image, vtkImageData* outputImage, const DisplayStyle& style)
{
  switch(style.Style)
    {
    case DisplayStyle::COLOR:
      Helpers::ITKImageToVTKRGBImage(image, outputImage);
      break;
    case DisplayStyle::MAGNITUDE:
      Helpers::ITKImageToVTKMagnitudeImage(image, outputImage);
      break;
    case DisplayStyle::CHANNEL:
      Helpers::ITKImageChannelToVTKImage(image, style.Channel, outputImage);
      break;
    default:
      std::cerr << "No valid style to display!" << std::endl;
      return;
    }

  outputImage->Modified();
}

} // end namespace
