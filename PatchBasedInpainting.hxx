template <typename T>
void PatchBasedInpainting::SetPriorityFunction()
{
  this->PriorityFunction = new T(this->CurrentOutputImage, this->MaskImage, this->PatchRadius[0]);
}
