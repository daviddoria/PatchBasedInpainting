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

#include "Helpers.h"
#include "Testing.h"

int main(int argc, char*argv[])
{
  if(!Helpers::IsValidRGB(5,5,5))
    {
    throw std::runtime_error("(5,5,5) should be valid RGB!");
    }

  if(Helpers::IsValidRGB(500,500,500))
    {
    throw std::runtime_error("(500,500,500) should not be valid RGB!");
    }

  std::string replacedFileExtension = Helpers::ReplaceFileExtension("test.txt", "png");
  std::string correct_replacedFileExtension = "test.png";
  if(replacedFileExtension != correct_replacedFileExtension)
    {
    std::stringstream ss;
    ss << "ReplacedFileExtension was " << replacedFileExtension << " but should have been " << correct_replacedFileExtension;
    throw std::runtime_error(ss.str());
    }

  {
  std::string sequentialFileName = Helpers::GetSequentialFileName("test", 2, "png");
  std::string correct_sequentialFileName = "test_0002.png";
  if(replacedFileExtension != correct_replacedFileExtension)
    {
    std::stringstream ss;
    ss << "sequentialFileName was " << sequentialFileName << " but should have been " << correct_sequentialFileName;
    throw std::runtime_error(ss.str());
    }
  }

  {
  // Non-default length
  std::string sequentialFileName = Helpers::GetSequentialFileName("test", 2, "png", 5);
  std::string correct_sequentialFileName = "test_00002.png";
  if(sequentialFileName != correct_sequentialFileName)
    {
    std::stringstream ss;
    ss << "ReplacedFileExtension was " << sequentialFileName << " but should have been " << correct_sequentialFileName;
    throw std::runtime_error(ss.str());
    }
  }

  {
  int value = 4;
  std::string paddedString = Helpers::ZeroPad(value, 3);
  std::string correct_paddedString = "004";
  if(paddedString != correct_paddedString)
    {
    std::stringstream ss;
    ss << "paddedString was " << paddedString << " but should have been " << correct_paddedString;
    throw std::runtime_error(ss.str());
    }
  }

  unsigned int sideLength = Helpers::SideLengthFromRadius(5);
  unsigned int correct_sideLength = 11;
  if(sideLength != correct_sideLength)
    {
    std::stringstream ss;
    ss << "sideLength was " << sideLength << " but should have been " << correct_sideLength;
    throw std::runtime_error(ss.str());
    }

  {
  std::string string1 = "Hello";
  std::string string2 = "Goodbye";
  if(Helpers::StringsMatch(string1, string2))
    {
    std::stringstream ss;
    ss << "Strings " << string1 << " and " << string2 << " should not match!";
    throw std::runtime_error(ss.str());
    }
  }

  {
  std::string string1 = "Hello";
  std::string string2 = "Hello";
  if(!Helpers::StringsMatch(string1, string2))
    {
    std::stringstream ss;
    ss << "Strings " << string1 << " and " << string2 << " should match!";
    throw std::runtime_error(ss.str());
    }
  }

  {
  float value = .2;
  float roundedValue = Helpers::RoundAwayFromZero(value);
  float correct_roundedValue = 1;
  if(roundedValue != correct_roundedValue)
    {
    std::stringstream ss;
    ss << "Rounded " << value << " to " << roundedValue << " instead of " << correct_roundedValue;
    throw std::runtime_error(ss.str());
    }
  }

  {
  float value = -.2;
  float roundedValue = Helpers::RoundAwayFromZero(value);
  float correct_roundedValue = -1;
  if(roundedValue != correct_roundedValue)
    {
    std::stringstream ss;
    ss << "Rounded " << value << " to " << roundedValue << " instead of " << correct_roundedValue;
    throw std::runtime_error(ss.str());
    }
  }

  {
  std::vector<unsigned int> v;
  v.push_back(10);
  v.push_back(3);
  v.push_back(5);
  unsigned int minLocation = Helpers::argmin(v);
  unsigned int correct_minLocation = 1;
  if(minLocation != correct_minLocation)
    {
    std::stringstream ss;
    ss << "argmin was " << minLocation << " but should have been " << correct_minLocation;
    throw std::runtime_error(ss.str());
    }
  }

  {
  std::vector<unsigned int> v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);

  std::vector<unsigned int> correctNormalized;
  correctNormalized.push_back(.26726);
  correctNormalized.push_back(0.53452);
  correctNormalized.push_back(0.80178);

  Helpers::NormalizeVector(v);
  if(!Testing::VectorsEqual(v, correctNormalized))
    {
    std::stringstream ss;
    ss << "Vector not correctly normalized!";
    for(unsigned int i = 0; i < v.size(); ++i)
      {
      ss << "Element " << i << " was " << v[i] << " but should have been " << correctNormalized[i] << std::endl;
      }
    throw std::runtime_error(ss.str());
    }
  }

  {
  std::vector<unsigned int> v;
  v.push_back(10);
  v.push_back(20);
  v.push_back(30);

  unsigned int correctMedian = 20;
  unsigned int computedMedian = Helpers::VectorMedian(v);
  if(computedMedian != correctMedian)
    {
    std::stringstream ss;
    ss << "median was " << computedMedian << " but should have been " << correctMedian;
    throw std::runtime_error(ss.str());
    }
  }

  {
  std::vector<float> v;
  v.push_back(10);
  v.push_back(20);

  float retrievedVectorValue = Helpers::index(v, 1);
  float correct_retrievedVectorValue = 20;
  if(retrievedVectorValue != correct_retrievedVectorValue)
    {
    std::stringstream ss;
    ss << "Vector index() not working - returned " << retrievedVectorValue << " but should have been " << correct_retrievedVectorValue;
    throw std::runtime_error(ss.str());
    }
  }

  {
  float value = 1.2;

  float retrievedVectorValue = Helpers::index(value, 0);

  if(retrievedVectorValue != value)
    {
    std::stringstream ss;
    ss << "Vector index() not working - returned " << retrievedVectorValue << " but should have been " << value;
    throw std::runtime_error(ss.str());
    }
  }
  return EXIT_SUCCESS;
}
