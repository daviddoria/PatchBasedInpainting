#include "ImageInput.h"

ImageInput::ImageInput(const QString& name, const QString& fileName) : Name(name), FileName(fileName)
{
  // std::cout << "ImageInput - Name: " << Name.toStdString() << " FileName: " << FileName.toStdString() << std::endl;
}

bool ImageExists(const QVector<ImageInput>& imageInputs, const QString& name)
{
  for(int imageId = 0; imageId < imageInputs.size(); ++imageId)
    {
    if(QString::compare(imageInputs[imageId].Name, name) == 0) // Strings are identical
      {
      return true;
      }
    }
  return false;
}
