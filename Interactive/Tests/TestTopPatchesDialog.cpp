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

#include "Interactive/TopPatchesDialog.h"

#include "itkVectorImage.h"

int main(int argc, char*argv[])
{
  typedef itk::VectorImage<float, 2> ImageType;
  itk::Index<2> corner = {{0,0}};
  itk::Size<2> imageSize = {{100,100}};
  itk::ImageRegion<2> region(corner, imageSize);

  ImageType::Pointer image = ImageType::New();
  image->SetNumberOfComponentsPerPixel(3);
  image->SetRegions(region);
  image->Allocate();

  QApplication app( argc, argv );

  const unsigned int patchHalfWidth = 5;
  itk::Size<2> regionSize = {{patchHalfWidth * 2 + 1, patchHalfWidth * 2 + 1}};

  Node node0(10,10);
  itk::Index<2> corner0 = {{10,10}};
  itk::ImageRegion<2> region0(corner0, regionSize);

  Node node1(20,20);
  itk::Index<2> corner1 = {{20,20}};
  itk::ImageRegion<2> region1(corner1, regionSize);

//   std::vector<itk::ImageRegion<2> > regions;
//   regions.push_back(region0);
//   regions.push_back(region1);

  std::vector<Node> nodes;
  nodes.push_back(node0);
  nodes.push_back(node1);

//   TopPatchesWidget<ImageType> topPatchesWidget(image);
//   topPatchesWidget.GetPatchesModel()->SetRegions(regions);
//   topPatchesWidget.show();

  Mask::Pointer mask = Mask::New();
  mask->SetRegions(region);
  mask->Allocate();
  
  TopPatchesDialog<ImageType> dialog(image, mask, patchHalfWidth);
  dialog.SetSourceNodes(nodes);
  dialog.exec();

  int result = dialog.result();
  std::cout << result << std::endl;
  if(result) // The user clicked 'ok'
    {
    //std::cout << dialog.GetUserText() << std::endl;
    }
  else
    {
    // The user clicked 'cancel' or closed the dialog
    std::cout << "invalid" << std::endl;
    }

  dialog.exec();
  return app.exec();
}
