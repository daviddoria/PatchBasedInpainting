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
  void signal_RefreshImage();
  void signal_RefreshSource(itk::ImageRegion<2>);
  void signal_RefreshTarget(itk::ImageRegion<2>);

};

template <typename TGraph, typename TImage>
class DisplayVisitor : public InpaintingVisitorParent<TGraph>, public SignalParent
{

private:
  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;
  unsigned int NumberOfFinishedVertices;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

public:
  DisplayVisitor(TImage* const image, Mask* const mask, const unsigned int halfWidth) :
  Image(image), MaskImage(mask), HalfWidth(halfWidth), NumberOfFinishedVertices(0)
  {

  }

  void initialize_vertex(VertexDescriptorType v, TGraph& g) const
  { 

  };

  void discover_vertex(VertexDescriptorType v, TGraph& g) const 
  { 

  };

  void vertex_match_made(VertexDescriptorType target, VertexDescriptorType source, TGraph& g)
  {
    //Node targetNode;
    itk::Index<2> targetIndex = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetIndex, this->HalfWidth);
    emit signal_RefreshTarget(targetRegion);

    itk::Index<2> sourceIndex = ITKHelpers::CreateIndex(source);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourceIndex, this->HalfWidth);
    emit signal_RefreshSource(sourceRegion);
  };

  void paint_vertex(VertexDescriptorType target, VertexDescriptorType source, TGraph& g) const
  {
    // Do nothing
  };

  bool accept_painted_vertex(VertexDescriptorType v, TGraph& g) const
  {
    return true;
  };

  void finish_vertex(VertexDescriptorType v, VertexDescriptorType sourceNode, TGraph& g)
  {
    emit signal_RefreshImage();
  };

};

#endif
