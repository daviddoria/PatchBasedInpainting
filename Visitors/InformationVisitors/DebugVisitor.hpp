#ifndef DebugVisitor_HPP
#define DebugVisitor_HPP

#include <boost/graph/graph_traits.hpp>

#include "Visitors/InpaintingVisitorParent.h"

/**

 */
template <typename TGraph>
struct DebugVisitor : public InpaintingVisitorParent<TGraph>
{
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;
  
  void initialize_vertex(VertexDescriptorType v, TGraph& g) const
  { 

  };

  void discover_vertex(VertexDescriptorType v, TGraph& g) const 
  { 

  };

  void vertex_match_made(VertexDescriptorType target, VertexDescriptorType source, TGraph& g) const{};

  void paint_vertex(VertexDescriptorType target, VertexDescriptorType source, TGraph& g) const{};

  bool accept_painted_vertex(VertexDescriptorType v, TGraph& g) const{};

  void finish_vertex(VertexDescriptorType v, VertexDescriptorType sourceNode, TGraph& g){};
};

#endif
