#ifndef FeatureVectorPrecomputedPCLNormalsDescriptorVisitor_HPP
#define FeatureVectorPrecomputedPCLNormalsDescriptorVisitor_HPP

// Custom
#include "PixelDescriptors/FeatureVectorPixelDescriptor.h"
#include "DescriptorConcept.hpp"
#include "DescriptorVisitorParent.h"

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

  TDescriptorMap& descriptorMap;

  NormalCloudType Normals;

  FeatureVectorPrecomputedPCLNormalsDescriptorVisitor(TDescriptorMap& in_descriptorMap, NormalCloudType& normals) :
  descriptorMap(in_descriptorMap), Normals(normals)
  {
    
  }

  void initialize_vertex(VertexDescriptorType v, TGraph& g) const
  {
    //std::cout << "Initializing " << v[0] << " " << v[1] << std::endl;
    pcl::Normal n = Normals(v[0], v[1]);
    if(pcl::isFinite(n))
      {
      //std::vector<float> featureVector(descriptorValues, descriptorValues + sizeof(descriptorValues) / sizeof(float) );
      std::vector<float> featureVector(n.normal, n.normal + sizeof(n.normal) / sizeof(float) );
      // std::cout << "Normal has length: " << featureVector.size() << std::endl; // To test if the above worked correctly
      //std::cout << "n: " << featureVector[0] << " " << featureVector[1] << " " << featureVector[2] << std::endl;
      DescriptorType descriptor(featureVector);
      descriptor.SetVertex(v);
      descriptor.SetStatus(PixelDescriptor::SOURCE_NODE);
      put(descriptorMap, v, descriptor);
      }
    else
      {
      std::vector<float> featureVector(3, 0);

      DescriptorType descriptor(featureVector);
      descriptor.SetVertex(v);
      descriptor.SetStatus(PixelDescriptor::INVALID);
      put(descriptorMap, v, descriptor);
      }

    //std::cout << "There were " << numberOfMissingPoints << " missing points when computing the descriptor for node " << index << std::endl;
  };

  void discover_vertex(VertexDescriptorType v, TGraph& g) const
  {
    std::cout << "Discovered " << v[0] << " " << v[1] << std::endl;
    DescriptorType& descriptor = get(descriptorMap, v);
    descriptor.SetStatus(DescriptorType::TARGET_NODE);
    // put(descriptorMap, v, descriptor); // TODO: This shouldn't be necessary since we retrieve it as a reference, but we put it here just for good measure. It should be tested and removed.
  };

}; // FeatureVectorPrecomputedPolyDataDescriptorVisitor

#endif
