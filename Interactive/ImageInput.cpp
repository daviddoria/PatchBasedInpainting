#include "ImageInput.h"

ImageInput::ImageInput(const QString& name, const QString& fileName, const Qt::CheckState display,
            const Qt::CheckState save, Layer* const layer):
Name(name), FileName(fileName), Display(display), Save(save), ImageLayer(layer)
{
  std::cout << "ImageInput - Name: " << Name.toStdString() << " FileName: " << FileName.toStdString() << std::endl;
}
