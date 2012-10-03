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
#include "NearestNeighbor/LinearSearchBest/LidarTextureDerivatives.hpp"
#include "NearestNeighbor/LinearSearchBest/LidarHSVTextureGradient.hpp"

// Multi-Nearest neighbors functions
#include "NearestNeighbor/LinearSearchKNNProperty.hpp"
#include "NearestNeighbor/LinearSearchKNNPropertyLimitLocalReuse.hpp"
#include "NearestNeighbor/LinearSearchKNNPropertyLimitReuse.hpp"
#include "NearestNeighbor/LinearSearchKNNPropertyNoReuse.hpp"
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
#include "DifferenceFunctions/WeightedSumSquaredPixelDifference.hpp"

// Utilities
#include "Utilities/PatchHelpers.h"
#include "Utilities/IndirectPriorityQueue.h"

// Inpainting
#include "Algorithms/InpaintingAlgorithm.hpp"
#include "Algorithms/InpaintingAlgorithmWithLocalSearch.hpp"
#include "SearchRegions/NeighborhoodSearch.hpp"

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
  RGBImageType::Pointer blurredRGBImage = RGBImageType::New();
  float blurVariance = 2.0f;
  MaskOperations::MaskedBlur(rgbImage.GetPointer(), mask, blurVariance, blurredRGBImage.GetPointer());

  ITKHelpers::WriteRGBImage(blurredRGBImage.GetPointer(), "BlurredRGBImage.png");

  // Blur the image slightly so that the SSD comparisons are not so noisy
  typename HSVDxDyImageType::Pointer slightlyBlurredHSVDxDyImage = TImage::New();
  float slightBlurVariance = 1.0f;
  MaskOperations::MaskedBlur(hsvDxDyImage.GetPointer(), mask, slightBlurVariance, slightlyBlurredHSVDxDyImage.GetPointer());

//  ITKHelpers::WriteRGBImage(slightlyBlurredHSVDxDyImage.GetPointer(), "SlightlyBlurredHSVDxDyImage.png"); // only if 3-channel uchar
  ITKHelpers::WriteImage(slightlyBlurredHSVDxDyImage.GetPointer(), "SlightlyBlurredHSVDxDyImage.mha");

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
  BlurredImageInpainterType blurredRGBImagePatchInpainter(patchHalfWidth, blurredRGBImage, mask);

  // Create an inpainter for the slightly blurred image.
  typedef PatchInpainter<TImage> SlightlyBlurredHSVDxDyImageImageInpainterType;
  SlightlyBlurredHSVDxDyImageImageInpainterType slightlyBlurredHSVDxDyImageImagePatchInpainter(patchHalfWidth, slightlyBlurredHSVDxDyImage, mask);

  // Create a composite inpainter. (Note: the mask is inpainted in InpaintingVisitor::FinishVertex)
  CompositePatchInpainter inpainter;
  inpainter.AddInpainter(&originalImagePatchInpainter);
  inpainter.AddInpainter(&hsvImagePatchInpainter);
  inpainter.AddInpainter(&blurredRGBImagePatchInpainter);
  inpainter.AddInpainter(&slightlyBlurredHSVDxDyImageImagePatchInpainter);

  // Create the priority function
  typedef PriorityCriminisi<RGBImageType> PriorityType;
  PriorityType priorityFunction(blurredRGBImage, mask, patchHalfWidth);
//  priorityFunction.SetDebugLevel(1);

  // Queue
  typedef IndirectPriorityQueue<VertexListGraphType> BoundaryNodeQueueType;
  BoundaryNodeQueueType boundaryNodeQueue(graph);

  // Create the descriptor visitor (used for SSD comparisons).
  typedef ImagePatchDescriptorVisitor<VertexListGraphType, TImage, ImagePatchDescriptorMapType>
      ImagePatchDescriptorVisitorType;
//  ImagePatchDescriptorVisitorType imagePatchDescriptorVisitor(originalImage, mask,
//                                  imagePatchDescriptorMap, patchHalfWidth); // Use the non-blurred image for the SSD comparisons
  ImagePatchDescriptorVisitorType imagePatchDescriptorVisitor(slightlyBlurredHSVDxDyImage, mask,
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
  inpaintingVisitor.SetDebugImages(true); // This produces PatchesCopied* images showing where patches were copied from/to at each iteration
//  inpaintingVisitor.SetAllowNewPatches(false);
  inpaintingVisitor.SetAllowNewPatches(true); // we can do this as long as we use one of the LinearSearchKNNProperty*Reuse (like LinearSearchKNNPropertyLimitLocalReuse) in the steps below

  InitializePriority(mask, boundaryNodeQueue, &priorityFunction);

  // Initialize the boundary node queue from the user provided mask image.
  InitializeFromMaskImage<InpaintingVisitorType, VertexDescriptorType>(mask, &inpaintingVisitor);
  std::cout << "PatchBasedInpaintingNonInteractive: There are " << boundaryNodeQueue.CountValidNodes()
            << " nodes in the boundaryNodeQueue" << std::endl;

#define DUseWeightedDifference

#ifdef DUseWeightedDifference
  // Use a weighted difference
  typedef WeightedSumSquaredPixelDifference<typename TImage::PixelType> PixelDifferenceType;
  // The absolute value of the depth derivative range is usually about [0,12], so to make
  // it comparable to to the color image channel range of [0,255], we multiply by 255/12 ~= 20.
//  float depthDerivativeWeight = 20.0f;

  // This should not be computed "by eye" by looking at the Dx and Dy channels of the PTX scan, because there are typically
  // huge depth discontinuties around the objects that are going to be inpainted. We'd have to look at the masked version of this
  // image to determine the min/max values of the unmasked pixels. They will be much smaller than the min/max values of the original
  // image, which will make the depth derivative weights much higher (~100 or so)

//  std::vector<typename TImage::PixelType> channelMins = ITKHelpers::ComputeMinOfAllChannels(originalImage);
//  std::vector<typename TImage::PixelType> channelMaxs = ITKHelpers::ComputeMaxOfAllChannels(originalImage);
  typename TImage::PixelType channelMins;
  ITKHelpers::ComputeMinOfAllChannels(originalImage, channelMins);

  typename TImage::PixelType channelMaxs;
  ITKHelpers::ComputeMaxOfAllChannels(originalImage, channelMaxs);

  float minX = fabs(channelMins[3]);
  float maxX = fabs(channelMaxs[3]);
  float maxValueX = std::max(minX, maxX);
  std::cout << "maxValueX = " << maxValueX << std::endl;
  float depthDerivativeWeightX = 255.0f / maxValueX;
  std::cout << "Computed depthDerivativeWeightX = " << depthDerivativeWeightX << std::endl;

  float minY = fabs(channelMins[4]);
  float maxY = fabs(channelMaxs[4]);
  float maxValueY = std::max(minY, maxY);
  std::cout << "maxValueY = " << maxValueY << std::endl;
  float depthDerivativeWeightY = 255.0f / maxValueY;
  std::cout << "Computed depthDerivativeWeightY = " << depthDerivativeWeightY << std::endl;

  std::vector<float> weights = {1.0f, 1.0f, 1.0f, depthDerivativeWeightX, depthDerivativeWeightY};
  PixelDifferenceType pixelDifferenceFunctor(weights);

  typedef ImagePatchDifference<ImagePatchPixelDescriptorType,
      PixelDifferenceType > PatchDifferenceType;
  PatchDifferenceType patchDifferenceFunctor(pixelDifferenceFunctor);
#else
  // Use an unweighted pixel difference
  typedef ImagePatchDifference<ImagePatchPixelDescriptorType,
      SumSquaredPixelDifference<typename TImage::PixelType> > PatchDifferenceType;

  PatchDifferenceType patchDifferenceFunctor;
#endif

//#define DAllowReuse // comment/uncomment this line to toggle allowing patches to be used as the source patch more than once

#ifdef DAllowReuse
  // Create the first (KNN) neighbor finder
  typedef LinearSearchKNNProperty<ImagePatchDescriptorMapType, PatchDifferenceType> KNNSearchType;
  KNNSearchType linearSearchKNN(imagePatchDescriptorMap, numberOfKNN, patchDifferenceFunctor);
#else
//  typedef LinearSearchKNNPropertyNoReuse<ImagePatchDescriptorMapType, PatchDifferenceType> KNNSearchType;
//  KNNSearchType linearSearchKNN(imagePatchDescriptorMap, numberOfKNN,
//                                patchDifferenceFunctor, inpaintingVisitor.GetUsedNodesSetPointer());

//  typedef LinearSearchKNNPropertyLimitReuse<ImagePatchDescriptorMapType, PatchDifferenceType> KNNSearchType;
//  KNNSearchType linearSearchKNN(imagePatchDescriptorMap, mask, numberOfKNN,
//                                patchDifferenceFunctor, inpaintingVisitor.GetCopiedPixelsImage());

  typedef LinearSearchKNNPropertyLimitLocalReuse<ImagePatchDescriptorMapType, PatchDifferenceType, RGBImageType> KNNSearchType;
  KNNSearchType linearSearchKNN(imagePatchDescriptorMap, mask, numberOfKNN,
                                patchDifferenceFunctor, inpaintingVisitor.GetSourcePixelMapImage(),
                                rgbImage.GetPointer());
  linearSearchKNN.SetDebugImages(true);
#endif

  // Setup the second (1-NN) neighbor finder
  typedef std::vector<VertexDescriptorType>::iterator VertexDescriptorVectorIteratorType;

  // This is templated on TImage because we need it to write out debug patches from this searcher (since we are not using an RGB image to compute the histograms)
//  typedef LinearSearchBestTexture<ImagePatchDescriptorMapType, HSVImageType,
//      VertexDescriptorVectorIteratorType, TImage> BestSearchType; // Use the histogram of the gradient magnitudes of a scalar represetnation of the image (e.g. magnitude image)
//  typedef LinearSearchBestLidarTextureDerivatives<ImagePatchDescriptorMapType, HSVImageType,
//      VertexDescriptorVectorIteratorType, TImage> BestSearchType; // Use the concatenated histograms of the absolute value of the derivatives of each channel
  typedef LinearSearchBestLidarTextureGradient<ImagePatchDescriptorMapType, HSVDxDyImageType,
      VertexDescriptorVectorIteratorType, RGBImageType> BestSearchType; // Use the concatenated histograms of the gradient magnitudes of each channel. This HSVDxDyImageType must match the hsvDxDyImage provided below

//  BestSearchType linearSearchBest(imagePatchDescriptorMap, hsvDxDyImage.GetPointer(), mask); // use non-blurred for texture sorting
  Debug debug;
//  debug.SetDebugOutputs(true);
//  debug.SetDebugImages(true);
//  linearSearchBest.SetDebugOutputs(true);
//  linearSearchBest.SetDebugImages(true);

   // use slightly blurred for texture sorting
  BestSearchType linearSearchBest(imagePatchDescriptorMap, slightlyBlurredHSVDxDyImage.GetPointer(),
                                  mask, rgbImage.GetPointer(), debug);
  linearSearchBest.WritePatches = true;


  // Setup the two step neighbor finder
  TwoStepNearestNeighbor<KNNSearchType, BestSearchType>
      twoStepNearestNeighbor(linearSearchKNN, linearSearchBest);

  // #define DFullSearch // comment/uncomment this line to set the search region

#ifdef DFullSearch
  // Perform the inpainting (full search)
  InpaintingAlgorithm(graph, inpaintingVisitor, &boundaryNodeQueue,
                      twoStepNearestNeighbor, &inpainter);
#else
//  unsigned int searchRadius = originalImage->GetLargestPossibleRegion().GetSize()[0]/2/2; // half of the image radius

  // Search the region that is the same size as the image, but centered at the patch. For the very center patch, this will search the whole image.
//  unsigned int searchRadius = originalImage->GetLargestPossibleRegion().GetSize()[0]/2;

  // Use a region twice the size of the image, this should always include (almost) all of the image in the search region.
  // Since only the [0] dimension is used, if the image is not square we could still miss parts of image edge when searching around patches near the edge of the image.
  unsigned int searchRadius = originalImage->GetLargestPossibleRegion().GetSize()[0];

  NeighborhoodSearch<VertexDescriptorType> neighborhoodSearch(originalImage->GetLargestPossibleRegion(),
                                                              searchRadius);


  // Perform the inpainting (local search)
  InpaintingAlgorithmWithLocalSearch(graph, inpaintingVisitor, &boundaryNodeQueue,
                                     twoStepNearestNeighbor, &inpainter, neighborhoodSearch);
#endif

}

#endif
