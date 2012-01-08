#include "NamedVTKImage.h"

#include "Helpers/Helpers.h"

NamedVTKImage FindImageByName(const std::vector<NamedVTKImage>& namedImages, const std::string& imageName)
{
  for(unsigned int i = 0; i < namedImages.size(); ++i)
    {
    if(Helpers::StringsMatch(namedImages[i].Name, imageName))
      {
      return namedImages[i];
      }
    }
  std::stringstream ss;
  ss << "No image named " << imageName << " found!";
  throw std::runtime_error(ss.str());
}
