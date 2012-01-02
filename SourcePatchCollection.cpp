#include "SourcePatchCollection.h"

#include "Helpers.h"

SourcePatchCollection::SourcePatchCollection(Mask* const maskImage, const unsigned int patchRadius) : MaskImage(maskImage), PatchRadius(patchRadius)
{

}


void SourcePatchCollection::Clear()
{
  this->SourcePatches.clear();
}

SourcePatchCollection::Iterator SourcePatchCollection::begin() const
{
  return SourcePatches.begin();
}

SourcePatchCollection::Iterator SourcePatchCollection::end() const
{
  return SourcePatches.end();
}

const Patch* SourcePatchCollection::GetPatch(const itk::ImageRegion<2>& region)
{
  for(Iterator iterator = this->SourcePatches.begin(); iterator != this->SourcePatches.end(); ++iterator)
    {
    if((*iterator).GetRegion() == region)
      {
      return &(*iterator);
      }
    }
  assert("Requested a patch that is not in the collection!" && 0); // Should never get here.
  return NULL;
}

SourcePatchCollection::PatchContainer SourcePatchCollection::FindSourcePatchesInRegion(const itk::ImageRegion<2>& region)
{
  // Add all patches in 'region' that are entirely valid to the list of source patches.
  try
  {
    PatchContainer patches;
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
          patches.insert(Patch(currentPatchRegion));
          }
        }

      ++iterator;
      }
    return patches;
    //DebugMessage<unsigned int>("Number of source patches: ", this->SourcePatches.size());
  }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in AddAllSourcePatchesInRegion!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void SourcePatchCollection::AddPatches(const SourcePatchCollection::PatchContainer& patches)
{
  std::set_union(this->SourcePatches.begin(), this->SourcePatches.end(), patches.begin(), patches.end(),
                 std::inserter<PatchContainer>(this->SourcePatches, this->SourcePatches.end()));
}
