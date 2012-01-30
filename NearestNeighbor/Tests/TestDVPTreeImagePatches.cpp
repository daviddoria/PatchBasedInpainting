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

// Pixel descriptors
#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

// Visitor
#include "Visitors/DefaultInpaintingVisitor.hpp"

// Nearest neighbors
#include "NearestNeighbor/metric_space_search.hpp"

// Topologies
#include "Topologies/ImagePatchTopology.hpp"

// Initializers
//#include "Initializers/InitializeFromMaskImage.hpp"
#include "Initializers/SimpleInitializer.hpp"

// ITK
#include "itkImageFileReader.h"

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>

namespace boost 
{
  enum vertex_image_patch_t { vertex_image_patch };
  BOOST_INSTALL_PROPERTY(vertex, image_patch);
};

int main(int argc, char *argv[])
{
  // Verify arguments
  if(argc != 4)
    {
    std::cerr << "Required arguments: image.mha imageMask.mha patchRadius" << std::endl;
    return EXIT_FAILURE;
    }

  // Parse arguments
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];

  std::stringstream ssPatchRadius;
  ssPatchRadius << argv[3];
  unsigned int patch_half_width = 0;
  ssPatchRadius >> patch_half_width;

  // Output arguments
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;
  std::cout << "Patch half width: " << patch_half_width << std::endl;

  typedef FloatVectorImageType ImageType;

  typedef  itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename.c_str());
  imageReader->Update();

  ImageType::Pointer image = ImageType::New();
  ITKHelpers::DeepCopy(imageReader->GetOutput(), image.GetPointer());

  typedef  itk::ImageFileReader<Mask> MaskReaderType;

  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFilename);
  maskReader->Update();
  std::cout << "hole pixels: " << maskReader->GetOutput()->CountHolePixels() << std::endl;
  std::cout << "valid pixels: " << maskReader->GetOutput()->CountValidPixels() << std::endl;

  // Create the graph
  typedef boost::grid_graph<2> VertexListGraphType;
  boost::array<std::size_t, 2> graphSideLengths = { { imageReader->GetOutput()->GetLargestPossibleRegion().GetSize()[0],
                                                      imageReader->GetOutput()->GetLargestPossibleRegion().GetSize()[1] } };
  VertexListGraphType graph(graphSideLengths);
  typedef boost::graph_traits<VertexListGraphType>::vertex_descriptor VertexDescriptorType;

  // Create the topology
  typedef ImagePatchTopology<ImageType> TopologyType;
  TopologyType space;

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

  // Create the nearby hole map. A node is on the current boundary if this property is true.
  typedef boost::vector_property_map<std::vector<VertexDescriptorType>, IndexMapType> NearbyHoleMapType;
  NearbyHoleMapType nearbyHoleMap(num_vertices(graph), indexMap);

  // Create the priority compare functor
  typedef std::less<float> PriorityCompareType;
  PriorityCompareType lessThanFunctor;

  // Create the descriptor map. This is where the data for each pixel is stored. The Topology
  typedef boost::vector_property_map<TopologyType::point_type, IndexMapType> DescriptorMapType;
  DescriptorMapType descriptorMap(num_vertices(graph), indexMap);

  DefaultInpaintingVisitor visitor;

  // Initialize
  SimpleInitializer(maskReader->GetOutput(), &visitor, graph);

  // Create the nearest neighbor finder
  std::cout << "Creating tree..." << std::endl;
  typedef dvp_tree<VertexDescriptorType, TopologyType, DescriptorMapType > TreeType;
  TreeType tree(graph, space, descriptorMap);
  typedef multi_dvp_tree_search<VertexListGraphType, TreeType> SearchType;
  SearchType nearestNeighborFinder;
  nearestNeighborFinder.graph_tree_map[&graph] = &tree;
  std::cout << "Finished creating tree." << std::endl;

  return EXIT_SUCCESS;
}
