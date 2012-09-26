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

#ifndef DescriptorVisitorParent_HPP
#define DescriptorVisitorParent_HPP

// Boost
#include <boost/graph/graph_traits.hpp>

/**
 * This is an abstract visitor that complies with the DescriptorVisitorConcept. It is available
 * so that we can store a container of DescriptorVisitors via their parent class pointer (e.g.
 * std::vector<DescriptorVisitorParent*> )
 */
template <typename TGraph>
struct DescriptorVisitorParent
{
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  virtual void InitializeVertex(VertexDescriptorType v) const = 0;
  
  virtual void DiscoverVertex(VertexDescriptorType v) const = 0;

}; // DescriptorVisitorParent

#endif
