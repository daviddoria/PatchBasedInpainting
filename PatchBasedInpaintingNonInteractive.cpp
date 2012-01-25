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
#include "Helpers/HelpersOutput.h"
#include "DefaultInpaintingVisitor.hpp"
#include "NearestNeighbor/topological_search.hpp"
#include "PatchInpainter.hpp"
#include "InpaintingGridNoInit.hpp"

// ITK
#include "itkImageFileReader.h"

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/topology.hpp>

namespace boost {

  enum vertex_hole_priority_t { vertex_hole_priority };
  
  BOOST_INSTALL_PROPERTY(vertex, hole_priority);

};

int main(int argc, char *argv[])
{
  // Verify arguments
  if(argc != 5)
    {
    std::cerr << "Required arguments: image.mha imageMask.mha patchRadius output.mha" << std::endl;
    return EXIT_FAILURE;
    }

  // Parse arguments
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];

  std::stringstream ssPatchRadius;
  ssPatchRadius << argv[3];
  int patchRadius = 0;
  ssPatchRadius >> patchRadius;

  std::string outputFilename = argv[4];

  // Output arguments
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;
  std::cout << "Patch radius: " << patchRadius << std::endl;
  std::cout << "Output: " << outputFilename << std::endl;

  typedef  itk::ImageFileReader<FloatVectorImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename.c_str());
  imageReader->Update();

  typedef  itk::ImageFileReader<Mask> MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFilename);
  maskReader->Update();
  
  // Create the graph
  typedef boost::grid_graph<2> VertexListGraphType;
  boost::array<std::size_t, 2> graphSideLengths = { { imageReader->GetOutput()->GetLargestPossibleRegion().GetSize()[0], imageReader->GetOutput()->GetLargestPossibleRegion().GetSize()[1] } };
  VertexListGraphType graph(graphSideLengths);

  // Create the visitor
  //typedef default_inpainting_visitor<VertexListGraphType, boost::graph_traits<InpaintingVisitorType>::vertex_descriptor> InpaintingVisitorType;
  typedef default_inpainting_visitor InpaintingVisitorType;
  InpaintingVisitorType visitor;
  
  // Create the topology
  typedef boost::hypercube_topology<0> TopologyType;
  TopologyType space;
  
  // Create the position map
  typedef boost::property_map<VertexListGraphType, boost::vertex_index_t>::const_type GridIndexMapType;
  GridIndexMapType gridIndexMap(get(boost::vertex_index, graph));

  // Create the color map
  std::vector<boost::default_color_type> vertexColorData(num_vertices(graph), boost::white_color);
  //typedef boost::iterator_property_map<std::vector<boost::default_color_type>::iterator, GridIndexMapType> ColorMapType;
  //ColorMapType colorMap(vertexColorData.begin(), gridIndexMap);
  //ColorMapType colorMap(vertexColorData, gridIndexMap);
  typedef boost::vector_property_map<boost::default_color_type, GridIndexMapType> ColorMapType;
  ColorMapType colorMap(num_vertices(graph), gridIndexMap);
  
  // Create the priority map
  //std::vector<float> vertexPriorityData(num_vertices(graph), 0.0f);
  //typedef boost::iterator_property_map<std::vector<float>::iterator, GridIndexMapType> PriorityMapType;
  //PriorityMapType priorityMap(vertexPriorityData.begin(), gridIndexMap);
  typedef boost::vector_property_map<float, GridIndexMapType> PriorityMapType;
  PriorityMapType priorityMap(num_vertices(graph), gridIndexMap);

  // Create the priority compare functor
  typedef std::less<float> PriorityCompareType;
  PriorityCompareType lessThanFunctor;

  // Create the nearest neighbor finder
  typedef linear_neighbor_search<> SearchType;
  SearchType linearSearch;

  // Create the patch inpainter
  PatchInpainter patchInpainter;

  inpainting_grid_no_init<VertexListGraphType, InpaintingVisitorType, 
                          TopologyType, GridIndexMapType, 
                          ColorMapType, PriorityMapType,
                          PriorityCompareType, SearchType, PatchInpainter>
                    (graph, visitor, space, gridIndexMap,
                     colorMap, priorityMap, 
                     lessThanFunctor, linearSearch, patchInpainter);

//   HelpersOutput::WriteImage<FloatVectorImageType>(inpainting->GetCurrentOutputImage(), outputFilename + ".mha");
//   HelpersOutput::WriteVectorImageAsRGB(inpainting->GetCurrentOutputImage(), outputFilename);

  return EXIT_SUCCESS;
}
