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

#include "ValidPixelIterator.h"
#include "Testing.h"

int main(int argc, char*argv[])
{
  typedef itk::Image<float*, 2> ImageType;
  ImageType::Pointer image = ImageType::New();
  image->SetRegions(Testing::GetImageRegion());
  image->Allocate();

  itk::ImageRegionIterator<ImageType> setupIterator(image, image->GetLargestPossibleRegion());

  while(!setupIterator.IsAtEnd())
    {
    setupIterator.Set(NULL);

    ++setupIterator;
    }

  ValidPixelIterator<ImageType> validPixelIterator(image, image->GetLargestPossibleRegion());

  unsigned int validPixelCounter = 0;
  for(ValidPixelIterator<ImageType>::ConstIterator iterator = validPixelIterator.begin(); iterator != validPixelIterator.end(); ++iterator)
    {
    std::cout << " " << *iterator;
    validPixelCounter++;
    }

  std::cout << "Number of valid pixels: " << validPixelCounter << std::endl;
  return EXIT_SUCCESS;
}
