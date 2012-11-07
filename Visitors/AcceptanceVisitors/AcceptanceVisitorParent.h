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

#ifndef AcceptanceVisitorParent_HPP
#define AcceptanceVisitorParent_HPP

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

/**
 * This is an abstract visitor that allows child visitors be stored as a vector<AcceptanceVisitorParent*>.
 */
template <typename TGraph>
struct AcceptanceVisitorParent
{
  std::string VisitorName;

  AcceptanceVisitorParent() : VisitorName("NoName")
  {

  }
  
  AcceptanceVisitorParent(const std::string& name) : VisitorName(name)
  {
    
  }
  
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  virtual bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy = 0.0f) const = 0;

  void SetName(const std::string& name)
  {
    VisitorName = name;
  }
}; // AcceptanceVisitorParent

// Would rather do this, but c++ says "templates cannot be virtual".
//struct AcceptanceVisitorParent
//{
//  std::string VisitorName;

//  AcceptanceVisitorParent() : VisitorName("NoName")
//  {

//  }

//  AcceptanceVisitorParent(const std::string& name) : VisitorName(name)
//  {

//  }

//  template <typename TVertexDescriptor>
//  virtual bool AcceptMatch(TVertexDescriptor target, TVertexDescriptor source,
//                           float& computedEnergy = 0.0f) const = 0;

//  void SetName(const std::string& name)
//  {
//    VisitorName = name;
//  }
//}; // AcceptanceVisitorParent

#endif
