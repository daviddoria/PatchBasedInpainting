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

#ifndef ValidPixelIterator_h
#define ValidPixelIterator_h

// Custom
#include "ImageProcessing/Mask.h"

// ITK
#include "itkImageRegion.h"

/**
\class ValidPixelIterator
\brief Iterate over all non-NULL pixels. TImage must be a pointer type.
*/
template <typename TImage>
class ValidPixelIterator
{
private:

  typedef std::vector<itk::Index<2> > PixelContainer;
  PixelContainer ValidPixels;
  const TImage* Image;

public:
  ValidPixelIterator(const TImage* const image, const itk::ImageRegion<2>& region);

  typedef PixelContainer::const_iterator ConstIterator;
  ConstIterator begin() const;
  ConstIterator end() const;

};

#include "ValidPixelIterator.hxx"

#endif
