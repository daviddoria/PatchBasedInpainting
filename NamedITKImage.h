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

#ifndef NamedITKImage_H
#define NamedITKImage_H

// ITK
#include "itkImageBase.h"

// STL
#include <vector>

// We would rather use this (inherited from ImageBase, but I don't think you can create an object:
// NamedITKImage::Pointer maskImage = dynamic_cast<NamedITKImage*>(Mask::New().GetPointer());
// doesn't make sense because NamedITKImage is the derived class in this case!
// struct NamedITKImage : public itk::ImageBase<2>
// {
//   NamedITKImage() : Vectors(false), Name("Unnamed") {}
//
//   bool Vectors; // Should the image be displayed as vectors (little lines) vs scalars (pixels).
//   std::string Name;
// };

struct NamedITKImage
{
  NamedITKImage(itk::ImageBase<2>* const image = NULL, const bool vectors = false, const std::string& name = "Unnamed") : Image(image), Vectors(vectors), Name(name) {}

  //itk::ImageBase<2>* Image;
  itk::ImageBase<2>::Pointer Image;
  bool Vectors; // Should the image be displayed as vectors (little lines) vs scalars (pixels).
  std::string Name;
};

#endif
