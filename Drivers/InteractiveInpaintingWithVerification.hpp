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

#ifndef InteractiveInpaintingWithVerification_HPP
#define InteractiveInpaintingWithVerification_HPP

// Custom
#include "Utilities/IndirectPriorityQueue.h"

// Qt
#include <QtConcurrentRun>

// Submodules
#include <Helpers/Helpers.h>

// Pixel descriptors
#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

// Descriptor visitors
#include "Visitors/DescriptorVisitors/ImagePatchDescriptorVisitor.hpp"
#include "Visitors/DescriptorVisitors/CompositeDescriptorVisitor.hpp"

// Nearest neighbors visitor
#include "Visitors/NearestNeighborsDisplayVisitor.hpp"

// Nearest neighbors
#include "NearestNeighbor/LinearSearchKNNProperty.hpp"
#include "NearestNeighbor/DefaultSearchBest.hpp"
#include "NearestNeighbor/LinearSearchBest/Property.hpp"
#include "NearestNeighbor/VisualSelectionBest.hpp"
#include "NearestNeighbor/FirstValidDescriptor.hpp"

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

//#include "Visitors/AcceptanceVisitors/IntraSourcePatchAcceptanceVisitor.hpp"
#include "Visitors/AcceptanceVisitors/NeverAccept.hpp"

// Information visitors
#include "Visitors/InformationVisitors/DisplayVisitor.hpp"

// Inpainting visitors
#include "Visitors/InpaintingVisitor.hpp"
#include "Visitors/AcceptanceVisitors/DefaultAcceptanceVisitor.hpp"
#include "Visitors/InpaintingVisitor.hpp"
#include "Visitors/ReplayVisitor.hpp"
#include "Visitors/InformationVisitors/LoggerVisitor.hpp"
#include "Visitors/CompositeInpaintingVisitor.hpp"
#include "Visitors/InformationVisitors/DebugVisitor.hpp"

// Nearest neighbors
#include "NearestNeighbor/LinearSearchBest/Property.hpp"

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
#include "Algorithms/InpaintingAlgorithmWithVerification.hpp"

// Priority
#include "Priority/PriorityCriminisi.h"

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>

// GUI
#include "Interactive/BasicViewerWidget.h"
#include "Interactive/TopPatchesWidget.h"
#include "Interactive/TopPatchesDialog.h"
#include "Interactive/PriorityViewerWidget.h"

template <typename TImage>
void InteractiveInpaintingWithVerification(TImage* const originalImage, Mask* const mask, const unsigned int patchHalfWidth)
{
  itk::ImageRegion<2> fullRegion = originalImage->GetLargestPossibleRegion();

  // Blur the image
  typedef TImage BlurredImageType; // Usually the blurred image is the same type as the original image.
  typename BlurredImageType::Pointer blurredImage = BlurredImageType::New();
  float blurVariance = 2.0f;
  MaskOperations::MaskedBlur(originalImage, mask, blurVariance, blurredImage.GetPointer());

  ITKHelpers::WriteRGBImage(blurredImage.GetPointer(), "BlurredImage.png");

  typedef ImagePatchPixelDescriptor<TImage> ImagePatchPixelDescriptorType;

  // Create the graph
  typedef boost::grid_graph<2> VertexListGraphType;
  boost::array<std::size_t, 2> graphSideLengths = { { fullRegion.GetSize()[0],
                                                      fullRegion.GetSize()[1] } };
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

  // Create the patch inpainter.
  typedef PatchInpainter<TImage> OriginalImageInpainterType;
  OriginalImageInpainterType originalImageInpainter(patchHalfWidth, originalImage, mask);

  typedef PatchInpainter<BlurredImageType> BlurredImageInpainterType;
  BlurredImageInpainterType blurredImageInpainter(patchHalfWidth, originalImage, mask);

  // Create a composite inpainter.
  CompositePatchInpainter inpainter;
  inpainter.AddInpainter(&originalImageInpainter);
  inpainter.AddInpainter(&blurredImageInpainter);

  // Create the priority function
//   typedef PriorityRandom PriorityType;
//   PriorityType priorityFunction;
  typedef PriorityCriminisi<TImage> PriorityType;
  PriorityType priorityFunction(blurredImage, mask, patchHalfWidth);

  // Queue
  typedef IndirectPriorityQueue<VertexListGraphType> BoundaryNodeQueueType;
  BoundaryNodeQueueType boundaryNodeQueue(graph);

  // Create the descriptor visitor
  typedef ImagePatchDescriptorVisitor<VertexListGraphType, TImage, ImagePatchDescriptorMapType>
          ImagePatchDescriptorVisitorType;

  ImagePatchDescriptorVisitorType imagePatchDescriptorVisitor(originalImage, mask, imagePatchDescriptorMap, patchHalfWidth);

  typedef SumSquaredPixelDifference<typename TImage::PixelType> PixelDifferenceType;
  typedef ImagePatchDifference<ImagePatchPixelDescriptorType, PixelDifferenceType >
            ImagePatchDifferenceType;

  typedef CompositeDescriptorVisitor<VertexListGraphType> CompositeDescriptorVisitorType;
  CompositeDescriptorVisitorType compositeDescriptorVisitor;
  compositeDescriptorVisitor.AddVisitor(&imagePatchDescriptorVisitor);

  typedef CompositeAcceptanceVisitor<VertexListGraphType> CompositeAcceptanceVisitorType;
  CompositeAcceptanceVisitorType compositeAcceptanceVisitor;

//   typedef ANDAcceptanceVisitor<VertexListGraphType> CompositeAcceptanceVisitorType;
//   CompositeAcceptanceVisitorType compositeAcceptanceVisitor;

  // Source region to source region comparisons
   SourceValidTargetValidCompare<VertexListGraphType, TImage, AverageFunctor>
         validRegionAverageAcceptance(originalImage, mask, patchHalfWidth,
         AverageFunctor(), 10, "validRegionAverageAcceptance");
   compositeAcceptanceVisitor.AddRequiredPassVisitor(&validRegionAverageAcceptance);

  // We don't want to do this - the variation over the patch makes this no good.
  // Prefer the DilatedRegionAcceptanceVisitor with a VarianceFunctor instead.
//   SourceValidTargetValidCompare<VertexListGraphType, TImage, VarianceFunctor>
//           validRegionVarianceAcceptance(image, mask, patchHalfWidth,
//           VarianceFunctor(), 1000, "validRegionVarianceAcceptance");
//   compositeAcceptanceVisitor.AddVisitor(&validRegionVarianceAcceptance);

  // If the hole is less than 15% of the patch, always accept the initial best match
  HoleSizeAcceptanceVisitor<VertexListGraphType> holeSizeAcceptanceVisitor(mask, patchHalfWidth, .15);
  compositeAcceptanceVisitor.AddOverrideVisitor(&holeSizeAcceptanceVisitor);

//   HistogramDifferenceAcceptanceVisitor<VertexListGraphType, TImage>
//            histogramDifferenceAcceptanceVisitor(image, mask, patchHalfWidth, 2.0f);
//   compositeAcceptanceVisitor.AddRequiredPassVisitor(&histogramDifferenceAcceptanceVisitor);
//
//   HoleHistogramDifferenceAcceptanceVisitor<VertexListGraphType, TImage>
//          holeHistogramDifferenceAcceptanceVisitor(image, mask, patchHalfWidth, 2.0f);
//   compositeAcceptanceVisitor.AddRequiredPassVisitor(&holeHistogramDifferenceAcceptanceVisitor);

//   QuadrantHistogramCompareAcceptanceVisitor<VertexListGraphType, TImage>
//           quadrantHistogramCompareAcceptanceVisitor(image, mask, patchHalfWidth, 2.0f);
//   compositeAcceptanceVisitor.AddRequiredPassVisitor(&quadrantHistogramCompareAcceptanceVisitor);

  // AllQuadrantHistogramCompareAcceptanceVisitor<VertexListGraphType, TImage>
  //          allQuadrantHistogramCompareAcceptanceVisitor(image, mask, patchHalfWidth, 8.0f); // 8 (2 for each quadrant) is reasonable
//   AllQuadrantHistogramCompareAcceptanceVisitor<VertexListGraphType, TImage>
//                allQuadrantHistogramCompareAcceptanceVisitor(image, mask, patchHalfWidth, 1.0f); // Crazy low
//   compositeAcceptanceVisitor.AddRequiredPassVisitor(&allQuadrantHistogramCompareAcceptanceVisitor);

//   ScoreThresholdAcceptanceVisitor<VertexListGraphType, ImagePatchDescriptorMapType,
//                                   ImagePatchDifferenceType> scoreThresholdAcceptanceVisitor(mask, patchHalfWidth,
//                                                             imagePatchDescriptorMap, 10);
//   compositeAcceptanceVisitor.AddOverrideVisitor(&scoreThresholdAcceptanceVisitor);

  // Source region to hole region comparisons
//   SourceHoleTargetValidCompare<VertexListGraphType, TImage, AverageFunctor>
//                    holeRegionAverageAcceptance(image, mask, patchHalfWidth,
//                                              AverageFunctor(), 100, "holeRegionAverageAcceptance");
//   compositeAcceptanceVisitor.AddRequiredPassVisitor(&holeRegionAverageAcceptance);

//   SourceHoleTargetValidCompare<VertexListGraphType, TImage, VarianceFunctor>
//             holeRegionVarianceAcceptance(image, mask, patchHalfWidth,
//                                          VarianceFunctor(), 1000, "holeRegionVarianceAcceptance");
//   compositeAcceptanceVisitor.AddRequiredPassVisitor(&holeRegionVarianceAcceptance);

  // Compare the source region variance in the target patch to the source region variance in the source patch
//   DilatedSourceValidTargetValidAcceptanceVisitor<VertexListGraphType, TImage, VarianceFunctor>
//              dilatedValidValidVarianceDifferenceAcceptanceVisitor(image, mask, patchHalfWidth,
//                                                                 VarianceFunctor(), 1000,
//                                                                   "dilatedVarianceDifferenceAcceptanceVisitor");
//   compositeAcceptanceVisitor.AddRequiredPassVisitor(&dilatedValidValidVarianceDifferenceAcceptanceVisitor);

  // Compare the hole variance to the source region variance
//   DilatedSourceHoleTargetValidAcceptanceVisitor<VertexListGraphType, TImage, VarianceFunctor>
//            dilatedHoleValidVarianceDifferenceAcceptanceVisitor(image, mask, patchHalfWidth,
//                                                       VarianceFunctor(), 1000,
//                                                       "dilatedHoleValidVarianceDifferenceAcceptanceVisitor");
//   compositeAcceptanceVisitor.AddRequiredPassVisitor(&dilatedHoleValidVarianceDifferenceAcceptanceVisitor);

//   PatchDistanceAcceptanceVisitor<VertexListGraphType> patchDistanceAcceptanceVisitor(100);
//   compositeAcceptanceVisitor.AddRequiredPassVisitor(&patchDistanceAcceptanceVisitor);

//   CorrelationAcceptanceVisitor<VertexListGraphType, TImage> correlationAcceptanceVisitor(image, mask, patchHalfWidth, 100);
//   compositeAcceptanceVisitor.AddRequiredPassVisitor(&correlationAcceptanceVisitor);

//   IntraSourcePatchAcceptanceVisitor<VertexListGraphType, TImage>
//           intraSourcePatchAcceptanceVisitor(image, mask, patchHalfWidth, 100);
//   compositeAcceptanceVisitor.AddVisitor(&intraSourcePatchAcceptanceVisitor);

  // Create the inpainting visitor
  typedef InpaintingVisitor<VertexListGraphType, BoundaryNodeQueueType,
                            CompositeDescriptorVisitorType, CompositeAcceptanceVisitorType, PriorityType,
                            TImage>
                            InpaintingVisitorType;
  InpaintingVisitorType inpaintingVisitor(mask, boundaryNodeQueue,
                                          compositeDescriptorVisitor, compositeAcceptanceVisitor,
                                          &priorityFunction, patchHalfWidth,
                                          "InpaintingVisitor", originalImage);

//  typedef DisplayVisitor<VertexListGraphType, TImage> DisplayVisitorType;
//  DisplayVisitorType displayVisitor(image, mask, patchHalfWidth);

//  typedef DebugVisitor<VertexListGraphType, TImage, BoundaryStatusMapType, BoundaryNodeQueueType> DebugVisitorType;
//  DebugVisitorType debugVisitor(image, mask, patchHalfWidth, boundaryStatusMap, boundaryNodeQueue);

  LoggerVisitor<VertexListGraphType> loggerVisitor("log.txt");

  typedef CompositeInpaintingVisitor<VertexListGraphType> CompositeInpaintingVisitorType;
  CompositeInpaintingVisitorType compositeInpaintingVisitor;
  compositeInpaintingVisitor.AddVisitor(&inpaintingVisitor);
//  compositeInpaintingVisitor.AddVisitor(&displayVisitor);
//  compositeInpaintingVisitor.AddVisitor(&debugVisitor);
  compositeInpaintingVisitor.AddVisitor(&loggerVisitor);

  InitializePriority(mask, boundaryNodeQueue, &priorityFunction);

  // Initialize the boundary node queue from the user provided mask image.
  InitializeFromMaskImage<CompositeInpaintingVisitorType, VertexDescriptorType>(mask, &compositeInpaintingVisitor);
  std::cout << "InteractiveInpaintingWithVerification: There are " << boundaryNodeQueue.size()
            << " nodes in the boundaryNodeQueue" << std::endl;

  // Create the nearest neighbor finders
  typedef LinearSearchKNNProperty<ImagePatchDescriptorMapType,
                                  ImagePatchDifferenceType > KNNSearchType;
  KNNSearchType knnSearch(imagePatchDescriptorMap, 100);

//   typedef LinearSearchKNNProperty<ImagePatchDescriptorMapType,
//                                   ImagePatchDifference<ImagePatchPixelDescriptorType> > KNNSearchType;
//   KNNSearchType knnSearch(smallImagePatchDescriptorMap, 1000);

  // For debugging we use LinearSearchBestProperty instead of DefaultSearchBest because it can output the difference value.
  typedef LinearSearchBestProperty<ImagePatchDescriptorMapType,
                                   ImagePatchDifferenceType > BestSearchType;
  BestSearchType bestSearch(imagePatchDescriptorMap);

  // If the acceptance tests fail, prompt the user to select a patch.
  typedef VisualSelectionBest<TImage> ManualSearchType;
  ManualSearchType manualSearchBest(originalImage, mask, patchHalfWidth);

  // Run the remaining inpainting with interaction
  std::cout << "Running inpainting..." << std::endl;
//    QtConcurrent::run(boost::bind(InpaintingAlgorithmWithVerification<
//                                  VertexListGraphType, CompositeInpaintingVisitorType,
//                                  BoundaryNodeQueueType, KNNSearchType, BestSearchType,
//                                  ManualSearchType, CompositePatchInpainter>,
//                                  graph, compositeInpaintingVisitor, &boundaryNodeQueue, knnSearch,
//                                  bestSearch, &manualSearchBest, &inpainter));

  InpaintingAlgorithmWithVerification(graph, compositeInpaintingVisitor, &boundaryNodeQueue, knnSearch,
                                      bestSearch, &manualSearchBest, &inpainter);

}

#endif
