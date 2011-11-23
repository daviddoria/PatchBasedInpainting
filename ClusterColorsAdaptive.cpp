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
#include <vtkDoubleArray.h>
#include <vtkKMeansStatistics.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkIntArray.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkTable.h>

void ClusterColorsAdaptive::SetNumberOfColors(const unsigned int numberOfColors)
{
  this->NumberOfColors = numberOfColors;
}

void ClusterColorsAdaptive::GenerateColors()
{
  GenerateColorsVTK();
}

void ClusterColorsAdaptive::GenerateColorsVTK()
{
  EnterFunction("GenerateColorsVTK()");
  std::cout << "this->NumberOfColors: " << this->NumberOfColors << std::endl;
  
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

  typedef itk::ShrinkImageFilter <FloatVectorImageType, FloatVectorImageType> ShrinkImageFilterType;
  ShrinkImageFilterType::Pointer shrinkFilter = ShrinkImageFilterType::New();
  shrinkFilter->SetInput(this->Image);
  //unsigned int downsampleFactor = 4;
  //unsigned int downsampleFactor = 5;
  unsigned int downsampleFactor = 20;
  shrinkFilter->SetShrinkFactor(0, downsampleFactor);
  shrinkFilter->SetShrinkFactor(1, downsampleFactor);
  shrinkFilter->Update();

  std::cout << "There are " << this->Image->GetLargestPossibleRegion().GetSize()[0] * this->Image->GetLargestPossibleRegion().GetSize()[1]
            << " original image points." << std::endl;
  std::cout << "There are " << shrinkFilter->GetOutput()->GetLargestPossibleRegion().GetSize()[0] * shrinkFilter->GetOutput()->GetLargestPossibleRegion().GetSize()[1]
            << " reduced points." << std::endl;
  
  //itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(this->Image, this->Image->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(shrinkFilter->GetOutput(), shrinkFilter->GetOutput()->GetLargestPossibleRegion());
  unsigned int counter = 0;
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


  vtkSmartPointer<vtkKMeansStatistics> kMeansStatistics =
    vtkSmartPointer<vtkKMeansStatistics>::New();

  kMeansStatistics->SetInput( vtkStatisticsAlgorithm::INPUT_DATA, inputData );
  kMeansStatistics->SetColumnStatus( inputData->GetColumnName( 0 ) , 1 );
  kMeansStatistics->SetColumnStatus( inputData->GetColumnName( 1 ) , 1 );
  kMeansStatistics->SetColumnStatus( inputData->GetColumnName( 2 ) , 1 );
  kMeansStatistics->RequestSelectedColumns();
  kMeansStatistics->SetAssessOption( true );
  kMeansStatistics->SetDefaultNumberOfClusters( this->NumberOfColors );
  kMeansStatistics->Update() ;

  vtkMultiBlockDataSet* outputMetaDS = vtkMultiBlockDataSet::SafeDownCast( kMeansStatistics->GetOutputDataObject( vtkStatisticsAlgorithm::OUTPUT_MODEL ) );
  vtkSmartPointer<vtkTable> outputMeta = vtkTable::SafeDownCast( outputMetaDS->GetBlock( 0 ) );

  vtkDoubleArray* coord0 = vtkDoubleArray::SafeDownCast(outputMeta->GetColumnByName("coord 0")); // Note these must match the colName << "coord " << column; line above!
  vtkDoubleArray* coord1 = vtkDoubleArray::SafeDownCast(outputMeta->GetColumnByName("coord 1"));
  vtkDoubleArray* coord2 = vtkDoubleArray::SafeDownCast(outputMeta->GetColumnByName("coord 2"));

  if(this->NumberOfColors != coord0->GetNumberOfTuples())
    {
    std::cout << "Something is wrong: this->NumberOfColors != coord0->GetNumberOfTuples()" << std::endl;
    exit(-1);
    }
  this->Colors.clear();
  ColorMeasurementVectorType color;
  for(unsigned int i = 0; i < coord0->GetNumberOfTuples(); ++i)
    {
    color[0] = coord0->GetValue(i);
    color[1] = coord1->GetValue(i);
    color[2] = coord2->GetValue(i);
    this->Colors.push_back(color);
    }

  CreateSamplesFromColors();
  
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