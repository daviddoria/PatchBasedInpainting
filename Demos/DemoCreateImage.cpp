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
#include "Helpers.h"

// ITK
#include "itkImage.h"

typedef itk::Image<float,2> FloatScalarImageType;

void OutputImageType(const itk::ImageBase<2>* input);

int main(int argc, char *argv[])
{
  FloatScalarImageType::Pointer floatImage = FloatScalarImageType::New();

  itk::ImageBase<2>::Pointer floatCopy = Helpers::CreateImageWithSameType(floatImage);
  Helpers::OutputImageType(floatCopy);

  return EXIT_SUCCESS;
}
