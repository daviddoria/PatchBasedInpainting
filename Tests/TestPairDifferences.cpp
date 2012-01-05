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

#include "PairDifferences.h"

#include <iostream>
#include <stdexcept>

int main(int argc, char*argv[])
{
  // Explicitly test the names.
  if(PairDifferences::NameOfDifference(PairDifferences::AveragePixelDifference) != "AveragePixelDifference")
    {
    std::cerr << "AveragePixelDifference name incorrect! Is " << PairDifferences::NameOfDifference(PairDifferences::AveragePixelDifference)
              << " but should be 'AveragePixelDifference'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PairDifferences::NameOfDifference(PairDifferences::SumPixelDifference) != "SumPixelDifference")
    {
    std::cerr << "SumPixelDifference name incorrect! Is " << PairDifferences::NameOfDifference(PairDifferences::SumPixelDifference)
              << " but should be 'SumPixelDifference'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PairDifferences::NameOfDifference(PairDifferences::ColorDifference) != "ColorDifference")
    {
    std::cerr << "ColorDifference name incorrect! Is " << PairDifferences::NameOfDifference(PairDifferences::ColorDifference)
              << " but should be 'Color'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PairDifferences::NameOfDifference(PairDifferences::DepthDifference) != "DepthDifference")
    {
    std::cerr << "DepthDifference name incorrect! Is " << PairDifferences::NameOfDifference(PairDifferences::DepthDifference)
              << " but should be 'Depth'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PairDifferences::NameOfDifference(PairDifferences::Invalid) != "Invalid")
    {
    std::cerr << "'Invalid' name incorrect! Is " << PairDifferences::NameOfDifference(PairDifferences::Invalid)
              << " but should be 'INVALID'" << std::endl;
    return EXIT_FAILURE;
    }

  // Explicitly test the types
  if(PairDifferences::TypeOfDifference("AveragePixelDifference") != PairDifferences::AveragePixelDifference)
    {
    std::cerr << "AveragePixelDifference type incorrect! Is " << PairDifferences::TypeOfDifference("AveragePixelDifference")
              << " but should be 'AveragePixelDifference'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PairDifferences::TypeOfDifference("SumPixelDifference") != PairDifferences::SumPixelDifference)
    {
    std::cerr << "SumPixelDifference type incorrect! Is " << PairDifferences::TypeOfDifference("SumPixelDifference")
              << " but should be 'SumPixelDifference'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PairDifferences::TypeOfDifference("ColorDifference") != PairDifferences::ColorDifference)
    {
    std::cerr << "Color type incorrect! Is " << PairDifferences::TypeOfDifference("ColorDifference")
              << " but should be 'ColorDifference'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PairDifferences::TypeOfDifference("DepthDifference") != PairDifferences::DepthDifference)
    {
    std::cerr << "DepthDifference type incorrect! Is " << PairDifferences::TypeOfDifference("DepthDifference")
              << " but should be 'DepthDifference'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PairDifferences::TypeOfDifference("INVALID") != PairDifferences::Invalid)
    {
    std::cerr << "'Invalid' type incorrect! Is " << PairDifferences::TypeOfDifference("INVALID")
              << " but should be " << PairDifferences::Invalid << std::endl;
    return EXIT_FAILURE;
    }

  // Test the names and types by ensuring they are inverses
  if(PairDifferences::NameOfDifference(PairDifferences::TypeOfDifference("TotalPixelDifference")) != "TotalPixelDifference")
    {
    std::runtime_error("TotalPixelDifference is not invertible!");
    }

  if(PairDifferences::NameOfDifference(PairDifferences::TypeOfDifference("AveragePixelDifference")) != "AveragePixelDifference")
    {
    std::runtime_error("AveragePixelDifference is not invertible!");
    }

  if(PairDifferences::NameOfDifference(PairDifferences::TypeOfDifference("ColorDifference")) != "ColorDifference")
    {
    std::runtime_error("ColorDifference is not invertible!");
    }

  if(PairDifferences::NameOfDifference(PairDifferences::TypeOfDifference("DepthDifference")) != "DepthDifference")
    {
    std::runtime_error("DepthDifference is not invertible!");
    }

  // Test other functions

  PairDifferences pairDifferences;
  if(pairDifferences.GetNumberOfDifferences() != 0)
    {
    std::cerr << "There should be 0 differences immediately after construction!" << std::endl;
    return EXIT_FAILURE;
    }

  pairDifferences.SetDifferenceByType(PairDifferences::AveragePixelDifference, 1.0f);
  
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

  if(pairDifferences.GetDifferenceByType(PairDifferences::AveragePixelDifference) != 1.0f)
    {
    std::cerr << "Difference value retrieved incorrectly! Is " << pairDifferences.GetDifferenceByType(PairDifferences::AveragePixelDifference)
              << " but should be 1.0" << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
