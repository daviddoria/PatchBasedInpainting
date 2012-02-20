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

// Custom
#include "ImageProcessing/Mask.h"
#include "Helpers/ITKHelpers.h"
#include "PixelDescriptors/ImagePatchPixelDescriptor.h"
#include "NearestNeighbor/LinearSearchKNNProperty.hpp"
#include "Initializers/InitializeFromMaskImage.hpp"
#include "DifferenceFunctions/ImagePatchDifference.hpp"
#include "Interactive/TopPatchesDialog.h"
#include "Visitors/DescriptorVisitors/ImagePatchDescriptorVisitor.hpp"

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>

// Qt
#include <QApplication>

// ITK
#include "itkImage.h"
#include "itkImageFileReader.h"

int main(int argc, char *argv[])
{
  if(argc != 3)
    {
    std::cerr << "Required arguments: image mask" << std::endl;
    return EXIT_FAILURE;
    }

  std::string imageFileName = argv[1];
  std::string maskFileName = argv[2];

  typedef itk::VectorImage<float, 2> ImageType;
  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFileName);
  imageReader->Update();

  std::cout << "There are " << imageReader->GetOutput()->GetNumberOfComponentsPerPixel()
            << " components per image pixel." << std::endl;

  typedef itk::ImageFileReader<Mask> MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFileName);
  maskReader->Update();

  itk::Index<2> centerGood = {{503,156}};
  itk::Index<2> centerBad = {{503,146}};

  const unsigned int patchRadius = 15;
  itk::ImageRegion<2> regionGood = ITKHelpers::GetRegionInRadiusAroundPixel(centerGood, patchRadius);
  itk::ImageRegion<2> regionBad = ITKHelpers::GetRegionInRadiusAroundPixel(centerBad, patchRadius);


  typedef ImagePatchPixelDescriptor<ImageType> ImagePatchPixelDescriptorType;

  // Create the graph
  typedef boost::grid_graph<2> VertexListGraphType;
  boost::array<std::size_t, 2> graphSideLengths = { { imageReader->GetOutput()->GetLargestPossibleRegion().GetSize()[0],
                                                      imageReader->GetOutput()->GetLargestPossibleRegion().GetSize()[1] } };
  VertexListGraphType graph(graphSideLengths);
  typedef boost::graph_traits<VertexListGraphType>::vertex_descriptor VertexDescriptorType;

  // Get the index map
  typedef boost::property_map<VertexListGraphType, boost::vertex_index_t>::const_type IndexMapType;
  IndexMapType indexMap(get(boost::vertex_index, graph));

  // Create the descriptor map. This is where the data for each pixel is stored.
  typedef boost::vector_property_map<ImagePatchPixelDescriptorType, IndexMapType> ImagePatchDescriptorMapType;
  ImagePatchDescriptorMapType imagePatchDescriptorMap(num_vertices(graph), indexMap);

  typedef ImagePatchDescriptorVisitor<VertexListGraphType, ImageType, ImagePatchDescriptorMapType> VisitorType;
  VisitorType visitor(imageReader->GetOutput(), maskReader->GetOutput(), imagePatchDescriptorMap, patchRadius);

  InitializeFromMaskImage<VisitorType, VertexDescriptorType>(maskReader->GetOutput(), &visitor);

  // Create the nearest neighbor finders
  typedef LinearSearchKNNProperty<ImagePatchDescriptorMapType,
                                  ImagePatchDifference<ImagePatchPixelDescriptorType> > KNNSearchType;
  KNNSearchType knnSearch(imagePatchDescriptorMap, 100);

  VertexDescriptorType goodTargetNode = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(centerGood);
  //VertexDescriptorType badTargetNode = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(centerBad);

  VertexDescriptorType targetNode = goodTargetNode;

  std::vector<VertexDescriptorType> bestSourceNodes;
  typename boost::graph_traits<VertexListGraphType>::vertex_iterator vi,vi_end;
  tie(vi,vi_end) = vertices(graph);
  knnSearch(vi, vi_end, goodTargetNode, bestSourceNodes);

  std::cout << "There are " << bestSourceNodes.size() << " bestSourceNodes." << std::endl;

  // Setup the GUI system
  QApplication app( argc, argv );

  TopPatchesDialog<ImageType> topPatchesDialog(imageReader->GetOutput(), maskReader->GetOutput(), patchRadius);
  topPatchesDialog.SetQueryNode(targetNode);
  topPatchesDialog.SetSourceNodes(bestSourceNodes);
  topPatchesDialog.exec();

  return EXIT_SUCCESS;
}
