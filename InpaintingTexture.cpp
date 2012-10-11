/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
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

// Pixel descriptors
#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

// Visitors
#include "Visitors/NearestNeighborsDefaultVisitor.hpp"

// Descriptor visitors
#include "Visitors/DescriptorVisitors/ImagePatchDescriptorVisitor.hpp"

// Inpainting visitors
#include "Visitors/InpaintingVisitor.hpp"
#include "Visitors/AcceptanceVisitors/DefaultAcceptanceVisitor.hpp"

// Nearest neighbors functions
#include "NearestNeighbor/LinearSearchBest/HistogramCorrelation.hpp"
#include "NearestNeighbor/LinearSearchBest/HistogramDifference.hpp"
#include "NearestNeighbor/LinearSearchBest/HoleHistogramDifference.hpp"
#include "NearestNeighbor/LinearSearchBest/HistogramNewColors.hpp"
#include "NearestNeighbor/LinearSearchBest/DualHistogramDifference.hpp"
#include "NearestNeighbor/LinearSearchBest/AdaptiveDualHistogramDifference.hpp"
#include "NearestNeighbor/LinearSearchBest/AdaptiveDualQuadrantHistogramDifference.hpp"
#include "NearestNeighbor/LinearSearchBest/IntroducedEnergy.hpp"
#include "NearestNeighbor/LinearSearchBest/First.hpp"
#include "NearestNeighbor/LinearSearchBest/FirstAndWrite.hpp"
#include "NearestNeighbor/LinearSearchBest/Texture.hpp"
#include "NearestNeighbor/LinearSearchBest/ColorTexture.hpp"
#include "NearestNeighbor/LinearSearchKNNProperty.hpp"
#include "NearestNeighbor/TwoStepNearestNeighbor.hpp"

// Initializers
#include "Initializers/InitializeFromMaskImage.hpp"

// Inpainters
#include "Inpainters/PatchInpainter.hpp"
#include "Inpainters/CompositePatchInpainter.hpp"

// Difference functions
#include "DifferenceFunctions/ImagePatchDifference.hpp"
#include "DifferenceFunctions/SumAbsolutePixelDifference.hpp"
#include "DifferenceFunctions/SumSquaredPixelDifference.hpp"

// Utilities
#include "Utilities/PatchHelpers.h"
#include "Utilities/IndirectPriorityQueue.h"

// Inpainting
#include "Algorithms/InpaintingAlgorithm.hpp"

// Priority
#include "Priority/PriorityRandom.h"
#include "Priority/PriorityCriminisi.h"

// ITK
#include "itkImageFileReader.h"

// Submodules
#include <Helpers/Helpers.h>
#include <ITKVTKHelpers/ITKVTKHelpers.h>
#include <Mask/MaskOperations.h>

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>

#include "Drivers/InpaintingTexture.hpp"

// Run with: Data/trashcan.png Data/trashcan.mask 15 filled.png
int main(int argc, char *argv[])
{
  // Verify arguments
  if(argc != 7)
  {
    std::cerr << "Required arguments: image.png imageMask.mask patchHalfWidth numberOfKNN binsPerChannel output.png" << std::endl;
    std::cerr << "Input arguments: ";
    for(int i = 1; i < argc; ++i)
    {
      std::cerr << argv[i] << " ";
    }
    return EXIT_FAILURE;
  }

  std::stringstream ssArguments;
  for(int i = 1; i < argc; ++i)
  {
    ssArguments << argv[i] << " ";
    std::cout << argv[i] << " ";
  }

  // Parse arguments
  std::string imageFileName;
  std::string maskFileName;

  unsigned int patchHalfWidth = 0;

  unsigned int numberOfKNN = 0;

  unsigned int binsPerChannel = 0;

  std::string outputFileName;

  ssArguments >> imageFileName >> maskFileName >> patchHalfWidth >> numberOfKNN >> binsPerChannel >> outputFileName;

  // Output arguments
  std::cout << "Reading image: " << imageFileName << std::endl;
  std::cout << "Reading mask: " << maskFileName << std::endl;
  std::cout << "Patch half width: " << patchHalfWidth << std::endl;
  std::cout << "numberOfKNN: " << numberOfKNN << std::endl;
  std::cout << "Output: " << outputFileName << std::endl;

  // typedef itk::Image<itk::CovariantVector<unsigned char, 3>, 2> OriginalImageType; // This doesn't allow for direct "a - b" pixel comparisons, because (100 - 150) or similar will underflow!
  // typedef itk::Image<itk::CovariantVector<float, 3>, 2> OriginalImageType; // This is quite slow
  typedef itk::Image<itk::CovariantVector<int, 3>, 2> OriginalImageType;
//  typedef itk::Image<itk::CovariantVector<short, 3>, 2> OriginalImageType; // Can't use this (signed range: -32768 to 32767 (not high enough, needs to handle 255*255 = 65025) )

  typedef  itk::ImageFileReader<OriginalImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFileName);
  imageReader->Update();

  OriginalImageType* originalImage = imageReader->GetOutput();

  // Read the mask
  Mask::Pointer mask = Mask::New();
  mask->Read(maskFileName);

  InpaintingTexture(originalImage, mask, patchHalfWidth, numberOfKNN);

  // If the output filename is a png file, then use the RGBImage writer so that it is first
  // casted to unsigned char. Otherwise, write the file directly.
  if(Helpers::GetFileExtension(outputFileName) == "png")
  {
    ITKHelpers::WriteRGBImage(originalImage, outputFileName);
  }
  else
  {
    ITKHelpers::WriteImage(originalImage, outputFileName);
  }

  return EXIT_SUCCESS;
}
