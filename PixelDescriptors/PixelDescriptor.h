/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
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

#ifndef PixelDescriptor_H
#define PixelDescriptor_H

// STL
#include <iostream>

// Boost
#include <boost/array.hpp>

/**
\class PixelDescriptor
\brief This class is the parent class for all descriptors.
       Its main job is to store the vertex at which the descriptor was generated.
*/
class PixelDescriptor
{
public:

  /** If a patch is invalid, it means any comparison with it should return inf. */
  enum StatusEnum {SOURCE_PATCH, TARGET_PATCH, INVALID};

private:
  typedef boost::array<size_t, 2> VertexType;
  VertexType Vertex;

  /** Indicate if the patch is a source patch, a target patch, or invalid. */
  StatusEnum Status;
  
public:
  void SetVertex(VertexType& v)
  {
    this->Vertex = v;
  }

  VertexType GetVertex() const
  {
    return this->Vertex;
  }

  /** Get the status of the patch. */
  StatusEnum GetStatus() const {return Status;}

  /** Set the status of the patch. */
  void SetStatus(StatusEnum status) {Status = status;}
  
};

#endif
