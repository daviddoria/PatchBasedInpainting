#ifndef InpaintingVisitorParent_HPP
#define InpaintingVisitorParent_HPP

#include "Priority/Priority.h"

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

/**
 * This is an abstract visitor that allows child visitors be stored as a vector<InpaintingVisitorParent*>.
 */
template <typename TGraph>
struct InpaintingVisitorParent
{
  InpaintingVisitorParent(const std::string& visitorName = "InpaintingVisitorParent") : VisitorName(visitorName) {}
  
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  // Const functions
  virtual void InitializeVertex(VertexDescriptorType v) const = 0;

  virtual void DiscoverVertex(VertexDescriptorType v) const = 0;

  virtual void PaintPatch(VertexDescriptorType target, VertexDescriptorType source) const = 0;
  
  virtual void PaintVertex(VertexDescriptorType target, VertexDescriptorType source) const = 0;

  virtual bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const = 0;

  virtual void InpaintingComplete() const = 0;

  // Non-const functions
  virtual void PotentialMatchMade(VertexDescriptorType target, VertexDescriptorType source) = 0;

  virtual void FinishVertex(VertexDescriptorType v, VertexDescriptorType sourceNode) = 0;

  std::string VisitorName;
}; // InpaintingVisitorParent

#endif
