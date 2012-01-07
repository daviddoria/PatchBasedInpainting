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

#include "PixelDifference.h"
#include "PixelDifferenceChannel.h"
#include "PixelDifferenceSquared.h"
#include "PixelDifferenceAllOrNothing.h"

#include "Testing.h"
#include "Types.h"

#include <iostream>
#include <stdexcept>

static void TestPixelDifference();
static void TestPixelDifferenceSquared();
static void TestPixelDifferenceAllOrNothing();
static void TestPixelDifferenceChannel();

int main(int argc, char*argv[])
{
  try
  {
    TestPixelDifference();
    TestPixelDifferenceSquared();
    TestPixelDifferenceAllOrNothing();
    TestPixelDifferenceChannel();
  }
  catch (std::runtime_error ex)
  {
    std::cerr << ex.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void TestPixelDifference()
{
  // Scalar
  if(!Testing::ValuesEqual(PixelDifference::Difference(2.5, 1.1), 1.4))
    {
    std::stringstream ss;
    ss << "Difference should be 1.4 but is: " << PixelDifference::Difference(2.5, 1.1);
    throw std::runtime_error(ss.str());
    }

  // Vector
  FloatVectorImageType::PixelType a;
  a.SetSize(3);
  a[0] = 0;
  a[1] = 1;
  a[2] = 2;

  FloatVectorImageType::PixelType b;
  b.SetSize(3);
  b[0] = 3;
  b[1] = 4;
  b[2] = 5;

  {
  // Static
  //float difference = PixelDifference<FloatVectorImageType::PixelType>::Difference(a, b);
  float difference = PixelDifference::Difference(a, b);
  float correctDifference = 9.0f;
  if(!Testing::ValuesEqual(difference, correctDifference))
    {
    std::stringstream ss;
    ss << "PixelDifferenceFullSquared: Static specified pixel length: Difference should be " << correctDifference << " but is: " << difference;
    throw std::runtime_error(ss.str());
    }
  }

  {
  // Object
  PixelDifference differenceFunction;
  float difference = differenceFunction.Difference(a, b);
  float correctDifference = 9.0f;
  if(!Testing::ValuesEqual(difference, correctDifference))
    {
    std::stringstream ss;
    ss << "PixelDifferenceFullSquared: Object specified pixel length: Difference should be " << correctDifference << " but is: " << difference;
    throw std::runtime_error(ss.str());
    }
  }
}


void TestPixelDifferenceSquared()
{
  // Scalar
  {
  float correctDifference = 1.96f;
  float difference = PixelDifferenceSquared::Difference(2.5, 1.1);
  
  if(!Testing::ValuesEqual(difference, correctDifference))
    {
    std::stringstream ss;
    ss << "Difference should be " << correctDifference << " but is: " << difference << std::endl;
    throw std::runtime_error(ss.str());
    }
  }

  // Vector
  FloatVectorImageType::PixelType a;
  a.SetSize(3);
  a[0] = 0;
  a[1] = 1;
  a[2] = 2;

  FloatVectorImageType::PixelType b;
  b.SetSize(3);
  b[0] = 3;
  b[1] = 4;
  b[2] = 5;

  {
  // Static
  //float difference = PixelDifference<FloatVectorImageType::PixelType>::Difference(a, b);
  float difference = PixelDifferenceSquared::Difference(a, b);
  float correctDifference = 81.0f;
  if(!Testing::ValuesEqual(difference, correctDifference))
    {
    std::stringstream ss;
    ss << "PixelDifferenceFullSquared: Static specified pixel length: Difference should be " << correctDifference << " but is: " << difference;
    throw std::runtime_error(ss.str());
    }
  }

  {
  // Object
  PixelDifferenceSquared differenceFunction;
  float difference = differenceFunction.Difference(a, b);
  float correctDifference = 81.0f;
  if(!Testing::ValuesEqual(difference, correctDifference))
    {
    std::stringstream ss;
    ss << "PixelDifferenceFullSquared: Object specified pixel length: Difference should be " << correctDifference << " but is: " << difference;
    throw std::runtime_error(ss.str());
    }
  }
}

void TestPixelDifferenceAllOrNothing()
{
  {
  float correctDifference = 1;
  float difference = PixelDifferenceAllOrNothing<unsigned char>::Difference(2, 7);
  if(!Testing::ValuesEqual(difference, correctDifference))
    {
    std::stringstream ss;
    ss << "Difference should be " << correctDifference << " but is: " << difference;
    throw std::runtime_error(ss.str());
    }
  }

  {
  float correctDifference = 0;
  float difference = PixelDifferenceAllOrNothing<unsigned char>::Difference(2, 2);
  if(!Testing::ValuesEqual(difference, correctDifference))
    {
    std::stringstream ss;
    ss << "Difference should be " << correctDifference << " but is: " << difference;
    throw std::runtime_error(ss.str());
    }
  }
}

void TestPixelDifferenceChannel()
{
  FloatVectorImageType::PixelType a;
  a.SetSize(2);
  a[0] = 0;
  a[1] = 1;

  FloatVectorImageType::PixelType b;
  b.SetSize(2);
  b[0] = 3;
  b[1] = 5;

  {
  // Static specified pixel length
  float difference = PixelDifferenceChannel<FloatVectorImageType::PixelType, 0>::Difference(a, b);
  float correctDifference = 3.0f;
  if(!Testing::ValuesEqual(difference, correctDifference))
    {
    std::stringstream ss;
    ss << "PixelDifferenceChannel 0: Difference should be " << correctDifference << " but is: " << difference;
    throw std::runtime_error(ss.str());
    }
  }

  {
  // Static specified pixel length
  float difference = PixelDifferenceChannel<FloatVectorImageType::PixelType, 1>::Difference(a, b);
  float correctDifference = 4.0f;
  if(!Testing::ValuesEqual(difference, correctDifference))
    {
    std::stringstream ss;
    ss << "PixelDifferenceChannel 1: Difference should be " << correctDifference << " but is: " << difference;
    throw std::runtime_error(ss.str());
    }
  }

}
