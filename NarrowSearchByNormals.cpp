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

// Custom
#include "Helpers/HelpersOutput.h"

// Pixel descriptors
#include "PixelDescriptors/ImagePatchPixelDescriptor.h"
#include "PixelDescriptors/FeatureVectorPixelDescriptor.h"

// Descriptor visitors
#include "Visitors/ImagePatchDescriptorVisitor.hpp"
#include "Visitors/FeatureVectorPrecomputedPCLNormalsDescriptorVisitor.hpp"
#include "Visitors/CompositeDescriptorVisitor.hpp"

// Inpainting visitors
#include "Visitors/InpaintingVisitor.hpp"

// Nearest neighbors
#include "NearestNeighbor/LinearSearchBestProperty.hpp"
#include "NearestNeighbor/LinearSearchKNNProperty.hpp"
#include "NearestNeighbor/TwoStepNearestNeighbor.hpp"

// Initializers
#include "Initializers/InitializeFromMaskImage.hpp"
#include "Initializers/InitializePriority.hpp"

// Inpainters
#include "Inpainters/MaskedGridPatchInpainter.hpp"
#include "Inpainters/HoleListPatchInpainter.hpp"

// Difference functions
#include "DifferenceFunctions/ImagePatchDifference.hpp"
#include "DifferenceFunctions/FeatureVectorDifference.hpp"

// Inpainting
#include "Algorithms/InpaintingAlgorithm.hpp"

// Priority
#include "Priority/PriorityRandom.h"

// ITK
#include "itkImageFileReader.h"

// PCL
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/detail/d_ary_heap.hpp>

// Debug
#include "Helpers/HelpersOutput.h"

// Run with: Data/trashcan.mha Data/trashcan_mask.mha 15 Data/trashcan.vtp Intensity filled.mha
int main(int argc, char *argv[])
{
  // Verify arguments
  if(argc != 6)
    {
    std::cerr << "Required arguments: image.mha imageMask.mha patch_half_width normals.pcd output.mha" << std::endl;
    std::cerr << "Input arguments: ";
    for(int i = 1; i < argc; ++i)
      {
      std::cerr << argv[i] << " ";
      }
    return EXIT_FAILURE;
    }

  // Parse arguments
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];

  std::stringstream ssPatchRadius;
  ssPatchRadius << argv[3];
  unsigned int patch_half_width = 0;
  ssPatchRadius >> patch_half_width;

  std::string normalsFileName = argv[4];

  std::string outputFilename = argv[5];

  // Output arguments
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;
  std::cout << "Patch half width: " << patch_half_width << std::endl;
  std::cout << "Reading normals: " << normalsFileName << std::endl;
  std::cout << "Output: " << outputFilename << std::endl;

  pcl::PointCloud<pcl::Normal>::Ptr cloudNormals(new pcl::PointCloud<pcl::Normal>);

  if (pcl::io::loadPCDFile<pcl::Normal> (normalsFileName, *cloudNormals) == -1) //* load the file
  {
    PCL_ERROR ("Couldn't read file test_pcd.pcd \n");
    return (-1);
  }
  
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

  typedef ImagePatchPixelDescriptor<ImageType> ImagePatchPixelDescriptorType;
  typedef FeatureVectorPixelDescriptor FeatureVectorPixelDescriptorType;

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

  // Create the node fill status map. Each pixel is either filled (true) or not filled (false).
  typedef boost::vector_property_map<bool, IndexMapType> FillStatusMapType;
  FillStatusMapType fillStatusMap(num_vertices(graph), indexMap);

  // Create the boundary status map. A node is on the current boundary if this property is true. 
  // This property helps the boundaryNodeQueue because we can mark here if a node has become no longer
  // part of the boundary, so when the queue is popped we can check this property to see if it should
  // actually be processed.
  typedef boost::vector_property_map<bool, IndexMapType> BoundaryStatusMapType;
  BoundaryStatusMapType boundaryStatusMap(num_vertices(graph), indexMap);

  // Create the descriptor map. This is where the data for each pixel is stored.
  typedef boost::vector_property_map<ImagePatchPixelDescriptorType, IndexMapType> ImagePatchDescriptorMapType;
  ImagePatchDescriptorMapType imagePatchDescriptorMap(num_vertices(graph), indexMap);

  // Create the descriptor map. This is where the data for each pixel is stored.
  typedef boost::vector_property_map<FeatureVectorPixelDescriptorType, IndexMapType> FeatureVectorDescriptorMapType;
  FeatureVectorDescriptorMapType featureVectorDescriptorMap(num_vertices(graph), indexMap);

  // Create the patch inpainter. The inpainter needs to know the status of each pixel to determine if they should be inpainted.
  typedef MaskedGridPatchInpainter<FillStatusMapType> InpainterType;
  InpainterType patchInpainter(patch_half_width, fillStatusMap);

  // Create the priority function
  typedef PriorityRandom PriorityType;
  PriorityType priorityFunction;

  // Create the boundary node queue. The priority of each node is used to order the queue.
  typedef boost::vector_property_map<std::size_t, IndexMapType> IndexInHeapMap;
  IndexInHeapMap index_in_heap(indexMap);

  // Create the priority compare functor
  typedef std::less<float> PriorityCompareType;
  PriorityCompareType lessThanFunctor;

  typedef boost::d_ary_heap_indirect<VertexDescriptorType, 4, IndexInHeapMap, PriorityMapType, PriorityCompareType> BoundaryNodeQueueType;
  BoundaryNodeQueueType boundaryNodeQueue(priorityMap, index_in_heap, lessThanFunctor);

  // Create the descriptor visitors
  typedef FeatureVectorPrecomputedPCLNormalsDescriptorVisitor<VertexListGraphType, FeatureVectorDescriptorMapType> FeatureVectorPrecomputedPCLNormalsDescriptorVisitorType;
  FeatureVectorPrecomputedPCLNormalsDescriptorVisitorType featureVectorPrecomputedPCLNormalsDescriptorVisitor(featureVectorDescriptorMap, *cloudNormals);

  typedef ImagePatchDescriptorVisitor<VertexListGraphType, ImageType, ImagePatchDescriptorMapType> ImagePatchDescriptorVisitorType;
  ImagePatchDescriptorVisitorType imagePatchDescriptorVisitor(image, mask, imagePatchDescriptorMap, patch_half_width);

  typedef CompositeDescriptorVisitor<VertexListGraphType> CompositeDescriptorVisitorType;
  CompositeDescriptorVisitorType compositeDescriptorVisitor;
  compositeDescriptorVisitor.AddVisitor(&imagePatchDescriptorVisitor);
  compositeDescriptorVisitor.AddVisitor(&featureVectorPrecomputedPCLNormalsDescriptorVisitor);

  // Create the inpainting visitor
  typedef InpaintingVisitor<VertexListGraphType, ImageType, BoundaryNodeQueueType, FillStatusMapType,
                            CompositeDescriptorVisitorType, PriorityType, PriorityMapType, BoundaryStatusMapType> InpaintingVisitorType;
  InpaintingVisitorType inpaintingVisitor(image, mask, boundaryNodeQueue, fillStatusMap,
                                          compositeDescriptorVisitor, priorityMap, &priorityFunction, patch_half_width, boundaryStatusMap);

  InitializePriority(mask, boundaryNodeQueue, priorityMap, &priorityFunction, boundaryStatusMap);

  // Initialize the boundary node queue from the user provided mask image.
  InitializeFromMaskImage(mask, &inpaintingVisitor, graph, fillStatusMap);
  std::cout << "PatchBasedInpaintingNonInteractive: There are " << boundaryNodeQueue.size()
            << " nodes in the boundaryNodeQueue" << std::endl;

  // Create the nearest neighbor finder
  typedef LinearSearchKNNProperty<FeatureVectorDescriptorMapType, FeatureVectorDifference> KNNSearchType;
  KNNSearchType linearSearchKNN(featureVectorDescriptorMap);

  typedef LinearSearchBestProperty<ImagePatchDescriptorMapType, ImagePatchDifference<ImagePatchPixelDescriptorType> > BestSearchType;
  BestSearchType linearSearchBest(imagePatchDescriptorMap);

  TwoStepNearestNeighbor<KNNSearchType, BestSearchType> twoStepNearestNeighbor(linearSearchKNN, linearSearchBest);

  // Perform the inpainting
  inpainting_loop(graph, inpaintingVisitor, boundaryStatusMap, boundaryNodeQueue, twoStepNearestNeighbor, patchInpainter);

  HelpersOutput::WriteImage<ImageType>(image, outputFilename);

  return EXIT_SUCCESS;
}
