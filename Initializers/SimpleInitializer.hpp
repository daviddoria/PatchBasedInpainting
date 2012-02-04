#ifndef SimpleInitializer_HPP
#define SimpleInitializer_HPP

template <typename TVisitor, typename TGraph>
inline void SimpleInitializer(TVisitor* const visitor, TGraph& g)
{
  std::cout << "SimpleInitializer" << std::endl;

  typedef typename boost::graph_traits<TGraph>::vertex_iterator VertexIter;
  // typename boost::graph_traits<TGraph>::vertex_descriptor node;
  VertexIter ui,ui_end; tie(ui,ui_end) = vertices(g);
  for(VertexIter iter = ui; iter != ui_end; ++iter)
    {
    visitor->initialize_vertex(*iter, g);
    }

}

#endif
