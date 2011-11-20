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

#include "DebugOutputs.h"

DebugOutputs::DebugOutputs()
{
  this->DebugFunctionEnterLeave = false;
  this->DebugMessages = false;
  this->DebugImages = false;
}

void DebugOutputs::SetDebugFunctionEnterLeave(const bool value)
{
  this->DebugFunctionEnterLeave = value;
}


void DebugOutputs::EnterFunction(const std::string& functionName) const
{
  if(this->DebugFunctionEnterLeave)
    {
    std::cout << "Enter " << functionName << std::endl;
    }
}

void DebugOutputs::LeaveFunction(const std::string& functionName) const
{
  if(this->DebugFunctionEnterLeave)
    {
    std::cout << "Leave " << functionName << std::endl;
    }
}

void DebugOutputs::DebugMessage(const std::string& message) const
{
  if(this->DebugMessages)
    {
    std::cout << message << std::endl;
    }
}

void DebugOutputs::SetDebugMessages(const bool flag)
{
  this->DebugMessages = flag;
}

void DebugOutputs::SetDebugImages(const bool flag)
{
  this->DebugImages = flag;
}