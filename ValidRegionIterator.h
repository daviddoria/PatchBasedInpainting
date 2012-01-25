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

#ifndef ValidRegionIterator_h
#define ValidRegionIterator_h

// Custom
#include "ImageProcessing/Mask.h"

// ITK
#include "itkImageRegion.h"

/**
\class ValidRegionIterator
\brief Iterate over all fully valid regions of a specified 'regionSize' in a specified 'region'
*/
class ValidRegionIterator
{
private:
  const Mask* MaskImage;
  unsigned int PatchRadius;
  typedef std::vector<itk::ImageRegion<2> > RegionContainer;
  RegionContainer ValidRegions;

public:
  ValidRegionIterator(const Mask* const mask, const itk::ImageRegion<2>& region, const unsigned int patchRadius);

  typedef RegionContainer::const_iterator ConstIterator;
  ConstIterator begin() const;
  ConstIterator end() const;

};

#endif
