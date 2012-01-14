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

#ifndef ItemDifferenceVisitor_H
#define ItemDifferenceVisitor_H

#include "PatchPairVisitor.h"

#include "Mask.h"

/**
\class ItemDifferenceVisitor
\brief This class computes the difference between two Items.
*/
class ItemDifferenceVisitor
{
public:
  ItemDifferenceVisitor(const Item* const itemToCompare) : ItemToCompare(itemToCompare)
  {
  }

  void Visit(const Item* item)
  {
    item->Difference(ItemToCompare);
  }

private:

  const Item* ItemToCompare;
};

#endif
