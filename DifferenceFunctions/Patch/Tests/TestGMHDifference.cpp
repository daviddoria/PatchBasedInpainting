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

#include "GMHDifference.hpp"
#include "GMHDifferenceFast.hpp"

//#include "Testing.h"

// ITK
#include "itkImage.h"
#include "itkImageFileReader.h"

int main(int argc, char*argv[])
{
  // Verify arguments
  if(argc != 3)
  {
    std::cerr << "Required arguments: image.png image.mask" << std::endl;
    std::cerr << "Input arguments: ";
    for(int i = 1; i < argc; ++i)
    {
      std::cerr << argv[i] << " ";
    }
    return EXIT_FAILURE;
  }

  // Parse arguments
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];

  // Output arguments
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;

  itk::Size<2> patchSize = {{21,21}};

  itk::Index<2> targetCorner = {{319, 302}};
  itk::ImageRegion<2> targetRegion(targetCorner, patchSize);

  itk::Index<2> sourceCorner = {{341, 300}};
  itk::ImageRegion<2> sourceRegion(sourceCorner, patchSize);

  typedef itk::Image<itk::CovariantVector<float, 3>, 2> ImageType;

  typedef  itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

  Mask::Pointer mask = Mask::New();
  mask->Read(maskFilename);

  const unsigned int numberOfBinsPerChannel = 30;
  GMHDifference<ImageType> gmhDifference(imageReader->GetOutput(), mask, numberOfBinsPerChannel);
//  GMHDifferenceFast<ImageType> gmhDifference(imageReader->GetOutput(), mask, numberOfBinsPerChannel); // This produces 5.87 but it should produce 0.932

  float difference = gmhDifference.Difference(targetRegion, sourceRegion);

  std::cout << "GMHDifference: " << difference << std::endl;


  return EXIT_SUCCESS;
}
