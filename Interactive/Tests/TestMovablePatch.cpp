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

#include "MovablePatch.h"

// VTK
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

// Qt
#include <QColor>
#include <QGraphicsView>

int main(int argc, char*argv[])
{
  const unsigned int radius = 5;
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

  QGraphicsView* view = new QGraphicsView;
  QColor color;
  MovablePatch patch1(radius, renderer, view, color);
  MovablePatch patch2(radius, renderer, view);
/*
  void SetVisibility(const bool);
  bool GetVisibility();

  // The ITK region describing the position of the patch.
  itk::ImageRegion<2> GetRegion();

  void Display();*/

  throw;

  return EXIT_SUCCESS;
}
