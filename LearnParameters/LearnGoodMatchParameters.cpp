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

// Image processing
#include "ImageProcessing/MaskOperations.h"

// Acceptance visitors
#include "Visitors/AcceptanceVisitors/DefaultAcceptanceVisitor.hpp"
#include "Visitors/AcceptanceVisitors/VarianceDifferenceAcceptanceVisitor.hpp"
#include "Visitors/AcceptanceVisitors/AverageDifferenceAcceptanceVisitor.hpp"
#include "Visitors/AcceptanceVisitors/CompositeAcceptanceVisitor.hpp"
#include "Visitors/AcceptanceVisitors/FullPatchAverageDifference.hpp"
#include "Visitors/AcceptanceVisitors/FullPatchVarianceDifference.hpp"

// Pixel descriptors
#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

// Descriptor visitors
#include "Visitors/DescriptorVisitors/ImagePatchDescriptorVisitor.hpp"

// Nearest neighbors
#include "NearestNeighbor/LinearSearchBestProperty.hpp"

// Initializers
#include "Initializers/InitializeFromMaskImage.hpp"

// Difference functions
#include "DifferenceFunctions/ImagePatchDifference.hpp"
#include "DifferenceFunctions/SumAbsolutePixelDifference.hpp"

// ITK
#include "itkImageFileReader.h"

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>

// Run with: trashcan.mha trashcan_uniformRegion.mha 15 100
int main(int argc, char *argv[])
{
  // The "uniformRegionMask" should be "valid" only in a region that should match really well to itself.

  // Verify arguments
  if(argc != 5)
    {
    std::cerr << "Required arguments: image.mha uniformRegionMask.mha patchHalfWidth numberOfIterations" << std::endl;
    std::cerr << "Input arguments: ";
    for(int i = 1; i < argc; ++i)
      {
      std::cerr << argv[i] << " ";
      }
    return EXIT_FAILURE;
    }

  // Parse arguments
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];

  std::stringstream ssPatchRadius;
  ssPatchRadius << argv[3];
  unsigned int patchHalfWidth = 0;
  ssPatchRadius >> patchHalfWidth;

  std::stringstream ssIterations;
  ssIterations << argv[4];
  unsigned int numberOfIterations = 1000;
  ssIterations >> numberOfIterations;
  
  // Output arguments
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;
  std::cout << "Patch half width: " << patchHalfWidth << std::endl;
  std::cout << "numberOfIterations: " << numberOfIterations << std::endl;

  typedef FloatVectorImageType ImageType;

  typedef  itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

  ImageType::Pointer image = ImageType::New();
  ITKHelpers::DeepCopy(imageReader->GetOutput(), image.GetPointer());

  Mask::Pointer mask = Mask::New();
  mask->Read(maskFilename);

  std::cout << "Number of uniform region pixels: " << mask->CountHolePixels() << std::endl;

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

  // Create the descriptor visitor
  typedef ImagePatchDescriptorVisitor<VertexListGraphType, ImageType, ImagePatchDescriptorMapType>
      ImagePatchDescriptorVisitorType;
  ImagePatchDescriptorVisitorType imagePatchDescriptorVisitor(image, mask, imagePatchDescriptorMap, patchHalfWidth);

  // Create the acceptance visitor
//   typedef DefaultAcceptanceVisitor<VertexListGraphType> AcceptanceVisitorType;
//   AcceptanceVisitorType defaultAcceptanceVisitor;

  FullPatchAverageDifference<VertexListGraphType, ImageType> fullPatchAverageDifference(image, patchHalfWidth);
  FullPatchVarianceDifference<VertexListGraphType, ImageType> fullPatchVarianceDifference(image, patchHalfWidth);
  
  //AverageDifferenceAcceptanceVisitor<VertexListGraphType, ImageType> averageDifferenceAcceptanceVisitor(image, mask, patchHalfWidth, 100);
  //VarianceDifferenceAcceptanceVisitor<VertexListGraphType, ImageType> varianceDifferenceAcceptanceVisitor(image, mask, patchHalfWidth, 100);

  // Can't use a composite visitor because the energies will be added together so we can't study them.
//   CompositeAcceptanceVisitor<VertexListGraphType> compositeAcceptanceVisitor;
//   compositeAcceptanceVisitor.AddVisitor(averageDifferenceAcceptanceVisitor);
//   compositeAcceptanceVisitor.AddVisitor(varianceDifferenceAcceptanceVisitor);

  // Initialize the boundary node queue from the user provided mask image.
  InitializeFromMaskImage<ImagePatchDescriptorVisitorType, VertexDescriptorType>(mask.GetPointer(), &imagePatchDescriptorVisitor);

  typedef ImagePatchDifference<ImagePatchPixelDescriptorType, SumAbsolutePixelDifference<ImageType::PixelType> > ImagePatchDifferenceType;
  ImagePatchDifferenceType pixelDifferenceFunctor;
  
  // Create the nearest neighbor finder
  typedef LinearSearchBestProperty<ImagePatchDescriptorMapType,
                                   ImagePatchDifferenceType > BestSearchType;
  BestSearchType searchBest(imagePatchDescriptorMap);

  std::cout << "Starting random matching..." << std::endl;
  
  float totalAverageDifferences = 0.0f;
  float totalVarianceDifferences = 0.0f;
  float totalCorrespondingDifferences = 0.0f;

  std::ofstream averageDifferencesStream("AverageDifferences.txt");
  std::ofstream varianceDifferencesStream("VarianceDifferences.txt");
  std::ofstream correspondingDifferencesStream("CorrespondingDifferences.txt");

  for(unsigned int iteration = 0; iteration < numberOfIterations; ++iteration)
  {
    itk::ImageRegion<2> targetRegion = MaskOperations::RandomValidRegion(mask, patchHalfWidth);
    itk::Index<2> targetPixel = ITKHelpers::GetRegionCenter(targetRegion);
    std::cout << "Target pixel: " << targetPixel << std::endl;
    VertexDescriptorType targetNode = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(targetPixel);

    imagePatchDescriptorVisitor.DiscoverVertex(targetNode);

    typename boost::graph_traits<VertexListGraphType>::vertex_iterator vi,vi_end;
    tie(vi,vi_end) = vertices(graph);
    VertexDescriptorType sourceNode = searchBest(vi, vi_end, targetNode);

    float varianceDifference = 0.0f;
    fullPatchVarianceDifference.AcceptMatch(targetNode, sourceNode, varianceDifference);
    totalVarianceDifferences += varianceDifference;
    varianceDifferencesStream << varianceDifference << std::endl;
    
    float averageDifference = 0.0f;
    fullPatchAverageDifference.AcceptMatch(targetNode, sourceNode, averageDifference);
    totalAverageDifferences += averageDifference;
    averageDifferencesStream << averageDifference << std::endl;
    
    float correspondingPixelDifference = pixelDifferenceFunctor(get(imagePatchDescriptorMap, targetNode), get(imagePatchDescriptorMap, sourceNode));
    totalCorrespondingDifferences += correspondingPixelDifference;
    std::cout << "correspondingPixelDifference: " << correspondingPixelDifference << std::endl;
    correspondingDifferencesStream << correspondingPixelDifference << std::endl;

    // Reset the node to be a source node
    get(imagePatchDescriptorMap, targetNode).SetStatus(PixelDescriptor::SOURCE_NODE);
  }

  float averageAverageDifference = totalAverageDifferences / static_cast<float>(numberOfIterations);
  std::cout << "averageAverageDifference: " << averageAverageDifference << std::endl;

  float averageVarianceDifference = totalVarianceDifferences / static_cast<float>(numberOfIterations);
  std::cout << "averageVarianceDifference: " << averageVarianceDifference << std::endl;

  float averageCorrespondingDifference = totalCorrespondingDifferences / static_cast<float>(numberOfIterations);
  std::cout << "averageCorrespondingDifference: " << averageCorrespondingDifference << std::endl;

  return EXIT_SUCCESS;
}
