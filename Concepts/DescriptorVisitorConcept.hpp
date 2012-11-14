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

#ifndef DescriptorVisitorConcept_HPP
#define DescriptorVisitorConcept_HPP

#include <boost/graph/graph_concepts.hpp>
#include <boost/concept_check.hpp>

/**
 * This concept-check class defines the functions that a descriptor visitor must have in order to be 
 * used by an inpainting visitor.
 * 
 * \tparam TDescriptorVisitor The visitor whose compliance to this concept is to be assessed.
 * \tparam TVertexListGraph The type of the graph on which the visitor will be applied.
 */
template <typename TDescriptorVisitor, typename TVertexListGraph>
struct DescriptorVisitorConcept 
{
  typedef typename boost::graph_traits<TVertexListGraph>::vertex_descriptor Vertex;
  Vertex u;
  TVertexListGraph graph;
  TDescriptorVisitor visitor;

  BOOST_CONCEPT_USAGE(DescriptorVisitorConcept) 
  {
    // Function to initialize a vertex
    visitor.InitializeVertex(u);

    // Function called when a vertex has become the target patch
    visitor.DiscoverVertex(u);
  }

};

#endif
