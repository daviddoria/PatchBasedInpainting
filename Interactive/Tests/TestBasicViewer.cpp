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

#include "BasicViewerWidget.h"

#include "itkImage.h"

#include <Mask/Mask.h>

#include <QApplication>

static void Scalar();
static void CovariantVector();
static void Vector();

int main(int argc, char*argv[])
{
  QApplication app(argc,argv);

  Scalar();
  CovariantVector();
  Vector();

  return app.exec();
}

void Scalar()
{
  itk::Index<2> corner = {{0,0}};
  itk::Size<2> size = {{2,2}};
  itk::ImageRegion<2> region(corner,size);

  typedef itk::Image<unsigned char, 2> ImageType;
  ImageType::Pointer image = ImageType::New();
  image->SetRegions(region);
  image->Allocate();

  Mask::Pointer mask = Mask::New();
  mask->SetRegions(region);
  mask->Allocate();

  typedef BasicViewerWidget<ImageType> BasicViewerWidgetType;
//  std::shared_ptr<BasicViewerWidgetType> basicViewer(new BasicViewerWidgetType(image.GetPointer(), mask));
  BasicViewerWidgetType* basicViewer = new BasicViewerWidgetType(image.GetPointer(), mask);
  basicViewer->show();
}

void CovariantVector()
{
  itk::Index<2> corner = {{0,0}};
  itk::Size<2> size = {{2,2}};
  itk::ImageRegion<2> region(corner,size);

  typedef itk::Image<itk::CovariantVector<unsigned char, 3>, 2> ImageType;
  ImageType::Pointer image = ImageType::New();
  image->SetRegions(region);
  image->Allocate();

  Mask::Pointer mask = Mask::New();
  mask->SetRegions(region);
  mask->Allocate();

  typedef BasicViewerWidget<ImageType> BasicViewerWidgetType;
//  std::shared_ptr<BasicViewerWidgetType> basicViewer(new BasicViewerWidgetType(image.GetPointer(), mask));
  BasicViewerWidgetType* basicViewer = new BasicViewerWidgetType(image.GetPointer(), mask);
  basicViewer->show();
}

void Vector()
{
  itk::Index<2> corner = {{0,0}};
  itk::Size<2> size = {{2,2}};
  itk::ImageRegion<2> region(corner,size);

  typedef itk::VectorImage<unsigned char, 2> ImageType;
  ImageType::Pointer image = ImageType::New();
  image->SetRegions(region);
  image->SetNumberOfComponentsPerPixel(3);
  image->Allocate();

  Mask::Pointer mask = Mask::New();
  mask->SetRegions(region);
  mask->Allocate();

  typedef BasicViewerWidget<ImageType> BasicViewerWidgetType;
//  std::shared_ptr<BasicViewerWidgetType> basicViewer(new BasicViewerWidgetType(image.GetPointer(), mask));
  BasicViewerWidgetType* basicViewer = new BasicViewerWidgetType(image.GetPointer(), mask);
  basicViewer->show();
}
