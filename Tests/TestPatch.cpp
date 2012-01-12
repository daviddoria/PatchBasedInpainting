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

#include "ImagePatch.h"

#include <stdexcept>

static void TestConstructor();

// TODO: Break this into smaller tests.

int main(int argc, char*argv[])
{
  TestConstructor();

  // Create a patch
  itk::Index<2> corner0;
  corner0.Fill(0);

  itk::Size<2> patchSize;
  patchSize.Fill(10);

  itk::ImageRegion<2> region0(corner0, patchSize);
  UnsignedCharScalarImageType::Pointer image = UnsignedCharScalarImageType::New();
  ImagePatch<UnsignedCharScalarImageType> patch0(image.GetPointer(), region0);

  if(patch0.GetRegion() != region0)
    {
    throw std::runtime_error("Region was not set or retrieved correctly!");
    }

  if(patch0.GetCorner() != corner0)
    {
    throw std::runtime_error("Corner was not retrieved correctly!");
    }

  if(patch0 == patch0)
    {
    // Good, this is what we want.
    }
  else
    {
    throw std::runtime_error("patch0 should == patch0 but does not!");
    }

  if(patch0 != patch0)
    {
    throw std::runtime_error("patch0 != patch0 but it should!");
    }

  // Create another patch
  itk::Index<2> corner1;
  corner1.Fill(5);

  itk::ImageRegion<2> region1(corner1, patchSize);
  ImagePatch<UnsignedCharScalarImageType> patch1(image.GetPointer(), region1);

  if(patch0 == patch1)
    {
    throw std::runtime_error("patch0 == patch1 but should not!");
    }

  if(patch0 != patch1)
    {
    // Good, this is what we want.
    }
  else
    {
    throw std::runtime_error("patch0 != patch1 failed - they should not be equal!");
    }

  if(patch0 < patch1)
    {
    // Good, this is what we want.
    }
  else
    {
    throw std::runtime_error("patch0 < patch1 failed!");
    }

  return EXIT_SUCCESS;
}

void TestConstructor()
{
  throw;
}
