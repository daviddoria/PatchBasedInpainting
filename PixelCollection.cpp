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

#include "PixelCollection.h"

PixelCollection::Iterator PixelCollection::begin()
{
  return this->Pixels.begin();
}

PixelCollection::Iterator PixelCollection::end()
{
  return this->Pixels.end();
}

PixelCollection::ConstIterator PixelCollection::begin() const
{
  return this->Pixels.begin();
}

PixelCollection::ConstIterator PixelCollection::end() const
{
  return this->Pixels.end();
}

void PixelCollection::AddPixel(const Pixel& pixel)
{
  this->Pixels.insert(pixel);
}
