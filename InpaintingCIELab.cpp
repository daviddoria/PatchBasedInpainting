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
#include "Helpers/OutputHelpers.h"

#include "SearchRegions/NeighborhoodSearch.hpp"

// Pixel descriptors
#include "PixelDescriptors/ImagePatchPixelDescriptor.h"
#include "PixelDescriptors/ImagePatchVectorized.h"
#include "PixelDescriptors/ImagePatchVectorizedIndices.h"

// Acceptance visitors
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

// Information visitors
#include "Visitors/InformationVisitors/DisplayVisitor.hpp"

// Inpainting visitors
#include "Visitors/InpaintingVisitor.hpp"
#include "Visitors/ReplayVisitor.hpp"
#include "Visitors/InformationVisitors/LoggerVisitor.hpp"
#include "Visitors/CompositeInpaintingVisitor.hpp"
#include "Visitors/InformationVisitors/DebugVisitor.hpp"

// Nearest neighbors
#include "NearestNeighbor/LinearSearchKNNProperty.hpp"
#include "NearestNeighbor/DefaultSearchBest.hpp"
#include "NearestNeighbor/LinearSearchBestProperty.hpp"
#include "NearestNeighbor/VisualSelectionBest.hpp"

// Nearest neighbors visitor
#include "Visitors/NearestNeighborsDisplayVisitor.hpp"

// Initializers
#include "Initializers/InitializeFromMaskImage.hpp"
#include "Initializers/InitializePriority.hpp"

// Inpainters
//#include "Inpainters/MaskedGridPatchInpainter.hpp"
#include "Inpainters/MaskImagePatchInpainter.hpp"

// Difference functions
#include "DifferenceFunctions/ImagePatchDifference.hpp"
#include "DifferenceFunctions/ImagePatchVectorizedDifference.hpp"
#include "DifferenceFunctions/ImagePatchVectorizedIndicesDifference.hpp"
#include "DifferenceFunctions/SumAbsolutePixelDifference.hpp"
#include "DifferenceFunctions/WeightedSumAbsolutePixelDifference.hpp"
#include "DifferenceFunctions/WeightedFeatureVectorDifference.hpp"

// Inpainting algorithm
#include "Algorithms/InpaintingAlgorithmWithLocalSearch.hpp"

// Priority
#include "Priority/PriorityRandom.h"
#include "Priority/PriorityOnionPeel.h"

// ITK
#include "itkImageFileReader.h"

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/detail/d_ary_heap.hpp>

// Debug
#include "Helpers/OutputHelpers.h"

// Qt
#include <QApplication>
#include <QtConcurrentRun>

// GUI
#include "Interactive/BasicViewerWidget.h"
#include "Interactive/TopPatchesWidget.h"
#include "Interactive/TopPatchesDialog.h"
#include "Interactive/PriorityViewerWidget.h"

// Run with: Data/trashcan.mha Data/trashcan_mask.mha 15 filled.mha
int main(int argc, char *argv[])
{
  // Verify arguments
  if(argc != 5)
    {
    std::cerr << "Required arguments: image.mha imageMask.mha patchHalfWidth output.mha" << std::endl;
    std::cerr << "Input arguments: ";
    for(int i = 1; i < argc; ++i)
      {
      std::cerr << argv[i] << " ";
      }
    return EXIT_FAILURE;
    }

  // Setup the GUI system
  QApplication app( argc, argv );

  // Parse arguments
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];

  std::stringstream ssPatchRadius;
  ssPatchRadius << argv[3];
  unsigned int patchHalfWidth = 0;
  ssPatchRadius >> patchHalfWidth;

  std::string outputFilename = argv[4];

  // Output arguments
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;
  std::cout << "Patch half width: " << patchHalfWidth << std::endl;
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

  std::cout << "hole pixels: " << mask->CountHolePixels() << std::endl;
  std::cout << "valid pixels: " << mask->CountValidPixels() << std::endl;

  // Convert the image from RGB to CIELab
  FloatVectorImageType::Pointer cielabImage = FloatVectorImageType::New();
  ITKHelpers::ITKImageToCIELabImage(image, cielabImage);
  
  typedef ImagePatchPixelDescriptor<ImageType> ImagePatchPixelDescriptorType;

  // Create the graph
  typedef boost::grid_graph<2> VertexListGraphType;
  boost::array<std::size_t, 2> graphSideLengths = { { imageReader->GetOutput()->GetLargestPossibleRegion().GetSize()[0],
                                                      imageReader->GetOutput()->GetLargestPossibleRegion().GetSize()[1] } };
  VertexListGraphType graph(graphSideLengths);
  typedef boost::graph_traits<VertexListGraphType>::vertex_descriptor VertexDescriptorType;

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

  //ImagePatchDescriptorMapType smallImagePatchDescriptorMap(num_vertices(graph), indexMap);

  // Create the patch inpainter. The inpainter needs to know the status of each pixel to determine if they should be inpainted.
  typedef MaskImagePatchInpainter InpainterType;
  MaskImagePatchInpainter patchInpainter(patchHalfWidth, mask);

  // Create the priority function
//   typedef PriorityRandom PriorityType;
//   PriorityType priorityFunction;
  typedef PriorityOnionPeel PriorityType;
  PriorityType priorityFunction(mask, patchHalfWidth);

  // Create the boundary node queue. The priority of each node is used to order the queue.
  typedef boost::vector_property_map<std::size_t, IndexMapType> IndexInHeapMap;
  IndexInHeapMap index_in_heap(indexMap);

  // Create the priority compare functor (we want the highest priority nodes to be first in the queue)
  typedef std::greater<float> PriorityCompareType;
  PriorityCompareType lessThanFunctor;

  typedef boost::d_ary_heap_indirect<VertexDescriptorType, 4, IndexInHeapMap, PriorityMapType, PriorityCompareType>
                                    BoundaryNodeQueueType;
  BoundaryNodeQueueType boundaryNodeQueue(priorityMap, index_in_heap, lessThanFunctor);

  // Create the descriptor visitor
  typedef ImagePatchDescriptorVisitor<VertexListGraphType, ImageType, ImagePatchDescriptorMapType>
          ImagePatchDescriptorVisitorType;
  //ImagePatchDescriptorVisitorType imagePatchDescriptorVisitor(image, mask, imagePatchDescriptorMap, patchHalfWidth);
  ImagePatchDescriptorVisitorType imagePatchDescriptorVisitor(cielabImage, mask, imagePatchDescriptorMap, patchHalfWidth);

//   typedef ImagePatchDifference<ImagePatchPixelDescriptorType, SumAbsolutePixelDifference<ImageType::PixelType> >
//             ImagePatchDifferenceType;

  typedef WeightedSumAbsolutePixelDifference<ImageType::PixelType> PixelDifferenceFunctorType;
  PixelDifferenceFunctorType pixelDifferenceFunctor;
  std::vector<float> weights;
  weights.push_back(1.0f);
  weights.push_back(1.0f);
  weights.push_back(1.0f);
  weights.push_back(50.0f);
  weights.push_back(50.0f);
  pixelDifferenceFunctor.Weights = weights;

  typedef ImagePatchDifference<ImagePatchPixelDescriptorType, PixelDifferenceFunctorType >
          ImagePatchDifferenceType;
  ImagePatchDifferenceType imagePatchDifferenceFunction(pixelDifferenceFunctor);

  typedef CompositeDescriptorVisitor<VertexListGraphType> CompositeDescriptorVisitorType;
  CompositeDescriptorVisitorType compositeDescriptorVisitor;
  compositeDescriptorVisitor.AddVisitor(&imagePatchDescriptorVisitor);

  typedef CompositeAcceptanceVisitor<VertexListGraphType> CompositeAcceptanceVisitorType;
  CompositeAcceptanceVisitorType compositeAcceptanceVisitor;

  // If the hole is less than 15% of the patch, always accept the initial best match
  HoleSizeAcceptanceVisitor<VertexListGraphType> holeSizeAcceptanceVisitor(mask, patchHalfWidth, .15);
  compositeAcceptanceVisitor.AddOverrideVisitor(&holeSizeAcceptanceVisitor);

  AllQuadrantHistogramCompareAcceptanceVisitor<VertexListGraphType, ImageType>
               allQuadrantHistogramCompareAcceptanceVisitor(image, mask, patchHalfWidth, 2.9f * 4.0f);
  compositeAcceptanceVisitor.AddRequiredPassVisitor(&allQuadrantHistogramCompareAcceptanceVisitor);

  typedef InpaintingVisitor<VertexListGraphType, ImageType, BoundaryNodeQueueType,
                            CompositeDescriptorVisitorType, CompositeAcceptanceVisitorType, PriorityType,
                            PriorityMapType, BoundaryStatusMapType>
                            InpaintingVisitorType;
  InpaintingVisitorType inpaintingVisitor(image, mask, boundaryNodeQueue,
                                          compositeDescriptorVisitor, compositeAcceptanceVisitor, priorityMap,
                                          &priorityFunction, patchHalfWidth,
                                          boundaryStatusMap, outputFilename);

  typedef DisplayVisitor<VertexListGraphType, ImageType> DisplayVisitorType;
  DisplayVisitorType displayVisitor(image, mask, patchHalfWidth);

  typedef DebugVisitor<VertexListGraphType, ImageType, BoundaryStatusMapType, BoundaryNodeQueueType> DebugVisitorType;
  DebugVisitorType debugVisitor(image, mask, patchHalfWidth, boundaryStatusMap, boundaryNodeQueue);

  LoggerVisitor<VertexListGraphType> loggerVisitor("log.txt");

  typedef CompositeInpaintingVisitor<VertexListGraphType> CompositeInpaintingVisitorType;
  CompositeInpaintingVisitorType compositeInpaintingVisitor;
  compositeInpaintingVisitor.AddVisitor(&inpaintingVisitor);
  compositeInpaintingVisitor.AddVisitor(&displayVisitor);
  compositeInpaintingVisitor.AddVisitor(&debugVisitor);
  compositeInpaintingVisitor.AddVisitor(&loggerVisitor);

  InitializePriority(mask, boundaryNodeQueue, priorityMap, &priorityFunction, boundaryStatusMap);

  // Initialize the boundary node queue from the user provided mask image.
  InitializeFromMaskImage<CompositeInpaintingVisitorType, VertexDescriptorType>(mask, &compositeInpaintingVisitor);

  // Create the nearest neighbor finders
  typedef LinearSearchKNNProperty<ImagePatchDescriptorMapType,
                                  ImagePatchDifferenceType > KNNSearchType;
  KNNSearchType knnSearch(imagePatchDescriptorMap, 50000, 1, imagePatchDifferenceFunction);

  // For debugging we use LinearSearchBestProperty instead of DefaultSearchBest because it can output the difference value.
  typedef LinearSearchBestProperty<ImagePatchDescriptorMapType,
                                   ImagePatchDifferenceType > BestSearchType;
  BestSearchType bestSearch(imagePatchDescriptorMap, imagePatchDifferenceFunction);

  TopPatchesDialog<ImageType> topPatchesDialog(image, mask, patchHalfWidth);
  typedef VisualSelectionBest<ImageType> ManualSearchType;
  ManualSearchType manualSearchBest(image, mask, patchHalfWidth, &topPatchesDialog);

  BasicViewerWidget<ImageType> basicViewerWidget(image, mask);
  basicViewerWidget.show();
  // These connections are Qt::BlockingQueuedConnection because the algorithm quickly
  // goes on to fill the hole, and since we are sharing the image memory, we want to make sure these things are
  // refreshed at the right time, not after the hole has already been filled
  // (this actually happens, it is not just a theoretical thing).
  QObject::connect(&displayVisitor, SIGNAL(signal_RefreshImage()), &basicViewerWidget, SLOT(slot_UpdateImage()),
                   Qt::BlockingQueuedConnection);
  QObject::connect(&displayVisitor, SIGNAL(signal_RefreshSource(const itk::ImageRegion<2>&, const itk::ImageRegion<2>&)),
                   &basicViewerWidget, SLOT(slot_UpdateSource(const itk::ImageRegion<2>&, const itk::ImageRegion<2>&)),
                   Qt::BlockingQueuedConnection);
  QObject::connect(&displayVisitor, SIGNAL(signal_RefreshTarget(const itk::ImageRegion<2>&)),
                   &basicViewerWidget, SLOT(slot_UpdateTarget(const itk::ImageRegion<2>&)),
                   Qt::BlockingQueuedConnection);
  QObject::connect(&displayVisitor, SIGNAL(signal_RefreshResult(const itk::ImageRegion<2>&, const itk::ImageRegion<2>&)),
                   &basicViewerWidget, SLOT(slot_UpdateResult(const itk::ImageRegion<2>&, const itk::ImageRegion<2>&)),
                   Qt::BlockingQueuedConnection);

  // Display the priority of the boundary in a different window
//   PriorityViewerWidget<PriorityType, BoundaryStatusMapType>
//             priorityViewerWidget(&priorityFunction, image->GetLargestPossibleRegion().GetSize(), boundaryStatusMap);
//   priorityViewerWidget.show();
// 
//   QObject::connect(&displayVisitor, SIGNAL(signal_RefreshImage()), &priorityViewerWidget, SLOT(slot_UpdateImage()),
//                    Qt::BlockingQueuedConnection);

  // Passively display the top patches at every iteration
//   TopPatchesWidget<ImageType> topPatchesWidget(image, patchHalfWidth);
//   topPatchesWidget.show();
//   QObject::connect(&nearestNeighborsDisplayVisitor, SIGNAL(signal_Refresh(const std::vector<Node>&)),
//                    &topPatchesWidget, SLOT(SetNodes(const std::vector<Node>&)));

  // By specifying the radius as the image size/8, we are searching up to 1/4 of the image each time
  typedef NeighborhoodSearch<VertexDescriptorType> NeighborhoodSearchType;
  NeighborhoodSearchType neighborhoodSearch(image->GetLargestPossibleRegion(), image->GetLargestPossibleRegion().GetSize()[0]/8);

  // Run the remaining inpainting
  QtConcurrent::run(boost::bind(InpaintingAlgorithmWithLocalSearch<
                                VertexListGraphType, CompositeInpaintingVisitorType, BoundaryStatusMapType,
                                BoundaryNodeQueueType, NeighborhoodSearchType, KNNSearchType, BestSearchType,
                                ManualSearchType, InpainterType>,
                                graph, compositeInpaintingVisitor, &boundaryStatusMap, &boundaryNodeQueue, neighborhoodSearch,
                                knnSearch, bestSearch, boost::ref(manualSearchBest), patchInpainter));

  return app.exec();
}
