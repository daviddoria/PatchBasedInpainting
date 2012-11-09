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

#ifndef ClassicalImageInpaintingBasicViewer_HPP
#define ClassicalImageInpaintingBasicViewer_HPP

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
#include "NearestNeighbor/LinearSearchBest/Property.hpp"

// Pixel descriptors
#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

// Acceptance visitors
#include "Visitors/AcceptanceVisitors/GMHAcceptanceVisitor.hpp"

// Information visitors
#include "Visitors/InformationVisitors/DisplayVisitor.hpp"

// Inpainting visitors
#include "Visitors/InpaintingVisitor.hpp"
#include "Visitors/CompositeInpaintingVisitor.hpp"
#include "Visitors/AcceptanceVisitors/DefaultAcceptanceVisitor.hpp"

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
#include "DifferenceFunctions/SumSquaredPixelDifference.hpp"

// Utilities
#include "Utilities/PatchHelpers.h"

// Inpainting
#include "Algorithms/InpaintingAlgorithm.hpp"

// Priority
#include "Priority/PriorityCriminisi.h"

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>

// GUI
#include "Interactive/BasicViewerWidget.h"

template <typename TImage>
void ClassicalImageInpaintingBasicViewer(typename itk::SmartPointer<TImage> originalImage,
                              Mask::Pointer mask, const unsigned int patchHalfWidth)
{
  // Get the region so that we can reference it without referring to a particular image
  itk::ImageRegion<2> fullRegion = originalImage->GetLargestPossibleRegion();

  // Blur the image enough so that the gradients are useful for the priority computation.
  typedef TImage BlurredImageType; // Usually the blurred image is the same type as the original image.
  typename BlurredImageType::Pointer blurredImage = BlurredImageType::New();
  float blurVariance = 3.0f;
  MaskOperations::MaskedBlur(originalImage.GetPointer(), mask, blurVariance, blurredImage.GetPointer());

  // Blur the image a little bit so that the SSD comparisons are less wild.
  typedef TImage BlurredImageType; // Usually the blurred image is the same type as the original image.
  typename BlurredImageType::Pointer slightlyBlurredImage = BlurredImageType::New();
  float slightBlurVariance = 1.0f;
  MaskOperations::MaskedBlur(originalImage.GetPointer(), mask, slightBlurVariance, slightlyBlurredImage.GetPointer());

  typedef ImagePatchPixelDescriptor<TImage> ImagePatchPixelDescriptorType;

  // Create the graph
  typedef boost::grid_graph<2> VertexListGraphType;
  boost::array<std::size_t, 2> graphSideLengths = { { fullRegion.GetSize()[0],
                                                      fullRegion.GetSize()[1] } };
  std::shared_ptr<VertexListGraphType> graph(new VertexListGraphType(graphSideLengths));
  typedef boost::graph_traits<VertexListGraphType>::vertex_descriptor VertexDescriptorType;

  // Queue
  typedef IndirectPriorityQueue<VertexListGraphType> BoundaryNodeQueueType;
  std::shared_ptr<BoundaryNodeQueueType> boundaryNodeQueue(new BoundaryNodeQueueType(*graph));

  // Create the descriptor map. This is where the data for each pixel is stored.
  typedef boost::vector_property_map<ImagePatchPixelDescriptorType,
      BoundaryNodeQueueType::IndexMapType> ImagePatchDescriptorMapType;
  std::shared_ptr<ImagePatchDescriptorMapType> imagePatchDescriptorMap(
        new ImagePatchDescriptorMapType(num_vertices(*graph), *(boundaryNodeQueue->GetIndexMap())));

  // Create the patch inpainters.
  typedef PatchInpainter<TImage> OriginalImageInpainterType;
  std::shared_ptr<OriginalImageInpainterType> originalImageInpainter(
      new OriginalImageInpainterType(patchHalfWidth, originalImage, mask));

  typedef PatchInpainter<BlurredImageType> BlurredImageInpainterType;
  std::shared_ptr<BlurredImageInpainterType> blurredImageInpainter(
      new BlurredImageInpainterType(patchHalfWidth, blurredImage, mask));

  std::shared_ptr<BlurredImageInpainterType> slightlyBlurredImageInpainter(
      new BlurredImageInpainterType(patchHalfWidth, slightlyBlurredImage, mask));

  // Create a composite inpainter.
  /** We only have to store the composite inpainter in the class, as it stores shared_ptrs
    * to all of the individual inpainters. If the composite inpainter says in scope, the
    * individual inpainters do as well.
    */
  std::shared_ptr<CompositePatchInpainter> compositeInpainter(new CompositePatchInpainter);
  compositeInpainter->AddInpainter(originalImageInpainter);
  compositeInpainter->AddInpainter(blurredImageInpainter);
  compositeInpainter->AddInpainter(slightlyBlurredImageInpainter);

  // Create the priority function
  typedef PriorityCriminisi<TImage> PriorityType;
  std::shared_ptr<PriorityType> priorityFunction(
        new PriorityType(blurredImage, mask, patchHalfWidth));

  // Create the descriptor visitor
  typedef ImagePatchDescriptorVisitor<VertexListGraphType, TImage, ImagePatchDescriptorMapType>
          ImagePatchDescriptorVisitorType;

  typedef DefaultAcceptanceVisitor<VertexListGraphType> AcceptanceVisitorType;
  std::shared_ptr<AcceptanceVisitorType> acceptanceVisitor(new AcceptanceVisitorType);

  // Use the slightly blurred image here, as this is where the patch objects get created, and later these patch objects
  // are passed to the SSD function.
  std::shared_ptr<ImagePatchDescriptorVisitorType> imagePatchDescriptorVisitor(
        new ImagePatchDescriptorVisitorType(slightlyBlurredImage.GetPointer(), mask,
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

  typedef SumSquaredPixelDifference<typename TImage::PixelType> PixelDifferenceType;
  typedef ImagePatchDifference<ImagePatchPixelDescriptorType, PixelDifferenceType >
            ImagePatchDifferenceType;

  typedef ImagePatchDifference<ImagePatchPixelDescriptorType,
      SumSquaredPixelDifference<typename TImage::PixelType> > PatchDifferenceType;

  typedef LinearSearchBestProperty<ImagePatchDescriptorMapType,
                                   PatchDifferenceType> BestSearchType;
  std::shared_ptr<BestSearchType> linearSearchBest(new BestSearchType(*imagePatchDescriptorMap));


  typedef BasicViewerWidget<TImage> BasicViewerWidgetType;
//  std::shared_ptr<BasicViewerWidgetType> basicViewer(new BasicViewerWidgetType(originalImage, mask)); // This shared_ptr will go out of scope when this function ends, so the window will immediately close
  BasicViewerWidgetType* basicViewer = new BasicViewerWidgetType(originalImage, mask);
  std::cout << "basicViewer pointer: " << basicViewer << std::endl;
  basicViewer->ConnectVisitor(displayVisitor.get());
  basicViewer->show();

  // Run the remaining inpainting with interaction
  std::cout << "Running inpainting..." << std::endl;

  QtConcurrent::run(boost::bind(InpaintingAlgorithm<
                                VertexListGraphType, CompositeInpaintingVisitorType,
                                BoundaryNodeQueueType, BestSearchType,
                                CompositePatchInpainter>,
                                graph, compositeInpaintingVisitor, boundaryNodeQueue,
                                linearSearchBest, compositeInpainter));

}

#endif
