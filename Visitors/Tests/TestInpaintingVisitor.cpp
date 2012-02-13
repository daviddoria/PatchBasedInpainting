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

// Priority
#include "Priority/PriorityRandom.h"

// Pixel descriptors
#include "PixelDescriptors/ImagePatchPixelDescriptor.h"
#include "PixelDescriptors/FeatureVectorPixelDescriptor.h"

// Visitors
#include "Visitors/FeatureVectorPrecomputedPolyDataDescriptorVisitor.hpp"
#include "Visitors/InpaintingVisitor.hpp"

// Nearest neighbors
#include "NearestNeighbor/LinearSearchBestProperty.hpp"

// Initializers
#include "Initializers/InitializeFromMaskImage.hpp"
#include "Initializers/InitializePriority.hpp"

// Inpainters
#include "Inpainters/MaskedGridPatchInpainter.hpp"

// Difference functions
#include "DifferenceFunctions/FeatureVectorDifference.hpp"

// ITK
#include "itkVectorImage.h"

// VTK
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataReader.h>

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/detail/d_ary_heap.hpp>

int main(int argc, char *argv[])
{
  typedef  itk::VectorImage<float, 2> ImageType;
  ImageType::Pointer image = ImageType::New();

  Mask::Pointer mask = Mask::New();

  typedef ImagePatchPixelDescriptor<ImageType> ImagePatchPixelDescriptorType;
  typedef FeatureVectorPixelDescriptor FeatureVectorPixelDescriptorType;

  // Create the graph
  typedef boost::grid_graph<2> VertexListGraphType;
  const unsigned int graphSize = 10;
  boost::array<std::size_t, 2> graphSideLengths = { { graphSize, graphSize } };
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
  typedef boost::vector_property_map<FeatureVectorPixelDescriptorType, IndexMapType> FeatureVectorDescriptorMapType;
  FeatureVectorDescriptorMapType featureVectorDescriptorMap(num_vertices(graph), indexMap);

  // Create the priority function
  typedef PriorityRandom<itk::Index<2> > PriorityType;
  PriorityType priorityFunction;

  // Create the boundary node queue. The priority of each node is used to order the queue.
  typedef boost::vector_property_map<std::size_t, IndexMapType> IndexInHeapMap;
  IndexInHeapMap index_in_heap(indexMap);

  // Create the priority compare functor
  typedef std::less<float> PriorityCompareType;
  PriorityCompareType lessThanFunctor;

  typedef boost::d_ary_heap_indirect<VertexDescriptorType, 4, IndexInHeapMap, PriorityMapType, PriorityCompareType> BoundaryNodeQueueType;
  BoundaryNodeQueueType boundaryNodeQueue(priorityMap, index_in_heap, lessThanFunctor);

  unsigned int patch_half_width = 10;
  vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
  std::string featureName = "test";

  typedef FeatureVectorPrecomputedPolyDataDescriptorVisitor<VertexListGraphType, FeatureVectorDescriptorMapType> FeatureVectorPrecomputedPolyDataDescriptorVisitorType;
  FeatureVectorPrecomputedPolyDataDescriptorVisitorType featureVectorPrecomputedPolyDataDescriptorVisitor(featureVectorDescriptorMap, polydata, featureName);

  typedef InpaintingVisitor<VertexListGraphType, ImageType, BoundaryNodeQueueType, FillStatusMapType,
                            FeatureVectorPrecomputedPolyDataDescriptorVisitorType, PriorityType, PriorityMapType, BoundaryStatusMapType> InpaintingVisitorType;
  InpaintingVisitorType visitor(image, mask, boundaryNodeQueue, fillStatusMap,
                                featureVectorPrecomputedPolyDataDescriptorVisitor, priorityMap, &priorityFunction, patch_half_width, boundaryStatusMap);
  return 0;
}
