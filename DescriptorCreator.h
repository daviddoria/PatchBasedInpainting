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

#ifndef DescriptorCreator_H
#define DescriptorCreator_H

#include "ItemCreator.h"

// Custom
#include "Item.h"

// STL
#include <map>
#include <vector>

// ITK
#include "itkIndex.h"

/**
\class DescriptorCreator
\brief This class creates a descriptor for each pixel.
*/
class DescriptorCreator : public ItemCreator
{
public:

  virtual Item* CreateItem(const itk::Index<2>& index) const = 0;

private:
  std::map<itk::Index<2>, std::vector<float> > Descriptors;

};

#endif
