/**
 * This is a visitor type that complies with the InpaintingVisitorConcept. It computes
 * and differences feature vectors (std::vector<float>) at each pixel.
 */

#ifndef FeatureVectorInpaintingVisitor_HPP
#define FeatureVectorInpaintingVisitor_HPP

#include "Priority/Priority.h"

#include "PixelDescriptors/FeatureVectorPixelDescriptor.h"

#include "InpaintingVisitorParent.h"

#include "DescriptorConcept.hpp"

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

// VTK
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

// Helpers
#include "Helpers/ITKHelpers.h"

/**
 * This is a visitor that complies with the InpaintingVisitorConcept. It creates
 * and differences FeatureVectorPixelDescriptor objects at each pixel.
 */
template <typename TGraph, typename TImage, typename TBoundaryNodeQueue,
          typename TFillStatusMap, typename TDescriptorMap,
          typename TPriorityMap, typename TBoundaryStatusMap>
struct FeatureVectorInpaintingVisitor : public InpaintingVisitorParent<TGraph>
{
  typedef typename boost::property_traits<TDescriptorMap>::value_type DescriptorType;
  BOOST_CONCEPT_ASSERT((DescriptorConcept<DescriptorType, TGraph>));

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  TImage* image;
  Mask* mask;
  TBoundaryNodeQueue& boundaryNodeQueue;
  Priority* priorityFunction;
  TFillStatusMap& fillStatusMap;
  TDescriptorMap& descriptorMap;
  
  TPriorityMap& priorityMap;
  TBoundaryStatusMap& boundaryStatusMap;

  unsigned int half_width;
  unsigned int NumberOfFinishedVertices;

  vtkPolyData* FeaturePolyData;

  std::string FeatureName;

  vtkFloatArray* FeatureArray;
  
  typedef std::map<itk::Index<2>, unsigned int, itk::Index<2>::LexicographicCompare> CoordinateMapType;
  CoordinateMapType CoordinateMap;

  FeatureVectorInpaintingVisitor(TImage* const in_image, Mask* const in_mask,
                                TBoundaryNodeQueue& in_boundaryNodeQueue, TFillStatusMap& in_fillStatusMap,
                                TDescriptorMap& in_descriptorMap, TPriorityMap& in_priorityMap,
                                Priority* const in_priorityFunction,
                                const unsigned int in_half_width, TBoundaryStatusMap& in_boundaryStatusMap, vtkPolyData* const featurePolyData, const std::string& featureName) :
  image(in_image), mask(in_mask), boundaryNodeQueue(in_boundaryNodeQueue), priorityFunction(in_priorityFunction), fillStatusMap(in_fillStatusMap), descriptorMap(in_descriptorMap),
  priorityMap(in_priorityMap), boundaryStatusMap(in_boundaryStatusMap), half_width(in_half_width), NumberOfFinishedVertices(0), FeaturePolyData(featurePolyData), FeatureName(featureName),
  FeatureArray(NULL)
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

  void initialize_vertex(VertexDescriptorType v, TGraph& g) const
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
  };

  void discover_vertex(VertexDescriptorType v, TGraph& g) const
  {
    std::cout << "Discovered " << v[0] << " " << v[1] << std::endl;
    std::cout << "Priority: " << get(priorityMap, v) << std::endl;
    DescriptorType& descriptor = get(descriptorMap, v);
    descriptor.SetStatus(DescriptorType::TARGET_NODE);
  };

  void vertex_match_made(VertexDescriptorType target, VertexDescriptorType source, TGraph& g) const
  {
    std::cout << "Match made: target: " << target[0] << " " << target[1]
              << " with source: " << source[0] << " " << source[1] << std::endl;
    assert(get(fillStatusMap, source));
    assert(get(descriptorMap, source).GetStatus() == PixelDescriptor::SOURCE_NODE);
  };

  void paint_vertex(VertexDescriptorType target, VertexDescriptorType source, TGraph& g) const
  {
    itk::Index<2> target_index;
    target_index[0] = target[0];
    target_index[1] = target[1];

    itk::Index<2> source_index;
    source_index[0] = source[0];
    source_index[1] = source[1];

    assert(image->GetLargestPossibleRegion().IsInside(source_index));
    assert(image->GetLargestPossibleRegion().IsInside(target_index));

    image->SetPixel(target_index, image->GetPixel(source_index));
  };

  bool accept_painted_vertex(VertexDescriptorType v, TGraph& g) const
  {
    return true;
  };

  void finish_vertex(VertexDescriptorType v, TGraph& g)
  {
    // Construct the region around the vertex
    itk::Index<2> indexToFinish;
    indexToFinish[0] = v[0];
    indexToFinish[1] = v[1];

    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(indexToFinish, half_width);

    region.Crop(image->GetLargestPossibleRegion()); // Make sure the region is entirely inside the image

    // Mark all the pixels in this region as filled.
    // It does not matter which image we iterate over, we just want the indices.
    itk::ImageRegionConstIteratorWithIndex<TImage> gridIterator(image, region);

    while(!gridIterator.IsAtEnd())
      {
      VertexDescriptorType v;
      v[0] = gridIterator.GetIndex()[0];
      v[1] = gridIterator.GetIndex()[1];
      put(fillStatusMap, v, true);
      mask->SetPixel(gridIterator.GetIndex(), mask->GetValidValue());
      ++gridIterator;
      }

    // Additionally, initialize the filled vertices because they may now be valid.
    // This must be done in a separate loop like this because the mask image used to check for boundary pixels is incorrect until the above loop updates it.
    gridIterator.GoToBegin();
    while(!gridIterator.IsAtEnd())
      {
      VertexDescriptorType v;
      v[0] = gridIterator.GetIndex()[0];
      v[1] = gridIterator.GetIndex()[1];
      initialize_vertex(v, g);
      ++gridIterator;
      }

    // Update the priority function.
    this->priorityFunction->Update(indexToFinish);

    // Add pixels that are on the new boundary to the queue, and mark other pixels as not in the queue.
    itk::ImageRegionConstIteratorWithIndex<Mask> imageIterator(mask, region);

    while(!imageIterator.IsAtEnd())
      {
      VertexDescriptorType v;
      v[0] = imageIterator.GetIndex()[0];
      v[1] = imageIterator.GetIndex()[1];

      // Mark all nodes in the patch around this node as filled (in the FillStatusMap).
      // This makes them ignored if they are still in the boundaryNodeQueue.
      if(ITKHelpers::HasNeighborWithValue(imageIterator.GetIndex(), mask, mask->GetHoleValue()))
        {
        put(boundaryStatusMap, v, true);
        this->boundaryNodeQueue.push(v);
        float priority = this->priorityFunction->ComputePriority(imageIterator.GetIndex());
        //std::cout << "updated priority: " << priority << std::endl;
        put(priorityMap, v, priority);
        }
      else
        {
        put(boundaryStatusMap, v, false);
        }

      ++imageIterator;
      }

    {
    // Debug only - write the mask to a file
    HelpersOutput::WriteImage(mask, Helpers::GetSequentialFileName("debugMask", this->NumberOfFinishedVertices, "png"));
    HelpersOutput::WriteVectorImageAsRGB(image, Helpers::GetSequentialFileName("output", this->NumberOfFinishedVertices, "png"));
    this->NumberOfFinishedVertices++;
    }
  }; // finish_vertex

}; // FeatureVectorInpaintingVisitor

#endif
