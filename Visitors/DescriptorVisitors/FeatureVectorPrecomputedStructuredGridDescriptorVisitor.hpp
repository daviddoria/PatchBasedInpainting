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

#ifndef FeatureVectorPrecomputedStructuredGridDescriptorVisitor_HPP
#define FeatureVectorPrecomputedStructuredGridDescriptorVisitor_HPP

// Custom
#include "PixelDescriptors/FeatureVectorPixelDescriptor.h"
#include "Concepts/DescriptorConcept.hpp"
#include "DescriptorVisitorParent.h"

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

// ITK
#include "itkIndex.h"

// VTK
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkStructuredGrid.h>

/**
 * This is a visitor that complies with the DescriptorVisitorConcept. It creates
 * FeatureVectorPixelDescriptor objects at each node. These descriptors are computed
 * before hand and read in from a vts file (into a vtkStructuredGrid). The mapping from
 * 3D points to pixels is provided implicity by the grid.
 */
template <typename TGraph, typename TDescriptorMap>
struct FeatureVectorPrecomputedStructuredGridDescriptorVisitor : public DescriptorVisitorParent<TGraph>
{
  typedef typename boost::property_traits<TDescriptorMap>::value_type DescriptorType;
  BOOST_CONCEPT_ASSERT((DescriptorConcept<DescriptorType, TGraph>));

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  TDescriptorMap& DescriptorMap;

  vtkStructuredGrid* FeatureStructuredGrid;

  std::string FeatureName;

  vtkFloatArray* FeatureArray;

  FeatureVectorPrecomputedStructuredGridDescriptorVisitor(TDescriptorMap& in_descriptorMap,
                                                          vtkStructuredGrid* const featureStructuredGrid, const std::string& featureName) :
  DescriptorMap(in_descriptorMap), FeatureStructuredGrid(featureStructuredGrid), FeatureName(featureName)
  {
    FeatureArray = vtkFloatArray::SafeDownCast(FeatureStructuredGrid->GetPointData()->GetArray(featureName.c_str()));
    if(!FeatureArray)
    {
      std::stringstream ss;
      ss << "Structured grid does not have an array named \"" << featureName << "\"!";
      throw std::runtime_error(ss.str());
    }
    std::cout << "Feature " << featureName << " has " << FeatureArray->GetNumberOfComponents() << " components." << std::endl;
  }

  void InitializeVertex(VertexDescriptorType v, TGraph& g) const override
  {
    //std::cout << "Initializing " << v[0] << " " << v[1] << std::endl;
    unsigned int numberOfMissingPoints = 0;

    int queryPoint[3] = {v[0], v[1], 0};
    int dimensions[3];
    this->FeatureStructuredGrid->GetDimensions(dimensions);
    vtkIdType pointId = vtkStructuredData::ComputePointId(dimensions, queryPoint);
    if(this->FeatureStructuredGrid->IsPointVisible (pointId))
    {
      float descriptorValues[this->FeatureArray->GetNumberOfComponents()];
      this->FeatureArray->GetTupleValue(pointId, descriptorValues);

      std::vector<float> featureVector(descriptorValues, descriptorValues + sizeof(descriptorValues) / sizeof(float) );

      DescriptorType descriptor(featureVector);
      descriptor.SetVertex(v);
      descriptor.SetStatus(PixelDescriptor::SOURCE_NODE);
      put(this->DescriptorMap, v, descriptor);
    }
    else
    {
      //std::cout << index << " not found in the map!" << std::endl;
      numberOfMissingPoints++;

      std::vector<float> featureVector(this->FeatureArray->GetNumberOfComponents(), 0);

      DescriptorType descriptor(featureVector);
      descriptor.SetVertex(v);
      descriptor.SetStatus(PixelDescriptor::INVALID);
      put(this->DescriptorMap, v, descriptor);
    }

    //std::cout << "There were " << numberOfMissingPoints << " missing points when computing the descriptor for node " << index << std::endl;
  }

  void DiscoverVertex(VertexDescriptorType v, TGraph& g) const override
  {
    // std::cout << "Discovered " << v[0] << " " << v[1] << std::endl;
    DescriptorType& descriptor = get(this->DescriptorMap, v);
    descriptor.SetStatus(DescriptorType::TARGET_NODE);
  }

}; // FeatureVectorPrecomputedPolyDataDescriptorVisitor

#endif
