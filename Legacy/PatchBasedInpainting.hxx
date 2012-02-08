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

#include "PatchBasedInpainting.h" // Appease syntax parser

// Custom
#include "ImageProcessing/Derivatives.h"
#include "Helpers/Helpers.h"
#include "Helpers/HelpersOutput.h"
#include "ItemDifferenceVisitor.h"
#include "ImageProcessing/MaskOperations.h"
#include "PatchDifferencePixelWiseSum.h"
#include "Priority/Priority.h"
#include "Priority/PrioritySearchHighest.h"
#include "Types.h"
#include "ValidPixelIterator.h"
#include "ValidRegionIterator.h"

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/HelpersOutput.h"
#include "Helpers/ITKHelpers.h"

// STL
#include <iostream>

template <typename TImage>
PatchBasedInpainting<TImage>::PatchBasedInpainting(const TImage* const image, const Mask* const mask) : Graph(NULL), DVPTree(NULL), PriorityFunction(NULL)
{
  this->PatchRadius.Fill(3);

  // We don't want to modify the input images, so we copy them.
  this->MaskImage = Mask::New();
  this->MaskImage->DeepCopyFrom(mask);

  this->CurrentInpaintedImage = TImage::New();
  ITKHelpers::DeepCopy<TImage>(image, this->CurrentInpaintedImage);

  ColorImageInsideHole();

  this->FullImageRegion = image->GetLargestPossibleRegion();
  if(this->MaskImage->GetLargestPossibleRegion() != this->FullImageRegion)
    {
    std::stringstream ss;
    ss << "Mask and image size must match! Mask is " << this->MaskImage->GetLargestPossibleRegion().GetSize()
              << " while image is " << this->FullImageRegion << std::endl;
    throw std::runtime_error(ss.str());
    }

  // Set defaults
  this->NumberOfCompletedIterations = 0;
}

template <typename TImage>
TImage* PatchBasedInpainting<TImage>::GetCurrentOutputImage()
{
  return this->CurrentInpaintedImage;
}

template <typename TImage>
void PatchBasedInpainting<TImage>::SetPatchRadius(const unsigned int radius)
{
  // Since this is the radius of the patch, there are no restrictions for the radius to be odd or even.
  this->PatchRadius.Fill(radius);
}

template <typename TImage>
unsigned int PatchBasedInpainting<TImage>::GetPatchRadius() const
{
  return this->PatchRadius[0];
}

template <typename TImage>
const itk::ImageRegion<2>& PatchBasedInpainting<TImage>::GetFullRegion() const
{
  return this->FullImageRegion;
}

template <typename TImage>
void PatchBasedInpainting<TImage>::SetPriorityFunction(Priority* const priority)
{
  this->PriorityFunction = priority;
}

template <typename TImage>
void PatchBasedInpainting<TImage>::ColorImageInsideHole()
{
  // Color the target image bright green inside the hole. This is helpful when watching the inpainting proceed, as you can clearly see
  // the region that is being filled.

  typename TImage::PixelType fillColor;
  fillColor.SetSize(this->CurrentInpaintedImage->GetNumberOfComponentsPerPixel());
  fillColor.Fill(0);
  fillColor[0] = 255;
  // We could use MaskImage->ApplyColorToImage here to use a predefined QColor, but this would introduce a dependency on Qt in the non-GUI part of the code.
  this->MaskImage->template ApplyToImage<TImage>(this->CurrentInpaintedImage, fillColor);
}

template <typename TImage>
void PatchBasedInpainting<TImage>::Initialize()
{
  // If the user hasn't specified a priority function, use the simplest one.
  if(!this->PriorityFunction)
    {
    throw std::runtime_error("You must specify a Priority function to use!");
    }

  // If the user hasn't specified a priority function, use the simplest one.
//   if(!this->ItemCreatorObject)
//     {
//     throw std::runtime_error("You must specify an ItemCreator to use!");
//     }

  // Create the grid_graph from the image. 
  boost::array<std::size_t, 2> lengths = { { this->GetFullRegion().GetIndex()[0], this->GetFullRegion().GetIndex()[0] } };
  this->Graph = new GraphType(lengths);

  InitializeBoundaryNodes();

  InitializeDVPTree();

  AddNewObjectsInRegion(this->MaskImage->GetLargestPossibleRegion());

  this->NumberOfCompletedIterations = 0;
}

template <typename TImage>
void PatchBasedInpainting<TImage>::InitializeBoundaryNodes()
{
  // Find and add the boundary nodes.
  UnsignedCharScalarImageType::Pointer boundaryImage = UnsignedCharScalarImageType::New();
  this->MaskImage->FindBoundary(boundaryImage);
  std::vector<itk::Index<2> > boundaryPixels = ITKHelpers::GetNonZeroPixels(boundaryImage.GetPointer());

  // Add the vertex_descriptor for every boundary pixel to the set of boundary nodes
  for(unsigned int pixelId = 0; pixelId < boundaryPixels.size(); ++pixelId)
    {
    VertexDescriptor boundaryDescriptor = { { boundaryPixels[pixelId][0], boundaryPixels[pixelId][0] } };

    //this->BoundaryNodes.insert(boundaryDescriptor);
    }
}

template <typename TImage>
void PatchBasedInpainting<TImage>::InitializeDVPTree()
{
  itk::ImageRegionConstIteratorWithIndex<ItemImageType> iterator(this->ItemImage, this->ItemImage->GetLargestPossibleRegion());

  while(!iterator.IsAtEnd())
    {
//     itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(iterator.GetIndex(), this->PatchRadius);
//     if(this->MaskImage->IsValid(region))
//       {
//       TopologyPointType p;
//       // TODO: How to create an object here?
//       // boost::put(positionMap, v, p);
//       }
    ++iterator;
    }

  //this->DVPTree = new DVPTreeType(this->Graph, this->GraphTopology, positionMap);
}

template <typename TImage>
void PatchBasedInpainting<TImage>::AddNewObjectsInRegion(const itk::ImageRegion<2>& region)
{
  ValidRegionIterator validRegionIterator(this->MaskImage, region, this->PatchRadius[0]);

  for(ValidRegionIterator::ConstIterator iterator = validRegionIterator.begin(); iterator != validRegionIterator.end(); ++iterator)
    {
//     itk::Index<2> centerPixel = ITKHelpers::GetRegionCenter(*iterator);
//     Item* newItem = ItemCreatorObject->CreateItem(centerPixel);
//     this->ItemImage->SetPixel(centerPixel, newItem);
//     ++iterator;
    }
   
  // TODO: also add these new objects to the DVP tree (I know how to do this).
}

template <typename TImage>
typename PatchBasedInpainting<TImage>::VertexDescriptor PatchBasedInpainting<TImage>::FindBestMatch(const VertexDescriptor& queryNode)
{
  
  multi_dvp_tree_search<GraphType, DVPTreeType> nearestNeighborFinder;
  nearestNeighborFinder.graph_tree_map[this->Graph] = this->DVPTree;

  TopologyPointType queryPoint;
  
  // TODO: How to get the object from the VertexDescriptor?

  //VertexType nearestNeighbor = nearestNeighborFinder(queryPoint, g, this->Topology, positionMap);
}

template <typename TImage>
void PatchBasedInpainting<TImage>::SetDifferenceVisitor(ItemDifferenceVisitor* const differenceVisitor)
{
  this->DifferenceVisitor = std::shared_ptr<ItemDifferenceVisitor>(differenceVisitor);
}

template <typename TImage>
SourceTargetPair PatchBasedInpainting<TImage>::Iterate()
{
  // This gets the node at the "front" of the "queue" (it is actually a set, but the elements are still sorted).
  //VertexDescriptor targetNode = *(this->BoundaryNodes.begin());
  VertexDescriptor targetNode ;

  VertexDescriptor sourceNode = FindBestMatch(targetNode);

  // Copy the patch. This is the actual inpainting step.
  itk::Index<2> targetPixel;
  targetPixel[0] = targetNode[0];
  targetPixel[1] = targetNode[1];
  itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, this->GetPatchRadius());
  
  itk::Index<2> sourcePixel;
  sourcePixel[0] = sourceNode[0];
  sourcePixel[1] = sourceNode[1];
  itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourcePixel, this->GetPatchRadius());
  
//   ITKHelpers::CopySelfRegion(this->CurrentInpaintedImage, sourceRegion, targetRegion);
// 
//   // Update the mask
//   ITKHelpers::CopySelfRegion(this->MaskImage, sourceRegion, targetRegion);

  // Update the priority function
  this->PriorityFunction->Update(targetRegion);

  this->NumberOfCompletedIterations++;

  AddNewObjectsInRegion(targetRegion);

  AddBoundaryNodes(targetRegion);

  // TODO: remove the node that was used and all nodes that are no longer on the boundary from this->BoundaryNodes ( I know how to do this).

  SourceTargetPair sourceTargetPair(targetRegion, sourceRegion);
  return sourceTargetPair;
}

template <typename TImage>
void PatchBasedInpainting<TImage>::AddBoundaryNodes(const itk::ImageRegion<2>& region)
{
  // TODO: traverse the boundary of the region (I know how to do this)
  // for pixels that have a masked neighbor, add a node to this->BoundaryNodes
}

template <typename TImage>
void PatchBasedInpainting<TImage>::Inpaint()
{
  // This function is intended to be used by the command line version.
  // It will do the complete inpainting without updating any UI or the ability to stop before it is complete.

  // Start the procedure
  Initialize();

  this->NumberOfCompletedIterations = 0;
  while(HasMoreToInpaint())
    {
    Iterate();
    }
}

template <typename TImage>
bool PatchBasedInpainting<TImage>::HasMoreToInpaint()
{
  //return this->BoundaryPixels.empty();
}

template <typename TImage>
unsigned int PatchBasedInpainting<TImage>::GetNumberOfCompletedIterations()
{
  return this->NumberOfCompletedIterations;
}
