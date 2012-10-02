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

#ifndef INDIRECTPRIORITYQUEUE_H
#define INDIRECTPRIORITYQUEUE_H

// Boost
#include <boost/heap/binomial_heap.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>

template <typename TGraph>
struct IndirectPriorityQueue
{
  // Typedefs

  typedef typename boost::graph_traits<TGraph>::vertex_iterator VertexIteratorType;
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;
  typedef typename boost::property_map<TGraph, boost::vertex_index_t>::const_type IndexMapType;

  typedef std::less<float> PriorityCompareType;
  typedef boost::vector_property_map<float, IndexMapType> PriorityMapType;
  typedef boost::indirect_cmp<PriorityMapType, PriorityCompareType> IndirectComparisonType;

  typedef boost::heap::binomial_heap<VertexDescriptorType,
      boost::heap::compare<IndirectComparisonType> >
      QueueType;

  typedef typename QueueType::handle_type HandleType;
  typedef boost::vector_property_map<HandleType, IndexMapType> HandleMapType;

  typedef typename QueueType::value_type ValueType;

  // Member variables
  TGraph Graph;

  IndexMapType IndexMap;

  PriorityMapType PriorityMap;

  HandleMapType HandleMap;

  IndirectComparisonType IndirectComparison;

  QueueType Queue;


  /** Create the boundary status map. A node is on the current boundary if this property is true.
    * This property helps the boundaryNodeQueue because we can mark here if a node has become no longer
    * part of the boundary, so when the queue is popped we can check this property to see if it should
    * actually be processed. */
  typedef boost::vector_property_map<bool, IndexMapType> BoundaryStatusMapType;
  BoundaryStatusMapType BoundaryStatusMap;

  IndirectPriorityQueue(TGraph graph) :
    Graph(graph),
    IndexMap(get(boost::vertex_index, Graph)),
    PriorityMap(num_vertices(Graph), IndexMap),
    HandleMap(IndexMap),
    IndirectComparison(PriorityMap),
    Queue(IndirectComparison),
    BoundaryStatusMap(num_vertices(Graph), IndexMap)
  {
//    this->IndexMap = IndexMapType(get(boost::vertex_index, graph));
//    this->HandleMap = HandleMapType(indexMap);
//    this->IndirectComparison = IndirectComparisonType(priorityMap);
//    this->Queue = QueueType(indirectComparison);

    // Initialize the handle map
    HandleType invalidHandle(0); // An invalid node handle (a node_pointer of NULL)
    VertexIteratorType vertexIterator, vertexIteratorEnd;
    for( tie(vertexIterator, vertexIteratorEnd) = vertices(graph);
         vertexIterator != vertexIteratorEnd; ++vertexIterator)
    {
      put(this->HandleMap, *vertexIterator, invalidHandle);
    }
  }

  size_t CountValidNodes()
  {
    size_t numberOfValidNodes = 0;
    for (typename QueueType::iterator it = this->Queue.begin();
         it != this->Queue.end(); ++it)
    {
      bool valid = get(this->BoundaryStatusMap, *it);
      if(valid)
      {
        numberOfValidNodes++;
//        std::cout << "(" << queuedNode[0] << ", " << queuedNode[1] << ") : " << priority << std::endl;
      }
    }

    return numberOfValidNodes;
  }

  HandleType push(ValueType v)
  {
    return this->Queue.push(v);
  }

  bool empty()
  {
    // We need to check if the queue contains any valid items
//    return this->Queue.empty();

    for (typename QueueType::iterator it = this->Queue.begin();
         it != this->Queue.end(); ++it)
    {
      bool valid = get(this->BoundaryStatusMap, *it);
      if(valid)
      {
        return false;
      }
    }
    return true;
  }

  ValueType top()
  {
    // Find the next target to in-paint. Some of the nodes in the queue
    // can be already filled (they get "covered up" when a patch is filled around
    // a target node). So we do not just process the node at the front of the queue,
    // we also check that it has not been filled (by looking at its boundaryStatusMap
    // value).

    bool validNodeFound = false;
    ValueType topNode;
    while(!this->Queue.empty())
    {
      // Get the top node
      topNode = this->Queue.top();

      // Invalidate the top node
      typename HandleMapType::value_type invalidHandle(0);
      put(this->HandleMap, topNode, invalidHandle);

      // Pop the top node
      this->Queue.pop();

      validNodeFound = get(this->BoundaryStatusMap, topNode);

      if(validNodeFound)
      {
        break;
      }
    }

    if(!validNodeFound)
    {
      throw std::runtime_error("IndirectPriorityQueue: There were no valid nodes to return in top()!");
    }

    return topNode;
  }

  void pop()
  {
    this->Queue.pop();
  }

  void update(HandleType handle, ValueType value)
  {
    this->Queue.update(get(this->HandleMap, value), value);
  }

  void mark_as_invalid(ValueType v)
  {
    // This makes a patch ignored if it is still in the boundaryNodeQueue.
    put(this->BoundaryStatusMap, v, false);
  }

  void push_or_update(ValueType v, const float priority)
  {
    // Note: we must set the value in the priority map before pushing the node
    // into the queue (as the priority is what determines the node's position in the queue).

    put(this->PriorityMap, v, priority);

    put(this->BoundaryStatusMap, v, true);

    if(get(this->HandleMap, v).node_ != 0) // the node is not already in the queue (the node_pointer of the node_handle is not NULL)
    {
      update(get(this->HandleMap, v), v);
//          std::cout << "Updated priority of node (" << v[0] << ", " << v[1] << "): " << priority << std::endl;
    }
    else
    {
      HandleType handle = push(v);
      put(this->HandleMap, v, handle);
//          std::cout << "Added new node (" << v[0] << ", " << v[1] << "): " << priority << std::endl;
    }
  }

};

#endif // INDIRECTPRIORITYQUEUE_H
