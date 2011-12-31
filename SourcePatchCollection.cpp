#include "SourcePatchCollection.h"

#include "Helpers.h"

SourcePatchCollection::SourcePatchCollection(Mask* const maskImage, const unsigned int patchRadius) : MaskImage(maskImage), PatchRadius(patchRadius)
{

}

std::vector<Patch*> SourcePatchCollection::GetSourcePatches()
{
  return this->SourcePatches;
}

bool SourcePatchCollection::PatchExists(const itk::ImageRegion<2>& region)
{
  for(unsigned int patchId = 0; patchId < this->SourcePatches.size(); ++patchId)
    {
    if(this->SourcePatches[patchId]->GetRegion() == region)
      {
      return true;
      }
    }
  return false;
}

bool SourcePatchCollection::PatchExists(const Patch* const patch)
{
  for(unsigned int patchId = 0; patchId < this->SourcePatches.size(); ++patchId)
    {
    if(this->SourcePatches[patchId] == patch)
      {
      return true;
      }
    }
  return false;
}

void SourcePatchCollection::Clear()
{
  this->SourcePatches.clear();
}

std::vector<Patch> SourcePatchCollection::AddNewSourcePatchesInRegion(const itk::ImageRegion<2>& region)
{
  // Add all patches in 'region' that are entirely valid and are not already in the source patch list to the list of source patches.
  // Additionally, return the patches that were added.

  std::vector<Patch> newPatches;
  try
  {
    // Clearly we cannot add source patches from regions that are outside the image.
    itk::ImageRegion<2> newRegion = Helpers::CropToRegion(region, this->MaskImage->GetLargestPossibleRegion());

    itk::ImageRegionConstIterator<Mask> iterator(this->MaskImage, newRegion);

    while(!iterator.IsAtEnd())
      {
      itk::Index<2> currentPixel = iterator.GetIndex();
      itk::ImageRegion<2> currentPatchRegion = Helpers::GetRegionInRadiusAroundPixel(currentPixel, this->PatchRadius);

      if(this->MaskImage->GetLargestPossibleRegion().IsInside(currentPatchRegion))
        {
        if(this->MaskImage->IsValid(currentPatchRegion))
          {
          if(!PatchExists(currentPatchRegion))
            {
            //this->SourcePatches.push_back(Patch(this->OriginalImage, region));
            this->SourcePatches.push_back(new Patch(currentPatchRegion));
            newPatches.push_back(Patch(currentPatchRegion));
            //DebugMessage("Added a source patch.");
            }
          }
        }

      ++iterator;
      }
    //DebugMessage<unsigned int>("Number of new patches: ", newPatches.size());
    //DebugMessage<unsigned int>("Number of source patches: ", this->SourcePatches.size());

    if(this->SourcePatches.size() == 0)
      {
      std::cerr << "There must be at least 1 source patch!" << std::endl;
      exit(-1);
      }

    return newPatches;
  }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeSourcePatches!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }

}

void SourcePatchCollection::AddAllSourcePatchesInRegion(const itk::ImageRegion<2>& region)
{
  // Add all patches in 'region' that are entirely valid to the list of source patches.
  try
  {
    // Clearly we cannot add source patches from regions that are outside the image, so crop the desired region to be inside the image.
    itk::ImageRegion<2> newRegion = Helpers::CropToRegion(region, this->MaskImage->GetLargestPossibleRegion());

    itk::ImageRegionConstIterator<Mask> iterator(this->MaskImage, newRegion);

    while(!iterator.IsAtEnd())
      {
      itk::Index<2> currentPixel = iterator.GetIndex();
      itk::ImageRegion<2> currentPatchRegion = Helpers::GetRegionInRadiusAroundPixel(currentPixel, this->PatchRadius);

      if(this->MaskImage->GetLargestPossibleRegion().IsInside(currentPatchRegion))
        {
        if(this->MaskImage->IsValid(currentPatchRegion))
          {
          this->SourcePatches.push_back(new Patch(currentPatchRegion));
          }
        }

      ++iterator;
      }

    //DebugMessage<unsigned int>("Number of source patches: ", this->SourcePatches.size());
  }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in AddAllSourcePatchesInRegion!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }

}
