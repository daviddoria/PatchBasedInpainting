#ifndef LoggerVisitor_HPP
#define LoggerVisitor_HPP

// STL
#include <fstream>

// Custom
#include "Visitors/InpaintingVisitorParent.h"

// Can't do this - ambiguous overload errors
// template <typename T>
// std::ostream& operator<<(std::ostream& output, const T& object)
// {
//   output << object[0] << " " << object[1] << std::endl;
//   return output;
// }

/**
  * This visitor saves the information needed to reproduce the inpainting.
 */
template <typename TGraph>
struct LoggerVisitor : public InpaintingVisitorParent<TGraph>
{
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  std::ofstream OutputStream;
  LoggerVisitor(const std::string& filename) : OutputStream(filename.c_str())
  {

  }

  ~LoggerVisitor()
  {
  OutputStream.close();
  }

  void InitializeVertex(VertexDescriptorType v) const
  {

  };

  void DiscoverVertex(VertexDescriptorType v) const
  {

  };

  void PotentialMatchMade(VertexDescriptorType target, VertexDescriptorType source)
  {

  };

  void PaintVertex(VertexDescriptorType target, VertexDescriptorType source) const
  {

  };

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    return true;
  };

  void FinishVertex(VertexDescriptorType targetNode, VertexDescriptorType sourceNode)
  {
    //OutputStream << sourceNode << " : " << targetNode << std::endl;
     OutputStream << sourceNode[0] << " " << sourceNode[1] << " : "
                  << targetNode[0] << " " << targetNode[1] << std::endl;
  };

  void InpaintingComplete() const
  {
  }
};

#endif
