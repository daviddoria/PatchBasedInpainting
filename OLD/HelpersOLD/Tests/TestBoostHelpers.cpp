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

#include "BoostHelpers.h"

#include <boost/graph/grid_graph.hpp>
#include <boost/graph/detail/d_ary_heap.hpp>

int main()
{
  typedef boost::grid_graph<2> VertexListGraphType;
  const unsigned int sideLength = 10;
  boost::array<std::size_t, 2> graphSideLengths = { { sideLength, sideLength } };
  VertexListGraphType graph(graphSideLengths);
  typedef boost::graph_traits<VertexListGraphType>::vertex_descriptor VertexDescriptorType;

  typedef boost::property_map<VertexListGraphType, boost::vertex_index_t>::const_type IndexMapType;
  IndexMapType indexMap(get(boost::vertex_index, graph));

  typedef boost::vector_property_map<std::size_t, IndexMapType> IndexInHeapMap;
  IndexInHeapMap index_in_heap(indexMap);

  typedef boost::vector_property_map<float, IndexMapType> PriorityMapType;
  PriorityMapType priorityMap(num_vertices(graph), indexMap);

  typedef std::less<float> PriorityCompareType;
  PriorityCompareType lessThanFunctor;

  typedef boost::d_ary_heap_indirect<VertexDescriptorType, 4, IndexInHeapMap, PriorityMapType, PriorityCompareType> BoundaryNodeQueueType;
  BoundaryNodeQueueType nodeQueue(priorityMap, index_in_heap, lessThanFunctor);

  
  for(unsigned int i = 0; i < sideLength; ++i)
    {
    for(unsigned int j = 0; j < sideLength; ++j)
      {
      VertexDescriptorType v;
      v[0] = i;
      v[1] = j;
      float priority = drand48();
      put(priorityMap, v, priority);
      }
    }

  for(unsigned int i = 0; i < sideLength; ++i)
    {
    for(unsigned int j = 0; j < sideLength; ++j)
      {
      VertexDescriptorType v;
      v[0] = i;
      v[1] = j;
      nodeQueue.push(v);
      }
    }
    
  std::cout << "queue size: " << nodeQueue.size() << std::endl;
  
  // Test if adding a node again actually puts it in the queue twice. It does.
  VertexDescriptorType v;
  v[0] = 0;
  v[1] = 0;
  nodeQueue.push(v);

  std::cout << "queue size: " << nodeQueue.size() << std::endl;
  /*
  // Note: you cannot do this - the nodes are pushed before their priority is set, so they do not get ordered by their priority
  for(unsigned int i = 0; i < sideLength; ++i)
    {
    for(unsigned int j = 0; j < sideLength; ++j)
      {
      VertexDescriptorType v;
      v[0] = i;
      v[1] = j;
      nodeQueue.push(v);
      float priority = drand48();
      put(priorityMap, v, priority);
      }
    }*/

  BoostHelpers::OutputQueue(nodeQueue);
  return EXIT_SUCCESS;
}
