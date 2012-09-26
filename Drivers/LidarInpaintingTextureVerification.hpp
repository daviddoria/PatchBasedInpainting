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

#ifndef LidarInpaintingTextureVerification_HPP
#define LidarInpaintingTextureVerification_HPP

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
#include "NearestNeighbor/LinearSearchBest/HistogramCorrelation.hpp"
#include "NearestNeighbor/LinearSearchBest/HistogramDifference.hpp"
#include "NearestNeighbor/LinearSearchBest/HoleHistogramDifference.hpp"
#include "NearestNeighbor/LinearSearchBest/HistogramNewColors.hpp"
#include "NearestNeighbor/LinearSearchBest/DualHistogramDifference.hpp"
#include "NearestNeighbor/LinearSearchBest/AdaptiveDualHistogramDifference.hpp"
#include "NearestNeighbor/LinearSearchBest/AdaptiveDualQuadrantHistogramDifference.hpp"
#include "NearestNeighbor/LinearSearchBest/IntroducedEnergy.hpp"
#include "NearestNeighbor/LinearSearchBest/First.hpp"
#include "NearestNeighbor/LinearSearchBest/FirstAndWrite.hpp"
#include "NearestNeighbor/LinearSearchBest/Texture.hpp"
#include "NearestNeighbor/LinearSearchBest/ColorTexture.hpp"
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
#include "Utilities/IndirectPriorityQueue.h"

// Inpainting
#include "Algorithms/InpaintingAlgorithm.hpp"

// Priority
#include "Priority/PriorityRandom.h"
#include "Priority/PriorityCriminisi.h"

// Submodules
#include <Helpers/Helpers.h>
#include <ITKVTKHelpers/ITKVTKHelpers.h>
#include <Mask/MaskOperations.h>

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>

/** It is expected that this function be passed an RGBDxDy image. */
template <typename TImage>
void LidarInpaintingTextureVerification(TImage* const originalImage, Mask* const mask,
                                        const unsigned int patchHalfWidth, const unsigned int numberOfKNN)
{
  itk::ImageRegion<2> fullRegion = originalImage->GetLargestPossibleRegion();

  // Extract the RGB image
  typedef itk::Image<itk::CovariantVector<float, 3>, 2> RGBImageType;
  std::vector<unsigned int> firstThreeChannels = {0,1,2};
  RGBImageType::Pointer rgbImage = RGBImageType::New();
  ITKHelpers::ExtractChannels(originalImage, firstThreeChannels, rgbImage.GetPointer());

  // Create the HSV image
  typedef itk::Image<itk::CovariantVector<float, 3>, 2> HSVImageType;
  HSVImageType::Pointer hsvImage = HSVImageType::New();
  ITKVTKHelpers::ConvertRGBtoHSV(rgbImage.GetPointer(), hsvImage.GetPointer());

  ITKHelpers::WriteImage(hsvImage.GetPointer(), "HSVImage.mha");

  // Stack the HSV image with the original rest of the channels
  typedef itk::Image<itk::CovariantVector<float, 5>, 2> HSVDxDyImageType;
  HSVDxDyImageType::Pointer hsvDxDyImage = HSVDxDyImageType::New();
  ITKHelpers::DeepCopy(originalImage, hsvDxDyImage.GetPointer());

  ITKHelpers::ReplaceChannels(hsvDxDyImage.GetPointer(), firstThreeChannels, hsvImage.GetPointer());

  // Blur the image for gradient computation stability (Criminisi's data term)
  RGBImageType::Pointer blurredImage = RGBImageType::New();
  float blurVariance = 2.0f;
  MaskOperations::MaskedBlur(rgbImage.GetPointer(), mask, blurVariance, blurredImage.GetPointer());

  ITKHelpers::WriteRGBImage(blurredImage.GetPointer(), "BlurredImage.png");

  // Blur the image slightly so that the SSD comparisons are not so noisy
  typename TImage::Pointer slightlyBlurredImage = TImage::New();
  float slightBlurVariance = 1.0f;
//  MaskOperations::MaskedBlur(originalImage, mask, slightBlurVariance, slightlyBlurredImage.GetPointer());
  MaskOperations::MaskedBlur(hsvDxDyImage.GetPointer(), mask, slightBlurVariance, slightlyBlurredImage.GetPointer());

  ITKHelpers::WriteRGBImage(slightlyBlurredImage.GetPointer(), "SlightlyBlurredImage.png");

  // Create the graph
  typedef ImagePatchPixelDescriptor<TImage> ImagePatchPixelDescriptorType;

  typedef boost::grid_graph<2> VertexListGraphType;

  // We can't make this a signed type (size_t versus int) because we allow negative
  boost::array<std::size_t, 2> graphSideLengths = { { fullRegion.GetSize()[0],
                                                      fullRegion.GetSize()[1] } };
  VertexListGraphType graph(graphSideLengths);
  typedef boost::graph_traits<VertexListGraphType>::vertex_descriptor VertexDescriptorType;
  typedef boost::graph_traits<VertexListGraphType>::vertex_iterator VertexIteratorType;

  // Get the index map
  typedef boost::property_map<VertexListGraphType, boost::vertex_index_t>::const_type IndexMapType;
  IndexMapType indexMap(get(boost::vertex_index, graph));

  // Create the descriptor map. This is where the data for each pixel is stored.
  typedef boost::vector_property_map<ImagePatchPixelDescriptorType, IndexMapType> ImagePatchDescriptorMapType;
  ImagePatchDescriptorMapType imagePatchDescriptorMap(num_vertices(graph), indexMap);

  // Create the patch inpainter.
  typedef PatchInpainter<TImage> OriginalImageInpainterType;
  OriginalImageInpainterType originalImagePatchInpainter(patchHalfWidth, originalImage, mask);
  originalImagePatchInpainter.SetDebugImages(true);
  originalImagePatchInpainter.SetImageName("RGB");

  // Create an inpainter for the HSV image.
  typedef PatchInpainter<HSVImageType> HSVImageInpainterType;
  HSVImageInpainterType hsvImagePatchInpainter(patchHalfWidth, hsvImage, mask);

  // Create an inpainter for the blurred image.
  typedef PatchInpainter<RGBImageType> BlurredImageInpainterType;
  BlurredImageInpainterType blurredImagePatchInpainter(patchHalfWidth, blurredImage, mask);

  // Create an inpainter for the slightly blurred image.
  typedef PatchInpainter<TImage> SlightlyBlurredImageInpainterType;
  SlightlyBlurredImageInpainterType slightlyBlurredImagePatchInpainter(patchHalfWidth, slightlyBlurredImage, mask);

  // Create a composite inpainter.
  CompositePatchInpainter inpainter;
  inpainter.AddInpainter(&originalImagePatchInpainter);
  inpainter.AddInpainter(&hsvImagePatchInpainter);
  inpainter.AddInpainter(&blurredImagePatchInpainter);
  inpainter.AddInpainter(&slightlyBlurredImagePatchInpainter);

  // Create the priority function
  typedef PriorityCriminisi<RGBImageType> PriorityType;
  PriorityType priorityFunction(blurredImage, mask, patchHalfWidth);
//  priorityFunction.SetDebugLevel(1);

  // Queue
  typedef IndirectPriorityQueue<VertexListGraphType> BoundaryNodeQueueType;
  BoundaryNodeQueueType boundaryNodeQueue(graph);

  // Create the descriptor visitor (used for SSD comparisons).
  typedef ImagePatchDescriptorVisitor<VertexListGraphType, TImage, ImagePatchDescriptorMapType>
      ImagePatchDescriptorVisitorType;
//  ImagePatchDescriptorVisitorType imagePatchDescriptorVisitor(originalImage, mask,
//                                  imagePatchDescriptorMap, patchHalfWidth); // Use the non-blurred image for the SSD comparisons
  ImagePatchDescriptorVisitorType imagePatchDescriptorVisitor(slightlyBlurredImage, mask,
                                  imagePatchDescriptorMap, patchHalfWidth); // Use the slightly blurred HSV image for the SSD comparisons

  typedef DefaultAcceptanceVisitor<VertexListGraphType> AcceptanceVisitorType;
  AcceptanceVisitorType acceptanceVisitor;

  // Create the inpainting visitor. (The mask is inpainted in FinishVertex)
  typedef InpaintingVisitor<VertexListGraphType, BoundaryNodeQueueType,
      ImagePatchDescriptorVisitorType, AcceptanceVisitorType, PriorityType, TImage>
      InpaintingVisitorType;
  InpaintingVisitorType inpaintingVisitor(mask, boundaryNodeQueue,
                                          imagePatchDescriptorVisitor, acceptanceVisitor,
                                          &priorityFunction, patchHalfWidth, "InpaintingVisitor", originalImage);
  inpaintingVisitor.SetDebugImages(true);
  inpaintingVisitor.SetAllowNewPatches(false);

  InitializePriority(mask, boundaryNodeQueue, &priorityFunction);

  // Initialize the boundary node queue from the user provided mask image.
  InitializeFromMaskImage<InpaintingVisitorType, VertexDescriptorType>(mask, &inpaintingVisitor);
  std::cout << "PatchBasedInpaintingNonInteractive: There are " << boundaryNodeQueue.CountValidNodes()
            << " nodes in the boundaryNodeQueue" << std::endl;

  // Select squared or absolute pixel error
//  typedef ImagePatchDifference<ImagePatchPixelDescriptorType, SumAbsolutePixelDifference<TImage::PixelType> > PatchDifferenceType;
  typedef ImagePatchDifference<ImagePatchPixelDescriptorType,
      SumSquaredPixelDifference<typename TImage::PixelType> > PatchDifferenceType;

  // Create the first (KNN) neighbor finder
  typedef LinearSearchKNNProperty<ImagePatchDescriptorMapType, PatchDifferenceType> KNNSearchType;
  KNNSearchType linearSearchKNN(imagePatchDescriptorMap, numberOfKNN);

  // Setup the second (1-NN) neighbor finder
  typedef std::vector<VertexDescriptorType>::iterator VertexDescriptorVectorIteratorType;

  // This is templated on TImage because we need it to write out debug patches from this searcher (since we are not using an RGB image to compute the histograms)
//  typedef LinearSearchBestTexture<ImagePatchDescriptorMapType, HSVImageType,
//      VertexDescriptorVectorIteratorType, TImage> BestSearchType; // Use the histogram of the gradient magnitudes of a scalar represetnation of the image (e.g. magnitude image)
  typedef LinearSearchBestColorTexture<ImagePatchDescriptorMapType, HSVImageType,
      VertexDescriptorVectorIteratorType, TImage> BestSearchType; // Use the concatenated histograms of the gradient magnitudes of each channel

  BestSearchType linearSearchBest(imagePatchDescriptorMap, hsvImage.GetPointer(), mask);

  // Setup the two step neighbor finder

  // Without writing top KNN patches
//  TwoStepNearestNeighbor<KNNSearchType, BestSearchType>
//      twoStepNearestNeighbor(linearSearchKNN, linearSearchBest);

  // With writing top KNN patches
  typedef LinearSearchBestFirstAndWrite<ImagePatchDescriptorMapType,
      TImage, PatchDifferenceType> TopPatchesWriterType;
  TopPatchesWriterType topPatchesWriter(imagePatchDescriptorMap, originalImage, mask);

  TwoStepNearestNeighbor<KNNSearchType, BestSearchType, TopPatchesWriterType>
      twoStepNearestNeighbor(linearSearchKNN, linearSearchBest, topPatchesWriter);

  // Perform the inpainting
  InpaintingAlgorithm(graph, inpaintingVisitor, &boundaryNodeQueue,
                      twoStepNearestNeighbor, &inpainter);

}

#endif
