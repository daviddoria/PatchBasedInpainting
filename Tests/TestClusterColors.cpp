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
#include "HelpersOutput.h"
#include "Mask.h"
#include "Types.h"
#include "ClusterColors.h"

// ITK
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

int main(int argc, char *argv[])
{
  std::string inputFileName = argv[1];
  std::string outputPrefix = argv[2];

  std::cout << "Input: " << inputFileName << std::endl;
  std::cout << "Output prefix: " << outputPrefix << std::endl;
  
  typedef itk::ImageFileReader<FloatVectorImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(inputFileName);
  reader->Update();

  IntImageType::Pointer outputLabelImage = IntImageType::New();
  //ClusterColors(reader->GetOutput(), 10, outputLabelImage);
//   ClusterColorsUniform(reader->GetOutput(), 10, outputLabelImage);
//   HelpersOutput::WriteImage<IntImageType>(outputLabelImage, "/home/doriad/Debug/10.mha");
  
  for(unsigned int numberOfClusters = 1; numberOfClusters < 10; numberOfClusters ++)
    {
    std::cout << "numberOfClusters: " << numberOfClusters << std::endl;
    IntImageType::Pointer outputLabelImage = IntImageType::New();
    ClusterColorsUniform(reader->GetOutput(), numberOfClusters, outputLabelImage);
    std::stringstream ss;
    ss << "/home/doriad/Debug/" << outputPrefix << "_" << Helpers::ZeroPad(numberOfClusters, 4) << ".mha";
    HelpersOutput::WriteImage<IntImageType>(outputLabelImage, ss.str());
    }
    
  return EXIT_SUCCESS;
}
