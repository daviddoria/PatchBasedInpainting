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
#include "NearestNeighbor/LinearSearchBest/First.hpp"
#include "NearestNeighbor/VisualSelectionBest.hpp"
#include "NearestNeighbor/FirstValidDescriptor.hpp"
#include "NearestNeighbor/SortByRGBTextureGradient.hpp"
#include "NearestNeighbor/KNNSearchAndSort.hpp"

// Pixel descriptors
#include "PixelDescriptors/ImagePatchPixelDescriptor.h"
#include "PixelDescriptors/ImagePatchVectorized.h"
#include "PixelDescriptors/ImagePatchVectorizedIndices.h"

// Acceptance visitors
#include "Visitors/AcceptanceVisitors/GMHAcceptanceVisitor.hpp"

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
#include "Interactive/PriorityViewerWidget.h"

template <typename TImage>
void InteractiveInpaintingGMH(typename itk::SmartPointer<TImage> originalImage,
                              Mask::Pointer mask, const unsigned int patchHalfWidth)
{
  typedef boost::grid_graph<2> VertexListGraphType;

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
  std::shared_ptr<OriginalImageInpainterType> originalImageInpainter(
      new OriginalImageInpainterType(patchHalfWidth, originalImage, mask));

  typedef PatchInpainter<BlurredImageType> BlurredImageInpainterType;
  std::shared_ptr<BlurredImageInpainterType> blurredImageInpainter(
      new BlurredImageInpainterType(patchHalfWidth, originalImage, mask));

  // Create a composite inpainter.
  /** We only have to store the composite inpainter in the class, as it stores shared_ptrs
    * to all of the individual inpainters. If the composite inpainter says in scope, the
    * individual inpainters do as well.
    */
  std::shared_ptr<CompositePatchInpainter> compositeInpainter(new CompositePatchInpainter);
  compositeInpainter->AddInpainter(originalImageInpainter);
  compositeInpainter->AddInpainter(blurredImageInpainter);

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
                                            imagePatchDescriptorMap, patchHalfWidth));

  // Acceptance visitor
  unsigned int numberOfBinsPerChannel = 30;
  float maxAllowedDifference = 2.0f;
  typedef GMHAcceptanceVisitor<VertexListGraphType, TImage> GMHAcceptanceVisitorType;
  std::shared_ptr<GMHAcceptanceVisitorType> gmhAcceptanceVisitor(
      new GMHAcceptanceVisitorType(originalImage, mask, patchHalfWidth,
                                   maxAllowedDifference, numberOfBinsPerChannel));

//  // If the hole is less than 15% of the patch, always accept the initial best match
//  typedef HoleSizeAcceptanceVisitor<VertexListGraphType> HoleSizeAcceptanceVisitorType;
//  std::shared_ptr<HoleSizeAcceptanceVisitorType> holeSizeAcceptanceVisitor(new HoleSizeAcceptanceVisitorType(mask, patchHalfWidth, .15));

//  typedef CompositeAcceptanceVisitor<VertexListGraphType> CompositeAcceptanceVisitorType;
//  std::shared_ptr<CompositeAcceptanceVisitorType> compositeAcceptanceVisitor(new CompositeAcceptanceVisitorType);
//  compositeAcceptanceVisitor->AddRequiredPassVisitor(validRegionAverageAcceptance);
//  compositeAcceptanceVisitor->AddOverrideVisitor(holeSizeAcceptanceVisitor);

  // Create the inpainting visitor
//  typedef InpaintingVisitor<VertexListGraphType, BoundaryNodeQueueType,
//                            ImagePatchDescriptorVisitorType, CompositeAcceptanceVisitorType,
//                            PriorityType, TImage>
//                            InpaintingVisitorType;
//  std::shared_ptr<InpaintingVisitorType> inpaintingVisitor(
//        new InpaintingVisitorType(mask, boundaryNodeQueue,
//                                  imagePatchDescriptorVisitor, compositeAcceptanceVisitor,
//                                  priorityFunction, patchHalfWidth,
//                                  "InpaintingVisitor", originalImage.GetPointer()));

  typedef InpaintingVisitor<VertexListGraphType, BoundaryNodeQueueType,
                            ImagePatchDescriptorVisitorType, GMHAcceptanceVisitorType,
                            PriorityType, TImage>
                            InpaintingVisitorType;
  std::shared_ptr<InpaintingVisitorType> inpaintingVisitor(
        new InpaintingVisitorType(mask, boundaryNodeQueue,
                                  imagePatchDescriptorVisitor, gmhAcceptanceVisitor,
                                  priorityFunction, patchHalfWidth,
                                  "InpaintingVisitor", originalImage.GetPointer()));

  typedef DisplayVisitor<VertexListGraphType, TImage> DisplayVisitorType;
  std::shared_ptr<DisplayVisitorType> displayVisitor(
        new DisplayVisitorType(originalImage, mask, patchHalfWidth));

  typedef CompositeInpaintingVisitor<VertexListGraphType> CompositeInpaintingVisitorType;
  std::shared_ptr<CompositeInpaintingVisitorType> compositeInpaintingVisitor(new CompositeInpaintingVisitorType);
  compositeInpaintingVisitor->AddVisitor(inpaintingVisitor);
  compositeInpaintingVisitor->AddVisitor(displayVisitor);

  InitializePriority(mask, boundaryNodeQueue.get(), priorityFunction.get());

  // Initialize the boundary node queue from the user provided mask image.
  InitializeFromMaskImage<CompositeInpaintingVisitorType, VertexDescriptorType>(
        mask, compositeInpaintingVisitor.get());
  std::cout << "InteractiveInpaintingWithVerification: There are " << boundaryNodeQueue->size()
            << " nodes in the boundaryNodeQueue" << std::endl;

  typedef SumSquaredPixelDifference<typename TImage::PixelType> PixelDifferenceType;
  typedef ImagePatchDifference<ImagePatchPixelDescriptorType, PixelDifferenceType >
            ImagePatchDifferenceType;

  // Create the nearest neighbor finders
  typedef LinearSearchKNNProperty<ImagePatchDescriptorMapType,
                                  ImagePatchDifferenceType > KNNSearchType;
  unsigned int numberOfKNN = 100;
  std::shared_ptr<KNNSearchType> knnSearch(new KNNSearchType(imagePatchDescriptorMap, numberOfKNN));

  // Since we are using a KNNSearchAndSort, we just have to return the top patch after the sort,
  // so we use this trival Best searcher.
  typedef LinearSearchBestFirst BestSearchType;
  std::shared_ptr<BestSearchType> bestSearch;

  typedef SortByRGBTextureGradient<ImagePatchDescriptorMapType,
                                   TImage > NeighborSortType;
  std::shared_ptr<NeighborSortType> neighborSortType(
        new NeighborSortType(*imagePatchDescriptorMap, originalImage, mask, numberOfBinsPerChannel));

  typedef KNNSearchAndSort<KNNSearchType, NeighborSortType> SearchAndSortType;
  std::shared_ptr<SearchAndSortType> searchAndSort(new SearchAndSortType(knnSearch, neighborSortType));


  typedef BasicViewerWidget<TImage> BasicViewerWidgetType;
//  std::shared_ptr<BasicViewerWidgetType> basicViewer(new BasicViewerWidgetType(originalImage, mask)); // This shared_ptr will go out of scope when this function ends, so the window will immediately close
  BasicViewerWidgetType* basicViewer = new BasicViewerWidgetType(originalImage, mask);
  std::cout << "basicViewer pointer: " << basicViewer << std::endl;
  basicViewer->ConnectVisitor(displayVisitor.get());

  // If the acceptance tests fail, prompt the user to select a patch. Pass the basicViewer as the parent so that we can position the top pathces dialog properly.
  typedef VisualSelectionBest<TImage> ManualSearchType;
  std::shared_ptr<ManualSearchType> manualSearchBest(new ManualSearchType(originalImage, mask,
                                                                          patchHalfWidth, basicViewer));

  // Connect the viewer to the top patches selection widget
  basicViewer->ConnectWidget(manualSearchBest->GetTopPatchesDialog());
  basicViewer->show();

  // Run the remaining inpainting with interaction
  std::cout << "Running inpainting..." << std::endl;

  QtConcurrent::run(boost::bind(InpaintingAlgorithmWithVerification<
                                VertexListGraphType, CompositeInpaintingVisitorType,
                                BoundaryNodeQueueType, SearchAndSortType, BestSearchType,
                                ManualSearchType, CompositePatchInpainter>,
                                graph, compositeInpaintingVisitor, boundaryNodeQueue, searchAndSort,
                                bestSearch, manualSearchBest, compositeInpainter));

}

#endif
