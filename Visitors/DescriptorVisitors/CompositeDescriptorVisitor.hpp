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

#ifndef CompositeDescriptorVisitor_HPP
#define CompositeDescriptorVisitor_HPP

#include "DescriptorVisitorParent.h"

#include <boost/graph/graph_traits.hpp>

/**
 * This is a composite visitor type that models the DescriptorVisitorConcept and forwards
 * all calls to all of its internal visitors.
 */
template <typename TGraph>
struct CompositeDescriptorVisitor
{
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;
  
  typedef DescriptorVisitorParent<TGraph> DescriptorVisitorParentType;

  void InitializeVertex(VertexDescriptorType v) const
  { 
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
    {
      this->Visitors[visitorId]->InitializeVertex(v);
    }
  }

  void DiscoverVertex(VertexDescriptorType v) const
  { 
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
    {
      this->Visitors[visitorId]->DiscoverVertex(v);
    }
  }

  void AddVisitor(DescriptorVisitorParentType* vis)
  {
    this->Visitors.push_back(std::shared_ptr<DescriptorVisitorParentType>(vis));
  }

  void AddVisitor(std::shared_ptr<DescriptorVisitorParentType> vis)
  {
    this->Visitors.push_back(vis);
  }

private:
  std::vector<std::shared_ptr<DescriptorVisitorParentType> > Visitors;
};

#endif
