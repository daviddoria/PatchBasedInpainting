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

#ifndef Types_H
#define Types_H

#include "itkCovariantVector.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkVectorImage.h"

typedef itk::VectorImage<float,2> FloatVectorImageType;
typedef itk::VariableLengthVector<float> VectorType;

typedef itk::Image<itk::RGBPixel<unsigned char>, 2> RGBImageType;

// Scalar image types
typedef itk::Image<float,2> FloatScalarImageType;
typedef itk::Image<unsigned char,2> UnsignedCharScalarImageType;

// Vector types
typedef itk::CovariantVector<unsigned char, 3> UnsignedCharVectorType;
typedef itk::CovariantVector<float, 3> FloatVector3Type;
typedef itk::CovariantVector<float, 2> FloatVector2Type;

// Vector image types
typedef itk::Image<UnsignedCharVectorType ,2> UnsignedCharVectorImageType;
typedef itk::Image<FloatVector3Type , 2> FloatVector3ImageType;
typedef itk::Image<FloatVector2Type , 2> FloatVector2ImageType;


#endif