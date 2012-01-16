#include "NamedVTKImage.h"

#include "Helpers/Helpers.h"

NamedVTKImage::NamedVTKImage() : ImageData(NULL), Name("Unnamed"), DisplayType(SCALARS)
{

}

NamedVTKImage::NamedVTKImage(vtkImageData* const imageData, const std::string& imageName, const ImageDisplayTypeEnum displayType) :
ImageData(imageData), Name(imageName), DisplayType(displayType)
{

}

NamedVTKImage NamedVTKImage::FindImageByName(const std::vector<NamedVTKImage>& namedImages, const std::string& imageName)
{
  for(unsigned int i = 0; i < namedImages.size(); ++i)
    {
    if(namedImages[i].Name == imageName)
      {
      return namedImages[i];
      }
    }
  std::stringstream ss;
  ss << "No image named " << imageName << " found!";
  throw std::runtime_error(ss.str());
}
