#ifndef FeatureVectorPrecomputedStructuredGridDescriptorVisitor_HPP
#define FeatureVectorPrecomputedStructuredGridDescriptorVisitor_HPP

// Custom
#include "PixelDescriptors/FeatureVectorPixelDescriptor.h"
#include "DescriptorConcept.hpp"
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

  TDescriptorMap& descriptorMap;

  vtkStructuredGrid* FeatureStructuredGrid;

  std::string FeatureName;

  vtkFloatArray* FeatureArray;

  FeatureVectorPrecomputedStructuredGridDescriptorVisitor(TDescriptorMap& in_descriptorMap, vtkStructuredGrid* const featureStructuredGrid, const std::string& featureName) :
  descriptorMap(in_descriptorMap), FeatureStructuredGrid(featureStructuredGrid), FeatureName(featureName), FeatureArray(NULL)
  {

  }

  void initialize_vertex(VertexDescriptorType v, TGraph& g) const
  {
    //std::cout << "Initializing " << v[0] << " " << v[1] << std::endl;
    unsigned int numberOfMissingPoints = 0;

    int queryPoint[3] = {v[0], v[1], 0};
    int dimensions[3];
    FeatureStructuredGrid->GetDimensions(dimensions);
    vtkIdType pointId = vtkStructuredData::ComputePointId(dimensions, queryPoint);
    if(FeatureStructuredGrid->IsPointVisible (pointId))
      {
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
  };

  void discover_vertex(VertexDescriptorType v, TGraph& g) const
  {
    std::cout << "Discovered " << v[0] << " " << v[1] << std::endl;
    DescriptorType& descriptor = get(descriptorMap, v);
    descriptor.SetStatus(DescriptorType::TARGET_NODE);
  };

}; // FeatureVectorPrecomputedPolyDataDescriptorVisitor

#endif
