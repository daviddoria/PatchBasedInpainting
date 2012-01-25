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

#include "itkCastImageFilter.h"

// STL
#include <iomanip> // for setfill()

// VTK
#include <vtkImageData.h>

// Custom
#include "ImageProcessing/Mask.h"

namespace Helpers
{

template<typename T>
typename std::enable_if<std::is_fundamental<T>::value, T&>::type index(T& t, size_t)
{
  return t;
}

template<typename T>
typename T::value_type& index(T& v, size_t i)
{
  return v[i];
}

template <class T>
unsigned int argmin(const T& vec)
{
  typename T::value_type minValue = std::numeric_limits<typename T::value_type>::max();
  unsigned int minLocation = 0;
  for(unsigned int i = 0; i < vec.size(); ++i)
    {
    if(vec[i] < minValue)
      {
      minValue = vec[i];
      minLocation = i;
      }
    }

  return minLocation;
}


template<typename T>
void NormalizeVector(std::vector<T>& v)
{
  T total = static_cast<T>(0);
  for(unsigned int i = 0; i < v.size(); ++i)
    {
    total += v[i];
    }

  for(unsigned int i = 0; i < v.size(); ++i)
    {
    v[i] /= total;
    }
}


template<typename T>
typename T::value_type VectorMedian(T v)
{
  // Don't want to pass by reference because the elements are shuffled around.

  int n = v.size() / 2;
  std::nth_element(v.begin(), v.begin()+n, v.end());
  return v[n];
}

}// end namespace
