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

#ifndef Utilities_HPP
#define Utilities_HPP

/** This function allows the user to step through a container using a non-unit
  * stride length. */
template <typename TForwardIt>
bool try_advance(TForwardIt& it,
                 TForwardIt const end,
                 const unsigned int n)
{
  unsigned int i = 0;
  while (i < n && it != end)
  {
    ++i;
    ++it;
  }

  return i == n;
}

#endif
