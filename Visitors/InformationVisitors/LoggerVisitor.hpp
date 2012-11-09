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

#ifndef LoggerVisitor_HPP
#define LoggerVisitor_HPP

// STL
#include <fstream>

// Custom
#include "Visitors/InpaintingVisitorParent.h"

// Can't do this - ambiguous overload errors
// template <typename T>
// std::ostream& operator<<(std::ostream& output, const T& object)
// {
//   output << object[0] << " " << object[1] << std::endl;
//   return output;
// }

/**
  * This visitor saves the information needed to reproduce the inpainting.
 */
template <typename TGraph>
struct LoggerVisitor : public InpaintingVisitorParent<TGraph>
{
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  std::ofstream OutputStream;

  LoggerVisitor(const std::string& filename, const std::string& visitorName = "LoggerVisitor") :
  InpaintingVisitorParent<TGraph>(visitorName), OutputStream(filename.c_str())
  {

  }

  ~LoggerVisitor()
  {
    this->OutputStream.close();
  }

  void FinishVertex(VertexDescriptorType targetNode, VertexDescriptorType sourceNode) override
  {
    //OutputStream << sourceNode << " : " << targetNode << std::endl;
     this->OutputStream << sourceNode[0] << " " << sourceNode[1] << " : "
                        << targetNode[0] << " " << targetNode[1] << std::endl;
  }

};

#endif
