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

typedef itk::VectorImage<float, 2> ImageType;

struct DemoDriver
{

  typedef boost::grid_graph<2> VertexListGraphType;
  typedef boost::graph_traits<VertexListGraphType>::vertex_descriptor VertexDescriptorType;
  typedef ImagePatchPixelDescriptor<ImageType> ImagePatchPixelDescriptorType;
  typedef boost::property_map<VertexListGraphType, boost::vertex_index_t>::const_type IndexMapType;
  typedef boost::vector_property_map<ImagePatchPixelDescriptorType, IndexMapType> ImagePatchDescriptorMapType;
  typedef ImagePatchDescriptorVisitor<VertexListGraphType, ImageType, ImagePatchDescriptorMapType> VisitorType;
  typedef LinearSearchKNNProperty<ImagePatchDescriptorMapType,
                                  ImagePatchDifference<ImagePatchPixelDescriptorType> > KNNSearchType;
  VertexListGraphType* graph;

  unsigned int PatchRadius;

  ImageType* Image;
  Mask* MaskImage;

  VisitorType* Visitor;

  KNNSearchType* KNNSearch;

  ImagePatchDescriptorMapType* ImagePatchDescriptorMap;
  
  DemoDriver(ImageType* const image, Mask* const mask) : graph(NULL), PatchRadius(15), Image(image), MaskImage(mask)
  {
    boost::array<std::size_t, 2> graphSideLengths = { { image->GetLargestPossibleRegion().GetSize()[0],
                                                        image->GetLargestPossibleRegion().GetSize()[1] } };
    //VertexListGraphType graph(graphSideLengths);
    graph = new VertexListGraphType(graphSideLengths);
    
    // Get the index map
    IndexMapType indexMap(get(boost::vertex_index, *graph));

    // Create the descriptor map. This is where the data for each pixel is stored.
    ImagePatchDescriptorMap = new ImagePatchDescriptorMapType(num_vertices(*graph), indexMap);

    Visitor = new VisitorType(Image, MaskImage, *ImagePatchDescriptorMap, PatchRadius);

    InitializeFromMaskImage<VisitorType, VertexDescriptorType>(MaskImage, Visitor);

    // Create the nearest neighbor finders
    KNNSearch = new KNNSearchType(*ImagePatchDescriptorMap, 1000, 2);
  }

  void DisplayTopPatches(VertexDescriptorType targetNode)
  {
    Visitor->DiscoverVertex(targetNode);

    std::vector<VertexDescriptorType> bestSourceNodes;
    typename boost::graph_traits<VertexListGraphType>::vertex_iterator vi,vi_end;
    tie(vi,vi_end) = vertices(*graph);
    (*KNNSearch)(vi, vi_end, targetNode, bestSourceNodes);
    std::cout << "There are " << bestSourceNodes.size() << " bestSourceNodes." << std::endl;
    
    ImagePatchDifference<ImagePatchPixelDescriptorType> patchDifferenceFunctor;
    float patchDifference = patchDifferenceFunctor(get(*ImagePatchDescriptorMap, targetNode), get(*ImagePatchDescriptorMap, bestSourceNodes[0]));
    std::cout << "Best patch error: " << patchDifference << std::endl;

    // Compute the average of the valid pixels in the target region
    std::vector<itk::Index<2> > validPixelsTargetRegion = MaskImage->GetValidPixelsInRegion(get(*ImagePatchDescriptorMap, targetNode).GetRegion());
    typename ImageType::PixelType targetRegionSourcePixelVariance = ITKHelpers::VarianceOfPixelsAtIndices(Image, validPixelsTargetRegion);
    std::cout << "targetRegionSourcePixelVariance: " << targetRegionSourcePixelVariance << std::endl;
    
    // Compute the average of the pixels in the source region corresponding to hole pixels in the target region.
    std::vector<itk::Offset<2> > holeOffsets = MaskImage->GetHoleOffsetsInRegion(get(*ImagePatchDescriptorMap, targetNode).GetRegion());
    std::vector<itk::Index<2> > sourcePatchValidPixels = ITKHelpers::OffsetsToIndices(holeOffsets, get(*ImagePatchDescriptorMap, bestSourceNodes[0]).GetRegion().GetIndex());
    typename ImageType::PixelType sourceRegionTargetPixelVariance = ITKHelpers::VarianceOfPixelsAtIndices(Image, sourcePatchValidPixels);
    std::cout << "sourceRegionTargetPixelVariance: " << sourceRegionTargetPixelVariance << std::endl;

    TopPatchesDialog<ImageType>* topPatchesDialog = new TopPatchesDialog<ImageType>(Image, MaskImage, PatchRadius);
    topPatchesDialog->SetQueryNode(targetNode);
    topPatchesDialog->SetSourceNodes(bestSourceNodes);
    //topPatchesDialog->exec();
    topPatchesDialog->show();

    // Return the node to an invalid state
    ImagePatchPixelDescriptorType& descriptor = get(*ImagePatchDescriptorMap, targetNode);
    descriptor.SetStatus(PixelDescriptor::INVALID);
  }

};

int main(int argc, char *argv[])
{
  if(argc != 3)
    {
    std::cerr << "Required arguments: image mask" << std::endl;
    return EXIT_FAILURE;
    }

  std::string imageFileName = argv[1];
  std::string maskFileName = argv[2];

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

  // Setup the GUI system
  QApplication app( argc, argv );

  DemoDriver demoDriver(imageReader->GetOutput(), maskReader->GetOutput());

  std::cout << "Good patch:" << std::endl;
  itk::Index<2> centerGood = {{503,156}};
  DemoDriver::VertexDescriptorType goodTargetNode = Helpers::ConvertFrom<DemoDriver::VertexDescriptorType,
                                                                         itk::Index<2> >(centerGood);
  demoDriver.DisplayTopPatches(goodTargetNode);

  std::cout << "Bad patch:" << std::endl;
   itk::Index<2> centerBad = {{503,146}}; // The top 1000 matches to this patch are completely wrong
  // itk::Index<2> centerBad = {{503,147}}; // This also has 0/1000 good matches
  //itk::Index<2> centerBad = {{503,155}}; // This works fine
  DemoDriver::VertexDescriptorType badTargetNode = Helpers::ConvertFrom<DemoDriver::VertexDescriptorType,
                                                                        itk::Index<2> >(centerBad);
  demoDriver.DisplayTopPatches(badTargetNode);

  app.exec();

  return EXIT_SUCCESS;
}
