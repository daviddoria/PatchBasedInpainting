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

  QScopedPointer<PatchBasedInpaintingGUI> patchBasedInpaintingGUI;

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
      throw std::runtime_error("Must provide at least an image and a mask!");
      }

    bool enterLeave;
    bool enterLeaveProvided = parser->GetCommandLineArgument( "-enterLeave", enterLeave );
    if(!enterLeaveProvided)
      {
      enterLeave = false;
      }
    patchBasedInpaintingGUI.reset(new PatchBasedInpaintingGUI(imageFileName, maskFileName, enterLeave));

    std::vector<std::string> extraFileNames;
    bool extraFileNamesProvided = parser->GetCommandLineArgument( "-extraImages", extraFileNames);

    if(extraFileNamesProvided)
      {
      if(extraFileNames.size() % 2 != 0)
        {
        std::cerr << "There must be an even number of values for the 'extraImages' key (name, file, name, file....)" << std::endl;
        return EXIT_FAILURE;
        }
      std::cout << "extraFileNames has " << extraFileNames.size() << " values." << std::endl;
      for(unsigned int i = 0; i < extraFileNames.size(); i += 2)
        {
        std::cout << "Extra file name value " << i << " " << extraFileNames[i] << " " << i + 1 << " " << extraFileNames[i + 1] << std::endl;
        ImageInput imageInput(extraFileNames[i].c_str(), extraFileNames[i + 1].c_str());
        patchBasedInpaintingGUI->AddImageInput(imageInput);
        }
      }
    }
  else
    {
    //std::cout << "Not using filename arguments." << std::endl;
    patchBasedInpaintingGUI.reset(new PatchBasedInpaintingGUI);
    }

  //patchBasedInpaintingGUI->SetDebugFunctionEnterLeave(true); // This should be set in the constructor instead
  //patchBasedInpaintingGUI->SetDebugMessages(true);
  patchBasedInpaintingGUI->showMaximized();
  //patchBasedInpaintingGUI->show();

  return app.exec();
}
