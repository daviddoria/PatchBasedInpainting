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
#include "NearestNeighbor/LinearSearchBestHistogramCorrelation.hpp"
#include "NearestNeighbor/LinearSearchBestHistogramDifference.hpp"
#include "NearestNeighbor/LinearSearchBestHoleHistogramDifference.hpp"
#include "NearestNeighbor/LinearSearchBestDualHistogramDifference.hpp"
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

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/detail/d_ary_heap.hpp>

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

  typedef  itk::ImageFileReader<OriginalImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFileName);
  imageReader->Update();

  OriginalImageType* originalImage = imageReader->GetOutput();

  itk::ImageRegion<2> fullRegion = originalImage->GetLargestPossibleRegion();

//  ImageType::Pointer image = ImageType::New();
//  ITKHelpers::DeepCopy(imageReader->GetOutput(), image.GetPointer());

  // Convert the image to HSV
  typedef itk::Image<itk::CovariantVector<float, 3>, 2> HSVImageType;
  HSVImageType::Pointer hsvImage = HSVImageType::New();
  //ITKVTKHelpers::ConvertRGBtoHSV(image.GetPointer(), hsvImage.GetPointer());
  ITKVTKHelpers::ConvertRGBtoHSV(originalImage, hsvImage.GetPointer());

  ITKHelpers::WriteImage(hsvImage.GetPointer(), "HSVImage.mha");

  // Read the mask
  Mask::Pointer mask = Mask::New();
  mask->Read(maskFileName);

  std::cout << "hole pixels: " << mask->CountHolePixels() << std::endl;
  std::cout << "valid pixels: " << mask->CountValidPixels() << std::endl;

  typedef ImagePatchPixelDescriptor<OriginalImageType> ImagePatchPixelDescriptorType;

  // Create the graph
  typedef boost::grid_graph<2> VertexListGraphType;
  boost::array<std::size_t, 2> graphSideLengths = { { fullRegion.GetSize()[0],
                                                      fullRegion.GetSize()[1] } };
  VertexListGraphType graph(graphSideLengths);
  typedef boost::graph_traits<VertexListGraphType>::vertex_descriptor VertexDescriptorType;
//  typedef boost::graph_traits<VertexListGraphType>::vertex_iterator VertexIteratorType;

  // Get the index map
  typedef boost::property_map<VertexListGraphType, boost::vertex_index_t>::const_type IndexMapType;
  IndexMapType indexMap(get(boost::vertex_index, graph));

  // Create the priority map
  typedef boost::vector_property_map<float, IndexMapType> PriorityMapType;
  PriorityMapType priorityMap(num_vertices(graph), indexMap);

  // Create the boundary status map. A node is on the current boundary if this property is true.
  // This property helps the boundaryNodeQueue because we can mark here if a node has become no longer
  // part of the boundary, so when the queue is popped we can check this property to see if it should
  // actually be processed.
  typedef boost::vector_property_map<bool, IndexMapType> BoundaryStatusMapType;
  BoundaryStatusMapType boundaryStatusMap(num_vertices(graph), indexMap);

  // Create the descriptor map. This is where the data for each pixel is stored.
  typedef boost::vector_property_map<ImagePatchPixelDescriptorType, IndexMapType> ImagePatchDescriptorMapType;
  ImagePatchDescriptorMapType imagePatchDescriptorMap(num_vertices(graph), indexMap);

  // Create the patch inpainter. The inpainter needs to know the status of each pixel to determine if they should be inpainted.
  typedef PatchInpainter<OriginalImageType> OriginalImageInpainterType;
  OriginalImageInpainterType originalImagePatchInpainter(patchHalfWidth, originalImage, mask);
  originalImagePatchInpainter.SetDebugImages(true);
  originalImagePatchInpainter.SetImageName("RGB");

  typedef PatchInpainter<HSVImageType> HSVImageInpainterType;
  HSVImageInpainterType hsvImagePatchInpainter(patchHalfWidth, hsvImage, mask);

  CompositePatchInpainter inpainter;
  inpainter.AddInpainter(&originalImagePatchInpainter);
  inpainter.AddInpainter(&hsvImagePatchInpainter);

  // Create the priority function
//  typedef PriorityRandom PriorityType;
//  bool random = false;
//  PriorityType priorityFunction(random);

//  typedef PriorityCriminisi<ImageType> PriorityType;
//  PriorityType priorityFunction(image, mask, patchHalfWidth);

  typedef PriorityOnionPeel PriorityType;
  PriorityType priorityFunction(mask, patchHalfWidth);

  // Create the boundary node queue. The priority of each node is used to order the queue.
  typedef boost::vector_property_map<std::size_t, IndexMapType> IndexInHeapMap;
  IndexInHeapMap index_in_heap(indexMap);

  // Create the priority compare functor (we want to use the highest priority pixels first, so the std::greater functor sorts the queue
  // such that the highest values (highest priorities) are first in the queue)
  typedef std::greater<float> PriorityCompareType;
  PriorityCompareType queueSortFunctor;

  typedef boost::d_ary_heap_indirect<VertexDescriptorType, 4, IndexInHeapMap, PriorityMapType, PriorityCompareType>
      BoundaryNodeQueueType;
  BoundaryNodeQueueType boundaryNodeQueue(priorityMap, index_in_heap, queueSortFunctor);

  // Create the descriptor visitor
  typedef ImagePatchDescriptorVisitor<VertexListGraphType, OriginalImageType, ImagePatchDescriptorMapType>
      ImagePatchDescriptorVisitorType;
  ImagePatchDescriptorVisitorType imagePatchDescriptorVisitor(originalImage, mask, imagePatchDescriptorMap, patchHalfWidth);

  typedef DefaultAcceptanceVisitor<VertexListGraphType> AcceptanceVisitorType;
  AcceptanceVisitorType acceptanceVisitor;

  // Create the inpainting visitor
  typedef InpaintingVisitor<VertexListGraphType, OriginalImageType, BoundaryNodeQueueType,
      ImagePatchDescriptorVisitorType, AcceptanceVisitorType, PriorityType,
      PriorityMapType, BoundaryStatusMapType>
      InpaintingVisitorType;
  InpaintingVisitorType inpaintingVisitor(originalImage, mask, boundaryNodeQueue,
                                          imagePatchDescriptorVisitor, acceptanceVisitor, priorityMap,
                                          &priorityFunction, patchHalfWidth,
                                          boundaryStatusMap, outputFileName);
  inpaintingVisitor.SetDebugImages(true);

  InitializePriority(mask, boundaryNodeQueue, priorityMap, &priorityFunction, boundaryStatusMap);

  // Initialize the boundary node queue from the user provided mask image.
  InitializeFromMaskImage<InpaintingVisitorType, VertexDescriptorType>(mask, &inpaintingVisitor);
  std::cout << "PatchBasedInpaintingNonInteractive: There are " << boundaryNodeQueue.size()
            << " nodes in the boundaryNodeQueue" << std::endl;

  // Select squared or absolute pixel error
//  typedef ImagePatchDifference<ImagePatchPixelDescriptorType, SumAbsolutePixelDifference<OriginalImageType::PixelType> > PatchDifferenceType;
  typedef ImagePatchDifference<ImagePatchPixelDescriptorType, SumSquaredPixelDifference<OriginalImageType::PixelType> > PatchDifferenceType;

  // Create the  neighbor finder
  typedef LinearSearchKNNProperty<ImagePatchDescriptorMapType, PatchDifferenceType> KNNSearchType;
  KNNSearchType linearSearchKNN(imagePatchDescriptorMap, numberOfKNN);

  typedef std::vector<VertexDescriptorType>::iterator VertexDescriptorVectorIteratorType;

  // This is templated on OriginalImageType because we need it to write out debug patches from this searcher (since we are not using an RGB image to compute the histograms)
  //typedef LinearSearchBestHistogram<ImagePatchDescriptorMapType, HSVImageType, OriginalImageType> BestSearchType;
//  typedef LinearSearchBestHistogramDifference<ImagePatchDescriptorMapType, HSVImageType, VertexIteratorType, OriginalImageType> BestSearchType;
//    typedef LinearSearchBestHistogramDifference<ImagePatchDescriptorMapType, HSVImageType, VertexDescriptorVectorIteratorType, OriginalImageType> BestSearchType;
//  typedef LinearSearchBestHoleHistogramDifference<ImagePatchDescriptorMapType, HSVImageType, VertexDescriptorVectorIteratorType, OriginalImageType> BestSearchType;
  typedef LinearSearchBestDualHistogramDifference<ImagePatchDescriptorMapType, HSVImageType, VertexDescriptorVectorIteratorType, OriginalImageType> BestSearchType;

  BestSearchType linearSearchBest(imagePatchDescriptorMap, hsvImage.GetPointer(), mask);
  linearSearchBest.SetNumberOfBinsPerDimension(binsPerChannel);
  // The range (0,1) is used because we use the HSV image.
  linearSearchBest.SetRangeMin(0.0f);
  linearSearchBest.SetRangeMax(1.0f);
  linearSearchBest.SetWriteDebugPatches(true, originalImage);

  TwoStepNearestNeighbor<KNNSearchType, BestSearchType> twoStepNearestNeighbor(linearSearchKNN, linearSearchBest);

  // Perform the inpainting
  InpaintingAlgorithm(graph, inpaintingVisitor, &boundaryStatusMap, &boundaryNodeQueue, twoStepNearestNeighbor, &inpainter);

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
