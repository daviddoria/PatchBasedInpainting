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
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkSmartPointer.h>

// Qt
#include <QApplication>
#include <QColor>
#include <QGraphicsView>

static void TestVisibility();
static void TestGetRegion();
static void TestConstructors();
static void TestUse();

int main(int argc, char*argv[])
{
  QApplication app(argc, argv);
//  TestVisibility();
//  TestGetRegion();
//  TestConstructors();
    TestUse();

  return EXIT_SUCCESS;
}

void TestUse()
{
  vtkSmartPointer<vtkRenderer> renderer =
    vtkSmartPointer<vtkRenderer>::New();
  renderer->SetBackground(.4, .5, .6);

  vtkSmartPointer<vtkRenderWindow> renderWindow =
    vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);

  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<vtkInteractorStyleImage> style =
    vtkSmartPointer<vtkInteractorStyleImage>::New();
  style->SetCurrentRenderer(renderer);

  renderWindowInteractor->SetInteractorStyle(style);

  renderWindowInteractor->SetRenderWindow(renderWindow);
  renderWindowInteractor->Initialize();

  const unsigned int radius = 5;
  MovablePatch<vtkInteractorStyleImage> movablePatch(radius, style);

  QColor color;
//  MovablePatch<vtkInteractorStyleImage> movablePatch(radius, style, nullptr, color);

  renderWindowInteractor->Start();

}

void TestConstructors()
{
  const unsigned int radius = 5;
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

  QGraphicsView* view = new QGraphicsView;

  // Default color
//  MovablePatch movablePatchDefaultColor(radius, renderer, view);

  // Specified color
  QColor color;
//  MovablePatch movablePatchSpecifiedColor(radius, renderer, view, color);

}

void TestVisibility()
{
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  QGraphicsView* view = new QGraphicsView;
  const unsigned int radius = 5;
//  MovablePatch movablePatch(radius, renderer, view);

//  movablePatch.SetVisibility(false);
//  if(movablePatch.GetVisibility() != false)
//  {
//    throw std::runtime_error("Visibility set or retrieved incorrectly!");
//  }
}

void TestGetRegion()
{
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  QGraphicsView* view = new QGraphicsView;
  const unsigned int radius = 5;
//  MovablePatch movablePatch(radius, renderer, view);

//  itk::ImageRegion<2> retrievedRegion = movablePatch.GetRegion();
//  itk::Index<2> correctIndex;
//  correctIndex.Fill(0);
//  itk::Size<2> correctSize;
//  itk::ImageRegion<2> correctRegion(correctIndex, correctSize);

//  if(retrievedRegion != correctRegion)
//  {
//    std::stringstream ss;
//    ss << "Region retrieved as " << retrievedRegion << " but is supposed to be " << correctRegion;
//    throw std::runtime_error(ss.str());
//  }
}
