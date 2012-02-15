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
#include "PixelDescriptors/FeatureVectorPixelDescriptor.h"

// Nearest neighbors
#include "NearestNeighbor/ThreeStepNearestNeighbor.hpp"
#include "NearestNeighbor/LinearSearchBestProperty.hpp"
#include "NearestNeighbor/LinearSearchKNNProperty.hpp"

// Differences
#include "DifferenceFunctions/FeatureVectorDifference.hpp"

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>

int main(int argc, char *argv[])
{
  // Create the graph
  typedef boost::grid_graph<2> VertexListGraphType;
  const unsigned int graphSideLength = 10;
  boost::array<std::size_t, 2> graphSideLengths = { { graphSideLength, graphSideLength } };
  VertexListGraphType graph(graphSideLengths);
  typedef boost::graph_traits<VertexListGraphType>::vertex_descriptor VertexDescriptorType;

  // Get the index map
  typedef boost::property_map<VertexListGraphType, boost::vertex_index_t>::const_type IndexMapType;
  IndexMapType indexMap(get(boost::vertex_index, graph));

  // Create the descriptor maps.
  typedef boost::vector_property_map<FeatureVectorPixelDescriptor, IndexMapType> DescriptorMapType1;
  DescriptorMapType1 descriptorMap1(num_vertices(graph), indexMap);
  
  typedef boost::vector_property_map<FeatureVectorPixelDescriptor, IndexMapType> DescriptorMapType2;
  DescriptorMapType2 descriptorMap2(num_vertices(graph), indexMap);
  
  typedef boost::vector_property_map<FeatureVectorPixelDescriptor, IndexMapType> DescriptorMapType3;
  DescriptorMapType3 descriptorMap3(num_vertices(graph), indexMap);

  // Create the nearest neighbor finders
  typedef LinearSearchKNNProperty<DescriptorMapType1, FeatureVectorDifference> KNNSearchType1;
  KNNSearchType1 linearSearchKNN1(descriptorMap1, 1000);

  typedef LinearSearchKNNProperty<DescriptorMapType2, FeatureVectorDifference> KNNSearchType2;
  KNNSearchType2 linearSearchKNN2(descriptorMap2, 1000);
  
  typedef LinearSearchBestProperty<DescriptorMapType3, FeatureVectorDifference> BestSearchType;
  BestSearchType linearSearchBest(descriptorMap3);

  ThreeStepNearestNeighbor<KNNSearchType1, KNNSearchType2, BestSearchType> twoStepNearestNeighbor(linearSearchKNN1, linearSearchKNN2, linearSearchBest);


  return EXIT_SUCCESS;
}
