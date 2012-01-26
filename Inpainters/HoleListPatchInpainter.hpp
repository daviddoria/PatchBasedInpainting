#ifndef HoleListPatchInpainter_HPP
#define HoleListPatchInpainter_HPP

/**
 * This class template is a patch inpainter (i.e. paints the holes in one patch with the values of 
 * some given source patch). This class requires that the vertices at the center of the target 
 * patches hold a list of all the holes in the target patch (as a container of vertex descriptors).
 * Also, it requires a property mapping back and forth between the identity of a vertex (vertex-descriptor)
 * and its coordinate (which should be an arithmetic type, e.g., such as a 2D vector of integers).
 * 
 * \tparam CoordinateMap A property-map to obtain the coordinates of a given vertex.
 * \tparam HoleListMap A lvalue property-map to obtain a reference to a list (i.e. container) of neighboring holes.
 * \tparam InverseCoordMap A property-map to obtain the vertex descriptor for a given set of coordinates.
 */
template <typename CoordinateMap, typename HoleListMap, typename InverseCoordMap>
struct hole_list_patch_inpainter 
{
  CoordinateMap coord;   //property-map used to get the grid-coordinates of a vertex.
  HoleListMap nearby_holes;   //property-map used to get the list of nearby holes around a target center vertex.
  InverseCoordMap inv_coord;   //property-map used to get the vertex-descriptor from some given grid-coordinates
  
  
  hole_list_patch_inpainter(CoordinateMap aCoord, HoleListMap aNearbyHoles, InverseCoordMap aInvCoord) : coord(aCoord), nearby_holes(aNearbyHoles), inv_coord(aInvCoord) { };
  
  template <typename Vertex, typename Graph, typename InpaintingVisitor>
  void operator()(Vertex target, Vertex source, Graph& g, InpaintingVisitor vis) {
    typedef typename boost::property_traits<HoleListMap>::value_type HoleList;
    typedef typename boost::property_traits<CoordinateMap>::value_type CoordType;
    
    BOOST_CONCEPT_ASSERT((boost::LvaluePropertyMapConcept<HoleListMap, Vertex>));
    BOOST_CONCEPT_ASSERT((boost::ReadablePropertyMapConcept<CoordinateMap, Vertex>));
    BOOST_CONCEPT_ASSERT((boost::ReadablePropertyMapConcept<InverseCoordMap, CoordType>));
    using boost::get; 
    
    HoleList& hole_list = nearby_holes[target];
    CoordType target_center = get(coord, target);
    CoordType source_center = get(coord, source);
    CoordType target_to_source = source_center - target_center;
    
    for( typename HoleList::iterator it = hole_list.begin(); it != hole_list.end(); ++it) 
    {
      
      Vertex target_vertex = *it;
      Vertex source_vertex = get(inv_coord, get(coord, target_vertex) + target_to_source);
      
      // Let the visitor do the actual painting of the hole:
      vis.paint_vertex(target_vertex, source_vertex, g);
    };
    
    //finally, paint the center of the target patch:
    vis.paint_vertex(target, source, g);
    // Note, I do this last because painting the target-center might screw up the list of nearby-holes,
    //  since that list might be destroyed when the hole is painted.
    
  };
  
};

#endif
