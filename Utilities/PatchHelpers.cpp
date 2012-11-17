#include "PatchHelpers.h"

namespace PatchHelpers
{
  bool CheckSurroundingRegionsOfAllHolePixels(const Mask* const mask, const unsigned int patchRadius)
  {
    std::vector<itk::Index<2> > maskPixels = mask->GetHolePixels();

    for(size_t i = 0; i < maskPixels.size(); ++i)
    {
      itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(maskPixels[i], patchRadius);
      if(!mask->GetLargestPossibleRegion().IsInside(region))
      {
        return false;
      }
    }

    return true;
  }

  std::vector<itk::Image<itk::CovariantVector<unsigned char, 3>, 2>::Pointer>
  ReadTopPatchesGrid(const std::string& fileName, const unsigned int patchSideLength,
                     const unsigned int gridWidth, const unsigned int gridHeight)
  {
    itk::Size<2> patchSize = {{patchSideLength, patchSideLength}};
    itk::Index<2> corner = {{0,0}};
    itk::ImageRegion<2> fullPatchRegion(corner, patchSize);

    // Make space for all the patches and a colored line dividing them (and the -1 is so there is no dividing line at the bottom)
    unsigned int padding = 4;

    typedef itk::Image<itk::CovariantVector<unsigned char, 3>, 2> ImageType;
    typedef itk::ImageFileReader<ImageType> ReaderType;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(fileName);
    reader->Update();

    ImageType* image = reader->GetOutput();

    std::vector<itk::Image<itk::CovariantVector<unsigned char, 3>, 2>::Pointer> patches;

    for(unsigned int gridRow = 0; gridRow < gridHeight; ++gridRow)
    {
      for(unsigned int gridColumn = 0; gridColumn < gridWidth; ++gridColumn)
      {
        ImageType::Pointer patchImage = ImageType::New();
        patchImage->SetRegions(fullPatchRegion);
        patchImage->Allocate();

        // The extra + currentPatchId * padding is to skip the padding
        int xPos = static_cast<itk::Index<2>::IndexValueType>((patchSideLength + padding) * gridColumn);
        int yPos = static_cast<itk::Index<2>::IndexValueType>((patchSideLength + padding) * gridRow);
        itk::Index<2> patchCorner = {{xPos, yPos}};
        itk::ImageRegion<2> currentPatchRegion(patchCorner, patchSize);

        ITKHelpers::CopyRegion(image, patchImage.GetPointer(), currentPatchRegion, patchImage->GetLargestPossibleRegion());

        patches.push_back(patchImage);
      }
    }

    return patches;
  }

} // end PatchHelpers namespace
