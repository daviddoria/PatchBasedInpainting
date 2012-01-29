#ifndef MaskedGridPatchInpainter_HPP
#define MaskedGridPatchInpainter_HPP

/**
 * This class template is a patch inpainter (i.e. paints the holes in one patch with the values of 
 * some given source patch). This class requires that the graph be a 2D grid-graph (like BGL's grid_graph).
 * This class uses the color-map that is given to it to check if the vertices in a patch are holes that need to be filled 
 * or not (i.e. it does a masking of the patch around a given target center vertex).
 * 
 * \tparam ColorMap A property-map to obtain the vertex color for a given vertex descriptor.
 */
template <typename FillStatusMap>
struct MaskedGridPatchInpainter 
{
  std::size_t patch_half_width;
  FillStatusMap& fillStatusMap;
  
  MaskedGridPatchInpainter(std::size_t aPatchHalfWidth, FillStatusMap& aFillStatusMap) : patch_half_width(aPatchHalfWidth), fillStatusMap(aFillStatusMap) { };

  template <typename Vertex, typename GridGraph, typename InpaintingVisitor>
  void operator()(Vertex target, Vertex source, GridGraph& g, InpaintingVisitor vis)
  {
    std::cout << "Painting " << target[0] << " " << target[1] << " with " << source[0] << " " << source[1] << std::endl;

    Vertex target_patch_corner;
    target_patch_corner[0] = target[0] - patch_half_width;
    target_patch_corner[1] = target[1] - patch_half_width;

    Vertex source_patch_corner;
    source_patch_corner[0] = source[0] - patch_half_width;
    source_patch_corner[1] = source[1] - patch_half_width;

    Vertex target_node;
    Vertex source_node;
    for(std::size_t i = 0; i < patch_half_width * 2 + 1; ++i) 
    {
      for(std::size_t j = 0; j < patch_half_width * 2 + 1; ++j) 
      {
        target_node[0] = target_patch_corner[0] + i;
        target_node[1] = target_patch_corner[1] + j;

        source_node[0] = source_patch_corner[0] + i;
        source_node[1] = source_patch_corner[1] + j;

        assert(get(fillStatusMap, source_node));

        // check the mask value
        if( get(fillStatusMap, target_node) == false )
        {
          vis.paint_vertex(target_node, source_node, g); //paint the vertex.
        }

      }
    }
  }

};

#endif
