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

/**
 * This is a visitor type that complies with the InpaintingVisitorConcept. It computes
 * and differences feature vectors (std::vector<float>) at each pixel.
 */

#ifndef FeatureVectorInpaintingVisitor_HPP
#define FeatureVectorInpaintingVisitor_HPP

// Custom
#include "Priority/Priority.h"

#include "PixelDescriptors/FeatureVectorPixelDescriptor.h"

#include "InpaintingVisitorParent.h"

#include "Concepts/DescriptorConcept.hpp"

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

// VTK
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

// Submodules
#include "ITKHelpers/ITKHelpers.h"
#include "VTKHelpers/VTKHelpers.h"

/**
 * This is a visitor that complies with the InpaintingVisitorConcept. It creates
 * and differences FeatureVectorPixelDescriptor objects at each pixel.
 */
template <typename TGraph, typename TImage, typename TBoundaryNodeQueue,
          typename TFillStatusMap, typename TDescriptorMap, typename TPriority,
          typename TPriorityMap, typename TBoundaryStatusMap>
struct FeatureVectorInpaintingVisitor : public InpaintingVisitorParent<TGraph>
{
  typedef typename boost::property_traits<TDescriptorMap>::value_type DescriptorType;
  BOOST_CONCEPT_ASSERT((DescriptorConcept<DescriptorType, TGraph>));

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  TImage* Image;
  Mask* MaskImage;
  TBoundaryNodeQueue& BoundaryNodeQueue;
  TPriority* PriorityFunction;
  TFillStatusMap& FillStatusMap;
  TDescriptorMap& DescriptorMap;
  
  TPriorityMap& PriorityMap;
  TBoundaryStatusMap& BoundaryStatusMap;

  unsigned int HalfWidth;
  unsigned int NumberOfFinishedVertices = 0;

  vtkPolyData* FeaturePolyData;

  std::string FeatureName;

  vtkFloatArray* FeatureArray = nullptr;
  
  typedef std::map<itk::Index<2>, unsigned int, itk::Index<2>::LexicographicCompare> CoordinateMapType;
  CoordinateMapType CoordinateMap;

  FeatureVectorInpaintingVisitor(TImage* const in_image, Mask* const in_mask,
                                TBoundaryNodeQueue& in_boundaryNodeQueue, TFillStatusMap& in_fillStatusMap,
                                TDescriptorMap& in_descriptorMap, TPriorityMap& in_priorityMap,
                                TPriority* const in_priorityFunction,
                                const unsigned int in_half_width, TBoundaryStatusMap& in_boundaryStatusMap, vtkPolyData* const featurePolyData, const std::string& featureName) :
    Image(in_image), MaskImage(in_mask), BoundaryNodeQueue(in_boundaryNodeQueue),
    PriorityFunction(in_priorityFunction), FillStatusMap(in_fillStatusMap), DescriptorMap(in_descriptorMap),
    PriorityMap(in_priorityMap), BoundaryStatusMap(in_boundaryStatusMap),
    HalfWidth(in_half_width), FeaturePolyData(featurePolyData), FeatureName(featureName)
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
    std::cout << "Discovered " << v[0] << " " << v[1] << std::endl;
    std::cout << "Priority: " << get(this->PriorityMap, v) << std::endl;
    DescriptorType& descriptor = get(this->DescriptorMap, v);
    descriptor.SetStatus(DescriptorType::TARGET_NODE);
  }

  void vertex_match_made(VertexDescriptorType target, VertexDescriptorType source, TGraph& g) const
  {
    std::cout << "Match made: target: " << target[0] << " " << target[1]
              << " with source: " << source[0] << " " << source[1] << std::endl;
    assert(get(fillStatusMap, source));
    assert(get(descriptorMap, source).GetStatus() == PixelDescriptor::SOURCE_NODE);
  }

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

    Image->SetPixel(target_index, Image->GetPixel(source_index));
  }

  bool accept_painted_vertex(VertexDescriptorType v, TGraph& g) const
  {
    return true;
  }

  void finish_vertex(VertexDescriptorType v, VertexDescriptorType sourceNode, TGraph& g)
  {
    // Construct the region around the vertex
    itk::Index<2> indexToFinish;
    indexToFinish[0] = v[0];
    indexToFinish[1] = v[1];

    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(indexToFinish, HalfWidth);

    region.Crop(Image->GetLargestPossibleRegion()); // Make sure the region is entirely inside the image

    // Mark all the pixels in this region as filled.
    // It does not matter which image we iterate over, we just want the indices.
    itk::ImageRegionConstIteratorWithIndex<TImage> gridIterator(Image, region);

    while(!gridIterator.IsAtEnd())
    {
      VertexDescriptorType v;
      v[0] = gridIterator.GetIndex()[0];
      v[1] = gridIterator.GetIndex()[1];
      put(FillStatusMap, v, true);
      MaskImage->SetPixel(gridIterator.GetIndex(), MaskImage->GetValidValue());
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
    this->PriorityFunction->Update(sourceNode, v);

    // Add pixels that are on the new boundary to the queue, and mark other pixels as not in the queue.
    itk::ImageRegionConstIteratorWithIndex<Mask> imageIterator(this->MaskImage, region);

    while(!imageIterator.IsAtEnd())
    {
      VertexDescriptorType v;
      v[0] = imageIterator.GetIndex()[0];
      v[1] = imageIterator.GetIndex()[1];

      // Mark all nodes in the patch around this node as filled (in the FillStatusMap).
      // This makes them ignored if they are still in the boundaryNodeQueue.
      if(ITKHelpers::HasNeighborWithValue(imageIterator.GetIndex(), this->MaskImage, this->MaskImage->GetHoleValue()))
      {
        put(BoundaryStatusMap, v, true);
        this->BoundaryNodeQueue.push(v);
        float priority = this->PriorityFunction->ComputePriority(imageIterator.GetIndex());
        //std::cout << "updated priority: " << priority << std::endl;
        put(this->PriorityMap, v, priority);
      }
      else
      {
        put(this->BoundaryStatusMap, v, false);
      }

      ++imageIterator;
    }

    {
    // Debug only - write the mask to a file
    HelpersOutput::WriteImage(this->MaskImage, Helpers::GetSequentialFileName("debugMask", this->NumberOfFinishedVertices, "png"));
    HelpersOutput::WriteVectorImageAsRGB(this->Image, Helpers::GetSequentialFileName("output", this->NumberOfFinishedVertices, "png"));
    this->NumberOfFinishedVertices++;
    }
  } // finish_vertex

}; // FeatureVectorInpaintingVisitor

#endif
