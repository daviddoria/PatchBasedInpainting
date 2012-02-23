#ifndef AcceptanceVisitorParent_HPP
#define AcceptanceVisitorParent_HPP

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

/**
 * This is an abstract visitor that allows child visitors be stored as a vector<AcceptanceVisitorParent*>.
 */
template <typename TGraph>
struct AcceptanceVisitorParent
{
  std::string VisitorName;

  AcceptanceVisitorParent() : VisitorName("NoName")
  {

  }
  
  AcceptanceVisitorParent(const std::string& name) : VisitorName(name)
  {
    
  }
  
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  virtual bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy = 0.0f) const = 0;

  void SetName(const std::string& name)
  {
    VisitorName = name;
  }
}; // AcceptanceVisitorParent

#endif
