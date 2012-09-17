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
#include "NearestNeighbor/LinearSearchBest/DualHistogramDifference.hpp"
#include "NearestNeighbor/LinearSearchBest/AdaptiveDualHistogramDifference.hpp"
#include "NearestNeighbor/LinearSearchBest/AdaptiveDualQuadrantHistogramDifference.hpp"
#include "NearestNeighbor/LinearSearchBest/IntroducedEnergy.hpp"
#include "NearestNeighbor/LinearSearchBest/First.hpp"
#include "NearestNeighbor/LinearSearchBest/FirstAndWrite.hpp"
#include "NearestNeighbor/LinearSearchBest/StrategySelection.hpp"
#include "NearestNeighbor/LinearSearchKNNProperty.hpp"
#include "NearestNeighbor/TwoStepNearestNeighbor.hpp"

// Initializers
#include "Initializers/InitializeFromMaskImage.hpp"
#include "Initializers/InitializePriority.hpp"

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
  ITKHelpers::WriteRGBImage(originalImage, "OriginalImage.png");
  ITKHelpers::WriteImage(originalImage, "OriginalImage.mha");

  itk::ImageRegion<2> fullRegion = originalImage->GetLargestPossibleRegion();

//  ImageType::Pointer image = ImageType::New();
//  ITKHelpers::DeepCopy(imageReader->GetOutput(), image.GetPointer());

  // Convert the image to HSV
  typedef itk::Image<itk::CovariantVector<float, 3>, 2> HSVImageType;
  HSVImageType::Pointer hsvImage = HSVImageType::New();
//  ITKVTKHelpers::ConvertRGBtoHSV(image.GetPointer(), hsvImage.GetPointer());
  ITKVTKHelpers::ConvertRGBtoHSV(originalImage, hsvImage.GetPointer());

  ITKHelpers::WriteImage(hsvImage.GetPointer(), "HSVImage.mha");

  // Read the mask
  Mask::Pointer mask = Mask::New();
  mask->Read(maskFileName);
  ITKHelpers::WriteImage(mask.GetPointer(), "Mask.mha");

  std::cout << "hole pixels: " << mask->CountHolePixels() << std::endl;
  std::cout << "valid pixels: " << mask->CountValidPixels() << std::endl;

  // Blur the image
  typedef OriginalImageType BlurredImageType; // Usually the blurred image is the same type as the original image.
  BlurredImageType::Pointer blurredImage = BlurredImageType::New();
  float blurVariance = 2.0f;
  MaskOperations::MaskedBlur(originalImage, mask, blurVariance, blurredImage.GetPointer());

  ITKHelpers::WriteRGBImage(blurredImage.GetPointer(), "BlurredImage.png");

  // Create the graph
  typedef ImagePatchPixelDescriptor<OriginalImageType> ImagePatchPixelDescriptorType;

  typedef boost::grid_graph<2> VertexListGraphType;

  // We can't make this a signed type (size_t versus int) because we allow negative
  boost::array<std::size_t, 2> graphSideLengths = { { fullRegion.GetSize()[0],
                                                      fullRegion.GetSize()[1] } };
  VertexListGraphType graph(graphSideLengths);
  typedef boost::graph_traits<VertexListGraphType>::vertex_descriptor VertexDescriptorType;
  typedef boost::graph_traits<VertexListGraphType>::vertex_iterator VertexIteratorType;

  // Get the index map
  typedef boost::property_map<VertexListGraphType, boost::vertex_index_t>::const_type IndexMapType;
  IndexMapType indexMap(get(boost::vertex_index, graph));

  // Create the descriptor map. This is where the data for each pixel is stored.
  typedef boost::vector_property_map<ImagePatchPixelDescriptorType, IndexMapType> ImagePatchDescriptorMapType;
  ImagePatchDescriptorMapType imagePatchDescriptorMap(num_vertices(graph), indexMap);

  // Create the patch inpainter.
  typedef PatchInpainter<OriginalImageType> OriginalImageInpainterType;
  OriginalImageInpainterType originalImagePatchInpainter(patchHalfWidth, originalImage, mask);
  originalImagePatchInpainter.SetDebugImages(true);
  originalImagePatchInpainter.SetImageName("RGB");

  // Create an inpainter for the HSV image.
  typedef PatchInpainter<HSVImageType> HSVImageInpainterType;
  HSVImageInpainterType hsvImagePatchInpainter(patchHalfWidth, hsvImage, mask);

  // Create an inpainter for the blurred image.
  typedef PatchInpainter<BlurredImageType> BlurredImageInpainterType;
  BlurredImageInpainterType blurredImagePatchInpainter(patchHalfWidth, blurredImage, mask);

  // Create a composite inpainter.
  CompositePatchInpainter inpainter;
  inpainter.AddInpainter(&originalImagePatchInpainter);
  inpainter.AddInpainter(&hsvImagePatchInpainter);
  inpainter.AddInpainter(&blurredImagePatchInpainter);

  // Create the priority function
  typedef PriorityCriminisi<BlurredImageType> PriorityType;
  PriorityType priorityFunction(blurredImage, mask, patchHalfWidth);
//  priorityFunction.SetDebugLevel(1);

  // Queue
  typedef IndirectPriorityQueue<VertexListGraphType> BoundaryNodeQueueType;
  BoundaryNodeQueueType boundaryNodeQueue(graph);

  // Create the descriptor visitor (RGB). This is one thing that would need to be changed to use HSV pixel comparisons
  // (along with the template paramater of the PatchDifferenceType defined further below).
  typedef ImagePatchDescriptorVisitor<VertexListGraphType, OriginalImageType, ImagePatchDescriptorMapType>
      ImagePatchDescriptorVisitorType;
  ImagePatchDescriptorVisitorType
      imagePatchDescriptorVisitor(originalImage, mask,
                                  imagePatchDescriptorMap, patchHalfWidth);

  typedef DefaultAcceptanceVisitor<VertexListGraphType> AcceptanceVisitorType;
  AcceptanceVisitorType acceptanceVisitor;

  // Create the inpainting visitor. (The mask is inpainted in FinishVertex)
  typedef InpaintingVisitor<VertexListGraphType, OriginalImageType, BoundaryNodeQueueType,
      ImagePatchDescriptorVisitorType, AcceptanceVisitorType, PriorityType>
      InpaintingVisitorType;
  InpaintingVisitorType inpaintingVisitor(originalImage, mask, boundaryNodeQueue,
                                          imagePatchDescriptorVisitor, acceptanceVisitor,
                                          &priorityFunction, patchHalfWidth,
                                          outputFileName);
  inpaintingVisitor.SetDebugImages(true);
  inpaintingVisitor.SetAllowNewPatches(true);

  InitializePriority(mask, boundaryNodeQueue, &priorityFunction);

  // Initialize the boundary node queue from the user provided mask image.
  InitializeFromMaskImage<InpaintingVisitorType, VertexDescriptorType>(mask, &inpaintingVisitor);
  std::cout << "PatchBasedInpaintingNonInteractive: There are " << boundaryNodeQueue.CountValidNodes()
            << " nodes in the boundaryNodeQueue" << std::endl;

  // Select squared or absolute pixel error
//  typedef ImagePatchDifference<ImagePatchPixelDescriptorType, SumAbsolutePixelDifference<OriginalImageType::PixelType> > PatchDifferenceType;
  typedef ImagePatchDifference<ImagePatchPixelDescriptorType,
      SumSquaredPixelDifference<OriginalImageType::PixelType> > PatchDifferenceType;

  // Create the first (KNN) neighbor finder
  typedef LinearSearchKNNProperty<ImagePatchDescriptorMapType, PatchDifferenceType> KNNSearchType;
  KNNSearchType linearSearchKNN(imagePatchDescriptorMap, numberOfKNN);

  typedef LinearSearchBestStrategySelection<ImagePatchDescriptorMapType, OriginalImageType> BestSearchType;
  BestSearchType linearSearchBest(imagePatchDescriptorMap, originalImage, mask);

  typedef LinearSearchBestFirstAndWrite<ImagePatchDescriptorMapType,
      OriginalImageType, PatchDifferenceType> TopPatchesWriterType;
  TopPatchesWriterType topPatchesWriter(imagePatchDescriptorMap, originalImage, mask);

  // Setup the two step neighbor finder
//  TwoStepNearestNeighbor<KNNSearchType, BestSearchType>
//      twoStepNearestNeighbor(linearSearchKNN, linearSearchBest);

  TwoStepNearestNeighbor<KNNSearchType, BestSearchType, TopPatchesWriterType>
      twoStepNearestNeighbor(linearSearchKNN, linearSearchBest, topPatchesWriter);

  // For debugging, look at the initial priority queue
//  PatchHelpers::DumpQueue(boundaryNodeQueue, boundaryStatusMap, priorityMap);

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
