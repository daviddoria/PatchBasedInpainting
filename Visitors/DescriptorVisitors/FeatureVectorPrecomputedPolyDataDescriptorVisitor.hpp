
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

#ifndef FeatureVectorPrecomputedPolyDataDescriptorVisitor_HPP
#define FeatureVectorPrecomputedPolyDataDescriptorVisitor_HPP

// Custom
#include "PixelDescriptors/FeatureVectorPixelDescriptor.h"
#include "Concepts/DescriptorConcept.hpp"
#include "DescriptorVisitorParent.h"

// Submodules
#include <VTKHelpers/VTKHelpers.h>

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

// ITK
#include "itkIndex.h"

// VTK
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

// STL
#include <map>

/**
 * This is a visitor that complies with the DescriptorVisitorConcept. It creates
 * FeatureVectorPixelDescriptor objects at each node. These descriptors are computed
 * before hand and read in from a vtp file (into a vtkPolyData). The mapping from
 * 3D points to pixels is provided in an int array (vtkIntArray) called "OriginalPixel".
 */
template <typename TGraph, typename TDescriptorMap>
struct FeatureVectorPrecomputedPolyDataDescriptorVisitor : public DescriptorVisitorParent<TGraph>
{
  typedef typename boost::property_traits<TDescriptorMap>::value_type DescriptorType;
  BOOST_CONCEPT_ASSERT((DescriptorConcept<DescriptorType, TGraph>));

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  TDescriptorMap& DescriptorMap;

  vtkPolyData* FeaturePolyData;

  std::string FeatureName;

  vtkFloatArray* FeatureArray = nullptr;

  typedef std::map<itk::Index<2>, unsigned int, itk::Index<2>::LexicographicCompare> CoordinateMapType;
  CoordinateMapType CoordinateMap;

  FeatureVectorPrecomputedPolyDataDescriptorVisitor(TDescriptorMap& DescriptorMap,
                                                    vtkPolyData* const featurePolyData,
                                                    const std::string& featureName) :
  DescriptorMap(DescriptorMap), FeaturePolyData(featurePolyData), FeatureName(featureName)
  {
    CreateIndexMap();
  }

  void CreateIndexMap()
  {
    std::cout << "Creating index map..." << std::endl;

    this->FeatureArray = vtkFloatArray::SafeDownCast(this->FeaturePolyData->GetPointData()->GetArray(this->FeatureName.c_str()));
    if(!this->FeatureArray)
    {
      std::stringstream ss;
      ss << "\"" << FeatureName << "\" array must exist in the PolyData's PointData!";
      VTKHelpers::OutputAllArrayNames(this->FeaturePolyData);
      throw std::runtime_error(ss.str());
    }

    // Add a map from all of the pixels to their corresponding point id.
    vtkIntArray* indexArray = vtkIntArray::SafeDownCast(this->FeaturePolyData->GetPointData()->GetArray("OriginalPixel"));
    if(!indexArray)
    {
      throw std::runtime_error("\"OriginalPixel\" array must exist in the PolyData's PointData!");
    }

    for(vtkIdType pointId = 0; pointId < this->FeaturePolyData->GetNumberOfPoints(); ++pointId)
    {
      //int* pixelIndexArray;
      int pixelIndexArray[2];
      indexArray->GetTupleValue(pointId, pixelIndexArray);

      itk::Index<2> pixelIndex;
      pixelIndex[0] = pixelIndexArray[0];
      pixelIndex[1] = pixelIndexArray[1];
      this->CoordinateMap[pixelIndex] = pointId;
    }
    std::cout << "Finished creating index map." << std::endl;
  }

  void InitializeVertex(VertexDescriptorType v, TGraph& g) const override
  {
    //std::cout << "Initializing " << v[0] << " " << v[1] << std::endl;
    // Create the patch object and associate with the node
    itk::Index<2> index;
    index[0] = v[0];
    index[1] = v[1];

    unsigned int pointId = 0;
    unsigned int numberOfMissingPoints = 0;
    // Look for 'index' in the map
    CoordinateMapType::const_iterator iter = this->CoordinateMap.find(index);
    if(iter != this->CoordinateMap.end())
    {
      pointId = iter->second;

      float descriptorValues[this->FeatureArray->GetNumberOfComponents()];
      this->FeatureArray->GetTupleValue(pointId, descriptorValues);

      std::vector<float> featureVector(descriptorValues, descriptorValues + sizeof(descriptorValues) / sizeof(float) );

      DescriptorType descriptor(featureVector);
      descriptor.SetVertex(v);
      descriptor.SetStatus(PixelDescriptor::SOURCE_NODE);
      put(descriptorMap, v, descriptor);

    }
    else
    {
      //std::cout << index << " not found in the map!" << std::endl;
      numberOfMissingPoints++;

      std::vector<float> featureVector(this->FeatureArray->GetNumberOfComponents(), 0);

      DescriptorType descriptor(featureVector);
      descriptor.SetVertex(v);
      descriptor.SetStatus(PixelDescriptor::INVALID);
      put(descriptorMap, v, descriptor);

    }

    //std::cout << "There were " << numberOfMissingPoints << " missing points when computing the descriptor for node " << index << std::endl;
  }

  void DiscoverVertex(VertexDescriptorType v, TGraph& g) const override
  {
    // std::cout << "Discovered " << v[0] << " " << v[1] << std::endl;
    DescriptorType& descriptor = get(descriptorMap, v);
    descriptor.SetStatus(DescriptorType::TARGET_NODE);
  }

}; // FeatureVectorPrecomputedPolyDataDescriptorVisitor

#endif
