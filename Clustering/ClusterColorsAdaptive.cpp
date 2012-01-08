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

#include "ClusterColorsAdaptive.h"

// ITK
#include "itkKdTreeBasedKmeansEstimator.h"
#include "itkShrinkImageFilter.h"

// VTK
#include <vtkCellLocator.h>
#include <vtkDoubleArray.h>
#include <vtkKMeansStatistics.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkIntArray.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkQuantizePolyDataPoints.h>
#include <vtkSmartPointer.h>
#include <vtkTable.h>
#include <vtkVertexGlyphFilter.h>

// Custom
#include "Helpers.h"
#include "ParallelSort.h"

ClusterColorsAdaptive::ClusterColorsAdaptive() : ClusterColors()
{
  this->DownsampleFactor = 1;
}

void ClusterColorsAdaptive::SetDownsampleFactor(const unsigned int downsampleFactor)
{
  this->DownsampleFactor = downsampleFactor;
}

void ClusterColorsAdaptive::SetNumberOfColors(const unsigned int numberOfColors)
{
  this->NumberOfColors = numberOfColors;
}

void ClusterColorsAdaptive::GenerateColors()
{
  //GenerateColorsVTKKMeans();
  //GenerateColorsVTKQuantize();
  GenerateColorsVTKBin();
}


void ClusterColorsAdaptive::GenerateColorsVTKBin()
{
  // This function creates a fixed number of bins, and then keeps the ones that have at least one color inside.

  EnterFunction("GenerateColorsVTKBin()");
  std::cout << "this->NumberOfColors: " << this->NumberOfColors << std::endl;

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

  // Create a vtkPoints of all of the pixels
  itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(this->Image, this->Image->GetLargestPossibleRegion());
  while(!imageIterator.IsAtEnd())
    {
    if(this->MaskImage && this->MaskImage->IsValid(imageIterator.GetIndex()))
      {
      FloatVectorImageType::PixelType pixel = imageIterator.Get();
      points->InsertNextPoint(pixel[0], pixel[1], pixel[2]);
      }
    ++imageIterator;
    }

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);

//   vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
//   glyphFilter->SetInputConnection(polyData->GetProducerPort());
//   glyphFilter->Update();


  double bounds[6];
  polyData->GetBounds(bounds);
  //std::cout << "bounds: " << bounds[0] << " " << bounds[1] << " " << bounds[2] << " " << bounds[3] << " " << bounds[4] << " " << bounds[5] << std::endl;

  float xWidth = bounds[1] - bounds[0];
  float yWidth = bounds[3] - bounds[2];
  float zWidth = bounds[5] - bounds[4];

  float binSize = std::min(std::min(xWidth,yWidth), zWidth);

  double epsilon = 1e-6;
  // We leave a little bit of a boundary around the grid so that pixels exactly on the border are still used.
  vtkSmartPointer<vtkImageData> grid = vtkSmartPointer<vtkImageData>::New();
  grid->SetOrigin(bounds[0] - epsilon, bounds[2] - epsilon, bounds[4] - epsilon);

  // This produces a bin size of approximately 10x10x10
  //unsigned int numVoxelsPerDimension = 25;

  unsigned int numVoxelsPerDimension = 0;
  while(this->Colors.size() < this->NumberOfColors)
    {
    numVoxelsPerDimension++;
    // Since we start at bounds[min]-epsilon, we must go to bounds[max]+epsilon, so the total width is bounds+2*epsilon
    grid->SetSpacing((bounds[1] + 2.0f*epsilon - bounds[0])/static_cast<double>(numVoxelsPerDimension),
                    (bounds[3] + 2.0f*epsilon - bounds[2])/static_cast<double>(numVoxelsPerDimension),
                    (bounds[5] + 2.0f*epsilon - bounds[4])/static_cast<double>(numVoxelsPerDimension));
    int extent[6];
    extent[0] = 0;
    extent[1] = numVoxelsPerDimension;
    extent[2] = 0;
    extent[3] = numVoxelsPerDimension;
    extent[4] = 0;
    extent[5] = numVoxelsPerDimension;
    grid->SetExtent(extent);
    grid->SetScalarTypeToInt();
    grid->SetNumberOfScalarComponents(1);
    grid->Update();

    std::vector<int> cellOccupancy(grid->GetNumberOfCells(), 0);

    vtkSmartPointer<vtkCellLocator> cellLocator = vtkSmartPointer<vtkCellLocator>::New();
    cellLocator->SetDataSet(grid);
    cellLocator->BuildLocator();

    int dims[3];
    grid->GetDimensions(dims);

    for(vtkIdType i = 0; i < polyData->GetNumberOfPoints(); i++)
      {
      double p[3];
      polyData->GetPoint(i,p);
      int cellId = cellLocator->FindCell(p);
      if(cellId < 0 || cellId > static_cast<int>(cellOccupancy.size()))
        {
        std::stringstream ss;
        ss << "Something is wrong, cellId = " << cellId << std::endl
           << "p = " << p[0] << " " << p[1] << " " << p[2] << std::endl;
        throw std::runtime_error(ss.str());
        }
      cellOccupancy[cellId]++;
      }

    this->Colors.clear();
    ColorMeasurementVectorType color;
    for(unsigned int i = 0; i < cellOccupancy.size(); ++i)
      {
      if(cellOccupancy[i] > 0)
        {
        double p[3];
        Helpers::GetCellCenter(grid, i, p);
        //std::cout << "Keeping cell: " << sorted[i].index << " with center: " << p[0] << " " << p[1] << " " << p[2] << std::endl;
        color[0] = p[0];
        color[1] = p[1];
        color[2] = p[2];
        this->Colors.push_back(color);
        } // end if
      } // end for
    }// end while

  CreateKDTreeFromColors();

  std::cout << "There are " << this->Colors.size() << " colors." << std::endl;

  LeaveFunction("GenerateColorsVTKBin()");
}

void ClusterColorsAdaptive::GenerateColorsVTKTopBins()
{
  // This doesn't really make sense, because we want to make sure the entire space of colors from the image is covered by cluster centers.
  EnterFunction("GenerateColorsVTKTopBins()");
  std::cout << "this->NumberOfColors: " << this->NumberOfColors << std::endl;

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

  // Create a vtkPoints of all of the pixels
  itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(this->Image, this->Image->GetLargestPossibleRegion());
  while(!imageIterator.IsAtEnd())
    {
    FloatVectorImageType::PixelType pixel = imageIterator.Get();
    points->InsertNextPoint(pixel[0], pixel[1], pixel[2]);
    ++imageIterator;
    }

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);

//   vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
//   glyphFilter->SetInputConnection(polyData->GetProducerPort());
//   glyphFilter->Update();


  double bounds[6];
  polyData->GetBounds(bounds);
  //std::cout << "bounds: " << bounds[0] << " " << bounds[1] << " " << bounds[2] << " " << bounds[3] << " " << bounds[4] << " " << bounds[5] << std::endl;

  double epsilon = 1e-6;
  // We leave a little bit of a boundary around the grid so that pixels exactly on the border are still used.
  vtkSmartPointer<vtkImageData> grid = vtkSmartPointer<vtkImageData>::New();
  grid->SetOrigin(bounds[0] - epsilon, bounds[2] - epsilon, bounds[4] - epsilon);


  unsigned int numVoxelsPerDimension = 25; // This produces a bin size of approximately 10x10x10

//   grid->SetSpacing((bounds[1]-bounds[0])/static_cast<double>(numVoxelsPerDimension),
//                    (bounds[3]-bounds[2])/static_cast<double>(numVoxelsPerDimension),
//                    (bounds[5]-bounds[4])/static_cast<double>(numVoxelsPerDimension));
  // Since we start at bounds[min]-epsilon, we must go to bounds[max]+epsilon, so the total width is bounds+2*epsilon
  grid->SetSpacing((bounds[1] + 2.0f*epsilon - bounds[0])/static_cast<double>(numVoxelsPerDimension),
                   (bounds[3] + 2.0f*epsilon - bounds[2])/static_cast<double>(numVoxelsPerDimension),
                   (bounds[5] + 2.0f*epsilon - bounds[4])/static_cast<double>(numVoxelsPerDimension));
  int extent[6];
  extent[0] = 0;
  extent[1] = numVoxelsPerDimension;
  extent[2] = 0;
  extent[3] = numVoxelsPerDimension;
  extent[4] = 0;
  extent[5] = numVoxelsPerDimension;
  grid->SetExtent(extent);
  grid->SetScalarTypeToInt();
  grid->SetNumberOfScalarComponents(1);
  grid->Update();

  std::vector<int> cellOccupancy(grid->GetNumberOfCells(), 0);
  //std::cout << "cellOccupancy size: " << cellOccupancy.size() << std::endl;

  if(cellOccupancy.size() < this->NumberOfColors)
    {
    throw std::runtime_error("cellOccupancy.size() < this->NumberOfColors, this doesn't make sense!")
    }

  vtkSmartPointer<vtkCellLocator> cellLocator = vtkSmartPointer<vtkCellLocator>::New();
  cellLocator->SetDataSet(grid);
  cellLocator->BuildLocator();

  int dims[3];
  grid->GetDimensions(dims);

  for(vtkIdType i = 0; i < polyData->GetNumberOfPoints(); i++)
    {
    double p[3];
    polyData->GetPoint(i,p);
    int cellId = cellLocator->FindCell(p);
    if(cellId < 0 || cellId > static_cast<int>(cellOccupancy.size()))
      {
      std::stringstream ss;
      ss << "Something is wrong, cellId = " << cellId << std::endl
         << "p = " << p[0] << " " << p[1] << " " << p[2] << std::endl;
      throw std::runtime_error(ss.str());
      }
    cellOccupancy[cellId]++;
    }

  std::vector<ParallelSort::IndexedValue<int> > sorted = ParallelSort::ParallelSort<int>(cellOccupancy, ParallelSort::DESCENDING);

//   for(unsigned int i = 0; i < this->NumberOfColors; ++i)
//     {
//     double p[3];
//     Helpers::GetCellCenter(grid, sorted[i].index, p);
//     std::cout << "index: " << sorted[i].index << " value: " << sorted[i].value << " center: " << p[0] << " " << p[1] << " " << p[2] << std::endl;
//     }
  this->Colors.clear();
  ColorMeasurementVectorType color;
  for(unsigned int i = 0; i < this->NumberOfColors; ++i)
    {
    double p[3];
    Helpers::GetCellCenter(grid, sorted[i].index, p);
    //std::cout << "Keeping cell: " << sorted[i].index << " with center: " << p[0] << " " << p[1] << " " << p[2] << std::endl;
    color[0] = p[0];
    color[1] = p[1];
    color[2] = p[2];
    this->Colors.push_back(color);
    }

  CreateKDTreeFromColors();

  std::cout << "There are " << this->Colors.size() << " colors." << std::endl;
  std::cout << "There are " << this->Colors.size() << " colors." << std::endl;

  LeaveFunction("GenerateColorsVTKBin()");
}

void ClusterColorsAdaptive::GenerateColorsVTKQuantize()
{
  EnterFunction("GenerateColorsVTKQuantize()");
  std::cout << "this->NumberOfColors: " << this->NumberOfColors << std::endl;

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

  typedef itk::ShrinkImageFilter <FloatVectorImageType, FloatVectorImageType> ShrinkImageFilterType;
  ShrinkImageFilterType::Pointer shrinkFilter = ShrinkImageFilterType::New();
  shrinkFilter->SetInput(this->Image);
  shrinkFilter->SetShrinkFactor(0, this->DownsampleFactor);
  shrinkFilter->SetShrinkFactor(1, this->DownsampleFactor);
  shrinkFilter->Update();

  std::cout << "There are " << this->Image->GetLargestPossibleRegion().GetSize()[0] * this->Image->GetLargestPossibleRegion().GetSize()[1]
            << " original image points." << std::endl;
  std::cout << "There are " << shrinkFilter->GetOutput()->GetLargestPossibleRegion().GetSize()[0] * shrinkFilter->GetOutput()->GetLargestPossibleRegion().GetSize()[1]
            << " reduced points." << std::endl;

  // Create a vtkPoints of all of the pixels
  itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(shrinkFilter->GetOutput(), shrinkFilter->GetOutput()->GetLargestPossibleRegion());
  while(!imageIterator.IsAtEnd())
    {
    FloatVectorImageType::PixelType pixel = imageIterator.Get();
    points->InsertNextPoint(pixel[0], pixel[1], pixel[2]);
    ++imageIterator;
    }

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);

  vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
  glyphFilter->SetInputConnection(polyData->GetProducerPort());
  glyphFilter->Update();

  std::cout << "There are " << polyData->GetNumberOfPoints() << " points input to quantization." << std::endl;
  // Downsample the points until the desired number of points was achieved
  vtkSmartPointer<vtkQuantizePolyDataPoints> quantizeFilter = vtkSmartPointer<vtkQuantizePolyDataPoints>::New();
  quantizeFilter->SetInputConnection(glyphFilter->GetOutputPort());
  quantizeFilter->SetQFactor(20);
  quantizeFilter->Update();

  std::cout << "Test: There are " << quantizeFilter->GetOutput()->GetNumberOfPoints() << " points after quantization." << std::endl;

  this->Colors.clear();
  ColorMeasurementVectorType color;
  for(vtkIdType i = 0; i < quantizeFilter->GetOutput()->GetNumberOfPoints(); ++i)
    {
    double p[3];
    quantizeFilter->GetOutput()->GetPoint(i, p);
    color[0] = p[0];
    color[1] = p[1];
    color[2] = p[2];
    this->Colors.push_back(color);
    }

  CreateKDTreeFromColors();

  LeaveFunction("GenerateColorsVTK()");
}

void ClusterColorsAdaptive::GenerateColorsVTKKMeans()
{
  EnterFunction("GenerateColorsVTK()");
  std::cout << "this->NumberOfColors: " << this->NumberOfColors << std::endl;

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

  typedef itk::ShrinkImageFilter <FloatVectorImageType, FloatVectorImageType> ShrinkImageFilterType;
  ShrinkImageFilterType::Pointer shrinkFilter = ShrinkImageFilterType::New();
  shrinkFilter->SetInput(this->Image);

  shrinkFilter->SetShrinkFactor(0, this->DownsampleFactor);
  shrinkFilter->SetShrinkFactor(1, this->DownsampleFactor);
  shrinkFilter->Update();

  std::cout << "There are " << this->Image->GetLargestPossibleRegion().GetSize()[0] * this->Image->GetLargestPossibleRegion().GetSize()[1]
            << " original image points." << std::endl;
  std::cout << "There are " << shrinkFilter->GetOutput()->GetLargestPossibleRegion().GetSize()[0] * shrinkFilter->GetOutput()->GetLargestPossibleRegion().GetSize()[1]
            << " reduced points." << std::endl;

  //itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(this->Image, this->Image->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(shrinkFilter->GetOutput(), shrinkFilter->GetOutput()->GetLargestPossibleRegion());
  while(!imageIterator.IsAtEnd())
    {
    FloatVectorImageType::PixelType pixel = imageIterator.Get();
    points->InsertNextPoint(pixel[0], pixel[1], pixel[2]);
    ++imageIterator;
    }

  // Get the points into the format needed for KMeans
  vtkSmartPointer<vtkTable> inputData = vtkSmartPointer<vtkTable>::New();

  for( int column = 0; column < 3; ++column )
    {
    std::stringstream colName;
    colName << "coord " << column;
    vtkSmartPointer<vtkDoubleArray> doubleArray = vtkSmartPointer<vtkDoubleArray>::New();
    doubleArray->SetNumberOfComponents(1);
    doubleArray->SetName( colName.str().c_str() );
    doubleArray->SetNumberOfTuples(points->GetNumberOfPoints());

    for( int pointId = 0; pointId < points->GetNumberOfPoints(); ++pointId )
      {
      double p[3];
      points->GetPoint(pointId, p);

      doubleArray->SetValue( pointId, p[column] );
      }

    inputData->AddColumn( doubleArray );
    }


  vtkSmartPointer<vtkKMeansStatistics> kMeansStatistics = vtkSmartPointer<vtkKMeansStatistics>::New();
  kMeansStatistics->SetInput( vtkStatisticsAlgorithm::INPUT_DATA, inputData );
  kMeansStatistics->SetColumnStatus( inputData->GetColumnName( 0 ) , 1 );
  kMeansStatistics->SetColumnStatus( inputData->GetColumnName( 1 ) , 1 );
  kMeansStatistics->SetColumnStatus( inputData->GetColumnName( 2 ) , 1 );
  kMeansStatistics->RequestSelectedColumns();
  kMeansStatistics->SetAssessOption( false );
  kMeansStatistics->SetDeriveOption( false );
  kMeansStatistics->SetDefaultNumberOfClusters( this->NumberOfColors );
  kMeansStatistics->SetMaxNumIterations( this->MaxIterations);
  std::cout << "Max iterations: " << kMeansStatistics->GetMaxNumIterations() << std::endl;
  kMeansStatistics->Update() ;

  vtkMultiBlockDataSet* outputMetaDS = vtkMultiBlockDataSet::SafeDownCast( kMeansStatistics->GetOutputDataObject( vtkStatisticsAlgorithm::OUTPUT_MODEL ) );
  vtkSmartPointer<vtkTable> outputMeta = vtkTable::SafeDownCast( outputMetaDS->GetBlock( 0 ) );

  vtkDoubleArray* coord0 = vtkDoubleArray::SafeDownCast(outputMeta->GetColumnByName("coord 0")); // Note these must match the colName << "coord " << column; line above!
  vtkDoubleArray* coord1 = vtkDoubleArray::SafeDownCast(outputMeta->GetColumnByName("coord 1"));
  vtkDoubleArray* coord2 = vtkDoubleArray::SafeDownCast(outputMeta->GetColumnByName("coord 2"));

  if(static_cast<vtkIdType>(this->NumberOfColors) != coord0->GetNumberOfTuples())
    {
    throw std::runtime_error("Something is wrong: this->NumberOfColors != coord0->GetNumberOfTuples()");
    }
  this->Colors.clear();
  ColorMeasurementVectorType color;
  for(vtkIdType i = 0; i < coord0->GetNumberOfTuples(); ++i)
    {
    color[0] = coord0->GetValue(i);
    color[1] = coord1->GetValue(i);
    color[2] = coord2->GetValue(i);
    this->Colors.push_back(color);
    }

  CreateKDTreeFromColors();

  LeaveFunction("GenerateColorsVTK()");
}

void ClusterColorsAdaptive::GenerateColorsITK()
{
  // Create samples from an entire image
  const unsigned int numberOfComponents = 3; // RGB

  std::cout << "Create samples from an image." << std::endl;
  typedef itk::Vector< float, numberOfComponents > MeasurementVectorType;

  typedef itk::Statistics::ListSample< MeasurementVectorType > SampleType;
  SampleType::Pointer sample = SampleType::New();
  sample->SetMeasurementVectorSize( numberOfComponents );

  MeasurementVectorType mv;
//   mv[0] = 0;
//   mv[1] = 1;
//   mv[2] = 2;
//   sample->PushBack( mv );

  itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(this->Image, this->Image->GetLargestPossibleRegion());
  unsigned int counter = 0;
  while(!imageIterator.IsAtEnd())
    {
    // Get the value of the current pixel
    FloatVectorImageType::PixelType pixel = imageIterator.Get();
    //indexVector.push_back(imageIterator.GetIndex());
    //pointIdImage->SetPixel(imageIterator.GetIndex(), points->GetNumberOfPoints());
    mv[0] = pixel[0];
    mv[1] = pixel[1];
    mv[2] = pixel[2];
    counter++;
    if(counter > 20)
      {
      break;
      }
    std::cout << "mv: " << mv << std::endl;
    //std::cout << "mv.size" << mv.Size() << std::endl;
    sample->PushBack( mv );
    ++imageIterator;
    }

  std::cout << "Setup KMeans clustering..." << std::endl;
  typedef itk::Statistics::KdTreeGenerator< SampleType > TreeGeneratorType;
  TreeGeneratorType::Pointer treeGenerator = TreeGeneratorType::New();
  treeGenerator->SetSample( sample );
  treeGenerator->SetBucketSize( 16 );
  treeGenerator->Update();

  typedef TreeGeneratorType::KdTreeType TreeType;
  typedef itk::Statistics::KdTreeBasedKmeansEstimator<TreeType> EstimatorType;
  EstimatorType::Pointer estimator = EstimatorType::New();

  EstimatorType::ParametersType initialMeans(numberOfComponents * this->NumberOfColors);
  std::cout << "initialMeans.Size(): " << initialMeans.Size() << std::endl;
  for(unsigned int i = 0; i < initialMeans.Size(); ++i)
    {
    initialMeans[i] = 0.0;
    }
  std::cout << "Perform KMeans clustering..." << std::endl;
  estimator->SetParameters( initialMeans );
  estimator->SetKdTree( treeGenerator->GetOutput() );
  estimator->SetMaximumIteration( 200 );
  estimator->SetCentroidPositionChangesThreshold(0.0);
  estimator->StartOptimization();
 /*
  EstimatorType::ParametersType estimatedMeans = estimator->GetParameters();

  std::cout << "Build tree of cluster centers..." << std::endl;

  // Create a new set of measurements, this time we are building a tree of the cluster centers
  SampleType::Pointer clusterCentersSample = SampleType::New();
  clusterCentersSample->SetMeasurementVectorSize( numberOfComponents );
  std::cout << "numberOfComponents: " << numberOfComponents << std::endl;

  MeasurementVectorType measurement(numberOfComponents);
  for ( unsigned int i = 0 ; i < numberOfComponents * numberOfClusters ; i += numberOfComponents )
    {
    std::cout << "cluster[" << i << "] " << std::endl;
    std::cout << "    estimated mean : " << estimatedMeans[i] << " , " << estimatedMeans[i+1] << std::endl;

    measurement[0] = estimatedMeans[0 + numberOfComponents*i];
    measurement[1] = estimatedMeans[1 + numberOfComponents*i];
    measurement[2] = estimatedMeans[2 + numberOfComponents*i];
    std::cout << "measurement.size" << measurement.Size() << std::endl;
    clusterCentersSample->PushBack( measurement );
    }


  typedef itk::Statistics::KdTreeGenerator< SampleType > TreeGeneratorType;
  TreeGeneratorType::Pointer clusterCentersTreeGenerator = TreeGeneratorType::New();
  clusterCentersTreeGenerator->SetSample( clusterCentersSample );
  clusterCentersTreeGenerator->SetBucketSize( 16 );
  clusterCentersTreeGenerator->Update();

  typedef TreeGeneratorType::KdTreeType TreeType;
  typedef TreeType::NearestNeighbors NeighborsType;
  typedef TreeType::KdTreeNodeType NodeType;

  TreeType::Pointer tree = clusterCentersTreeGenerator->GetOutput();

  std::cout << "Classifying..." << std::endl;
  unsigned int numberOfNeighbors = 1;
  TreeType::InstanceIdentifierVectorType neighbors;
  imageIterator.GoToBegin();
  while(!imageIterator.IsAtEnd())
    {
    FloatVectorImageType::PixelType pixel = imageIterator.Get();
    MeasurementVectorType queryPoint;
    queryPoint[0] = pixel[0];
    queryPoint[1] = pixel[1];
    queryPoint[2] = pixel[2];
    tree->Search( queryPoint, numberOfNeighbors, neighbors ) ;
    tree->GetMeasurementVector( neighbors[0] );
    ++imageIterator;
    }*/

}

