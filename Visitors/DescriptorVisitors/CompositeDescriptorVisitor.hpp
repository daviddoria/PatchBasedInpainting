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
      Visitors[visitorId]->InitializeVertex(v);
      }
  }

  void DiscoverVertex(VertexDescriptorType v) const
  { 
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
    {
      Visitors[visitorId]->DiscoverVertex(v);
    }
  }

  void AddVisitor(DescriptorVisitorParentType* vis)
  {
    this->Visitors.push_back(std::shared_ptr<DescriptorVisitorParentType>(vis));
  }

private:
  std::vector<std::shared_ptr<DescriptorVisitorParentType> > Visitors;
};

#endif
