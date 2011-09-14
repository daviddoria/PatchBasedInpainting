
template <typename T>
void CriminisiInpainting::DebugMessage(const std::string& message, T value)
{
  if(this->DebugMessages)
    {
    std::stringstream ss;
    ss << value;
    std::cout << message << " " << ss.str() << std::endl;
    }
}
