/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
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

#ifndef PatchBasedInpainting_h
#define PatchBasedInpainting_h

// Custom
#include "CandidatePairs.h"
#include "ItemCreator.h"
#include "ItemDifferenceMap.h"
#include "ItemDifferenceVisitor.h"
#include "ImageProcessing/Mask.h"
#include "NearestNeighbor/metric_space_search.hpp"
#include "ImagePatchPixelDescriptor.h"
#include "PixelCollection.h"
#include "PatchPair.h"
#include "Priority/Priority.h"
#include "SourcePatchCollection.h"
#include "SourceTargetPair.h"
#include "SortByPriority.h"
#include "Topologies.h"
#include "Types.h"

// Boost
#include <boost/graph/grid_graph.hpp>

// ITK
#include "itkCovariantVector.h"
#include "itkImage.h"

// Create tags for the properties in the graphs:
namespace boost {

enum vertex_feature_vector_t { vertex_feature_vector };
enum vertex_image_patch_t { vertex_image_patch };
//enum vertex_priority_t { vertex_priority }; // Priority is already defined in graph/properties.hpp

BOOST_INSTALL_PROPERTY(vertex,feature_vector);
BOOST_INSTALL_PROPERTY(vertex,image_patch);
//BOOST_INSTALL_PROPERTY(vertex,priority);

};

/**
\class PatchBasedInpainting
\brief This class performs greedy patch based inpainting on an image.
*/
template <typename TImage>
class PatchBasedInpainting
{
public:
  /** Construct an inpainting object from an image and a mask.*/
  PatchBasedInpainting(const TImage* const image, const Mask* const mask);

  /** Specify the size of the patches to copy.*/
  void SetPatchRadius(const unsigned int radius);

  /** Get the radius of the patch to use for inpainting.*/
  unsigned int GetPatchRadius() const;

  /** Get the result/output of the inpainting so far. When the algorithm is complete, this will be the final output.*/
  TImage* GetCurrentOutputImage();

  /** A single step of the algorithm. The real work is done here.*/
  SourceTargetPair Iterate();

  /** A loop that calls Iterate() until the inpainting is complete.*/
  void Inpaint();

  /** Initialize everything.*/
  void Initialize();
  void InitializeBoundaryNodes();
  void InitializeDVPTree();

  /** Determine whether or not the inpainting is completed by seeing if there are any pixels in the mask that still need to be filled.*/
  bool HasMoreToInpaint();

  /** Return the number of completed iterations*/
  unsigned int GetNumberOfCompletedIterations();

  /** When an image is loaded, it's size is taken as the size that everything else should be. We don't want to keep referring to Image->GetLargestPossibleRegion,
      so we store the region in a member variable. For the same reason, if we want to know the size of the images that this class is operating on, the user should
      not have to query a specific image, but rather access this more global region definition.*/
  const itk::ImageRegion<2>& GetFullRegion() const;

  /** Set the priority function.*/
  void SetPriorityFunction(Priority* const priority);

  void SetDifferenceVisitor(ItemDifferenceVisitor* const differenceVisitor);

private:

//   typedef boost::adjacency_list<boost::vecS, //OutEdgeList
//                               boost::vecS,// VertexList
//                               boost::undirectedS, //Directed,
//                               boost::property< boost::vertex_feature_vector_t, std::vector<float>,
//                                     boost::property< boost::vertex_priority_t, float ,
//                                     boost::property< boost::vertex_image_patch_t, ImagePatchPixelDescriptor<TImage> > > > //VertexProperties
//                               > GraphType;

  typedef boost::grid_graph<2> GraphType;

  // GraphType Graph; // grid_graph has the default constructor private
  GraphType* Graph;

  typedef typename boost::graph_traits<GraphType>::vertex_descriptor VertexDescriptor;

  typedef boost::property_map<GraphType, boost::vertex_index_t>::const_type VertexIndexMapType;
  typedef dvp_tree<VertexDescriptor, FeatureVectorTopology, VertexIndexMapType> DVPTreeType;

  // DVPTreeType DVPTree; // no default constructor
  DVPTreeType* DVPTree;

  typedef FeatureVectorTopology::point_type TopologyPointType;

  FeatureVectorTopology GraphTopology;

  /** Find the best source patch.*/
  VertexDescriptor FindBestMatch(const VertexDescriptor& queryNode);

  /** Create objects that are valid and not yet created.*/
  void AddNewObjectsInRegion(const itk::ImageRegion<2>& region);
  
  /** Create boundary nodes for points on the boundary of the 'region' that are touching the hole.*/
  void AddBoundaryNodes(const itk::ImageRegion<2>& region);

  /** Change the color of the input image.*/
  void ColorImageInsideHole();

  /** The intermediate steps and eventually the result of the inpainting.*/
  typename TImage::Pointer CurrentInpaintedImage;

  /** The mask specifying the region to inpaint. It is updated as patches are copied.*/
  Mask::Pointer MaskImage;

  /** An exterior property to store the priority at each node. */
  std::vector<float> BoundaryNodePriority;

  /** The pixels on the current boundary.*/
  typedef std::pair<VertexDescriptor, float> PrioritizedNode;
  typedef std::set<PrioritizedNode, SortByPriority<PrioritizedNode> > BoundaryNodeSetType;
  BoundaryNodeSetType BoundaryNodes;

  /** The patch radius.*/
  itk::Size<2> PatchRadius;

  /** This tracks the number of iterations that have been completed.*/
  unsigned int NumberOfCompletedIterations;

  /** This is set when the image is loaded so that the region of all of the images can be addressed without referencing any specific image.*/
  itk::ImageRegion<2> FullImageRegion;

  /** The Priority function to use.*/
  std::shared_ptr<Priority> PriorityFunction;

  typedef itk::Image<std::shared_ptr<Item>, 2> ItemImageType;
  ItemImageType::Pointer ItemImage;

  ItemDifferenceMapType ItemDifferenceMap;

  std::shared_ptr<ItemDifferenceVisitor> DifferenceVisitor;
};

#include "PatchBasedInpainting.hxx"

#endif
