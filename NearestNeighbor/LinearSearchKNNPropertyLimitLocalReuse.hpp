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

#ifndef LinearSearchKNNPropertyLimitLocalReuse_HPP
#define LinearSearchKNNPropertyLimitLocalReuse_HPP

// STL
#include <algorithm> // for lower_bound()
#include <limits> // for infinity()
#include <set>

// Boost
#include <boost/utility.hpp> // for enable_if()
#include <boost/type_traits.hpp> // for is_same()

// Custom
#include "Utilities/Utilities.hpp"

// Submodules
#include <Mask/Mask.h>
#include <Utilities/Debug/Debug.h>

/**
  * This function template is similar to std::min_element but can be used when the comparison
  * involves computing a derived quantity (a.k.a. distance). This algorithm will search for the
  * the elements in the range [first,last) with the "smallest" distances (of course, both the
  * distance metric and comparison can be overriden to perform something other than the canonical
  * Euclidean distance and less-than comparison, which would yield the element with minimum distance).
  * This function will fill the output container with a number of nearest-neighbors.
  * \tparam DistanceValueType The value-type for the distance measures.
  * \tparam DistanceFunctionType The functor type to compute the distance measure.
  */
template <typename TPatchDescriptorPropertyMap, typename TPatchDistanceFunction,
          typename TImageToWrite>
class LinearSearchKNNPropertyLimitLocalReuse : public Debug
{
  typedef float DistanceValueType;

  /** The map of vertex descriptors. */
  TPatchDescriptorPropertyMap PatchDescriptorPropertyMap;

  /** How many top patches to return. */
  unsigned int K;

  /** The function to use to compare patches. */
  TPatchDistanceFunction PatchDistanceFunction;

  /** An image indicating where each pixel came from. */
  typedef itk::Image<itk::Index<2>, 2> SourcePixelMapImageType;
  SourcePixelMapImageType* SourcePixelMapImage;

  /** The current inpainting mask. */
  Mask* MaskImage;

  /** Store the full region so that it can be reference without an image or mask. */
  itk::ImageRegion<2> FullRegion;

  unsigned int Iteration = 0;

  TImageToWrite* ImageToWrite;

  float LocalRegionSizeMultiplier;

  float MaxAllowedUsedPixelsRatio;

public:
  LinearSearchKNNPropertyLimitLocalReuse(TPatchDescriptorPropertyMap patchDescriptorPropertyMap, Mask* mask, const unsigned int k = 1000,
                                         float localRegionSizeMultiplier = 3.0f, float maxAllowedUsedPixelsRatio = 0.5f,
                                         TPatchDistanceFunction patchDistanceFunction = TPatchDistanceFunction(),
                                         SourcePixelMapImageType* sourcePixelMapImage = nullptr, TImageToWrite* imageToWrite = nullptr) :
    PatchDescriptorPropertyMap(patchDescriptorPropertyMap), K(k), PatchDistanceFunction(patchDistanceFunction),
    SourcePixelMapImage(sourcePixelMapImage), MaskImage(mask), ImageToWrite(imageToWrite),
    LocalRegionSizeMultiplier(localRegionSizeMultiplier), MaxAllowedUsedPixelsRatio(maxAllowedUsedPixelsRatio)
  {
    this->FullRegion = this->MaskImage->GetLargestPossibleRegion();
  }

  /** Set the number of nearest neighbors to return. */
  void SetK(const unsigned int k)
  {
    this->K = k;
  }

  /** Get the number of nearest neighbors to return. */
  unsigned int GetK() const
  {
    return this->K;
  }

  //   template <typename ForwardIteratorType, typename OutputContainerType>
  //   void operator()(ForwardIteratorType first, ForwardIteratorType last,
  //                   typename ForwardIteratorType::value_type queryNode, OutputContainerType& output)
  /**
    * \tparam ForwardIterator The forward-iterator type.
    * \tparam OutputContainer The container type which can contain the list of nearest-neighbors (STL like container, with iterators, insert, size, and pop_back).
    * \param first Start of the range in which to search.
    * \param last One element past the last element in the range in which to search (usually container.end() ).
    * \param output The container that will have the sorted list of elements with the smallest distance.
    * \param compare A callable object that returns true if the first element is the preferred one (less-than) of the two.
    * \param max_neighbors The maximum number of elements of smallest distance to output in the sorted list.
    */
  template <typename ForwardIteratorType, typename OutputIteratorType>
  inline OutputIteratorType
  operator()(ForwardIteratorType first, ForwardIteratorType last,
             typename ForwardIteratorType::value_type queryNode,
             OutputIteratorType outputFirst)
  {
    if(this->DebugScreenOutputs)
    {
      std::cout << "LinearSearchKNNPropertyLimitLocalReuse searching " << last - first << " patches." << std::endl;
    }

    // Nothing to do if the input range is empty
    if(first == last)
    {
      return outputFirst;
    }

    // This doesn't work correctly - the image is written but it cannot be opened in a viewer like gwenview,
    // and in Paraview it does not seem to contain columns of patches and numbers? Perhaps because of the extreme
    // size of the image?
//    if(this->DebugImages && this->Iteration == 140)
//    {
//      if(this->ImageToWrite == nullptr)
//      {
//        throw std::runtime_error("LinearSearchBestLidarTextureGradient cannot WriteTopPatches without having an ImageToWrite!");
//      }
//      PatchHelpers::WriteTopPatches(this->ImageToWrite, this->PatchDescriptorPropertyMap, first, last,
//                                    "KNNPatches", this->Iteration);
//    }

    if(this->DebugImages)
    {
      ITKHelpers::WriteIndexImage(this->SourcePixelMapImage, Helpers::GetSequentialFileName("SourcePixelMap", this->Iteration, "mha", 3));
    }

    // Use a priority queue to keep the items sorted
    typedef std::pair<DistanceValueType, ForwardIteratorType> PairType;
    typedef compare_pair_first<DistanceValueType, ForwardIteratorType, std::greater<DistanceValueType> > CompareType;
    typedef std::priority_queue< PairType, std::vector<PairType>,
        CompareType> PriorityQueueType;

    PriorityQueueType outputQueue;

    typename TPatchDescriptorPropertyMap::value_type queryDescriptor = get(this->PatchDescriptorPropertyMap, queryNode);

    typedef typename ForwardIteratorType::value_type NodeType;

    // Create a region from the node
    itk::Index<2> queryIndex = Helpers::ConvertFrom<itk::Index<2>, NodeType>(queryNode);

//    itk::ImageRegion<2> queryRegion =
//        ITKHelpers::GetRegionInRadiusAroundPixel(queryIndex,
//                                                 get(this->PatchDescriptorPropertyMap, queryNode).GetRegion().GetSize()[0]/2);

    itk::ImageRegion<2> queryRegion = queryDescriptor.GetRegion();

    // Look for pixels that have already been used

    // Create a bigger region
    itk::ImageRegion<2> largerTargetRegion =
        ITKHelpers::GetRegionInRadiusAroundPixel(queryIndex,
                                                 queryRegion.GetSize()[0]/2.0f * this->LocalRegionSizeMultiplier);
    largerTargetRegion.Crop(this->FullRegion);
    std::cout << "The region to search for repeated pixels is " << largerTargetRegion << std::endl;

    itk::ImageRegionConstIterator<SourcePixelMapImageType> sourcePixelMapIterator(this->SourcePixelMapImage,
                                                                                  largerTargetRegion);

    typedef std::set<itk::Index<2>, itk::Index<2>::LexicographicCompare> UsedIndexSetType;
    UsedIndexSetType usedIndices;
    while(!sourcePixelMapIterator.IsAtEnd())
    {
      usedIndices.insert(sourcePixelMapIterator.Get());

      ++sourcePixelMapIterator;
    }

    // The queue stores the items in descending score order.
    #pragma omp parallel for
//    for(ForwardIteratorType current = first; current != last; ++current) // OpenMP 3 doesn't allow != in the loop ending condition
    for(ForwardIteratorType currentIterator = first; currentIterator < last; ++currentIterator)
    {
      NodeType currentNode = *currentIterator;

      typename TPatchDescriptorPropertyMap::value_type currentDescriptor = get(this->PatchDescriptorPropertyMap, currentNode);

      if(currentDescriptor.GetStatus() != PixelDescriptor::SOURCE_NODE)
      {
        throw std::runtime_error("LinearSearchKNNPropertyLimitLocalReuse: Node is not a source node!");
      }

      itk::ImageRegion<2> potentialSourceRegion = currentDescriptor.GetRegion();

      potentialSourceRegion.Crop(this->FullRegion);

      if(!this->MaskImage->IsValid(potentialSourceRegion))
      {
        throw std::runtime_error("LinearSearchKNNPropertyLimitLocalReuse: potentialSourceRegion is not fully valid!");
      }

      // Count the number of pixels that were already copied from this patch
      unsigned int usedPixelCounter = 0;

      itk::ImageRegionConstIteratorWithIndex<SourcePixelMapImageType>
          sourceRegionIterator(this->SourcePixelMapImage, potentialSourceRegion);

      // The image that this is iterating over is irrelevant, we just need the indices.
      itk::ImageRegionConstIteratorWithIndex<SourcePixelMapImageType>
          queryRegionIterator(this->SourcePixelMapImage, queryRegion);

      while(!sourceRegionIterator.IsAtEnd())
      {
        UsedIndexSetType::iterator usedIndexSetIterator;
        if(this->MaskImage->IsHole(queryRegionIterator.GetIndex()))
        {
          // We want to use the index value in the SourcePixelMapImage,
          // because the value might not equal the current index in the case where new patches are allowed.
          usedIndexSetIterator = usedIndices.find(sourceRegionIterator.Get());

          if(usedIndexSetIterator != usedIndices.end()) // found
          {
            usedPixelCounter++;
          }
        }

        ++sourceRegionIterator;
        ++queryRegionIterator;
      }

//      unsigned int maxUsedPixels = potentialSourceRegion.GetNumberOfPixels() / 4;
      unsigned int numberOfHolePixels = this->MaskImage->CountHolePixels(queryRegion);
      unsigned int maxAllowedUsedPixels = this->MaxAllowedUsedPixelsRatio * numberOfHolePixels;

      if(usedPixelCounter < maxAllowedUsedPixels)
      {
        DistanceValueType d = this->PatchDistanceFunction(currentDescriptor, queryDescriptor); // (source, target) (the query node is the target node)

        #pragma omp critical // There are weird crashes without this guard
        outputQueue.push(PairType(d, currentIterator));
      }
      else
      {
//        std::cout << "Prevented use because " << usedPixelCounter
//                  << " pixels were already used (out of " << numberOfHolePixels
//                  << " hole pixels)." << std::endl;
      }
    }

//    std::cout << "There are " << outputQueue.size() << " items in the queue." << std::endl;

    // Keep only the best K matches
    if(outputQueue.size() < this->K)
    {
      std::cerr << "Warning: LinearSearchKNNPropertyLimitLocalReuse only has " << outputQueue.size()
                << " nodes. (" << this->K << " were requested.)" << std::endl;
    }
    Helpers::KeepTopN(outputQueue, this->K);

//    std::cout << "There are " << outputQueue.size() << " items in the queue." << std::endl;

    if(outputQueue.size() < this->K)
    {
      std::stringstream ss;
      ss << "LinearSearchKNNPropertyLimitLocalReuse: Requested " << this->K << " items but only found " << outputQueue.size();
      throw std::runtime_error(ss.str());
    }

    std::cout << "Best patch score is: " << outputQueue.top().first << std::endl;

    // Copy the best matches from the queue into the output
    OutputIteratorType currentOutputIterator = outputFirst;
    while( !outputQueue.empty() )
    {
      *currentOutputIterator = *(outputQueue.top().second);
      outputQueue.pop();
      ++currentOutputIterator;
    }

    this->Iteration++;

    return currentOutputIterator;
  } // end operator()

  template <typename T1, typename T2, typename Compare>
  struct compare_pair_first : std::binary_function< std::pair<T1, T2>, std::pair<T1, T2>, bool>
  {
    Compare comp;
    compare_pair_first(const Compare& aComp = Compare()) : comp(aComp) { }
    bool operator()(const std::pair<T1, T2>& x, const std::pair<T1, T2>& y) const
    {
      return comp(x.first, y.first);
    }
  };

  /** This is the case where the output iterators contain nodes.
    * 'result' does not have to be passed by reference because it is an iterator.
    * The return type if this function is enabled is OutputIteratorType.
    */
  template <typename PairPriorityQueueType,
  typename OutputIteratorType>
  inline
  typename boost::enable_if<
  boost::is_same<
  typename std::iterator_traits<typename PairPriorityQueueType::value_type::second_type >::value_type,
  typename std::iterator_traits< OutputIteratorType >::value_type
  >,
  OutputIteratorType >::type copy_neighbors_from_queue(PairPriorityQueueType& Q, OutputIteratorType result)
  {
    OutputIteratorType first = result;
    while( !Q.empty() )
    {
      *result = *(Q.top().second);
      Q.pop();
      ++result;
    };
    std::reverse(first, result);
    return result;
  }

};

#endif
