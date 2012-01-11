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

#include "Image.h"

int main(int argc, char*argv[])
{
  itk::Index<2> corner;
  corner.Fill(0);

  itk::Size<2> size;
  size.Fill(100);

  itk::ImageRegion<2> region(corner, size);

  Image<float>::Pointer floatImage = Image<float>::New();
  floatImage->SetRegions(region);
  floatImage->Allocate();

  Image<int>::Pointer intImage = Image<int>::New();
  intImage->SetRegions(region);
  intImage->Allocate();

  std::vector<ImageParent*> images;
  images.push_back(floatImage);
  images.push_back(intImage);

  for(unsigned int i = 0; i < images.size(); ++i)
    {
    images[i]->DoSomething();
    }

  return EXIT_SUCCESS;
}
