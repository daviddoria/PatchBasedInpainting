#include "SourcePatchCollection.h"

#include "Helpers.h"

SourcePatchCollection::SourcePatchCollection(Mask* const maskImage, const unsigned int patchRadius) : MaskImage(maskImage), PatchRadius(patchRadius)
{

}


void SourcePatchCollection::Clear()
{
  this->SourcePatches.clear();
}

SourcePatchCollection::iterator SourcePatchCollection::begin() const
{
  return SourcePatches.begin();
}

SourcePatchCollection::iterator SourcePatchCollection::end() const
{
  return SourcePatches.end();
}

std::set<Patch> SourcePatchCollection::AddSourcePatchesInRegion(const itk::ImageRegion<2>& region)
{
  // Add all patches in 'region' that are entirely valid to the list of source patches.
  try
  {
    std::set<Patch> oldPatchSet = this->SourcePatches;
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
          this->SourcePatches.insert(Patch(currentPatchRegion));
          }
        }

      ++iterator;
      }
    std::set<Patch> newlyAddedPatches;
    std::set_symmetric_difference(oldPatchSet.begin(), oldPatchSet.end(),
                                  this->SourcePatches.begin(), this->SourcePatches.end(),
                                  inserter(newlyAddedPatches, newlyAddedPatches.begin()));
    return newlyAddedPatches;
    //DebugMessage<unsigned int>("Number of source patches: ", this->SourcePatches.size());
  }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in AddAllSourcePatchesInRegion!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }

}
