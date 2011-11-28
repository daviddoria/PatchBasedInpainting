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

#ifndef ClusterColors_H
#define ClusterColors_H

// Custom
#include "DebugOutputs.h"
#include "Mask.h"
#include "Types.h"

// STL
#include <vector>

// ITK
#include "itkListSample.h"
#include "itkKdTree.h"
#include "itkKdTreeGenerator.h"

class ClusterColors : public DebugOutputs
{
public:
  
  ClusterColors();
  
  void Construct(const FloatVectorImageType::Pointer image);
    
  typedef itk::Statistics::ListSample< ColorMeasurementVectorType > SampleType;
  typedef itk::Statistics::KdTreeGenerator< SampleType > TreeGeneratorType;
  typedef TreeGeneratorType::KdTreeType TreeType;
  
  // If the MembershipImage is not provided, compute the histogram.
  std::vector<float> HistogramRegion(const FloatVectorImageType::Pointer image, const itk::ImageRegion<2>& imageRegion,
                                     const Mask::Pointer mask, const itk::ImageRegion<2>& maskRegion);
  
  // If the MembershipImage is provided, compute the histogram (much faster).
  std::vector<float> HistogramRegion(const IntImageType::Pointer image, const itk::ImageRegion<2>& imageRegion,
                                     const Mask::Pointer mask, const itk::ImageRegion<2>& maskRegion);
  
  IntImageType::Pointer GetColorBinMembershipImage();

  std::vector<ColorMeasurementVectorType> GetColors();

  TreeType::Pointer GetKDTree();

  void SetMaxIterations(const unsigned int);
  
protected:
  
  virtual void GenerateColors() = 0;
  
  void CreateMembershipImage();
  void CreateSamplesFromColors();

  FloatVectorImageType::Pointer Image;
  std::vector<ColorMeasurementVectorType> Colors;
  
  SampleType::Pointer Sample;
  TreeGeneratorType::Pointer TreeGenerator;
  TreeType::Pointer KDTree;
  
  IntImageType::Pointer ColorBinMembershipImage;

  unsigned int MaxIterations;
};

#endif
