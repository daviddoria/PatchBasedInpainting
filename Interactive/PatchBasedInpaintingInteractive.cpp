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

#include <QApplication>
#include <QCleanlooksStyle>

#include "PatchBasedInpaintingGUI.h"

#include "itkCommandLineArgumentParser.h"

int main( int argc, char** argv )
{
  QApplication app( argc, argv );

  //QApplication::setStyle(new QCleanlooksStyle);

  PatchBasedInpaintingGUI* patchBasedInpaintingGUI;
  if(argc >= 3)
    {
    std::cout << "Using filename arguments." << std::endl;

    itk::CommandLineArgumentParser::Pointer parser = itk::CommandLineArgumentParser::New();
    parser->SetCommandLineArguments( argc, argv );

    std::string imageFileName;
    bool imageFileNameProvided = parser->GetCommandLineArgument( "-image", imageFileName);

    std::string maskFileName;
    bool maskFileNameProvided = parser->GetCommandLineArgument( "-mask", maskFileName );

    if(!imageFileNameProvided || !maskFileNameProvided)
      {
      std::cerr << "Must provide at least an image and a mask!" << std::endl;
      exit(-1);
      }

    bool enterLeave;
    bool enterLeaveProvided = parser->GetCommandLineArgument( "-enterLeave", enterLeave );
    if(!enterLeaveProvided)
      {
      enterLeave = false;
      }
    patchBasedInpaintingGUI = new PatchBasedInpaintingGUI(imageFileName, maskFileName, enterLeave);
    
    std::string blurredFileName;
    bool blurredFileNameProvided = parser->GetCommandLineArgument( "-blurredImage", blurredFileName );
    if(blurredFileNameProvided)
      {
      patchBasedInpaintingGUI->txtBlurredImage->setText(blurredFileName.c_str());
      }

    std::string membershipFileName;
    bool membershipFileNameProvided = parser->GetCommandLineArgument( "-membershipImage", membershipFileName );
    if(membershipFileNameProvided)
      {
      patchBasedInpaintingGUI->txtMembershipImage->setText(membershipFileName.c_str());
      }

    }
  else
    {
    //std::cout << "Not using filename arguments." << std::endl;
    patchBasedInpaintingGUI = new PatchBasedInpaintingGUI;
    }
  
  //patchBasedInpaintingGUI->SetDebugFunctionEnterLeave(true); // This should be set in the constructor instead
  //patchBasedInpaintingGUI->SetDebugMessages(true);
  patchBasedInpaintingGUI->showMaximized();
  //patchBasedInpaintingGUI->show();

  return app.exec();
}
