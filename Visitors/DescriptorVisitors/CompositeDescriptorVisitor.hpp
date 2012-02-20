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
  
  void initialize_vertex(VertexDescriptorType v) const
  { 
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      Visitors[visitorId]->initialize_vertex(v, g);
      }
  };

  void discover_vertex(VertexDescriptorType v) const
  { 
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      Visitors[visitorId]->discover_vertex(v, g);
      }
  };

  void AddVisitor(DescriptorVisitorParent<TGraph>* vis)
  {
    this->Visitors.push_back(vis);
  }

private:
  std::vector<DescriptorVisitorParent<TGraph>*> Visitors;
};

#endif
