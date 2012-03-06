#ifndef CompositeInpaintingVisitor_HPP
#define CompositeInpaintingVisitor_HPP

#include "InpaintingVisitorParent.h"

#include <boost/graph/graph_traits.hpp>

/**
 * This is a composite visitor type that complies with the InpaintingVisitorConcept and forwards
 * all calls to all of its internal visitors.
 */
template <typename TGraph>
struct CompositeInpaintingVisitor 
{
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  void InitializeVertex(VertexDescriptorType v) const
  {
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      // std::cout << Visitors[visitorId]->VisitorName << " InitializeVertex()" << std::endl;
      Visitors[visitorId]->InitializeVertex(v);
      }
  };

  void DiscoverVertex(VertexDescriptorType v) const
  { 
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      std::cout << Visitors[visitorId]->VisitorName << " DiscoverVertex()" << std::endl;
      Visitors[visitorId]->DiscoverVertex(v);
      }
  };

  void PotentialMatchMade(VertexDescriptorType a, VertexDescriptorType b) const
  { 
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      std::cout << Visitors[visitorId]->VisitorName << " PotentialMatchMade()" << std::endl;
      Visitors[visitorId]->PotentialMatchMade(a, b);
      }
  };

  void PaintVertex(VertexDescriptorType a, VertexDescriptorType b) const
  { 
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      std::cout << Visitors[visitorId]->VisitorName << " PaintVertex()" << std::endl;
      Visitors[visitorId]->PaintVertex(a, b);
      }
  };

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    bool acceptAll = true;
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      std::cout << Visitors[visitorId]->VisitorName << " AcceptMatch()" << std::endl;
      bool accept = Visitors[visitorId]->AcceptMatch(target, source);
      acceptAll = acceptAll && accept;
      }
    return acceptAll;
  };

  void FinishVertex(VertexDescriptorType v, VertexDescriptorType sourceNode) const
  { 
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      std::cout << Visitors[visitorId]->VisitorName << " FinishVertex()" << std::endl;
      Visitors[visitorId]->FinishVertex(v, sourceNode);
      }
  };

  void InpaintingComplete() const
  {
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      std::cout << Visitors[visitorId]->VisitorName << " InpaintingComplete()" << std::endl;
      Visitors[visitorId]->InpaintingComplete();
      }
  };

  void AddVisitor(InpaintingVisitorParent<TGraph>* vis)
  {
    std::cout << "Adding " << vis->VisitorName << std::endl;
    this->Visitors.push_back(vis);
  }

private:
  std::vector<InpaintingVisitorParent<TGraph>*> Visitors;
};

#endif
