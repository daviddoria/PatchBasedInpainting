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

// Submodules
#include <Mask/Mask.h>
#include <Helpers/Helpers.h>

// Custom
#include "PatchHelpers.h"
#include "Testing/Testing.h"

// STL
#include <stdexcept>

// ITK
#include "itkImage.h"

int main(int argc, char*argv[])
{
  if(argc < 2)
  {
    std::cerr << "Required arguments: fileName.png" << std::endl;
    return EXIT_FAILURE;
  }
//  typedef itk::Image<itk::CovariantVector<int, 3>, 2> ImageType;
  typedef itk::Image<itk::CovariantVector<unsigned char, 3>, 2> ImageType;

  std::string fileName = argv[1];

  std::vector<itk::Image<itk::CovariantVector<unsigned char, 3>, 2>::Pointer> patches =
    PatchHelpers::ReadTopPatchesGrid(fileName, 31, 10, 10);

  for(unsigned int i = 0; i < patches.size(); ++i)
  {
    ITKHelpers::WriteImage(patches[i].GetPointer(), Helpers::GetSequentialFileName("Patch", i, "png", 2));
  }

  return EXIT_SUCCESS;
}
