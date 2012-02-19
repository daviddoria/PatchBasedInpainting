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

#include "BoundaryNormals.h"

#include "Mask.h"

// STL
#include <iostream>
#include <stdexcept>

static void TestBoundaryNormals();

int main()
{
  TestBoundaryNormals();

  return EXIT_SUCCESS;
}

// Look from a pixel across the hole in a specified direction and return the pixel that exists on the other side of the hole.
void TestBoundaryNormals()
{
  throw std::runtime_error("TestBoundaryNormals not yet written!");
}
