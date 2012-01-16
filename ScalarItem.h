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

#ifndef ScalarItem_H
#define ScalarItem_H

#include "Item.h"

/**
\class ScalarItem
\brief This class stores a scalar.
*/
template <typename T>
class ScalarItem : public Item
{
public:

  /** Create an item.*/
  ScalarItem(const T& scalar) : Scalar(scalar) {}

  float Compare(const Item* const item) const
  {
    //ScalarItem<T> other = static_cast<ScalarItem<T> >(item);
    const ScalarItem<T>* other = dynamic_cast<const ScalarItem<T>* >(item);
    //return fabs(this->Scalar - static_cast<ScalarItem<T> >(item).Scalar);
  }

private:
  /** The scalar value. */
  T Scalar;

};

#include "ImagePatchItem.hxx"

#endif
