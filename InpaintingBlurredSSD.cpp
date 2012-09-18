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

// Custom
#include "Utilities/IndirectPriorityQueue.h"

// Submodules
#include <Helpers/Helpers.h>

// Pixel descriptors
#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

// Descriptor visitors
#include "Visitors/DescriptorVisitors/ImagePatchDescriptorVisitor.hpp"

// Inpainting visitors
#include "Visitors/InpaintingVisitor.hpp"
#include "Visitors/AcceptanceVisitors/DefaultAcceptanceVisitor.hpp"

// Nearest neighbors
#include "NearestNeighbor/LinearSearchBest/Property.hpp"
#include "NearestNeighbor/LinearSearchKNNProperty.hpp"
#include "NearestNeighbor/TwoStepNearestNeighbor.hpp"
#include "NearestNeighbor/LinearSearchBest/FirstAndWrite.hpp"

// Initializers
#include "Initializers/InitializeFromMaskImage.hpp"
#include "Initializers/InitializePriority.hpp"

// Inpainters
#include "Inpainters/CompositePatchInpainter.hpp"
#include "Inpainters/PatchInpainter.hpp"

// Difference functions
#include "DifferenceFunctions/ImagePatchDifference.hpp"
#include "DifferenceFunctions/SumAbsolutePixelDifference.hpp"
#include "DifferenceFunctions/SumSquaredPixelDifference.hpp"

// Utilities
#include "Utilities/PatchHelpers.h"

// Inpainting
#include "Algorithms/InpaintingAlgorithm.hpp"

// Priority
#include "Priority/PriorityCriminisi.h"

// ITK
#include "itkImageFileReader.h"

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>

// Run with: Data/trashcan.png Data/trashcan.mask 15 filled.png
int main(int argc, char *argv[])
{
  // Verify arguments
  if(argc != 5)
  {
    std::cerr << "Required arguments: image.png imageMask.mask patchHalfWidth output.png" << std::endl;
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

  std::stringstream ssPatchHalfWidth;
  ssPatchHalfWidth << argv[3];
  unsigned int patchHalfWidth = 0;
  ssPatchHalfWidth >> patchHalfWidth;

  std::string outputFileName = argv[4];

  // Output arguments
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;
  std::cout << "Patch half width: " << patchHalfWidth << std::endl;
  std::cout << "Output: " << outputFileName << std::endl;

  typedef itk::Image<itk::CovariantVector<int, 3>, 2> OriginalImageType;

  typedef  itk::ImageFileReader<OriginalImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

  OriginalImageType* originalImage = imageReader->GetOutput();

  itk::ImageRegion<2> fullRegion = originalImage->GetLargestPossibleRegion();

  Mask::Pointer mask = Mask::New();
  mask->Read(maskFilename);

  std::cout << "hole pixels: " << mask->CountHolePixels() << std::endl;
  std::cout << "valid pixels: " << mask->CountValidPixels() << std::endl;

  // Blur the image so that the gradients are not so noisy (for Criminisi priority)
  typedef OriginalImageType BlurredImageType; // Usually the blurred image is the same type as the original image.
  BlurredImageType::Pointer blurredImage = BlurredImageType::New();
  float blurVariance = 2.0f;
  MaskOperations::MaskedBlur(originalImage, mask, blurVariance, blurredImage.GetPointer());

  ITKHelpers::WriteRGBImage(blurredImage.GetPointer(), "BlurredImage.png");

  // Blur the image slightly so that the SSD comparisons are not so noisy
  BlurredImageType::Pointer slightBlurredImage = BlurredImageType::New();
  float slightBlurVariance = 1.0f;
  MaskOperations::MaskedBlur(originalImage, mask, slightBlurVariance, slightBlurredImage.GetPointer());

  ITKHelpers::WriteRGBImage(slightBlurredImage.GetPointer(), "SlightlyBlurredImage.png");

  typedef ImagePatchPixelDescriptor<OriginalImageType> ImagePatchPixelDescriptorType;

  // Create the graph
  typedef boost::grid_graph<2> VertexListGraphType;
  boost::array<std::size_t, 2> graphSideLengths = { { fullRegion.GetSize()[0],
                                                      fullRegion.GetSize()[1] } };
  VertexListGraphType graph(graphSideLengths);
  typedef boost::graph_traits<VertexListGraphType>::vertex_descriptor VertexDescriptorType;
  typedef boost::graph_traits<VertexListGraphType>::vertex_iterator VertexIteratorType;

  // Queue
  typedef IndirectPriorityQueue<VertexListGraphType> BoundaryNodeQueueType;
  BoundaryNodeQueueType boundaryNodeQueue(graph);

  // Create the descriptor map. This is where the data for each pixel is stored.
  typedef boost::vector_property_map<ImagePatchPixelDescriptorType,
      BoundaryNodeQueueType::IndexMapType> ImagePatchDescriptorMapType;
  ImagePatchDescriptorMapType imagePatchDescriptorMap(num_vertices(graph), boundaryNodeQueue.IndexMap);

  // Create the patch inpainter.
  typedef PatchInpainter<OriginalImageType> OriginalImageInpainterType;
  OriginalImageInpainterType originalImagePatchInpainter(patchHalfWidth, originalImage, mask);
  originalImagePatchInpainter.SetDebugImages(true);
  originalImagePatchInpainter.SetImageName("RGB");

  // Create an inpainter for the blurred image.
  typedef PatchInpainter<BlurredImageType> BlurredImageInpainterType;
  BlurredImageInpainterType blurredImagePatchInpainter(patchHalfWidth, blurredImage, mask);

  // Create an inpainter for the slightly blurred image.
  typedef PatchInpainter<BlurredImageType> BlurredImageInpainterType;
  BlurredImageInpainterType slightlyBlurredImagePatchInpainter(patchHalfWidth, slightBlurredImage, mask);

  // Create a composite inpainter.
  CompositePatchInpainter inpainter;
  inpainter.AddInpainter(&originalImagePatchInpainter);
  inpainter.AddInpainter(&blurredImagePatchInpainter);
  inpainter.AddInpainter(&slightlyBlurredImagePatchInpainter);

  // Create the priority function
  typedef PriorityCriminisi<BlurredImageType> PriorityType;
  PriorityType priorityFunction(blurredImage, mask, patchHalfWidth);


  // Create the descriptor visitor - here is where we specify which image to use (here the slightly blurred image) for the SSD computation
  typedef ImagePatchDescriptorVisitor<VertexListGraphType, BlurredImageType, ImagePatchDescriptorMapType>
      ImagePatchDescriptorVisitorType;
  ImagePatchDescriptorVisitorType imagePatchDescriptorVisitor(slightBlurredImage, mask, imagePatchDescriptorMap, patchHalfWidth);

  typedef DefaultAcceptanceVisitor<VertexListGraphType> AcceptanceVisitorType;
  AcceptanceVisitorType acceptanceVisitor;

  // Create the inpainting visitor
  typedef InpaintingVisitor<VertexListGraphType, OriginalImageType, BoundaryNodeQueueType,
                            ImagePatchDescriptorVisitorType, AcceptanceVisitorType, PriorityType>
                            InpaintingVisitorType;
  InpaintingVisitorType inpaintingVisitor(originalImage, mask, boundaryNodeQueue,
                                          imagePatchDescriptorVisitor, acceptanceVisitor,
                                          &priorityFunction, patchHalfWidth,
                                          outputFileName);
  inpaintingVisitor.SetAllowNewPatches(false);
  inpaintingVisitor.SetDebugImages(true);

  InitializePriority(mask, boundaryNodeQueue, &priorityFunction);

  // Initialize the boundary node queue from the user provided mask image.
  InitializeFromMaskImage<InpaintingVisitorType, VertexDescriptorType>(mask, &inpaintingVisitor);
  std::cout << "PatchBasedInpaintingNonInteractive: There are " << boundaryNodeQueue.CountValidNodes()
            << " nodes in the boundaryNodeQueue" << std::endl;

  // Create the nearest neighbor finder
//  typedef ImagePatchDifference<ImagePatchPixelDescriptorType,
//      SumAbsolutePixelDifference<OriginalImageType::PixelType> > PatchDifferenceType;

  typedef ImagePatchDifference<ImagePatchPixelDescriptorType,
      SumSquaredPixelDifference<OriginalImageType::PixelType> > PatchDifferenceType;

//  typedef LinearSearchBestProperty<ImagePatchDescriptorMapType,
//                                   PatchDifferenceType> BestSearchType;
//  BestSearchType linearSearchBest(imagePatchDescriptorMap);

  // Create the first (KNN) neighbor finder
  typedef LinearSearchKNNProperty<ImagePatchDescriptorMapType, PatchDifferenceType> KNNSearchType;
  unsigned int numberOfKNN = 100;
  KNNSearchType linearSearchKNN(imagePatchDescriptorMap, numberOfKNN);

  // Setup the second (1-NN) neighbor finder
  typedef std::vector<VertexDescriptorType>::iterator VertexDescriptorVectorIteratorType;

  typedef LinearSearchBestProperty<ImagePatchDescriptorMapType, PatchDifferenceType> BestSearchType;

  BestSearchType linearSearchBest(imagePatchDescriptorMap);

  // Setup the two step neighbor finder

  // Without writing top KNN patches
//  TwoStepNearestNeighbor<KNNSearchType, BestSearchType>
//      twoStepNearestNeighbor(linearSearchKNN, linearSearchBest);

  // With writing top KNN patches
  typedef LinearSearchBestFirstAndWrite<ImagePatchDescriptorMapType,
      OriginalImageType, PatchDifferenceType> TopPatchesWriterType;
  TopPatchesWriterType topPatchesWriter(imagePatchDescriptorMap, originalImage, mask);

  TwoStepNearestNeighbor<KNNSearchType, BestSearchType, TopPatchesWriterType>
      twoStepNearestNeighbor(linearSearchKNN, linearSearchBest, topPatchesWriter);

  // Perform the inpainting
  InpaintingAlgorithm(graph, inpaintingVisitor, &boundaryNodeQueue,
                      twoStepNearestNeighbor, &inpainter);

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
