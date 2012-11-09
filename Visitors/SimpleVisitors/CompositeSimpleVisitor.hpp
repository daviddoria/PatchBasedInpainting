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

#ifndef CompositeSimpleVisitor_HPP
#define CompositeSimpleVisitor_HPP

#include "Visitors/SimpleVisitors/VisitorSuperclass.hpp"

/**
 * This is a composite visitor type that complies with the InpaintingVisitorConcept and forwards
 * all calls to all of its internal visitors.
 */
template <typename TVertexDescriptor>
struct CompositeSimpleVisitor
{
  void PaintPatch(TVertexDescriptor target, TVertexDescriptor source) const
  {
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
    {
      // std::cout << Visitors[visitorId]->VisitorName << " InitializeVertex()" << std::endl;
      this->Visitors[visitorId]->PaintPatch(target, source);
    }
  }

  void FinishVertex(TVertexDescriptor target, TVertexDescriptor sourceNode)
  {
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
    {
      // std::cout << Visitors[visitorId]->VisitorName << " InitializeVertex()" << std::endl;
      this->Visitors[visitorId]->FinishVertex(target, sourceNode);
    }
  }
  
  void InpaintingComplete() const
  {
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
    {
      // std::cout << Visitors[visitorId]->VisitorName << " InitializeVertex()" << std::endl;
      this->Visitors[visitorId]->InpaintingComplete();
    }
  }

  void AddVisitor(VisitorSuperclass<TVertexDescriptor>* vis)
  {
    // std::cout << "Adding " << vis->VisitorName << std::endl;
    this->Visitors.push_back(vis);
  }

private:
  std::vector<VisitorSuperclass<TVertexDescriptor>*> Visitors;
};

#endif
