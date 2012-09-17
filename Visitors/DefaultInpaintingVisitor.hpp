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

#ifndef DefaultInpaintingVisitor_HPP
#define DefaultInpaintingVisitor_HPP

/**
 * This is a default visitor type that models with the InpaintingVisitorConcept and does 
 * nothing in all cases (can be used if there are no exogenous operations to do during the 
 * inpainting algorithm, which would be surprising given the nature of the algorithm).
 */
struct DefaultInpaintingVisitor 
{
  template <typename VertexType, typename Graph>
  void initialize_vertex(VertexType, Graph&) const { };

  template <typename VertexType, typename Graph>
  void discover_vertex(VertexType, Graph&) const { };

  template <typename VertexType, typename Graph>
  void vertex_match_made(VertexType, VertexType, Graph&) const { };

  template <typename VertexType, typename Graph>
  void paint_vertex(VertexType, VertexType, Graph&) const { };

  template <typename VertexType, typename Graph>
  bool accept_painted_vertex(VertexType, Graph&) const { return true; };

  template <typename VertexType, typename Graph>
  void finish_vertex(VertexType, Graph&) const { };
};

#endif
