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

// ITK
#include "itkImageFileReader.h"

// Driver
#include "Drivers/ClassicalImageInpaintingDebug.hpp"


// Custom
#include "Utilities/IndirectPriorityQueue.h"

// STL
#include <memory>

// Information visitors
#include "Visitors/InformationVisitors/PatchIndicatorVisitor.hpp"
#include "Visitors/InformationVisitors/IterationWriterVisitor.hpp"
#include "Visitors/InformationVisitors/TargetPatchWriterVisitor.hpp"

// Submodules
#include <Helpers/Helpers.h>

// Pixel descriptors
#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

// Descriptor visitors
#include "Visitors/DescriptorVisitors/ImagePatchDescriptorVisitor.hpp"

// Inpainting visitors
#include "Visitors/InpaintingVisitors/InpaintingVisitor.hpp"
#include "Visitors/AcceptanceVisitors/DefaultAcceptanceVisitor.hpp"
#include "Visitors/InpaintingVisitors/CompositeInpaintingVisitor.hpp"

// Nearest neighbors
#include "NearestNeighbor/LinearSearchBest/Property.hpp"
#include "NearestNeighbor/LinearSearchBest/FirstAndWrite.hpp"
#include "NearestNeighbor/LinearSearchKNNProperty.hpp"
#include "NearestNeighbor/KNNBestWrapper.hpp"

// Initializers
#include "Initializers/InitializeFromMaskImage.hpp"
#include "Initializers/InitializePriority.hpp"

// Inpainters
#include "Inpainters/CompositePatchInpainter.hpp"
#include "Inpainters/PatchInpainter.hpp"

// Difference functions
#include "DifferenceFunctions/Patch/ImagePatchDifference.hpp"
#include "DifferenceFunctions/Pixel/SumSquaredPixelDifference.hpp"

// Inpainting
#include "Algorithms/InpaintingAlgorithm.hpp"

// Priority
#include "Priority/PriorityCriminisi.h"
#include "Priority/PriorityRandom.h"
#include "Priority/PriorityConfidence.h"

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>

int main(int argc, char *argv[])
{
  // Verify arguments
  if(argc != 5)
  {
    std::cerr << "Required arguments: image.png imageMask.mask patchHalfWidth targetPatch.png" << std::endl;
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

  std::string targetPatchFileName = argv[4];

  // Output arguments
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;
  std::cout << "Patch half width: " << patchHalfWidth << std::endl;
  std::cout << "targetPatchFileName: " << targetPatchFileName << std::endl;

  typedef itk::Image<itk::CovariantVector<int, 3>, 2> OriginalImageType;

  typedef  itk::ImageFileReader<OriginalImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

  OriginalImageType* originalImage = imageReader->GetOutput();

  Mask::Pointer mask = Mask::New();
  mask->Read(maskFilename);

  itk::ImageRegion<2> fullRegion = originalImage->GetLargestPossibleRegion();

  // Blur the image
//  typedef TImage BlurredImageType; // Usually the blurred image is the same type as the original image.
//  typename BlurredImageType::Pointer blurredImage = BlurredImageType::New();
//  float blurVariance = 2.0f;
////  float blurVariance = 1.2f;
//  MaskOperations::MaskedBlur(originalImage.GetPointer(), mask, blurVariance, blurredImage.GetPointer());

//  ITKHelpers::WriteRGBImage(blurredImage.GetPointer(), "BlurredImage.png");

  typedef ImagePatchPixelDescriptor<OriginalImageType> ImagePatchPixelDescriptorType;

  // Create the graph
  typedef boost::grid_graph<2> VertexListGraphType;
  boost::array<std::size_t, 2> graphSideLengths = { { fullRegion.GetSize()[0],
                                                      fullRegion.GetSize()[1] } };
  std::shared_ptr<VertexListGraphType> graph(new VertexListGraphType(graphSideLengths));
  typedef boost::graph_traits<VertexListGraphType>::vertex_descriptor VertexDescriptorType;
  typedef boost::graph_traits<VertexListGraphType>::vertex_iterator VertexIteratorType;

  // Queue
  typedef IndirectPriorityQueue<VertexListGraphType> BoundaryNodeQueueType;
  std::shared_ptr<BoundaryNodeQueueType> boundaryNodeQueue(new BoundaryNodeQueueType(*graph));

  // Create the descriptor map. This is where the data for each pixel is stored.
  typedef boost::vector_property_map<ImagePatchPixelDescriptorType,
      BoundaryNodeQueueType::IndexMapType> ImagePatchDescriptorMapType;
  std::shared_ptr<ImagePatchDescriptorMapType> imagePatchDescriptorMap(new
      ImagePatchDescriptorMapType(num_vertices(*graph), *(boundaryNodeQueue->GetIndexMap())));

  // Create the descriptor visitor
  typedef ImagePatchDescriptorVisitor<VertexListGraphType, OriginalImageType, ImagePatchDescriptorMapType>
      ImagePatchDescriptorVisitorType;
  std::shared_ptr<ImagePatchDescriptorVisitorType> imagePatchDescriptorVisitor(new
      ImagePatchDescriptorVisitorType(originalImage, mask,
//        ImagePatchDescriptorVisitorType(blurredImage.GetPointer(), mask,
                                      imagePatchDescriptorMap, patchHalfWidth));

  // Create the inpainting visitor
//  typedef InpaintingVisitor<VertexListGraphType, BoundaryNodeQueueType,
//                            ImagePatchDescriptorVisitorType, AcceptanceVisitorType, PriorityType>
//                            InpaintingVisitorType;
//  std::shared_ptr<InpaintingVisitorType> inpaintingVisitor(new InpaintingVisitorType(mask, boundaryNodeQueue,
//                                          imagePatchDescriptorVisitor, acceptanceVisitor,
//                                          priorityFunction, patchHalfWidth, "InpaintingVisitor"));
//  inpaintingVisitor->SetAllowNewPatches(false);

//  // Initialize the boundary node queue from the user provided mask image.
//  InitializeFromMaskImage<InpaintingVisitorType, VertexDescriptorType>(mask, inpaintingVisitor.get());

//  // Create the nearest neighbor finder
//  typedef ImagePatchDifference<ImagePatchPixelDescriptorType,
//      SumSquaredPixelDifference<typename TImage::PixelType> > PatchDifferenceType;

//  // Write top patch grid at each iteration. To do this, we need a KNNSearcher
//  // to pass a list of valid patches to the FirstAndWrite class.
//  typedef LinearSearchKNNProperty<ImagePatchDescriptorMapType,
//                                  PatchDifferenceType > KNNSearchType;

//  unsigned int knn = 100;
//  std::shared_ptr<KNNSearchType> knnSearch(new KNNSearchType(imagePatchDescriptorMap, knn));

//  typedef LinearSearchBestFirstAndWrite<ImagePatchDescriptorMapType, TImage,
//                                   PatchDifferenceType> BestSearchType;
//  std::shared_ptr<BestSearchType> linearSearchBest(
//        new BestSearchType(*imagePatchDescriptorMap, originalImage, mask));

////  typedef KNNBestWrapper<KNNSearchType, BestSearchType> KNNWrapperType;
////  std::shared_ptr<KNNWrapperType> knnWrapper(new KNNWrapperType(knnSearch,
////                                                                linearSearchBest));


  return EXIT_SUCCESS;
}
