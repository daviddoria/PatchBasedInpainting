/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
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

#ifndef FinalImageWriterVisitor_HPP
#define FinalImageWriterVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Custom
#include "Visitors/InpaintingVisitorParent.h"
#include "Mask/Mask.h"
#include "ITKHelpers/ITKHelpers.h"
#include "BoostHelpers/BoostHelpers.h"

// ITK
#include "itkImage.h"
#include "itkImageRegion.h"

/**
  * This visitor writes out information and images at each iteration.
 */
template <typename TGraph, typename TImage>
struct FinalImageWriterVisitor : public InpaintingVisitorParent<TGraph>
{
  const TImage* Image;

  const std::string FileName;

  FinalImageWriterVisitor(TImage* const image, const std::string& fileName,
                          const std::string& visitorName = "DebugVisitor") :
  InpaintingVisitorParent<TGraph>(visitorName),
  Image(image), FileName(fileName)
  {

  }

  void InpaintingComplete() const override
  {
    // If the output filename is a png file, then use the RGBImage writer so that it is first
    // casted to unsigned char. Otherwise, write the file directly.
    if(Helpers::GetFileExtension(this->FileName) == "png")
    {
      ITKHelpers::WriteRGBImage(this->Image, this->FileName);
    }
    else
    {
      ITKHelpers::WriteImage(this->Image, this->FileName);
    }

  }

};

#endif
