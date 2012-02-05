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

#ifndef FeatureVectorPixelDescriptor_H
#define FeatureVectorPixelDescriptor_H

// STL
#include <vector>
#include <iostream>

/**
\class FeatureVectorPixelDescriptor
\brief This class stores a vector of floats - a vector valued descriptor.
*/
class FeatureVectorPixelDescriptor
{
public:

  typedef std::vector<float> FeatureVectorType;

  /** Default constructor to allow objects to be stored in a container.*/
  FeatureVectorPixelDescriptor(){};

  /** Construct a descriptor.*/
  FeatureVectorPixelDescriptor(const FeatureVectorType& v);

  /** Get the feature vector.*/
  FeatureVectorType GetFeatureVector() const;

  /** Output information about the descriptor. */
  friend std::ostream& operator<<(std::ostream& output, const FeatureVectorType& descriptor);

  /** If a patch is invalid, it means any comparison with it should return inf. */
  enum StatusEnum {SOURCE_PATCH, TARGET_PATCH, INVALID};

  /** Get the status of the patch. */
  StatusEnum GetStatus() const {return Status;}

  /** Set the status of the patch. */
  void SetStatus(StatusEnum status) {Status = status;}

private:
  /** The feature vector. */
  FeatureVectorType FeatureVector;

  /** Indicate if the patch is a source patch, a target patch, or invalid. */
  StatusEnum Status;

};

#endif
