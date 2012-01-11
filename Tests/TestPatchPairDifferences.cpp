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

#include "PatchPairDifferences.h"

#include <iostream>
#include <stdexcept>

int main(int argc, char*argv[])
{
  // Explicitly test the names.
  if(PatchPairDifferences::NameOfDifference(PatchPairDifferences::AveragePixelDifference) != "AveragePixelDifference")
    {
    std::cerr << "AveragePixelDifference name incorrect! Is " << PatchPairDifferences::NameOfDifference(PatchPairDifferences::AveragePixelDifference)
              << " but should be 'AveragePixelDifference'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PatchPairDifferences::NameOfDifference(PatchPairDifferences::SumPixelDifference) != "SumPixelDifference")
    {
    std::cerr << "SumPixelDifference name incorrect! Is " << PatchPairDifferences::NameOfDifference(PatchPairDifferences::SumPixelDifference)
              << " but should be 'SumPixelDifference'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PatchPairDifferences::NameOfDifference(PatchPairDifferences::ColorDifference) != "ColorDifference")
    {
    std::cerr << "ColorDifference name incorrect! Is " << PatchPairDifferences::NameOfDifference(PatchPairDifferences::ColorDifference)
              << " but should be 'Color'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PatchPairDifferences::NameOfDifference(PatchPairDifferences::DepthDifference) != "DepthDifference")
    {
    std::cerr << "DepthDifference name incorrect! Is " << PatchPairDifferences::NameOfDifference(PatchPairDifferences::DepthDifference)
              << " but should be 'Depth'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PatchPairDifferences::NameOfDifference(PatchPairDifferences::Invalid) != "Invalid")
    {
    std::cerr << "'Invalid' name incorrect! Is " << PatchPairDifferences::NameOfDifference(PatchPairDifferences::Invalid)
              << " but should be 'INVALID'" << std::endl;
    return EXIT_FAILURE;
    }

  // Explicitly test the types
  if(PatchPairDifferences::TypeOfDifference("AveragePixelDifference") != PatchPairDifferences::AveragePixelDifference)
    {
    std::cerr << "AveragePixelDifference type incorrect! Is " << PatchPairDifferences::TypeOfDifference("AveragePixelDifference")
              << " but should be 'AveragePixelDifference'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PatchPairDifferences::TypeOfDifference("SumPixelDifference") != PatchPairDifferences::SumPixelDifference)
    {
    std::cerr << "SumPixelDifference type incorrect! Is " << PatchPairDifferences::TypeOfDifference("SumPixelDifference")
              << " but should be 'SumPixelDifference'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PatchPairDifferences::TypeOfDifference("ColorDifference") != PatchPairDifferences::ColorDifference)
    {
    std::cerr << "Color type incorrect! Is " << PatchPairDifferences::TypeOfDifference("ColorDifference")
              << " but should be 'ColorDifference'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PatchPairDifferences::TypeOfDifference("DepthDifference") != PatchPairDifferences::DepthDifference)
    {
    std::cerr << "DepthDifference type incorrect! Is " << PatchPairDifferences::TypeOfDifference("DepthDifference")
              << " but should be 'DepthDifference'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PatchPairDifferences::TypeOfDifference("INVALID") != PatchPairDifferences::Invalid)
    {
    std::cerr << "'Invalid' type incorrect! Is " << PatchPairDifferences::TypeOfDifference("INVALID")
              << " but should be " << PatchPairDifferences::Invalid << std::endl;
    return EXIT_FAILURE;
    }

  // Test the names and types by ensuring they are inverses
  if(PatchPairDifferences::NameOfDifference(PatchPairDifferences::TypeOfDifference("TotalPixelDifference")) != "TotalPixelDifference")
    {
    std::runtime_error("TotalPixelDifference is not invertible!");
    }

  if(PatchPairDifferences::NameOfDifference(PatchPairDifferences::TypeOfDifference("AveragePixelDifference")) != "AveragePixelDifference")
    {
    std::runtime_error("AveragePixelDifference is not invertible!");
    }

  if(PatchPairDifferences::NameOfDifference(PatchPairDifferences::TypeOfDifference("ColorDifference")) != "ColorDifference")
    {
    std::runtime_error("ColorDifference is not invertible!");
    }

  if(PatchPairDifferences::NameOfDifference(PatchPairDifferences::TypeOfDifference("DepthDifference")) != "DepthDifference")
    {
    std::runtime_error("DepthDifference is not invertible!");
    }

  // Test other functions

  PatchPairDifferences pairDifferences;
  if(pairDifferences.GetNumberOfDifferences() != 0)
    {
    std::cerr << "There should be 0 differences immediately after construction!" << std::endl;
    return EXIT_FAILURE;
    }

  pairDifferences.SetDifferenceByType(PatchPairDifferences::AveragePixelDifference, 1.0f);
  
  std::vector<std::string> differenceNames = pairDifferences.GetDifferenceNames();
  if(differenceNames.size() != 1)
    {
    std::cerr << "There should be 1 name, but there are " << differenceNames.size() << std::endl;
    return EXIT_FAILURE;
    }

  if(differenceNames[0] != "AveragePixelDifference")
    {
    std::cerr << "Difference name retrieved incorrectly! Is " << differenceNames[0]
              << " but should be 'AveragePixelDifference'" << std::endl;
    return EXIT_FAILURE;
    }

  if(pairDifferences.GetDifferenceByType(PatchPairDifferences::AveragePixelDifference) != 1.0f)
    {
    std::cerr << "Difference value retrieved incorrectly! Is " << pairDifferences.GetDifferenceByType(PatchPairDifferences::AveragePixelDifference)
              << " but should be 1.0" << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
