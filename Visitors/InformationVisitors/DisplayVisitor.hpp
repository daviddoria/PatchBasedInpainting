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

#ifndef DisplayVisitor_HPP
#define DisplayVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Custom
#include "Visitors/InpaintingVisitorParent.h"

// Submodules
#include <Mask/Mask.h>
#include <ITKHelpers/ITKHelpers.h>

// ITK
#include "itkImage.h"
#include "itkImageRegion.h"

// VTK
#include <vtkRenderWindow.h>

// Qt
#include <QObject>

/**
  Class templates cannot have signals/slots, but using a parent class like this,
  we can derive the class template from this class to enable them to effectively
  have signals and slots.
 */
class SignalParent : public QObject
{
Q_OBJECT

signals:
  /** This signal is emitted to start the progress bar. */
  void signal_RefreshImage() const;

  /** We need the target region as well while updating the source region because
    * we may want to mask the source patch with the target patch's mask.*/
  void signal_RefreshSource(const itk::ImageRegion<2>& sourceRegion,
                            const itk::ImageRegion<2>& targetRegion);

  /** Indicate that the source region should be updated. */
  void signal_RefreshSource(const itk::ImageRegion<2>& sourceRegion);
  
  /** Indicate that the target region should be updated. */
  void signal_RefreshTarget(const itk::ImageRegion<2>);

  /** Indicate that the resulting patch updated. */
  void signal_RefreshResult(const itk::ImageRegion<2> sourceRegion,
                            const itk::ImageRegion<2> targetRegion);

};

/**
  This visitor emits signals to trigger GUI updates.
 */
template <typename TGraph, typename TImage>
class DisplayVisitor : public InpaintingVisitorParent<TGraph>, public SignalParent
{
private:
  /** The image. */
  const TImage* Image;

  /** The mask. */
  const Mask* MaskImage;

  /** The radius of the patches. */
  const unsigned int HalfWidth;

  /** The number of vertices that have been filled. */
  unsigned int NumberOfFinishedVertices = 0;

  /** The type of the nodes in the graph. */
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;
  
public:
  /** Constructor. */
  DisplayVisitor(const TImage* const image, const Mask* const mask, const unsigned int halfWidth,
                 const std::string& visitorName = "DisplayVisitor") :
  InpaintingVisitorParent<TGraph>(visitorName),
  Image(image), MaskImage(mask), HalfWidth(halfWidth)
  {

  }

  void PaintPatch(VertexDescriptorType target, VertexDescriptorType source) const
  {

  }
  
  void InitializeVertex(VertexDescriptorType v) const
  { 

  }

  /** When a vertex has been selected from the queue to be searched for, display it as the target patch. */
  void DiscoverVertex(VertexDescriptorType v) const
  {
    // std::cout << "DisplayVisitor::DiscoverVertex" << std::endl;
    emit signal_RefreshImage();
  }

  /** When a tentative match has been made (possibly pending verification), treat it as the source patch. */
  void PotentialMatchMade(VertexDescriptorType target, VertexDescriptorType source)
  {
    // std::cout << "DisplayVisitor::PotentialMatchMade" << std::endl;

    // Target node
    itk::Index<2> targetIndex = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetIndex, this->HalfWidth);

    // Source node
    itk::Index<2> sourceIndex = ITKHelpers::CreateIndex(source);
    // std::cout << "DisplayVisitor::PotentialMatchMade source " << sourceIndex << std::endl;
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourceIndex, this->HalfWidth);
    sourceRegion = ITKHelpers::CropRegionAtPosition(sourceRegion, this->MaskImage->GetLargestPossibleRegion(), targetRegion);

    // Crop target node now that we have already used the original position to crop the source node
    targetRegion.Crop(this->MaskImage->GetLargestPossibleRegion());

    emit signal_RefreshSource(sourceRegion, targetRegion);
    emit signal_RefreshSource(sourceRegion);
    emit signal_RefreshTarget(targetRegion);
  }

  /** This visitor should not be involved with the logic of filling patches. */
  void PaintVertex(VertexDescriptorType target, VertexDescriptorType source) const
  {
    // Do nothing
  }

  /** This visitor should not be involved with the logic of accepting patches. */
  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    return true;
  }

  /** Once a patch has been selected and approved, show the filled image. */
  void FinishVertex(VertexDescriptorType v, VertexDescriptorType sourceNode)
  {
    itk::Index<2> targetIndex = ITKHelpers::CreateIndex(v);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetIndex, this->HalfWidth);

    itk::Index<2> sourceIndex = ITKHelpers::CreateIndex(sourceNode);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourceIndex, this->HalfWidth);

    emit signal_RefreshResult(sourceRegion, targetRegion);
    emit signal_RefreshImage();
  }

  void InpaintingComplete() const
  {
  }

};

#endif
