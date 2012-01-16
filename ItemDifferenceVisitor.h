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

#include "ItemDifferenceMap.h"
#include "Mask.h"
#include "PatchPairVisitor.h"

/**
\class ItemDifferenceVisitor
\brief This class computes the difference between two Items.
*/
class ItemDifferenceVisitor
{
public:
  ItemDifferenceVisitor(){}

  ItemDifferenceVisitor(Item* const itemToCompare, ItemDifferenceMapType* const differenceMap) : ItemToCompare(itemToCompare), DifferenceMap(differenceMap)
  {
  }

  void SetItemToCompare(Item* const itemToCompare)
  {
    this->ItemToCompare = ItemToCompare;
  }

  void SetDifferenceMap(ItemDifferenceMapType* const differenceMap)
  {
    this->DifferenceMap = differenceMap;
  }
  
  void Visit(const Item& item)
  {
    float difference = item.Compare(ItemToCompare);
    PatchPairDifferences& differences = (*this->DifferenceMap)[&item];
    differences.SetDifferenceByType(PatchPairDifferences::AveragePixelDifference, difference);
  }

private:

  Item* ItemToCompare;
  ItemDifferenceMapType* DifferenceMap;

};

#endif
