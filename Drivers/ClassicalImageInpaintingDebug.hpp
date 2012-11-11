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

#ifndef ClassicalImageInpaintingDebug_HPP
#define ClassicalImageInpaintingDebug_HPP

// Custom
#include "Utilities/IndirectPriorityQueue.h"

// STL
#include <memory>

// Information visitors
#include "Visitors/InformationVisitors/PatchIndicatorVisitor.hpp"
#include "Visitors/InformationVisitors/IterationWriterVisitor.hpp"

// Submodules
#include <Helpers/Helpers.h>

// Pixel descriptors
#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

// Descriptor visitors
#include "Visitors/DescriptorVisitors/ImagePatchDescriptorVisitor.hpp"

// Inpainting visitors
#include "Visitors/InpaintingVisitor.hpp"
#include "Visitors/AcceptanceVisitors/DefaultAcceptanceVisitor.hpp"
#include "Visitors/CompositeInpaintingVisitor.hpp"

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
#include "DifferenceFunctions/ImagePatchDifference.hpp"
#include "DifferenceFunctions/SumAbsolutePixelDifference.hpp"
#include "DifferenceFunctions/SumSquaredPixelDifference.hpp"

// Utilities
#include "Utilities/PatchHelpers.h"

// Inpainting
#include "Algorithms/InpaintingAlgorithm.hpp"

// Priority
#include "Priority/PriorityCriminisi.h"
#include "Priority/PriorityRandom.h"
#include "Priority/PriorityConfidence.h"

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>

template <typename TImage>
void ClassicalImageInpaintingDebug(typename itk::SmartPointer<TImage> originalImage,
                                   Mask* const mask,
                                   const unsigned int patchHalfWidth)
{
  itk::ImageRegion<2> fullRegion = originalImage->GetLargestPossibleRegion();

  // Blur the image
  typedef TImage BlurredImageType; // Usually the blurred image is the same type as the original image.
  typename BlurredImageType::Pointer blurredImage = BlurredImageType::New();
  float blurVariance = 2.0f;
  MaskOperations::MaskedBlur(originalImage.GetPointer(), mask, blurVariance, blurredImage.GetPointer());

  ITKHelpers::WriteRGBImage(blurredImage.GetPointer(), "BlurredImage.png");

  typedef ImagePatchPixelDescriptor<TImage> ImagePatchPixelDescriptorType;

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

  // Create the patch inpainter.
  typedef PatchInpainter<TImage> OriginalImageInpainterType;
  std::shared_ptr<OriginalImageInpainterType> originalImagePatchInpainter(new
      OriginalImageInpainterType(patchHalfWidth, originalImage, mask));
  // Show the inpainted image at each iteration
//  originalImagePatchInpainter.SetDebugImages(true);
//  originalImagePatchInpainter.SetImageName("RGB");

  // Create an inpainter for the blurred image.
  typedef PatchInpainter<BlurredImageType> BlurredImageInpainterType;
  std::shared_ptr<BlurredImageInpainterType> blurredImagePatchInpainter(new
     BlurredImageInpainterType(patchHalfWidth, blurredImage, mask));

  // Create a composite inpainter.
  std::shared_ptr<CompositePatchInpainter> inpainter(new CompositePatchInpainter);
  inpainter->AddInpainter(originalImagePatchInpainter);
  inpainter->AddInpainter(blurredImagePatchInpainter);

  // Create the priority function
  typedef PriorityCriminisi<BlurredImageType> PriorityType;
  std::shared_ptr<PriorityType> priorityFunction(new PriorityType(blurredImage, mask, patchHalfWidth));

  // Create the descriptor visitor
  typedef ImagePatchDescriptorVisitor<VertexListGraphType, TImage, ImagePatchDescriptorMapType>
      ImagePatchDescriptorVisitorType;
  std::shared_ptr<ImagePatchDescriptorVisitorType> imagePatchDescriptorVisitor(new
      ImagePatchDescriptorVisitorType(originalImage.GetPointer(), mask,
                                      imagePatchDescriptorMap, patchHalfWidth));

  typedef DefaultAcceptanceVisitor<VertexListGraphType> AcceptanceVisitorType;
  std::shared_ptr<AcceptanceVisitorType> acceptanceVisitor(new AcceptanceVisitorType);

  // Create the inpainting visitor
  typedef InpaintingVisitor<VertexListGraphType, BoundaryNodeQueueType,
                            ImagePatchDescriptorVisitorType, AcceptanceVisitorType, PriorityType>
                            InpaintingVisitorType;
  std::shared_ptr<InpaintingVisitorType> inpaintingVisitor(new InpaintingVisitorType(mask, boundaryNodeQueue,
                                          imagePatchDescriptorVisitor, acceptanceVisitor,
                                          priorityFunction, patchHalfWidth, "InpaintingVisitor"));
  inpaintingVisitor->SetAllowNewPatches(false);
//  inpaintingVisitor.SetDebugImages(true); // Write PatchesCopied images that show the source and target patch at each iteration

  typedef PatchIndicatorVisitor<VertexListGraphType, TImage> PatchIndicatorVisitorType;
  PatchIndicatorVisitorType patchIndicatorVisitor(originalImage, mask, patchHalfWidth);

  typedef IterationWriterVisitor <VertexListGraphType, TImage> IterationWriterVisitorType;
  IterationWriterVisitorType iterationWriterVisitor(originalImage, mask);

  // Create the composite inpainting visitor
  typedef CompositeInpaintingVisitor<VertexListGraphType> CompositeInpaintingVisitorType;
  std::shared_ptr<CompositeInpaintingVisitorType> compositeInpaintingVisitor(
        new CompositeInpaintingVisitorType);
  compositeInpaintingVisitor->AddVisitor(inpaintingVisitor);
  compositeInpaintingVisitor->AddVisitor(&patchIndicatorVisitor);
  compositeInpaintingVisitor->AddVisitor(&iterationWriterVisitor);

  InitializePriority(mask, boundaryNodeQueue.get(), priorityFunction.get());

  // Initialize the boundary node queue from the user provided mask image.
  InitializeFromMaskImage<InpaintingVisitorType, VertexDescriptorType>(mask, inpaintingVisitor.get());

  // Create the nearest neighbor finder
  typedef ImagePatchDifference<ImagePatchPixelDescriptorType,
      SumSquaredPixelDifference<typename TImage::PixelType> > PatchDifferenceType;

  // Do not write top patch grid at each iteration
//  typedef LinearSearchBestProperty<ImagePatchDescriptorMapType,
//                                   PatchDifferenceType> BestSearchType;
//  std::shared_ptr<BestSearchType> linearSearchBest(new BestSearchType(*imagePatchDescriptorMap));

  // Write top patch grid at each iteration. To do this, we need a KNNSearcher
  // to pass a list of valid patches to the FirstAndWrite class.
  typedef LinearSearchKNNProperty<ImagePatchDescriptorMapType,
                                  PatchDifferenceType > KNNSearchType;

  unsigned int knn = 100;
  std::shared_ptr<KNNSearchType> knnSearch(new KNNSearchType(imagePatchDescriptorMap, knn));

  typedef LinearSearchBestFirstAndWrite<ImagePatchDescriptorMapType, TImage,
                                   PatchDifferenceType> BestSearchType;
  std::shared_ptr<BestSearchType> linearSearchBest(
        new BestSearchType(*imagePatchDescriptorMap, originalImage, mask));

  typedef KNNBestWrapper<KNNSearchType, BestSearchType> KNNWrapperType;
  std::shared_ptr<KNNWrapperType> knnWrapper(new KNNWrapperType(knnSearch,
                                                                linearSearchBest));

  // Perform the inpainting
//  InpaintingAlgorithm(graph, inpaintingVisitor, boundaryNodeQueue,
//                      linearSearchBest, inpainter);

//  InpaintingAlgorithm<VertexListGraphType, InpaintingVisitorType,
//                      BoundaryNodeQueueType, BestSearchType,
//                      CompositePatchInpainter>(graph, inpaintingVisitor, boundaryNodeQueue,
//                      linearSearchBest, inpainter);

  InpaintingAlgorithm<VertexListGraphType, CompositeInpaintingVisitorType,
                      BoundaryNodeQueueType, KNNWrapperType,
                      CompositePatchInpainter>(graph, compositeInpaintingVisitor,
                                               boundaryNodeQueue,
                                               knnWrapper, inpainter);

}

#endif
