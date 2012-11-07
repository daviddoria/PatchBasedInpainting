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

#ifndef DummyPatchesDriver_HPP
#define DummyPatchesDriver_HPP

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
#include "Visitors/AcceptanceVisitors/AlwaysAccept.hpp"
#include "Visitors/InpaintingVisitor.hpp"
#include "Visitors/CompositeInpaintingVisitor.hpp"

// Nearest neighbors
#include "NearestNeighbor/FirstValidDescriptor.hpp"

// Initializers
#include "Initializers/InitializeFromMaskImage.hpp"
#include "Initializers/InitializePriority.hpp"

// Inpainters
#include "Inpainters/PatchInpainter.hpp"

// Difference functions
#include "DifferenceFunctions/ImagePatchDifference.hpp"

// Utilities
#include "Utilities/PatchHelpers.h"

// Inpainting
#include "Algorithms/InpaintingAlgorithm.hpp"

// Priority
#include "Priority/PriorityRandom.h"

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>

// GUI
#include "Interactive/BasicViewerWidget.h"

template <typename TImage>
void DummyPatchesDriver(typename itk::SmartPointer<TImage> originalImage,
                        Mask::Pointer mask, const unsigned int patchHalfWidth)
{
  // Get the region so that we can reference it without referring to a particular image
  itk::ImageRegion<2> fullRegion = originalImage->GetLargestPossibleRegion();

  typedef ImagePatchPixelDescriptor<TImage> ImagePatchPixelDescriptorType;

  // Create the graph
  typedef boost::grid_graph<2> VertexListGraphType;
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

  // Create the patch inpainters.
  typedef PatchInpainter<TImage> InpainterType;
  std::shared_ptr<InpainterType> imageInpainter(
      new InpainterType(patchHalfWidth, originalImage, mask));

  // Create the priority function
  typedef PriorityRandom PriorityType;
  std::shared_ptr<PriorityType> priorityFunction(new PriorityType);

  // Queue
  typedef IndirectPriorityQueue<VertexListGraphType> BoundaryNodeQueueType;
  std::shared_ptr<BoundaryNodeQueueType> boundaryNodeQueue(new BoundaryNodeQueueType(*graph));

  // Create the descriptor visitor
  typedef ImagePatchDescriptorVisitor<VertexListGraphType, TImage, ImagePatchDescriptorMapType>
          ImagePatchDescriptorVisitorType;

  typedef AlwaysAccept<VertexListGraphType> AcceptanceVisitorType;
  std::shared_ptr<AcceptanceVisitorType> acceptanceVisitor(new AcceptanceVisitorType);

  // Use the slightly blurred image here, as this is where the patch objects get created, and later these patch objects
  // are passed to the SSD function.
  std::shared_ptr<ImagePatchDescriptorVisitorType> imagePatchDescriptorVisitor(
        new ImagePatchDescriptorVisitorType(originalImage, mask,
                                            imagePatchDescriptorMap, patchHalfWidth));

  typedef InpaintingVisitor<VertexListGraphType, BoundaryNodeQueueType,
                            ImagePatchDescriptorVisitorType, AcceptanceVisitorType,
                            PriorityType>
                            InpaintingVisitorType;
  std::shared_ptr<InpaintingVisitorType> inpaintingVisitor(
        new InpaintingVisitorType(mask, boundaryNodeQueue,
                                  imagePatchDescriptorVisitor, acceptanceVisitor,
                                  priorityFunction, patchHalfWidth,
                                  "InpaintingVisitor"));

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

  // Since we are using a KNNSearchAndSort, we just have to return the top patch after the sort,
  // so we use this trival Best searcher.
  typedef FirstValidDescriptor<ImagePatchDescriptorMapType> BestSearchType;
  std::shared_ptr<BestSearchType> bestSearch(new BestSearchType(imagePatchDescriptorMap));

  typedef BasicViewerWidget<TImage> BasicViewerWidgetType;
  BasicViewerWidgetType* basicViewer = new BasicViewerWidgetType(originalImage, mask);
  std::cout << "basicViewer pointer: " << basicViewer << std::endl;
  basicViewer->ConnectVisitor(displayVisitor.get());
  basicViewer->show();

  // Run the remaining inpainting with interaction
  std::cout << "Running inpainting..." << std::endl;

  QtConcurrent::run(boost::bind(InpaintingAlgorithm<
                                VertexListGraphType, CompositeInpaintingVisitorType,
                                BoundaryNodeQueueType, BestSearchType,
                                InpainterType>,
                                graph, compositeInpaintingVisitor, boundaryNodeQueue,
                                bestSearch, imageInpainter));

}

#endif
