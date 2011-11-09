template<typename TDifferenceFunction>
float SelfPatchCompare::PatchAverageSourceDifference(const Patch& sourcePatch)
{
  float totalDifference = 0.0f;
  
  FloatVectorImageType::InternalPixelType *buffptr = this->Image->GetBufferPointer();
  unsigned int offsetDifference = (this->Image->ComputeOffset(this->Pairs->TargetPatch.Region.GetIndex())
                                   - this->Image->ComputeOffset(sourcePatch.Region.GetIndex())) * this->NumberOfComponentsPerPixel;

  FloatVectorImageType::PixelType sourcePixel;
  sourcePixel.SetSize(this->NumberOfComponentsPerPixel);
  
  FloatVectorImageType::PixelType targetPixel;
  targetPixel.SetSize(this->NumberOfComponentsPerPixel);
  
  FloatVectorImageType::PixelType differencePixel;
  differencePixel.SetSize(this->NumberOfComponentsPerPixel);
  
  //TDifferenceFunction differenceFunction(sourcePixel);
  TDifferenceFunction differenceFunction(this->NumberOfComponentsPerPixel);
  float difference = 0;
  for(unsigned int pixelId = 0; pixelId < this->ValidOffsets.size(); ++pixelId)
    {
    
    for(unsigned int i = 0; i < this->NumberOfComponentsPerPixel; ++i)
      {
      sourcePixel[i] = buffptr[this->ValidOffsets[pixelId] + i];
      targetPixel[i] = buffptr[this->ValidOffsets[pixelId] - offsetDifference + i];
      }
  
    difference = differenceFunction.Difference(sourcePixel, targetPixel);

    totalDifference += difference;
    }

  float averageDifference = totalDifference/static_cast<float>(this->ValidOffsets.size());
  return averageDifference;
}
