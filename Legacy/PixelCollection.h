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

#ifndef PixelCollection_H
#define PixelCollection_H

// ITK
#include "itkIndex.h"

// STL
#include <set>

/**
\class PixelCollection
\brief This class stores pixel locations.
*/

class PixelCollection
{
private:
  typedef itk::Index<2> Pixel;
  typedef std::set<Pixel, Pixel::LexicographicCompare> PixelContainer;
  PixelContainer Pixels;

public:
  typedef PixelContainer::iterator Iterator;
  typedef PixelContainer::const_iterator ConstIterator;
  Iterator begin();
  Iterator end();
  ConstIterator begin() const;
  ConstIterator end() const;

  void AddPixel(const Pixel& pixel);



};

#endif
