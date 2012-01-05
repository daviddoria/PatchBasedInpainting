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

#ifndef TestHelpers_H
#define TestHelpers_H

#include "Mask.h"

namespace TestHelpers
{
  bool ValuesEqual(const float a, const float b);

  template<typename TImage>
  bool ImagesEqual(const TImage* const image1, const TImage* const image2);

  template<typename TImage>
  void GetBlankImage(TImage* const image1);

  template<typename TImage>
  void GetBlankImage(TImage* const image1, const unsigned int numberOfChannels);

  void GetMask(Mask* const mask);

} // end namespace

#include "TestHelpers.hxx"

#endif
