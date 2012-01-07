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

// VTK
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkDoubleArray.h>
#include <vtkTable.h>
#include <vtkIntArray.h>
#include <vtkKMeansStatistics.h>

//ClusterColors::ClusterColors(FloatVectorImageType::Pointer image, const unsigned int numberOfClusters, IntImageType::Pointer outputLabelImage)
void ClusterColors(FloatVectorImageType::Pointer image, const unsigned int numberOfClusters, IntImageType::Pointer outputLabelImage)
{
  std::cout << "ClusterColors()" << std::endl;
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

  itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(image,image->GetLargestPossibleRegion());
  typedef itk::Image<int, 2> PointIdImageType;
  PointIdImageType::Pointer pointIdImage = PointIdImageType::New();
  pointIdImage->SetRegions(image->GetLargestPossibleRegion());
  pointIdImage->Allocate();
  std::vector<itk::Index<2> > indexVector;
  while(!imageIterator.IsAtEnd())
    {
    // Get the value of the current pixel
    FloatVectorImageType::PixelType pixel = imageIterator.Get();
    indexVector.push_back(imageIterator.GetIndex());
    points->InsertNextPoint (pixel[0], pixel[1], pixel[2]);
    pointIdImage->SetPixel(imageIterator.GetIndex(), points->GetNumberOfPoints());
    ++imageIterator;
    }

  std::cout << "setup kmeans" << std::endl;
  // Get the points into the format needed for KMeans
  vtkSmartPointer<vtkTable> inputData = vtkSmartPointer<vtkTable>::New();

  for(unsigned int c = 0; c < 3; ++c )
    {
    std::stringstream colName;
    colName << "coord " << c;
    vtkSmartPointer<vtkDoubleArray> doubleArray = vtkSmartPointer<vtkDoubleArray>::New();
    doubleArray->SetNumberOfComponents(1);
    doubleArray->SetName( colName.str().c_str() );
    doubleArray->SetNumberOfTuples(points->GetNumberOfPoints());

    for( int r = 0; r < points->GetNumberOfPoints(); ++ r )
      {
      double p[3];
      points->GetPoint(r, p);

      doubleArray->SetValue( r, p[c] );
      }

    inputData->AddColumn( doubleArray );
    }

  std::cout << "compute kmeans" << std::endl;
  vtkSmartPointer<vtkKMeansStatistics> kMeansStatistics = vtkSmartPointer<vtkKMeansStatistics>::New();
  kMeansStatistics->SetInput( vtkStatisticsAlgorithm::INPUT_DATA, inputData );
  kMeansStatistics->SetColumnStatus( inputData->GetColumnName( 0 ) , 1 );
  kMeansStatistics->SetColumnStatus( inputData->GetColumnName( 1 ) , 1 );
  kMeansStatistics->SetColumnStatus( inputData->GetColumnName( 2 ) , 1 );
  //kMeansStatistics->SetColumnStatus( "Testing", 1 );
  kMeansStatistics->RequestSelectedColumns();
  kMeansStatistics->SetAssessOption( true );
  kMeansStatistics->SetDefaultNumberOfClusters( numberOfClusters );
  kMeansStatistics->Update() ;

  // Display the results
  //kMeansStatistics->GetOutput()->Dump();
/*
  vtkSmartPointer<vtkIntArray> clusterArray = vtkSmartPointer<vtkIntArray>::New();
  clusterArray->SetNumberOfComponents(1);
  clusterArray->SetName( "ClusterId" );*/

//   UnsignedCharScalarImageType::Pointer labelImage = UnsignedCharScalarImageType::New();
//   labelImage->SetRegions(image->GetLargestPossibleRegion());
//   labelImage->Allocate();

  // Create the set of cluster centers
  vtkSmartPointer<vtkPoints> clusterCenters = vtkSmartPointer<vtkPoints>::New();
  for(unsigned int r = 0; r < kMeansStatistics->GetOutput()->GetNumberOfRows(); r++)
    {
    vtkVariant v = kMeansStatistics->GetOutput()->GetValue(r, kMeansStatistics->GetOutput()->GetNumberOfColumns() - 1);
    clusterCenters->InsertNextPoint()

    //Find the closest points to TestPoint
    std::cout << "The closest point is point " << id << std::endl;


    //clusterArray->InsertNextValue(v.ToInt());
    }

  //Create the tree
  vtkSmartPointer<vtkKdTree> kDTree = vtkSmartPointer<vtkKdTree>::New();
  kDTree->BuildLocatorFromPoints(points);

  outputLabelImage->SetRegions(image->GetLargestPossibleRegion());
  outputLabelImage->Allocate();

  itk::ImageRegionIterator<IntImageType> outputIterator(outputLabelImage,outputLabelImage->GetLargestPossibleRegion());
  while(!outputIterator.IsAtEnd())
    {
    // Get the value of the current pixel
    FloatVectorImageType::PixelType pixel = image.GetPixel(outputIterator.GetIndex());
    double queryColor[3] = {pixel[0], pixel[1], pixel[2]};

    double closestPointDist;
    vtkIdType id = kDTree->FindClosestPoint(queryColor, closestPointDist);


    //std::cout << "Point " << r << " is in cluster " << v.ToInt() << std::endl;
    outputLabelImage->SetPixel(indexVector[r], v.ToInt());

    outputIterator.Set(points->GetNumberOfPoints());
    ++outputIterator;
    }



  /*
  std::cout << "setup output" << std::endl;
  outputLabelImage->SetRegions(image->GetLargestPossibleRegion());
  outputLabelImage->Allocate();

  for(unsigned int r = 0; r < kMeansStatistics->GetOutput()->GetNumberOfRows(); r++)
    {
    vtkVariant v = kMeansStatistics->GetOutput()->GetValue(r,kMeansStatistics->GetOutput()->GetNumberOfColumns() - 1);
    //std::cout << "Point " << r << " is in cluster " << v.ToInt() << std::endl;
    outputLabelImage->SetPixel(indexVector[r], v.ToInt());
    //clusterArray->InsertNextValue(v.ToInt());
    }
  */
}
