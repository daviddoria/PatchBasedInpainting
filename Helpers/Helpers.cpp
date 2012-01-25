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

// Custom
#include "ImageProcessing/itkRGBToLabColorSpacePixelAccessor.h"
#include "ImageProcessing/Derivatives.h"

namespace Helpers
{

bool IsValidRGB(const int r, const int g, const int b)
{
  if(r > 255 || r < 0 || g > 255 || g < 0 || b > 255 || b < 0)
  {
    return false;
  }
  return true;
}

std::string GetSequentialFileName(const std::string& filePrefix, const unsigned int iteration, const std::string& fileExtension, const unsigned int paddedLength)
{
  std::stringstream padded;
  padded << filePrefix << "_" << ZeroPad(iteration, paddedLength) << "." << fileExtension;
  return padded.str();
}

float RoundAwayFromZero(const float number)
{
  float signMultiplier = 0;
  if(number >= 0)
    {
    signMultiplier = 1.0;
    }
  else
    {
    signMultiplier = -1.0;
    }
  float absNumber = fabs(number);
  float rounded = ceil(absNumber) * signMultiplier;

  return rounded;
}


unsigned int SideLengthFromRadius(const unsigned int radius)
{
  return radius*2 + 1;
}

std::string ZeroPad(const unsigned int number, const unsigned int paddedLength)
{
  std::stringstream padded;
  padded << std::setfill('0') << std::setw(paddedLength) << number;

  return padded.str();
}


std::string ReplaceFileExtension(const std::string& fileName, const std::string& newExtension)
{
  // This should be called like:
  // std::string newFilenName = ReplaceFileExtension("oldfile.png", "jpg");
  // To produce "oldfile.jpg"

  std::string newFileName = fileName;
  const unsigned int fileExtensionLength = 3;
  newFileName.replace(newFileName.size() - fileExtensionLength, fileExtensionLength, newExtension);
  return newFileName;
}

bool StringsMatch(const std::string& a, const std::string& b)
{
  // STL compare returns 0 if strings match. This is unintuitive, so this function returns the expected value.
  if(a.compare(b) == 0)
    {
    return true;
    }
  else
    {
    return false;
    }
}


} // end namespace
