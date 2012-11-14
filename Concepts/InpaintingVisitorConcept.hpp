/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#ifndef InpaintingVisitorConcept_HPP
#define InpaintingVisitorConcept_HPP

#include <boost/graph/graph_concepts.hpp>
#include <boost/concept_check.hpp>

/**
 * This concept-check class defines the functions that a visitor must have in order to be 
 * used by the inpainting algorithms.
 * 
 * Required concepts:
 * 
 *  The visitor should be copy-constructible.
 *
 * \tparam TInpaintingVisitor The visitor whose compliance to this concept is to be assessed.
 * \tparam TVertexListGraph The type of the graph on which the visitor will be applied.
 */
template <typename TInpaintingVisitor, typename TVertexListGraph>
struct InpaintingVisitorConcept
{

  typedef typename boost::graph_traits<TVertexListGraph>::vertex_descriptor Vertex;
  Vertex u;
  TVertexListGraph graph;
  TInpaintingVisitor visitor;

  BOOST_CONCEPT_ASSERT((boost::CopyConstructibleConcept<TInpaintingVisitor>));

  BOOST_CONCEPT_USAGE(InpaintingVisitorConcept) 
  {
    visitor.InitializeVertex(u);  //function called on all vertices during the initialization phase.
    visitor.DiscoverVertex(u);  //function called when a live vertex is taken out of the priority-queue.
    const Vertex& target = u;
    const Vertex& source = u;

    // Function called when a source vertex has been found that matches well to the
    // current target-vertex (the same vertex that was just discovered).
    visitor.PotentialMatchMade(target, source);

    // Function called to paint the value of a target vertex with the value of the source vertex.
    // Ideally we would like to do this, but PatchInpainter::PaintPatch cannot be a template (c++ rules say so)
    // vis.PaintVertex(target, source);

    // Function called to check if the match that was determined should be accepted.
    bool was_successfully_painted = visitor.AcceptMatch(target, source);
    boost::ignore_unused_variable_warning(was_successfully_painted);

    // Function called when a vertex has been inpainted and removed from the set of target pixels.
    visitor.FinishVertex(u, u);
  }

};

#endif
