#ifndef InpaintingVisitorWrapper_HPP
#define InpaintingVisitorWrapper_HPP

template <typename InpaintingVisitor, typename ColorMap>
struct inpainting_visitor_wrapper 
{
  typedef typename boost::property_traits<ColorMap>::value_type ColorValue;
  typedef boost::color_traits<ColorValue> Color;
  InpaintingVisitor vis;
  ColorMap color;
  inpainting_visitor_wrapper(InpaintingVisitor aVis, ColorMap aColor) : vis(aVis), color(aColor) { };
  
  template <typename VertexType, typename Graph>
  void discover_vertex(VertexType u, Graph& g) const { 
    vis.discover_vertex(u, g);  // forward to the user's visitor.
  };
  template <typename VertexType, typename Graph>
  void vertex_match_made(VertexType target, VertexType source, Graph& g) const {
    vis.vertex_match_made(target, source, g);  //forward to user's visitor.
  };
  template <typename VertexType, typename Graph>
  void paint_vertex(VertexType target, VertexType source, Graph& g) const { 
    vis.paint_vertex(target, source, g);  //forward to user's visitor.
    //check if the painting was acceptable:
    if( vis.accept_painted_vertex(target, g) ) {
      //if yes, then this vertex is no longer a hole, so, color it white and finish it.
      using boost::put;
      put(color, target, Color::white());
      vis.finish_vertex(target, g);  //tell the user's visitor that the target vertex has be successfully painted.
    };
  };
  
};

#endif
