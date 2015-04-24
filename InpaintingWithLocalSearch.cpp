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

// Helpers
//#include "Helpers/OutputHelpers.h"
// Custom
#include "Utilities/IndirectPriorityQueue.h"

#include "SearchRegions/NeighborhoodSearch.hpp"

#include "NearestNeighbor/LinearSearchBest/Property.hpp"

// Pixel descriptors
#include "PixelDescriptors/ImagePatchPixelDescriptor.h"
#include "PixelDescriptors/ImagePatchVectorized.h"
#include "PixelDescriptors/ImagePatchVectorizedIndices.h"

// Acceptance visitors
#include "Visitors/AcceptanceVisitors/AlwaysAccept.hpp"
#include "Visitors/AcceptanceVisitors/DefaultAcceptanceVisitor.hpp"
#include "Visitors/AcceptanceVisitors/AverageDifferenceAcceptanceVisitor.hpp"
#include "Visitors/AcceptanceVisitors/CompositeAcceptanceVisitor.hpp"
#include "Visitors/AcceptanceVisitors/ANDAcceptanceVisitor.hpp"
#include "Visitors/AcceptanceVisitors/DilatedSourceHoleTargetValidAcceptanceVisitor.hpp"
#include "Visitors/AcceptanceVisitors/DilatedSourceValidTargetValidAcceptanceVisitor.hpp"
#include "Visitors/AcceptanceVisitors/SourceHoleTargetValidCompare.hpp"
#include "Visitors/AcceptanceVisitors/SourceValidTargetValidCompare.hpp"
#include "Visitors/AcceptanceVisitors/HoleSizeAcceptanceVisitor.hpp"
#include "Visitors/AcceptanceVisitors/VarianceFunctor.hpp"
#include "Visitors/AcceptanceVisitors/AverageFunctor.hpp"
#include "Visitors/AcceptanceVisitors/ScoreThresholdAcceptanceVisitor.hpp"
#include "Visitors/AcceptanceVisitors/CorrelationAcceptanceVisitor.hpp"
#include "Visitors/AcceptanceVisitors/PatchDistanceAcceptanceVisitor.hpp"
#include "Visitors/AcceptanceVisitors/HistogramDifferenceAcceptanceVisitor.hpp"
#include "Visitors/AcceptanceVisitors/HoleHistogramDifferenceAcceptanceVisitor.hpp"
// #include "Visitors/AcceptanceVisitors/QuadrantHistogramCompareAcceptanceVisitor.hpp"
#include "Visitors/AcceptanceVisitors/AllQuadrantHistogramCompareAcceptanceVisitor.hpp"

// Descriptor visitors
#include "Visitors/DescriptorVisitors/ImagePatchDescriptorVisitor.hpp"
#include "Visitors/DescriptorVisitors/ImagePatchVectorizedIndicesVisitor.hpp"
#include "Visitors/DescriptorVisitors/ImagePatchVectorizedVisitor.hpp"
#include "Visitors/DescriptorVisitors/CompositeDescriptorVisitor.hpp"

// Inpainting visitors
#include "Visitors/InpaintingVisitors/InpaintingVisitor.hpp"
#include "Visitors/ReplayVisitor.hpp"
#include "Visitors/InformationVisitors/LoggerVisitor.hpp"
//#include "Visitors/CompositeInpaintingVisitor.hpp"
#include "Visitors/InformationVisitors/DebugVisitor.hpp"

// Nearest neighbors
#include "NearestNeighbor/LinearSearchKNNProperty.hpp"
#include "NearestNeighbor/DefaultSearchBest.hpp"
//#include "NearestNeighbor/LinearSearchBestProperty.hpp"
//#include "NearestNeighbor/VisualSelectionBest.hpp"

// Nearest neighbors visitor
#include "Visitors/NearestNeighborsDisplayVisitor.hpp"

// Initializers
#include "Initializers/InitializeFromMaskImage.hpp"
#include "Initializers/InitializePriority.hpp"

// Inpainters
#include "Inpainters/PatchInpainter.hpp"

// Difference functions
#include "DifferenceFunctions/Patch/ImagePatchDifference.hpp"
#include "DifferenceFunctions/Pixel/SumAbsolutePixelDifference.hpp"

// Inpainting algorithm
#include "Algorithms/InpaintingAlgorithmWithLocalSearch.hpp"

// Priority
#include "Priority/PriorityRandom.h"
#include "Priority/PriorityCriminisi.h"

// ITK
#include "itkImageFileReader.h"

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/detail/d_ary_heap.hpp>

// Run with: Data/trashcan.mha Data/trashcan_mask.mha 15 filled.mha
int main(int argc, char *argv[])
{
  // Verify arguments
  if(argc != 6)
    {
    std::cerr << "Required arguments: image.mha imageMask.mha patchHalfWidth neighborhoodRadius output.mha" << std::endl;
    std::cerr << "Input arguments: ";
    for(int i = 1; i < argc; ++i)
      {
      std::cerr << argv[i] << " ";
      }
    return EXIT_FAILURE;
    }

  // Parse arguments
  std::string imageFileName = argv[1];
  std::string maskFileName = argv[2];

  std::stringstream ssPatchRadius;
  ssPatchRadius << argv[3];
  unsigned int patchHalfWidth = 0;
  ssPatchRadius >> patchHalfWidth;

  // The percent of the image size to use as the neighborhood (0 - 1)
  std::stringstream ssNeighborhoodPercent;
  ssNeighborhoodPercent << argv[4];
  float neighborhoodPercent = 0;
  ssNeighborhoodPercent >> neighborhoodPercent;

  std::string outputFileName = argv[5];

  // Output arguments
  std::cout << "Reading image: " << imageFileName << std::endl;
  std::cout << "Reading mask: " << maskFileName << std::endl;
  std::cout << "Patch half width: " << patchHalfWidth << std::endl;
  std::cout << "Neighborhood percent: " << neighborhoodPercent << std::endl;
  std::cout << "Output: " << outputFileName << std::endl;

  typedef itk::Image<itk::CovariantVector<int, 3>, 2> ImageType;

  typedef  itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFileName);
  imageReader->Update();

  ImageType::Pointer image = ImageType::New();
  ITKHelpers::DeepCopy(imageReader->GetOutput(), image.GetPointer());

  Mask::Pointer mask = Mask::New();
  mask->Read(maskFileName);

  std::cout << "hole pixels: " << mask->CountHolePixels() << std::endl;
  std::cout << "valid pixels: " << mask->CountValidPixels() << std::endl;

  typedef ImagePatchPixelDescriptor<ImageType> ImagePatchPixelDescriptorType;

  // Create the graph
  typedef boost::grid_graph<2> VertexListGraphType;
  boost::array<std::size_t, 2> graphSideLengths = { { imageReader->GetOutput()->GetLargestPossibleRegion().GetSize()[0],
                                                      imageReader->GetOutput()->GetLargestPossibleRegion().GetSize()[1] } };
//  VertexListGraphType graph(graphSideLengths);
  std::shared_ptr<VertexListGraphType> graph(new VertexListGraphType(graphSideLengths));
  typedef boost::graph_traits<VertexListGraphType>::vertex_descriptor VertexDescriptorType;


  //ImagePatchDescriptorMapType smallImagePatchDescriptorMap(num_vertices(graph), indexMap);

  // Create the patch inpainter. The inpainter needs to know the status of each pixel to determine if they should be inpainted.

  typedef PatchInpainter<ImageType> ImageInpainterType;
  std::shared_ptr<ImageInpainterType> imagePatchInpainter(new
      ImageInpainterType(patchHalfWidth, image, mask));

  // Create the priority function
   typedef PriorityRandom PriorityType;
   std::shared_ptr<PriorityType> priorityFunction(new PriorityType);
//  typedef PriorityCriminisi<ImageType> PriorityType;
//  std::shared_ptr<PriorityType> priorityFunction(new PriorityType(image, mask, patchHalfWidth));

  typedef IndirectPriorityQueue<VertexListGraphType> BoundaryNodeQueueType;
  std::shared_ptr<BoundaryNodeQueueType> boundaryNodeQueue(new BoundaryNodeQueueType(*graph));

  // Create the descriptor map. This is where the data for each pixel is stored.
  typedef boost::vector_property_map<ImagePatchPixelDescriptorType, BoundaryNodeQueueType::IndexMapType> ImagePatchDescriptorMapType;
//  ImagePatchDescriptorMapType imagePatchDescriptorMap(num_vertices(graph), indexMap);
  std::shared_ptr<ImagePatchDescriptorMapType> imagePatchDescriptorMap(new
      ImagePatchDescriptorMapType(num_vertices(*graph), *(boundaryNodeQueue->GetIndexMap())));

  // Create the descriptor visitor
  typedef ImagePatchDescriptorVisitor<VertexListGraphType, ImageType, ImagePatchDescriptorMapType>
          ImagePatchDescriptorVisitorType;
//  ImagePatchDescriptorVisitorType imagePatchDescriptorVisitor(image, mask, imagePatchDescriptorMap, patchHalfWidth);
  std::shared_ptr<ImagePatchDescriptorVisitorType> imagePatchDescriptorVisitor(new
      ImagePatchDescriptorVisitorType(image.GetPointer(), mask,
                                      imagePatchDescriptorMap, patchHalfWidth));
/*   ImagePatchDescriptorVisitor(TImage* const in_image, Mask* const in_mask,
  std::shared_ptr<TDescriptorMap> in_descriptorMap,
  const unsigned int in_half_width) : */
  typedef ImagePatchDifference<ImagePatchPixelDescriptorType, SumAbsolutePixelDifference<ImageType::PixelType> >
            ImagePatchDifferenceType;

//  typedef CompositeDescriptorVisitor<VertexListGraphType> CompositeDescriptorVisitorType;
//  CompositeDescriptorVisitorType compositeDescriptorVisitor;
//  compositeDescriptorVisitor.AddVisitor(imagePatchDescriptorVisitor);

  // Create the descriptor visitor


//  typedef CompositeAcceptanceVisitor<VertexListGraphType> CompositeAcceptanceVisitorType;
//  CompositeAcceptanceVisitorType compositeAcceptanceVisitor;

  typedef DefaultAcceptanceVisitor<VertexListGraphType> AcceptanceVisitorType;
  std::shared_ptr<AcceptanceVisitorType> acceptanceVisitor(new AcceptanceVisitorType);

//  typedef AlwaysAccept<VertexListGraphType> AcceptanceVisitorType;
//  AcceptanceVisitorType acceptanceVisitor;



  // If the hole is less than 15% of the patch, always accept the initial best match
//  HoleSizeAcceptanceVisitor<VertexListGraphType> holeSizeAcceptanceVisitor(mask, patchHalfWidth, .15);
//  compositeAcceptanceVisitor.AddOverrideVisitor(&holeSizeAcceptanceVisitor);

//  AllQuadrantHistogramCompareAcceptanceVisitor<VertexListGraphType, ImageType>
//               allQuadrantHistogramCompareAcceptanceVisitor(image, mask, patchHalfWidth, 8.0f); // Crazy low
//  compositeAcceptanceVisitor.AddRequiredPassVisitor(&allQuadrantHistogramCompareAcceptanceVisitor);

//  template <typename TGraph, typename TBoundaryNodeQueue,
//            typename TDescriptorVisitor, typename TAcceptanceVisitor, typename TPriority>

  typedef InpaintingVisitor<VertexListGraphType, BoundaryNodeQueueType,
                            ImagePatchDescriptorVisitorType, AcceptanceVisitorType, PriorityType>
                            InpaintingVisitorType;
  std::shared_ptr<InpaintingVisitorType> inpaintingVisitor(new InpaintingVisitorType(mask, boundaryNodeQueue,
                                                                                     imagePatchDescriptorVisitor, acceptanceVisitor,
                                                                                     priorityFunction, patchHalfWidth, "InpaintingVisitor"));

//  typedef InpaintingVisitor<VertexListGraphType, BoundaryNodeQueueType,
//                            ImagePatchDescriptorVisitorType, AcceptanceVisitorType, PriorityType>
//                            InpaintingVisitorType;
//  std::shared_ptr<InpaintingVisitorType> inpaintingVisitor(new InpaintingVisitorType(mask, boundaryNodeQueue,
//                                          imagePatchDescriptorVisitor, acceptanceVisitor,
//                                          priorityFunction, patchHalfWidth, "InpaintingVisitor"));

//  typedef DebugVisitor<VertexListGraphType, ImageType, BoundaryStatusMapType, BoundaryNodeQueueType> DebugVisitorType;
//  DebugVisitorType debugVisitor(image, mask, patchHalfWidth, boundaryStatusMap, boundaryNodeQueue);

  LoggerVisitor<VertexListGraphType> loggerVisitor("log.txt");

  InitializePriority(mask, boundaryNodeQueue.get(), priorityFunction.get());
  // Initialize the boundary node queue from the user provided mask image.
  InitializeFromMaskImage<InpaintingVisitorType, VertexDescriptorType>(mask, inpaintingVisitor.get());

  // For debugging we use LinearSearchBestProperty instead of DefaultSearchBest because it can output the difference value.
  typedef LinearSearchBestProperty<ImagePatchDescriptorMapType,
                                   ImagePatchDifferenceType > BestSearchType;
  std::shared_ptr<BestSearchType> linearSearchBest(new BestSearchType(*imagePatchDescriptorMap));

  typedef NeighborhoodSearch<VertexDescriptorType, ImagePatchDescriptorMapType> NeighborhoodSearchType;
  NeighborhoodSearchType neighborhoodSearch(image->GetLargestPossibleRegion(), image->GetLargestPossibleRegion().GetSize()[0] * neighborhoodPercent, *imagePatchDescriptorMap);
  
  InpaintingAlgorithmWithLocalSearch<VertexListGraphType, InpaintingVisitorType,
                      BoundaryNodeQueueType, NeighborhoodSearchType,
                      ImageInpainterType, BestSearchType>(graph, inpaintingVisitor, boundaryNodeQueue,
                      linearSearchBest, imagePatchInpainter, neighborhoodSearch);

  // If the output filename is a png file, then use the RGBImage writer so that it is first
  // casted to unsigned char. Otherwise, write the file directly.
  if(Helpers::GetFileExtension(outputFileName) == "png")
  {
    ITKHelpers::WriteRGBImage(image.GetPointer(), outputFileName);
  }
  else
  {
    ITKHelpers::WriteImage(image.GetPointer(), outputFileName);
  }

  return EXIT_SUCCESS;
}
