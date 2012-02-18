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

#ifndef Node_H
#define Node_H

/**
\class Node
\brief This is a class to represent a position on an integer 2D grid.
*/
class Node
{
private:
  int coord[2];
public:

  Node()
  {
    coord[0] = -1;
    coord[1] = -1;
  }
  
  Node(int component0, int component1)
  {
    coord[0] = component0;
    coord[1] = component1;
  }

  template <typename T>
  Node(const T& object)
  {
    CreateFromObject(object);
  }

  int operator[](const unsigned int& component) const
  {
    return coord[component];
  }

  template <typename T>
  void CreateFromObject(const T& object)
  {
    coord[0] = object[0];
    coord[1] = object[1];
  }
};

#endif
