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

#ifndef DescriptorConcept_HPP
#define DescriptorConcept_HPP

#include <boost/graph/graph_concepts.hpp>
#include <boost/concept_check.hpp>

#include "PixelDescriptors/PixelDescriptor.h"

/**
 * This concept-check class defines the functions that a descriptor must have in order to be 
 * used by the inpainting algorithms.
 * 
 * Valid Expressions:
 * 
 *  descriptor.SetVertex(v);  Store the vertex that it is describing
 * 
 *  vis.SetStatus(StatusEnum);  Store if it is a source, target, or invalid descriptor.
 *
 * \tparam TDescriptor The descriptor type.
 * \tparam VertexListGraphType The type of the graph that the descriptor describes a vertex of.
 */
template <typename TDescriptor, typename VertexListGraphType>
struct DescriptorConcept 
{
  typedef typename boost::graph_traits<VertexListGraphType>::vertex_descriptor VertexDescriptor;
  VertexDescriptor v;
  TDescriptor descriptor;
  BOOST_CONCEPT_USAGE(DescriptorConcept ) 
  {
    descriptor.SetVertex(v);
    descriptor.SetStatus(PixelDescriptor::INVALID);
  }

};

#endif
