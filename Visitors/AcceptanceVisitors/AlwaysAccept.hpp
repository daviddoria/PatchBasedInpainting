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

#ifndef AlwaysAccept_HPP
#define AlwaysAccept_HPP

#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

#include <boost/graph/graph_traits.hpp>

/**
 * Always accept the match. This is probably only used for testing things.
 */
template <typename TGraph>
struct AlwaysAccept : public AcceptanceVisitorParent<TGraph>
{
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& energy) const
  {
    return true;
  }
};

// Can't do this because the CompositeAcceptanceVisitor needs to be able to store a AcceptanceVisitorParent pointer
//struct AlwaysAccept
//{
//  template <typename TVertex>
//  bool AcceptMatch(TVertex target, TVertex source, float& energy) const
//  {
//    return true;
//  }
//};

// Can't do this because c++ does not allow virtual templates
//struct AlwaysAccept : public AcceptanceVisitorParent<float>
//{
//  template <typename TVertex>
//  bool AcceptMatch(TVertex target, TVertex source, float& energy) const
//  {
//    return true;
//  }
//};

#endif
