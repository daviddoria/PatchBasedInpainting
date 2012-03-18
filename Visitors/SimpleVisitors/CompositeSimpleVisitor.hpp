#ifndef CompositeSimpleVisitor_HPP
#define CompositeSimpleVisitor_HPP

#include "Visitors/SimpleVisitors/VisitorSuperclass.hpp"

/**
 * This is a composite visitor type that complies with the InpaintingVisitorConcept and forwards
 * all calls to all of its internal visitors.
 */
template <typename TVertexDescriptor>
struct CompositeSimpleVisitor
{
  void PaintPatch(TVertexDescriptor target, TVertexDescriptor source) const
  {
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      // std::cout << Visitors[visitorId]->VisitorName << " InitializeVertex()" << std::endl;
      Visitors[visitorId]->PaintPatch(target, source);
      }
  }

  void FinishVertex(TVertexDescriptor target, TVertexDescriptor sourceNode)
  {
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      // std::cout << Visitors[visitorId]->VisitorName << " InitializeVertex()" << std::endl;
      Visitors[visitorId]->FinishVertex(target, sourceNode);
      }
  }
  
  void InpaintingComplete() const
  {
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      // std::cout << Visitors[visitorId]->VisitorName << " InitializeVertex()" << std::endl;
      Visitors[visitorId]->InpaintingComplete();
      }
  }

  void AddVisitor(VisitorSuperclass<TVertexDescriptor>* vis)
  {
    // std::cout << "Adding " << vis->VisitorName << std::endl;
    this->Visitors.push_back(vis);
  }

private:
  std::vector<VisitorSuperclass<TVertexDescriptor>*> Visitors;
};

#endif
