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

#include "NamedVTKImage.h"

#include <stdexcept>

static void TestFindImageByName();
static void TestConstructor();

int main(int argc, char*argv[])
{
  TestConstructor();
  TestFindImageByName();

  return EXIT_SUCCESS;
};

void TestConstructor()
{
  NamedVTKImage namedVTKImage;

  if(namedVTKImage.DisplayType != NamedVTKImage::SCALARS)
    {
    throw std::runtime_error("DisplayType not initialized correctly!");
    }

  if(namedVTKImage.Name != "Unnamed")
    {
    throw std::runtime_error("Name not initialized correctly!");
    }

  if(namedVTKImage.ImageData != NULL)
    {
    throw std::runtime_error("ImageData not initialized correctly!");
    }

}

void TestFindImageByName()
{
  NamedVTKImage namedImage1(NULL, "Image1", NamedVTKImage::SCALARS);
  NamedVTKImage namedImage2(NULL, "Image2", NamedVTKImage::SCALARS);

  std::vector<NamedVTKImage> namedImages;
  namedImages.push_back(namedImage1);
  namedImages.push_back(namedImage2);

  NamedVTKImage retrievedImage = NamedVTKImage::FindImageByName(namedImages, "Image1");
  throw;
}
