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

#ifndef InpaintingVisitorParent_HPP
#define InpaintingVisitorParent_HPP

#include "Priority/Priority.h"

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

/**
 * This is an abstract visitor that allows child visitors be stored as a vector<InpaintingVisitorParent*>.
 */
template <typename TGraph>
struct InpaintingVisitorParent
{
  InpaintingVisitorParent(const std::string& visitorName = "InpaintingVisitorParent") : VisitorName(visitorName) {}
  
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  // There is no need to make these functions pure virtual - having empty implementations makes sense, and then
  // children can choose which functions to reimplement.
//  // Const functions
//  virtual void InitializeVertex(VertexDescriptorType v) const = 0;

//  virtual void DiscoverVertex(VertexDescriptorType v) const = 0;

//  virtual bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const = 0;

//  virtual void InpaintingComplete() const = 0;

//  // Non-const functions
//  virtual void PotentialMatchMade(VertexDescriptorType target, VertexDescriptorType source) = 0;

//  virtual void FinishVertex(VertexDescriptorType v, VertexDescriptorType sourceNode) = 0;


  // Const functions
  virtual void InitializeVertex(VertexDescriptorType v) const{}

  /** This is not const because we typically have to set some properties of the
    * target patch when it is discovered. */
  virtual void DiscoverVertex(VertexDescriptorType v) {}

  virtual bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const{return true;}

  virtual void InpaintingComplete() const{}

  // Non-const functions
  virtual void PotentialMatchMade(VertexDescriptorType target, VertexDescriptorType source){}

  virtual void FinishVertex(VertexDescriptorType v, VertexDescriptorType sourceNode){}

  std::string VisitorName;
}; // InpaintingVisitorParent

#endif
