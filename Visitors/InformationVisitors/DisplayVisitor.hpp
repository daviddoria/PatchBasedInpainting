#ifndef DisplayVisitor_HPP
#define DisplayVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Custom
#include "Visitors/InpaintingVisitorParent.h"
#include "ImageProcessing/Mask.h"
#include "Helpers/OutputHelpers.h"
#include "Helpers/ITKHelpers.h"
#include "Node.h"

// ITK
#include "itkImage.h"
#include "itkImageRegion.h"

// VTK
#include <vtkRenderWindow.h>

// Qt
#include <QObject>
/**

 */

class SignalParent : public QObject
{
Q_OBJECT

signals:
  // This signal is emitted to start the progress bar
  void signal_RefreshImage() const;

  // We need the target region as well while updating the source region because we may want to mask the source patch with the target patch's mask.
  void signal_RefreshSource(const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion);
  
  void signal_RefreshTarget(const itk::ImageRegion<2>);
  void signal_RefreshResult(const itk::ImageRegion<2> sourceRegion, const itk::ImageRegion<2> targetRegion);

};

template <typename TGraph, typename TImage>
class DisplayVisitor : public InpaintingVisitorParent<TGraph>, public SignalParent
{

private:
  const TImage* Image;
  const Mask* MaskImage;

  const unsigned int HalfWidth;
  unsigned int NumberOfFinishedVertices;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;
  
public:
  DisplayVisitor(const TImage* const image, const Mask* const mask, const unsigned int halfWidth,
                 const std::string& visitorName = "DisplayVisitor") :
  InpaintingVisitorParent<TGraph>(visitorName),
  Image(image), MaskImage(mask), HalfWidth(halfWidth), NumberOfFinishedVertices(0)
  {

  }

  void InitializeVertex(VertexDescriptorType v) const
  { 

  };

  void DiscoverVertex(VertexDescriptorType v) const
  {
    std::cout << "DisplayVisitor::DiscoverVertex" << std::endl;
    emit signal_RefreshImage();
  };

  void PotentialMatchMade(VertexDescriptorType target, VertexDescriptorType source)
  {
    std::cout << "DisplayVisitor::PotentialMatchMade" << std::endl;
    // Target node
    itk::Index<2> targetIndex = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetIndex, this->HalfWidth);
    emit signal_RefreshTarget(targetRegion);

    // Source node
    itk::Index<2> sourceIndex = ITKHelpers::CreateIndex(source);
    // std::cout << "DisplayVisitor::PotentialMatchMade source " << sourceIndex << std::endl;
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourceIndex, this->HalfWidth);
    emit signal_RefreshSource(sourceRegion, targetRegion);
  };

  void PaintVertex(VertexDescriptorType target, VertexDescriptorType source) const
  {
    // Do nothing
  };

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    return true;
  };

  void FinishVertex(VertexDescriptorType v, VertexDescriptorType sourceNode)
  {
    itk::Index<2> targetIndex = ITKHelpers::CreateIndex(v);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetIndex, this->HalfWidth);

    itk::Index<2> sourceIndex = ITKHelpers::CreateIndex(sourceNode);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourceIndex, this->HalfWidth);

    emit signal_RefreshResult(sourceRegion, targetRegion);
    emit signal_RefreshImage();
  };

  void InpaintingComplete() const
  {
  }

};

#endif
