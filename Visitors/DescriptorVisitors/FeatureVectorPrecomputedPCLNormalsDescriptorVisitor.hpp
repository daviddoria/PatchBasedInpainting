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

#ifndef FeatureVectorPrecomputedPCLNormalsDescriptorVisitor_HPP
#define FeatureVectorPrecomputedPCLNormalsDescriptorVisitor_HPP

// Custom
#include "PixelDescriptors/FeatureVectorPixelDescriptor.h"
#include "Concepts/DescriptorConcept.hpp"
#include "Visitors/DescriptorVisitors/DescriptorVisitorParent.h"

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

// PCL
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

/**
 * This is a visitor that complies with the DescriptorVisitorConcept. It creates
 * FeatureVectorPixelDescriptor objects at each node. These descriptors are computed
 * before hand and read in from a vtp file (into a vtkPolyData). The mapping from
 * 3D points to pixels is provided in an int array (vtkIntArray) called "OriginalPixel".
 */
template <typename TGraph, typename TDescriptorMap>
struct FeatureVectorPrecomputedPCLNormalsDescriptorVisitor : public DescriptorVisitorParent<TGraph>
{
  typedef pcl::PointCloud<pcl::Normal> NormalCloudType;

  typedef typename boost::property_traits<TDescriptorMap>::value_type DescriptorType;
  BOOST_CONCEPT_ASSERT((DescriptorConcept<DescriptorType, TGraph>));

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  TDescriptorMap& DescriptorMap;

  NormalCloudType Normals;

  FeatureVectorPrecomputedPCLNormalsDescriptorVisitor(TDescriptorMap& descriptorMap,
                                                      NormalCloudType& normals) :
  DescriptorMap(descriptorMap), Normals(normals)
  {
    
  }

  void InitializeVertex(VertexDescriptorType v, TGraph& g) const override
  {
    //std::cout << "Initializing " << v[0] << " " << v[1] << std::endl;
    pcl::Normal n = this->Normals(v[0], v[1]);
    if(pcl::isFinite(n))
    {
      //std::vector<float> featureVector(descriptorValues, descriptorValues + sizeof(descriptorValues) / sizeof(float) );
      std::vector<float> featureVector(n.normal, n.normal + sizeof(n.normal) / sizeof(float) );
      // std::cout << "Normal has length: " << featureVector.size() << std::endl; // To test if the above worked correctly
      assert(featureVector.size() == 3);
      std::cout << "initialize_vertex: featureVector: " << featureVector << std::endl;
      DescriptorType descriptor(featureVector);
      descriptor.SetVertex(v);
      descriptor.SetStatus(PixelDescriptor::SOURCE_NODE);
      put(this->DescriptorMap, v, descriptor);
    }
    else
    {
      std::cout << "Not finite! " << n.normal_x << " " << n.normal_y << " " << n.normal_z << std::endl;

      std::vector<float> featureVector(3, 0);

      DescriptorType descriptor(featureVector);
      descriptor.SetVertex(v);
      descriptor.SetStatus(PixelDescriptor::INVALID);
      put(this->DescriptorMap, v, descriptor);
    }

    //std::cout << "There were " << numberOfMissingPoints << " missing points when computing the descriptor for node " << index << std::endl;
  }

  void DiscoverVertex(VertexDescriptorType v, TGraph& g) const override
  {
    std::cout << "Discovered " << v[0] << " " << v[1] << std::endl;
    DescriptorType& descriptor = get(this->DescriptorMap, v);
    descriptor.SetStatus(DescriptorType::TARGET_NODE);
  }

}; // FeatureVectorPrecomputedPolyDataDescriptorVisitor

#endif
