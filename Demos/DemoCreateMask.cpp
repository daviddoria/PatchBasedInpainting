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

// Custom
#include "Helpers.h"
#include "HelpersOutput.h"
#include "Mask.h"
#include "Types.h"
#include "ClusterColorsUniform.h"
#include "ClusterColorsAdaptive.h"

// ITK
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

int main(int argc, char *argv[])
{
  std::string inputFileName = argv[1];
  std::string outputFileName = argv[2];

  std::cout << "Input: " << inputFileName << std::endl;
  std::cout << "Output: " << outputFileName << std::endl;

  typedef itk::ImageFileReader<FloatVectorImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(inputFileName);
  reader->Update();

  Mask::Pointer mask = Mask::New();
  FloatVectorImageType::PixelType green;
  green.SetSize(4);
  green[0] = 0;
  green[1] = 255;
  green[2] = 0;
  green[3] = 255;

  mask->CreateFromImage<FloatVectorImageType>(reader->GetOutput(), green);

  HelpersOutput::WriteImage<Mask>(mask, outputFileName);

  return EXIT_SUCCESS;
}

