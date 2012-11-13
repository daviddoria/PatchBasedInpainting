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

#ifndef InpaintingGMH_HPP
#define InpaintingGMH_HPP

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
#include "NearestNeighbor/FirstValidDescriptor.hpp"
#include "NearestNeighbor/SortByRGBTextureGradient.hpp"
#include "NearestNeighbor/KNNSearchAndSort.hpp"
#include "NearestNeighbor/KNNBestWrapper.hpp"

// Pixel descriptors
#include "PixelDescriptors/ImagePatchPixelDescriptor.h"
#include "PixelDescriptors/ImagePatchVectorized.h"
#include "PixelDescriptors/ImagePatchVectorizedIndices.h"

// Acceptance visitors
#include "Visitors/AcceptanceVisitors/AlwaysAccept.hpp"

// Inpainting visitors
#include "Visitors/InpaintingVisitor.hpp"
#include "Visitors/AcceptanceVisitors/DefaultAcceptanceVisitor.hpp"
#include "Visitors/InpaintingVisitor.hpp"
#include "Visitors/ReplayVisitor.hpp"
#include "Visitors/InformationVisitors/LoggerVisitor.hpp"
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
#include "Algorithms/InpaintingAlgorithm.hpp"

// Priority
#include "Priority/PriorityCriminisi.h"

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>

template <typename TImage>
void InpaintingGMH(typename itk::SmartPointer<TImage> originalImage,
                   Mask::Pointer mask, const unsigned int patchHalfWidth,
                   const unsigned int knn)
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
//  typedef PriorityConfidence PriorityType;
//  std::shared_ptr<PriorityType> priorityFunction(
//        new PriorityType(mask, patchHalfWidth));


  // Create the descriptor visitor
  typedef ImagePatchDescriptorVisitor<VertexListGraphType, TImage, ImagePatchDescriptorMapType>
          ImagePatchDescriptorVisitorType;

  // Use the slightly blurred image here, as this is where the patch objects get created, and later these patch objects
  // are passed to the SSD function.
  std::shared_ptr<ImagePatchDescriptorVisitorType> imagePatchDescriptorVisitor(
        new ImagePatchDescriptorVisitorType(slightlyBlurredImage.GetPointer(), mask,
                                            imagePatchDescriptorMap, patchHalfWidth));

  // Acceptance visitor. Use the slightly blurred image here, as this the gradients will be less noisy.
  unsigned int numberOfBinsPerChannel = 30;
  typedef AlwaysAccept<VertexListGraphType> AcceptanceVisitorType;
  std::shared_ptr<AcceptanceVisitorType> acceptanceVisitor(
        new AcceptanceVisitorType);

  typedef InpaintingVisitor<VertexListGraphType, BoundaryNodeQueueType,
                            ImagePatchDescriptorVisitorType, AcceptanceVisitorType,
                            PriorityType>
                            InpaintingVisitorType;
  std::shared_ptr<InpaintingVisitorType> inpaintingVisitor(
        new InpaintingVisitorType(mask, boundaryNodeQueue,
                                  imagePatchDescriptorVisitor, acceptanceVisitor,
                                  priorityFunction, patchHalfWidth,
                                  "InpaintingVisitor"));

  InitializePriority(mask, boundaryNodeQueue.get(), priorityFunction.get());

  // Initialize the boundary node queue from the user provided mask image.
  InitializeFromMaskImage<InpaintingVisitorType, VertexDescriptorType>(
        mask, inpaintingVisitor.get());
  std::cout << "InteractiveInpaintingWithVerification: There are " << boundaryNodeQueue->size()
            << " nodes in the boundaryNodeQueue" << std::endl;

  typedef SumSquaredPixelDifference<typename TImage::PixelType> PixelDifferenceType;
  typedef ImagePatchDifference<ImagePatchPixelDescriptorType, PixelDifferenceType >
            ImagePatchDifferenceType;

  // Create the nearest neighbor finders
  typedef LinearSearchKNNProperty<ImagePatchDescriptorMapType,
                                  ImagePatchDifferenceType > KNNSearchType;

  std::shared_ptr<KNNSearchType> knnSearch(new KNNSearchType(imagePatchDescriptorMap, knn));

  // Since we are using a KNNSearchAndSort, we just have to return the top patch after the sort,
  // so we use this trival Best searcher.
  typedef LinearSearchBestFirst BestSearchType;
  std::shared_ptr<BestSearchType> bestSearch;

  // Use the slightlyBlurredImage here because we want the gradients to be less noisy
  typedef SortByRGBTextureGradient<ImagePatchDescriptorMapType,
                                   TImage > NeighborSortType;
  std::shared_ptr<NeighborSortType> neighborSortType(
        new NeighborSortType(*imagePatchDescriptorMap, slightlyBlurredImage.GetPointer(), mask, numberOfBinsPerChannel));

  typedef KNNSearchAndSort<KNNSearchType, NeighborSortType, TImage> SearchAndSortType;
  std::shared_ptr<SearchAndSortType> searchAndSort(
        new SearchAndSortType(knnSearch, neighborSortType, originalImage));
  searchAndSort->SetDebugImages(true); // Write top patch grids before and after sorting at every iteration

  typedef KNNBestWrapper<SearchAndSortType, BestSearchType> KNNBestWrapperType;
  std::shared_ptr<KNNBestWrapperType> knnBestWrapper(new KNNBestWrapperType(searchAndSort, bestSearch));

  // Run the remaining inpainting with interaction
  std::cout << "Running inpainting..." << std::endl;

  InpaintingAlgorithm<VertexListGraphType, InpaintingVisitorType,
                      BoundaryNodeQueueType, KNNBestWrapperType,
                      CompositePatchInpainter>
                      (graph, inpaintingVisitor, boundaryNodeQueue,
                       knnBestWrapper, compositeInpainter);

}

#endif
