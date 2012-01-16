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

#ifndef NamedVTKImage_H
#define NamedVTKImage_H

// VTK
#include <vtkImageData.h>
#include <vtkSmartPointer.h>

// STL
#include <string>
#include <vector>

struct NamedVTKImage
{
  enum ImageDisplayTypeEnum {SCALARS, VECTORS};

  NamedVTKImage();
  NamedVTKImage(vtkImageData* const imageData, const std::string& imageName, const ImageDisplayTypeEnum displayType);

  vtkSmartPointer<vtkImageData> ImageData;
  std::string Name;
  ImageDisplayTypeEnum DisplayType; // Should the image be displayed as vectors (little lines) vs scalars (pixels).

  static NamedVTKImage FindImageByName(const std::vector<NamedVTKImage>&, const std::string&);
};

#endif
