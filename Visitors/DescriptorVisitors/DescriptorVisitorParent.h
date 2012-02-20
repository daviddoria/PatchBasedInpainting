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
