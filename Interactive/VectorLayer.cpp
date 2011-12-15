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

#include "VectorLayer.h"

#include <vtkActor.h>
//#include <vtkArrowSource.h>
//#include <vtkGlyph2D.h>
#include <vtkHedgeHog.h>
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>

VectorLayer::VectorLayer()
{
  this->Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->ImageData = vtkSmartPointer<vtkImageData>::New();
  this->Actor = vtkSmartPointer<vtkActor>::New();
  //this->GlyphFilter = vtkSmartPointer<vtkGlyph2D>::New();
  this->GlyphFilter = vtkSmartPointer<vtkHedgeHog>::New();
  this->Vectors = vtkSmartPointer<vtkPolyData>::New();

  /*
  vtkSmartPointer<vtkArrowSource> arrowSource = vtkSmartPointer<vtkArrowSource>::New();
  arrowSource->Update();

  this->GlyphFilter->SetInputConnection(this->Vectors->GetProducerPort());
  this->GlyphFilter->SetSourceConnection(arrowSource->GetOutputPort());
  this->GlyphFilter->OrientOn();
  this->GlyphFilter->SetVectorModeToUseVector(); // Use vectors, not normals
  this->GlyphFilter->SetScaleModeToScaleByVector(); // Scale the glyphs by the vector magnitude
  //this->GlyphFilter->SetScaleFactor(10);
  */

  this->GlyphFilter->SetInputConnection(this->Vectors->GetProducerPort());
  this->GlyphFilter->SetVectorModeToUseVector(); // Use vectors, not normals

  this->Mapper->SetInputConnection(this->GlyphFilter->GetOutputPort());
  this->Actor->SetMapper(this->Mapper);
}
