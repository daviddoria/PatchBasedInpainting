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

#ifndef InteractiveInpaintingGMH_HPP
#define InteractiveInpaintingGMH_HPP

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

//template <typename TVertexListGraph, typename TInpaintingVisitor,
//          typename TPriorityQueue, typename TKNNFinder, typename TBestNeighborFinder,
//          typename TManualNeighborFinder, typename TPatchInpainter>
//inline
//void TestFunction(std::shared_ptr<TVertexListGraph> graph,
//                  std::shared_ptr<TInpaintingVisitor> visitor,
//                  std::shared_ptr<TPriorityQueue> boundaryNodeQueue,
//                  std::shared_ptr<TKNNFinder> knnFinder,
//                  std::shared_ptr<TBestNeighborFinder> bestNeighborFinder,
//                  std::shared_ptr<TManualNeighborFinder> manualNeighborFinder,
//                  std::shared_ptr<TPatchInpainter> patchInpainter)

template <typename TVertexListGraph>
void TestFunction(std::shared_ptr<TVertexListGraph> graph)
{
  typedef typename boost::graph_traits<TVertexListGraph>::vertex_iterator VertexIterator;
  VertexIterator graphBegin;
  VertexIterator graphEnd;
  tie(graphBegin, graphEnd) = vertices(*graph);
}

template <typename TImage>
void InteractiveInpaintingGMH(typename itk::SmartPointer<TImage> originalImage, Mask::Pointer mask, const unsigned int patchHalfWidth)
{
  /** Store the viewer here so that it is created in the GUI thread and persists. */
  typedef BasicViewerWidget<TImage> BasicViewerWidgetType;
  std::shared_ptr<BasicViewerWidgetType> basicViewer(new BasicViewerWidgetType(originalImage, mask));
  basicViewer->show();

  typedef boost::grid_graph<2> VertexListGraphType;
  typedef DisplayVisitor<VertexListGraphType, TImage> DisplayVisitorType;
  std::shared_ptr<DisplayVisitorType> displayVisitor(
        new DisplayVisitorType(originalImage, mask, patchHalfWidth));

  itk::ImageRegion<2> fullRegion = originalImage->GetLargestPossibleRegion();

  // Blur the image
  typedef TImage BlurredImageType; // Usually the blurred image is the same type as the original image.
  typename BlurredImageType::Pointer blurredImage = BlurredImageType::New();
  float blurVariance = 2.0f;
  MaskOperations::MaskedBlur(originalImage.GetPointer(), mask, blurVariance, blurredImage.GetPointer());

  typedef ImagePatchPixelDescriptor<TImage> ImagePatchPixelDescriptorType;

  // Create the graph
  boost::array<std::size_t, 2> graphSideLengths = { { fullRegion.GetSize()[0],
                                                      fullRegion.GetSize()[1] } };
  std::shared_ptr<VertexListGraphType> graph(new VertexListGraphType(graphSideLengths));
  typedef boost::graph_traits<VertexListGraphType>::vertex_descriptor VertexDescriptorType;

  // Get the index map
  typedef boost::property_map<VertexListGraphType, boost::vertex_index_t>::const_type IndexMapType;
  std::shared_ptr<IndexMapType> indexMap(new IndexMapType(get(boost::vertex_index, *graph)));

  // Create the descriptor map. This is where the data for each pixel is stored.
  typedef boost::vector_property_map<ImagePatchPixelDescriptorType, IndexMapType> ImagePatchDescriptorMapType;
  std::shared_ptr<ImagePatchDescriptorMapType> imagePatchDescriptorMap(
        new ImagePatchDescriptorMapType(num_vertices(*graph), *indexMap));

  // Create the patch inpainter.
  typedef PatchInpainter<TImage> OriginalImageInpainterType;
  OriginalImageInpainterType originalImageInpainter(patchHalfWidth, originalImage, mask);

  typedef PatchInpainter<BlurredImageType> BlurredImageInpainterType;
  BlurredImageInpainterType blurredImageInpainter(patchHalfWidth, originalImage, mask);

  // Create a composite inpainter.
  /** We only have to store the composite inpainter in the class, as it stores shared_ptrs
    * to all of the individual inpainters. If the composite inpainter says in scope, the
    * individual inpainters do as well.
    */
  std::shared_ptr<CompositePatchInpainter> compositeInpainter(new CompositePatchInpainter);
  compositeInpainter->AddInpainter(&originalImageInpainter);
  compositeInpainter->AddInpainter(&blurredImageInpainter);

  // Create the priority function
  typedef PriorityCriminisi<TImage> PriorityType;
  std::shared_ptr<PriorityType> priorityFunction(
        new PriorityType(blurredImage, mask, patchHalfWidth));

  // Queue
  typedef IndirectPriorityQueue<VertexListGraphType> BoundaryNodeQueueType;
  std::shared_ptr<BoundaryNodeQueueType> boundaryNodeQueue(new BoundaryNodeQueueType(*graph));

  // Create the descriptor visitor
  typedef ImagePatchDescriptorVisitor<VertexListGraphType, TImage, ImagePatchDescriptorMapType>
          ImagePatchDescriptorVisitorType;

  std::shared_ptr<ImagePatchDescriptorVisitorType> imagePatchDescriptorVisitor(
        new ImagePatchDescriptorVisitorType(originalImage, mask,
                                            *imagePatchDescriptorMap, patchHalfWidth));

  typedef SumSquaredPixelDifference<typename TImage::PixelType> PixelDifferenceType;
  typedef ImagePatchDifference<ImagePatchPixelDescriptorType, PixelDifferenceType >
            ImagePatchDifferenceType;

  typedef CompositeDescriptorVisitor<VertexListGraphType> CompositeDescriptorVisitorType;
  std::shared_ptr<CompositeDescriptorVisitorType> compositeDescriptorVisitor(new CompositeDescriptorVisitorType);
  compositeDescriptorVisitor->AddVisitor(imagePatchDescriptorVisitor.get());


  // Source region to source region comparisons
   SourceValidTargetValidCompare<VertexListGraphType, TImage, AverageFunctor>
         validRegionAverageAcceptance(originalImage, mask, patchHalfWidth,
         AverageFunctor(), 10, "validRegionAverageAcceptance");

  // If the hole is less than 15% of the patch, always accept the initial best match
  HoleSizeAcceptanceVisitor<VertexListGraphType> holeSizeAcceptanceVisitor(mask, patchHalfWidth, .15);

  typedef CompositeAcceptanceVisitor<VertexListGraphType> CompositeAcceptanceVisitorType;
  std::shared_ptr<CompositeAcceptanceVisitorType> compositeAcceptanceVisitor(new CompositeAcceptanceVisitorType);
  compositeAcceptanceVisitor->AddRequiredPassVisitor(&validRegionAverageAcceptance);
  compositeAcceptanceVisitor->AddOverrideVisitor(&holeSizeAcceptanceVisitor);

  // Create the inpainting visitor
  typedef InpaintingVisitor<VertexListGraphType, BoundaryNodeQueueType,
                            CompositeDescriptorVisitorType, CompositeAcceptanceVisitorType, PriorityType,
                            TImage>
                            InpaintingVisitorType;
  std::shared_ptr<InpaintingVisitorType> inpaintingVisitor(
        new InpaintingVisitorType(mask, *boundaryNodeQueue,
                                  *compositeDescriptorVisitor, *compositeAcceptanceVisitor,
                                  priorityFunction.get(), patchHalfWidth,
                                  "InpaintingVisitor", originalImage.GetPointer()));

  typedef CompositeInpaintingVisitor<VertexListGraphType> CompositeInpaintingVisitorType;
  std::shared_ptr<CompositeInpaintingVisitorType> compositeInpaintingVisitor(new CompositeInpaintingVisitorType);
  compositeInpaintingVisitor->AddVisitor(inpaintingVisitor.get());
  compositeInpaintingVisitor->AddVisitor(displayVisitor.get());

  InitializePriority(mask, *boundaryNodeQueue, priorityFunction.get());

  // Initialize the boundary node queue from the user provided mask image.
  InitializeFromMaskImage<CompositeInpaintingVisitorType, VertexDescriptorType>(
        mask, compositeInpaintingVisitor.get());
  std::cout << "InteractiveInpaintingWithVerification: There are " << boundaryNodeQueue->size()
            << " nodes in the boundaryNodeQueue" << std::endl;

  // Create the nearest neighbor finders
  typedef LinearSearchKNNProperty<ImagePatchDescriptorMapType,
                                  ImagePatchDifferenceType > KNNSearchType;
  unsigned int numberOfKNN = 100;
  std::shared_ptr<KNNSearchType> knnSearch(new KNNSearchType(*imagePatchDescriptorMap, numberOfKNN));

  // For debugging we use LinearSearchBestProperty instead of DefaultSearchBest because it can output the difference value.
  typedef LinearSearchBestProperty<ImagePatchDescriptorMapType,
                                   ImagePatchDifferenceType > BestSearchType;
  std::shared_ptr<BestSearchType> bestSearch(new BestSearchType(*imagePatchDescriptorMap));

  // If the acceptance tests fail, prompt the user to select a patch.
  typedef VisualSelectionBest<TImage> ManualSearchType;
  std::shared_ptr<ManualSearchType> manualSearchBest(new ManualSearchType(originalImage, mask, patchHalfWidth));


//  QObject::connect(&displayVisitor, SIGNAL(signal_RefreshImage()), &basicViewerWidget, SLOT(slot_UpdateImage()),
//                   Qt::BlockingQueuedConnection);
//  QObject::connect(&displayVisitor, SIGNAL(signal_RefreshSource(const itk::ImageRegion<2>&,
//                                                                const itk::ImageRegion<2>&)),
//                   &basicViewerWidget, SLOT(slot_UpdateSource(const itk::ImageRegion<2>&,
//                                                              const itk::ImageRegion<2>&)),
//                   Qt::BlockingQueuedConnection);
//  QObject::connect(&displayVisitor, SIGNAL(signal_RefreshTarget(const itk::ImageRegion<2>&)),
//                   &basicViewerWidget, SLOT(slot_UpdateTarget(const itk::ImageRegion<2>&)),
//                   Qt::BlockingQueuedConnection);
//  QObject::connect(&displayVisitor, SIGNAL(signal_RefreshResult(const itk::ImageRegion<2>&,
//                                                                const itk::ImageRegion<2>&)),
//                   &basicViewerWidget, SLOT(slot_UpdateResult(const itk::ImageRegion<2>&,
//                                                              const itk::ImageRegion<2>&)),
//                   Qt::BlockingQueuedConnection);


  // Run the remaining inpainting with interaction
  std::cout << "Running inpainting..." << std::endl;

//    InpaintingAlgorithmWithVerification(graph, compositeInpaintingVisitor, &boundaryNodeQueue, knnSearch,
//                                        bestSearch, &manualSearchBest, &inpainter);

//    QtConcurrent::run(boost::bind(InpaintingAlgorithmWithVerification<
//                                  VertexListGraphType, CompositeInpaintingVisitorType,
//                                  BoundaryNodeQueueType, KNNSearchType, BestSearchType,
//                                  ManualSearchType, CompositePatchInpainter>,
//                                  graph, compositeInpaintingVisitor, boundaryNodeQueue, knnSearch,
//                                  bestSearch, manualSearchBest, compositeInpainter));

  QtConcurrent::run(boost::bind(TestFunction<VertexListGraphType>, graph));
}

#endif