#ifndef FeatureVectorFPFHDescriptorVisitor_HPP
#define FeatureVectorFPFHDescriptorVisitor_HPP

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
#include <pcl/features/fpfh.h>

// VTK
#include <vtkStructuredGrid.h>

// Custom
#include "ImageProcessing/Mask.h"

/**
 * This is a visitor that complies with the DescriptorVisitorConcept. It creates
 * FeatureVectorPixelDescriptor objects at each node. These descriptors are computed
 * before hand and read in from a vtp file (into a vtkPolyData). The mapping from
 * 3D points to pixels is provided in an int array (vtkIntArray) called "OriginalPixel".
 */
template <typename TGraph, typename TDescriptorMap, typename TPointCloud>
struct FeatureVectorFPFHDescriptorVisitor : public DescriptorVisitorParent<TGraph>
{
  typedef typename boost::property_traits<TDescriptorMap>::value_type DescriptorType;
  BOOST_CONCEPT_ASSERT((DescriptorConcept<DescriptorType, TGraph>));

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  TDescriptorMap& DescriptorMap;

  TPointCloud& PointCloud;
  Mask* MaskImage;

  FeatureVectorFPFHDescriptorVisitor(TDescriptorMap& in_descriptorMap, const PointCloud& pointCloud, Mask* const mask) :
  DescriptorMap(in_descriptorMap), PointCloud(pointCloud), MaskImage(mask)
  {
    
  }

  void initialize_vertex(VertexDescriptorType v, TGraph& g) const
  {
    //std::cout << "Initializing " << v[0] << " " << v[1] << std::endl;
    itk::Index<2> index = {{v[0], v[1]}};
    if(MaskImage->IsHole(index))
      {
      std::vector<float> featureVector(33, 0);

      DescriptorType descriptor(featureVector);
      descriptor.SetVertex(v);
      descriptor.SetStatus(PixelDescriptor::INVALID);
      put(DescriptorMap, v, descriptor);
      }

    typedef pcl::PointCloud<pcl::PointNormal> InputCloudType;
    typedef pcl::PointCloud<pcl::FPFHSignature33> OutputCloudType;

    // Initalize 'output'
    OutputCloudType::Ptr output(new OutputCloudType);
    output->resize(StructuredGrid->GetNumberOfPoints());

    // Create a tree
    typedef pcl::search::KdTree<InputCloudType::PointType> TreeType;
    TreeType::Ptr tree = typename TreeType::Ptr(new TreeType);

    unsigned int patch_half_width = 5;

    std::cout << "Computing descriptors..." << std::endl;
    itk::ImageRegion<2> fullRegion = mask->GetLargestPossibleRegion();
    itk::ImageRegionConstIteratorWithIndex<MaskImageType> imageIterator(mask, fullRegion);
    std::cout << "Full region: " << fullRegion << std::endl;

    //std::cout << "Computing descriptor for pixel " << imageIterator.GetIndex() << std::endl;
    //fout << "Computing descriptor for pixel " << imageIterator.GetIndex() << std::endl;
    itk::ImageRegion<2> patchRegion = Helpers::GetRegionInRadiusAroundPixel(imageIterator.GetIndex(), patch_half_width);
    //std::cout << "patchRegion: " << patchRegion << std::endl;
    if(!fullRegion.IsInside(patchRegion))
      {
      ++imageIterator;
      continue;
      }

    // Get a list of the pointIds in the region
    //std::vector<unsigned int> pointIds;
    std::vector<int> pointIds;

    itk::ImageRegionConstIteratorWithIndex<MaskImageType> patchIterator(mask, patchRegion);
    while(!patchIterator.IsAtEnd())
      {
      if(!patchIterator.Get())
        {
        pointIds.push_back(coordinateMap[patchIterator.GetIndex()]);
        }
      ++patchIterator;
      }

    if(pointIds.size() < 2)
      {
      unsigned int currentPointId = coordinateMap[imageIterator.GetIndex()];

      output->points[currentPointId] = emptyPoint;
      ++imageIterator;
      continue;
      }
    //std::cout << "There are " << pointIds.size() << " points in this patch." << std::endl;

    pcl::FPFHEstimation<pcl::PointNormal, pcl::PointNormal, pcl::FPFHSignature33> fpfhEstimation;
    // Provide the original point cloud (without normals)
    fpfhEstimation.setInputCloud (input);
    // Provide the point cloud with normals
    fpfhEstimation.setInputNormals(input);

    typedef pcl::search::KdTree<InputCloud::PointType> TreeType;
    TreeType::Ptr tree = typename TreeType::Ptr(new TreeType);
    
    // fpfhEstimation.setInputWithNormals(cloud, cloudWithNormals); PFHEstimation does not have this function
    // Use the same KdTree from the normal estimation
    fpfhEstimation.setSearchMethod(tree);

    OutputCloud::Ptr pfhFeatures(new OutputCloud);

    fpfhEstimation.setRadiusSearch (0.2);

    // Actually compute the features
    fpfhEstimation.compute (*pfhFeatures);
    
    unsigned int currentPointId = coordinateMap[imageIterator.GetIndex()];

    output->points[currentPointId] = vfhFeature->points[0];

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
      put(DescriptorMap, v, descriptor);
      }


    //std::cout << "There were " << numberOfMissingPoints << " missing points when computing the descriptor for node " << index << std::endl;
  };

  void discover_vertex(VertexDescriptorType v, TGraph& g) const
  {
    std::cout << "Discovered " << v[0] << " " << v[1] << std::endl;
    DescriptorType& descriptor = get(DescriptorMap, v);
    descriptor.SetStatus(DescriptorType::TARGET_NODE);
  };

}; // FeatureVectorFPFHDescriptorVisitor

#endif
