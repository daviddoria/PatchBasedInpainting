#ifndef DescriptorInpaintingVisitor_HPP
#define DescriptorInpaintingVisitor_HPP

/**
 * This is a visitor type that complies with the InpaintingVisitorConcept. It computes
 * and differences feature vectors (std::vector<float>) at each pixel.
 */
struct descriptor_inpainting_visitor 
{
  template <typename VertexType, typename Graph>
  void initialize_vertex(VertexType v, Graph& g) const 
  {
    // Compute the descriptor

    // Add the descriptor to the property map (TODO: How do we get the property map here?)
    // put(g, v, 
  };

  template <typename VertexType, typename Graph>
  void discover_vertex(VertexType v, Graph& g) const 
  {
    
  };

  template <typename VertexType, typename Graph>
  void vertex_match_made(VertexType a, VertexType b, Graph& g) const 
  { 
    
  };

  template <typename VertexType, typename Graph>
  void paint_vertex(VertexType a, VertexType b, Graph& g) const 
  { 
    
  };

  template <typename VertexType, typename Graph>
  bool accept_painted_vertex(VertexType v, Graph& g) const 
  { 
    return true; 
  };

  template <typename VertexType, typename Graph>
  void finish_vertex(VertexType v, Graph& g) const 
  { 
    
  };
};

#endif
