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

#ifndef CompositeInpaintingVisitor_HPP
#define CompositeInpaintingVisitor_HPP

#include "InpaintingVisitorParent.h"

// Boost
#include <boost/graph/graph_traits.hpp>

// STL
#include <memory>

/**
 * This is a composite visitor type that complies with the InpaintingVisitorConcept and forwards
 * all calls to all of its internal visitors.
 */
template <typename TGraph>
struct CompositeInpaintingVisitor 
{
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  typedef InpaintingVisitorParent<TGraph> InpaintingVisitorParentType;

  void InitializeVertex(VertexDescriptorType v) const
  {
    for(unsigned int visitorId = 0; visitorId < this->Visitors.size(); ++visitorId)
    {
      // std::cout << Visitors[visitorId]->VisitorName << " InitializeVertex()" << std::endl;
      this->Visitors[visitorId]->InitializeVertex(v);
    }
  }

  void DiscoverVertex(VertexDescriptorType v) const
  {
    for(unsigned int visitorId = 0; visitorId < this->Visitors.size(); ++visitorId)
    {
      // std::cout << Visitors[visitorId]->VisitorName << " DiscoverVertex()" << std::endl;
      this->Visitors[visitorId]->DiscoverVertex(v);
    }
  }

  void PotentialMatchMade(VertexDescriptorType a, VertexDescriptorType b) const
  {
    for(unsigned int visitorId = 0; visitorId < this->Visitors.size(); ++visitorId)
    {
      // std::cout << Visitors[visitorId]->VisitorName << " PotentialMatchMade()" << std::endl;
      Visitors[visitorId]->PotentialMatchMade(a, b);
    }
  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    bool acceptAll = true;
    for(unsigned int visitorId = 0; visitorId < this->Visitors.size(); ++visitorId)
    {
      // std::cout << Visitors[visitorId]->VisitorName << " AcceptMatch()" << std::endl;
      bool accept = this->Visitors[visitorId]->AcceptMatch(target, source);
      acceptAll = acceptAll && accept;
    }
    return acceptAll;
  }

  void FinishVertex(VertexDescriptorType v, VertexDescriptorType sourceNode) const
  {
    for(unsigned int visitorId = 0; visitorId < this->Visitors.size(); ++visitorId)
    {
      // std::cout << Visitors[visitorId]->VisitorName << " FinishVertex()" << std::endl;
      this->Visitors[visitorId]->FinishVertex(v, sourceNode);
    }
  }

  void InpaintingComplete() const
  {
    for(unsigned int visitorId = 0; visitorId < this->Visitors.size(); ++visitorId)
    {
      // std::cout << Visitors[visitorId]->VisitorName << " InpaintingComplete()" << std::endl;
      this->Visitors[visitorId]->InpaintingComplete();
    }
  }

  void AddVisitor(InpaintingVisitorParentType* vis)
  {
    // std::cout << "Adding " << vis->VisitorName << std::endl;
    this->Visitors.push_back(std::shared_ptr<InpaintingVisitorParentType>(vis));
  }

  void AddVisitor(std::shared_ptr<InpaintingVisitorParentType> vis)
  {
    // std::cout << "Adding " << vis->VisitorName << std::endl;
    this->Visitors.push_back(vis);
    std::cout << "There are now " << this->Visitors.size() << " visitors." << std::endl;
  }

  size_t GetNumberOfVisitors()
  {
    return this->Visitors.size();
  }

private:
  std::vector<std::shared_ptr<InpaintingVisitorParentType> > Visitors;

};

#endif
