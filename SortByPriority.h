#ifndef SortByPriority_h
#define SortByPriority_h

#include <boost/graph/graph_traits.hpp>
//#include <boost/property_map/property_map.hpp>
#include <boost/graph/adjacency_list.hpp>

template <typename TGraph>
class SortByPriority
{
public:
  
  SortByPriority() : graph(NULL) {}

  void SetGraph(TGraph* g)
  {
    this->graph = g;
    this->priorityMap = get(boost::vertex_priority, graph);
  }
 

  bool operator()(const typename boost::graph_traits<TGraph>::vertex_descriptor& item1, const typename boost::graph_traits<TGraph>::vertex_descriptor& item2) const
  {
    if(get(priorityMap, item1) < get(priorityMap, item2))
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  
private:
  TGraph* graph;
  typename boost::property_map<TGraph, boost::vertex_priority_t>::type priorityMap;
};

#endif
