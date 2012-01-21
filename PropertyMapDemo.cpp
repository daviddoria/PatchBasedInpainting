#include <iostream>
#include <vector>

#include <boost/graph/adjacency_list.hpp>

struct FeatureVectorPixelDescriptor
{
  std::vector<float> FeatureVector;
};

class Image {};

struct ImagePatchPixelDescriptor
{
  Image image;
};

struct CompositeDescriptor
{
  ImagePatchPixelDescriptor imagePatchPixelDescriptor;
  FeatureVectorPixelDescriptor featureVectorPixelDescriptor;
};

template <typename TGraph>
struct ComputeFeatureVector
{
  void Compute(TGraph g, typename TGraph::vertex_descriptor vertexToSet)
  {
    std::vector<float> featureVector;
    g[vertexToSet].FeatureVector = featureVector;
  }
};

template <typename TGraph>
struct ComputeImagePatch
{
  void Compute(TGraph g, typename TGraph::vertex_descriptor vertexToSet)
  {
    Image image;
    g[vertexToSet].image = image;
  }
};

typedef boost::adjacency_list<boost::vecS, //OutEdgeList
                              boost::vecS,// VertexList
                              boost::undirectedS, //Directed,
                              FeatureVectorPixelDescriptor //VertexProperties
                              > FeatureVectorGraph;

typedef boost::adjacency_list<boost::vecS, //OutEdgeList
                              boost::vecS,// VertexList
                              boost::undirectedS, //Directed,
                              ImagePatchPixelDescriptor //VertexProperties
                              > ImagePatchGraph;

typedef boost::adjacency_list<boost::vecS, //OutEdgeList
                              boost::vecS,// VertexList
                              boost::undirectedS, //Directed,
                              CompositeDescriptor //VertexProperties
                              > CompositeDescriptorGraph;

class InpaintingParent
{
public:
  virtual void CreateGraph() = 0;
  virtual void ComputeAllDescriptors() = 0;
};

template <typename TGraph, typename TComputeDescriptor>
class PatchBasedInpainting : public InpaintingParent
{
public:
  void CreateGraph()
  {
    // Create two vertices (pixels) and an edge
    typename TGraph::vertex_descriptor v0 = boost::add_vertex(g);

    typename TGraph::vertex_descriptor v1 = boost::add_vertex(g);
    // g[v1].A = 5;

    boost::add_edge(v0, v1, g);
  }

  void ComputeAllDescriptors()
  {
    // Get the vertices
    typedef typename boost::property_map<TGraph, boost::vertex_index_t>::type IndexMap;
    IndexMap index = get(boost::vertex_index, g);

    TComputeDescriptor descriptorComputer;

    typedef typename boost::graph_traits<TGraph>::vertex_iterator vertex_iter;
    std::pair<vertex_iter, vertex_iter> firstLastPair;
    for (firstLastPair = vertices(g); firstLastPair.first != firstLastPair.second; ++firstLastPair.first)
    {
      typename TGraph::vertex_descriptor currentVertex = *firstLastPair.first;
      descriptorComputer.Compute(g, currentVertex);
    }
    std::cout << std::endl;
  }

private:
  TGraph g;

};


int main(int,char*[])
{
  //PatchBasedInpainting<FeatureVectorGraph> patchBasedInpainting;

  enum UserChoice {UseFeatureVectors, UseImagePatches, UseComposite};

  UserChoice userChoice = UseFeatureVectors;

  InpaintingParent* inpainting;
  if(userChoice == UseFeatureVectors)
  {
    inpainting = new PatchBasedInpainting<FeatureVectorGraph, ComputeFeatureVector<FeatureVectorGraph> >;
  }
  else if(userChoice == UseImagePatches)
  {
    inpainting = new PatchBasedInpainting<ImagePatchGraph, ComputeImagePatch<ImagePatchGraph> >;
  }
  else if(userChoice == UseComposite)
  {
    //inpainting = new PatchBasedInpainting<CompositeDescriptorGraph, CompositeDescriptor<FeatureVectorGraph> >;
  }

  inpainting->CreateGraph();
  inpainting->ComputeAllDescriptors();

  return 0;
}
