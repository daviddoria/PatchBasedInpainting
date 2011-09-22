
template <typename T>
void CriminisiInpainting::DebugMessage(const std::string& message, const T value)
{
  if(this->DebugMessages)
    {
    std::stringstream ss;
    ss << value;
    std::cout << message << " " << ss.str() << std::endl;
    }
}

template<typename TImage>
void CriminisiInpainting::InitializeImage(typename TImage::Pointer image)
{
  image->SetRegions(this->FullImageRegion);
  image->Allocate();
  image->FillBuffer(0);
}
