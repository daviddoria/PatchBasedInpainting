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

#ifndef NeighborhoodSearch_HPP
#define NeighborhoodSearch_HPP

// Custom
#include <Helpers/Helpers.h>
#include <ITKHelpers/ITKHelpers.h>
#include <PixelDescriptors/PixelDescriptor.h>
// ITK
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImage.h"

/**
  * This class returns a region around a pixel of a specified radius. It is intended
  * for use with InpaintingAlgorithmWithLocalSearch to reduce the search space for
  * the source patch.
  */
template <typename TVertexDescriptorType, typename TImagePatchDescriptorMap>
struct NeighborhoodSearch
{
  itk::ImageRegion<2> FullRegion;

  typedef std::vector<TVertexDescriptorType> VectorType;

  unsigned int Radius;

  TImagePatchDescriptorMap ImagePatchDescriptorMap;
  
  /**
    * 'fullRegion' is the region of the image that is being inpainted.
    */
  NeighborhoodSearch(const itk::ImageRegion<2>& fullRegion, const unsigned int radius,
                     TImagePatchDescriptorMap imagePatchDescriptorMap) :
    FullRegion(fullRegion), Radius(radius), ImagePatchDescriptorMap(imagePatchDescriptorMap)
  {

  }

  VectorType operator()(const TVertexDescriptorType& center)
  {
    // Convert to an ITK type
    itk::Index<2> centerIndex =
        Helpers::ConvertFrom<itk::Index<2>, TVertexDescriptorType>(center);
    
    // Get the region around the center pixel
    itk::ImageRegion<2> region =
        ITKHelpers::GetRegionInRadiusAroundPixel(centerIndex, this->Radius);

    // Ensure the region is entirely inside the image
    region.Crop(this->FullRegion);
    
    std::vector<itk::Index<2> > indices = ITKHelpers::GetIndicesInRegion(region);

//    VectorType vertices(region.GetNumberOfPixels());
    VectorType vertices;

    for(unsigned int indexId = 0; indexId < indices.size(); ++indexId)
    {
      TVertexDescriptorType vert =
          Helpers::ConvertFrom<TVertexDescriptorType, itk::Index<2> > (indices[indexId]);

      if(get(this->ImagePatchDescriptorMap, vert).GetStatus() == PixelDescriptor::SOURCE_NODE)
      {
        vertices.push_back(vert);
      }
    }
    return vertices;
  }

};

#endif
