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

#ifndef DEBUGOUTPUTS_H
#define DEBUGOUTPUTS_H

#include <string>

class DebugOutputs
{
public:
  // Specify if you want to output the parts of the call stack that you have specified.
  void SetDebugFunctionEnterLeave(const bool value);
  
  // Specify if you want to write debuging images.
  void SetDebugImages(const bool);
  
  // Specify if you want to see debugging messages.
  void SetDebugMessages(const bool);

protected:
  bool DebugFunctionEnterLeave;
  bool DebugImages;
  bool DebugMessages;
  
  void EnterFunction(const std::string&);
  void LeaveFunction(const std::string&);
  
  // Output a message if DebugMessages is set to true.
  void DebugMessage(const std::string&);
  
  // Output a message and a value if DebugMessages is set to true.
  template <typename T>
  void DebugMessage(const std::string& message, const T value);

};

#include "DebugOutputs.hxx"

#endif
