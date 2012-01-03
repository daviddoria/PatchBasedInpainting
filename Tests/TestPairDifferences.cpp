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

int main(int argc, char*argv[])
{
  // Explicitly test the names.
  if(PairDifferences::NameOfDifference(PairDifferences::AverageAbsoluteDifference) != "Av.Abs.")
    {
    std::cerr << "AverageAbsoluteDifference name incorrect! Is " << PairDifferences::NameOfDifference(PairDifferences::AverageAbsoluteDifference)
              << " but should be 'Av.Abs.'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PairDifferences::NameOfDifference(PairDifferences::ColorDifference) != "Color")
    {
    std::cerr << "ColorDifference name incorrect! Is " << PairDifferences::NameOfDifference(PairDifferences::ColorDifference)
              << " but should be 'Color'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PairDifferences::NameOfDifference(PairDifferences::DepthDifference) != "Depth")
    {
    std::cerr << "DepthDifference name incorrect! Is " << PairDifferences::NameOfDifference(PairDifferences::DepthDifference)
              << " but should be 'Depth'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PairDifferences::NameOfDifference(PairDifferences::CombinedDifference) != "Combined")
    {
    std::cerr << "CombinedDifference name incorrect! Is " << PairDifferences::NameOfDifference(PairDifferences::CombinedDifference)
              << " but should be 'Combined'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PairDifferences::NameOfDifference(PairDifferences::MembershipDifference) != "Membership")
    {
    std::cerr << "MembershipDifference name incorrect! Is " << PairDifferences::NameOfDifference(PairDifferences::MembershipDifference)
              << " but should be 'Membership'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PairDifferences::NameOfDifference(PairDifferences::HistogramIntersection) != "Hist.Int.")
    {
    std::cerr << "HistogramIntersection name incorrect! Is " << PairDifferences::NameOfDifference(PairDifferences::HistogramIntersection)
              << " but should be 'Hist.Int.'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PairDifferences::NameOfDifference(PairDifferences::Invalid) != "INVALID")
    {
    std::cerr << "'Invalid' name incorrect! Is " << PairDifferences::NameOfDifference(PairDifferences::Invalid)
              << " but should be 'INVALID'" << std::endl;
    return EXIT_FAILURE;
    }

  // Explicitly test the types
  if(PairDifferences::TypeOfDifference("Av.Abs.") != PairDifferences::AverageAbsoluteDifference)
    {
    std::cerr << "Av.Abs. type incorrect! Is " << PairDifferences::TypeOfDifference("Av.Abs.")
              << " but should be 'Av.Abs.'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PairDifferences::TypeOfDifference("Color") != PairDifferences::ColorDifference)
    {
    std::cerr << "Color type incorrect! Is " << PairDifferences::TypeOfDifference("Color")
              << " but should be 'Color'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PairDifferences::TypeOfDifference("Depth") != PairDifferences::DepthDifference)
    {
    std::cerr << "DepthDifference type incorrect! Is " << PairDifferences::TypeOfDifference("Depth")
              << " but should be 'Depth'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PairDifferences::TypeOfDifference("Combined") != PairDifferences::CombinedDifference)
    {
    std::cerr << "CombinedDifference type incorrect! Is " << PairDifferences::TypeOfDifference("Combined")
              << " but should be 'Combined'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PairDifferences::TypeOfDifference("Membership") != PairDifferences::MembershipDifference)
    {
    std::cerr << "MembershipDifference type incorrect! Is " << PairDifferences::TypeOfDifference("Membership")
              << " but should be 'Membership'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PairDifferences::TypeOfDifference("Hist.Int.") != PairDifferences::HistogramIntersection)
    {
    std::cerr << "HistogramIntersection type incorrect! Is " << PairDifferences::TypeOfDifference("Hist.Int.")
              << " but should be 'Hist.Int.'" << std::endl;
    return EXIT_FAILURE;
    }

  if(PairDifferences::TypeOfDifference("INVALID") != PairDifferences::Invalid)
    {
    std::cerr << "'Invalid' type incorrect! Is " << PairDifferences::TypeOfDifference("INVALID")
              << " but should be 'INVALID'" << std::endl;
    return EXIT_FAILURE;
    }

  // Test the names and types by ensuring they are inverses
//   if(PairDifferences::NameOfDifference(PairDifferences::TypeOfDifference) != "Av.Abs.")
//     {
//     std::cerr << "AverageAbsoluteDifference name incorrect! Is " << PairDifferences::NameOfDifference(AverageAbsoluteDifference)
//               << " but should be 'Av. Abs.'" << std::endl;
//     return EXIT_FAILURE;
//     }
// 
//   if(PairDifferences::NameOfDifference(ColorDifference) != "Color")
//     {
//     std::cerr << "ColorDifference name incorrect! Is " << PairDifferences::NameOfDifference(ColorDifference)
//               << " but should be 'Color'" << std::endl;
//     return EXIT_FAILURE;
//     }
// 
//   if(PairDifferences::NameOfDifference(DepthDifference) != "Depth")
//     {
//     std::cerr << "DepthDifference name incorrect! Is " << PairDifferences::NameOfDifference(DepthDifference)
//               << " but should be 'Depth'" << std::endl;
//     return EXIT_FAILURE;
//     }
// 
//   if(PairDifferences::NameOfDifference(CombinedDifference) != "Combined")
//     {
//     std::cerr << "CombinedDifference name incorrect! Is " << PairDifferences::NameOfDifference(CombinedDifference)
//               << " but should be 'Combined'" << std::endl;
//     return EXIT_FAILURE;
//     }
// 
//   if(PairDifferences::NameOfDifference(MembershipDifference) != "Membership")
//     {
//     std::cerr << "MembershipDifference name incorrect! Is " << PairDifferences::NameOfDifference(MembershipDifference)
//               << " but should be 'Membership'" << std::endl;
//     return EXIT_FAILURE;
//     }
// 
//   if(PairDifferences::NameOfDifference(HistogramIntersection) != "Av.Abs.")
//     {
//     std::cerr << "HistogramIntersection name incorrect! Is " << PairDifferences::NameOfDifference(HistogramIntersection)
//               << " but should be 'Hist.Int.'" << std::endl;
//     return EXIT_FAILURE;
//     }
// 
//   if(PairDifferences::NameOfDifference(Invalid) != "INVALID")
//     {
//     std::cerr << "'Invalid' name incorrect! Is " << PairDifferences::NameOfDifference(Invalid)
//               << " but should be 'INVALID'" << std::endl;
//     return EXIT_FAILURE;
//     }
// 
//   if(PairDifferences::NameOfDifference(123) != "INVALID")
//     {
//     std::cerr << "'123' name incorrect! Is " << PairDifferences::NameOfDifference(123)
//               << " but should be 'INVALID'" << std::endl;
//     return EXIT_FAILURE;
//     }


  PairDifferences pairDifferences;
  if(pairDifferences.GetNumberOfDifferences() != 0)
    {
    std::cerr << "There should be 0 differences immediately after construction!" << std::endl;
    return EXIT_FAILURE;
    }

  pairDifferences.SetDifferenceByType(PairDifferences::AverageAbsoluteDifference, 1.0f);
  
  std::vector<std::string> differenceNames = pairDifferences.GetDifferenceNames();
  if(differenceNames.size() != 1)
    {
    std::cerr << "There should be 1 name, but there are " << differenceNames.size() << std::endl;
    return EXIT_FAILURE;
    }

  if(differenceNames[0] != "Av.Abs.")
    {
    std::cerr << "Difference name retrieved incorrectly! Is " << differenceNames[0]
              << " but should be 'Av.Abs.'" << std::endl;
    return EXIT_FAILURE;
    }

  if(pairDifferences.GetDifferenceByType(PairDifferences::AverageAbsoluteDifference) != 1.0f)
    {
    std::cerr << "Difference value retrieved incorrectly! Is " << pairDifferences.GetDifferenceByType(PairDifferences::AverageAbsoluteDifference)
              << " but should be 1.0" << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
