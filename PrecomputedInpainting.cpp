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
#include "Helpers/OutputHelpers.h"

// Inpainting visitors
#include "Visitors/SimpleVisitors/InpaintPatchVisitor.hpp"
#include "Visitors/InformationVisitors/IterationWriterVisitor.hpp"
#include "Visitors/SimpleVisitors/CompositeSimpleVisitor.hpp"

// Inpainters
//#include "Inpainters/MaskImagePatchInpainter.hpp"
#include "Inpainters/ImageAndMaskPatchInpainter.hpp"

// Difference functions
#include "DifferenceFunctions/ImagePatchDifference.hpp"

// Inpainting
#include "Algorithms/InpaintingPrecomputedAlgorithm.hpp"

// ITK
#include "itkImageFileReader.h"

// STL
#include <string>
#include <fstream>
#include <limits>

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/detail/d_ary_heap.hpp>

// Run with: Data/trashcan.mha Data/trashcan_mask.mha 15 precomputed.txt filled.mha
int main(int argc, char *argv[])
{
  // Verify arguments
  if(argc != 6)
    {
    std::stringstream ss;
    ss << "Required arguments: image.mha imageMask.mha patch_half_width precomputed.txt output.mha" << std::endl;
    ss << "Input arguments: ";
    for(int i = 1; i < argc; ++i)
      {
      ss << argv[i] << " ";
      }
    throw std::runtime_error(ss.str());
    return EXIT_FAILURE;
    }

  // Parse arguments
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];

  std::stringstream ssPatchRadius;
  ssPatchRadius << argv[3];
  unsigned int patchHalfWidth = 0;
  ssPatchRadius >> patchHalfWidth;

  std::string precomputedFilename = argv[4];

  std::string outputFilename = argv[5];

  // Output arguments
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;
  std::cout << "Patch half width: " << patchHalfWidth << std::endl;
  std::cout << "Precomputed file: " << precomputedFilename << std::endl;
  std::cout << "Output: " << outputFilename << std::endl;

  typedef FloatVectorImageType ImageType;

  typedef  itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

  ImageType::Pointer image = ImageType::New();
  ITKHelpers::DeepCopy(imageReader->GetOutput(), image.GetPointer());

  Mask::Pointer mask = Mask::New();
  mask->Read(maskFilename);

  typedef std::pair<itk::Index<2>, itk::Index<2> > NodePairType;
  typedef std::queue<NodePairType> NodePairQueueType;
  NodePairQueueType nodePairQueue;

  std::ifstream inputStream(precomputedFilename.c_str());
  std::string line;
  while(getline(inputStream, line))
    {
    std::stringstream ss;
    ss << line;
    itk::Index<2> targetNode;
    itk::Index<2> sourceNode;

    ss >> sourceNode[0] >> sourceNode[1];
    std::cout << "Source node: ";
    Helpers::OutputNode(sourceNode);

    ss.ignore(std::numeric_limits<std::streamsize>::max(),':'); // Ignore the colon
    
    ss >> targetNode[0] >> targetNode[1];
    std::cout << "Target node: ";
    Helpers::OutputNode(targetNode);
    
    NodePairType nodePair;
    nodePair.first = targetNode;
    nodePair.second = sourceNode;
    nodePairQueue.push(nodePair);
    }

  // Create the patch inpainter. The inpainter needs to know the status of each pixel to
  // determine if they should be inpainted.
//   typedef MaskImagePatchInpainter InpainterType;
//   InpainterType patchInpainter(patchHalfWidth, mask);

  ImageAndMaskPatchInpainter<ImageType> patchInpainter(image, mask, patchHalfWidth);

  IterationWriterVisitor<itk::Index<2>, ImageType> iterationWriterVisitor(image, mask);

  // Create the inpainting visitor
  InpaintPatchVisitor<itk::Index<2>, ImageType> inpaintPatchVisitor(image, mask, patchHalfWidth);

  typedef CompositeSimpleVisitor<itk::Index<2> > CompositeSimpleVisitorType;
  CompositeSimpleVisitorType compositeSimpleVisitor;
  compositeSimpleVisitor.AddVisitor(&inpaintPatchVisitor);
  compositeSimpleVisitor.AddVisitor(&iterationWriterVisitor);
  
  // Perform the inpainting
  InpaintingPrecomputedAlgorithm(nodePairQueue, compositeSimpleVisitor, patchInpainter);

  OutputHelpers::WriteImage(image.GetPointer(), outputFilename);

  return EXIT_SUCCESS;
}
