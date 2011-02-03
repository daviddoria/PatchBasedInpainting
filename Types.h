/*=========================================================================
 *
 *  Copyright David Doria 2010 daviddoria@gmail.com
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

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkCovariantVector.h"

// Image types
typedef itk::Image<float,2> FloatImageType;
typedef itk::Image<unsigned char,2> UnsignedCharImageType;
typedef itk::Image<itk::CovariantVector<unsigned char, 3> ,2> ColorImageType;
typedef itk::Image< itk::CovariantVector<float, 2>, 2 > VectorImageType;

// Reader types
typedef  itk::ImageFileReader< UnsignedCharImageType  > UnsignedCharImageReaderType;
typedef  itk::ImageFileReader< ColorImageType  > ColorImageReaderType;

typedef itk::Image<itk::CovariantVector<float, 5>, 2> RGBDIImageType;

#endif