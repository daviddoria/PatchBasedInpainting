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

#include "ClusterColors.h"

// ITK
#include "itkImageRegionConstIterator.h"
#include "itkDecisionRule.h"
#include "itkVector.h"
#include "itkListSample.h"
#include "itkKdTree.h"
#include "itkWeightedCentroidKdTreeGenerator.h"
#include "itkKdTreeBasedKmeansEstimator.h"
#include "itkMinimumDecisionRule.h"
#include "itkEuclideanDistanceMetric.h"
#include "itkDistanceToCentroidMembershipFunction.h"
#include "itkSampleClassifierFilter.h"
#include "itkNormalVariateGenerator.h"


void ClusterColorsUniform(FloatVectorImageType::Pointer image, const unsigned int binsPerAxis, IntImageType::Pointer outputLabelImage)
{
  // Create samples from an entire image
  const unsigned int numberOfComponents = 3; // RGB
  
  std::cout << "Create samples from an image." << std::endl;
  typedef itk::Vector< float, numberOfComponents > MeasurementVectorType;

  typedef itk::Statistics::ListSample< MeasurementVectorType > SampleType;
  SampleType::Pointer sample = SampleType::New();
  sample->SetMeasurementVectorSize( numberOfComponents );

  MeasurementVectorType mv;
  
  unsigned int step = 256/binsPerAxis;
  std::cout << "Step: " << step << std::endl;
  for(unsigned int r = 0; r < 255; r += step)
    {
    for(unsigned int g = 0; g < 255; g += step)
      {
      for(unsigned int b = 0; b < 255; b += step)
	{
	mv[0] = r;
	mv[1] = g;
	mv[2] = b;

	sample->PushBack( mv );
	}
      }
    }
  std::cout << "Number of colors: " << sample->Size() << std::endl;
  
  // Create a KDTree
  typedef itk::Statistics::KdTreeGenerator< SampleType > TreeGeneratorType;
  TreeGeneratorType::Pointer treeGenerator = TreeGeneratorType::New();
  treeGenerator->SetSample( sample );
  treeGenerator->SetBucketSize( 16 );
  treeGenerator->Update();

  typedef TreeGeneratorType::KdTreeType TreeType;
  typedef TreeType::NearestNeighbors NeighborsType;
  typedef TreeType::KdTreeNodeType NodeType;

  TreeType::Pointer tree = treeGenerator->GetOutput();

  MeasurementVectorType queryPoint;
  itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(image,image->GetLargestPossibleRegion());

  outputLabelImage->SetRegions(image->GetLargestPossibleRegion());
  outputLabelImage->Allocate();
  outputLabelImage->FillBuffer(0);
  
  while(!imageIterator.IsAtEnd())
    {
    // Get the value of the current pixel
    FloatVectorImageType::PixelType pixel = imageIterator.Get();

    queryPoint[0] = pixel[0];
    queryPoint[1] = pixel[1];
    queryPoint[2] = pixel[2];

    TreeType::InstanceIdentifierVectorType neighbors;
    tree->Search( queryPoint, 1u, neighbors );
    //tree->GetMeasurementVector( neighbors[0] );
    outputLabelImage->SetPixel(imageIterator.GetIndex(), neighbors[0]);
    ++imageIterator;
    }

}

void ClusterColors(FloatVectorImageType::Pointer image, const unsigned int numberOfClusters, IntImageType::Pointer outputLabelImage)
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
  
  itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(image,image->GetLargestPossibleRegion());
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
 
  EstimatorType::ParametersType initialMeans(numberOfComponents * numberOfClusters);
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
