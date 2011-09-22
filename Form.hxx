
template <typename T>
void Form::DebugMessage(const std::string& message, const T value)
{
  if(this->DebugMessages)
    {
    std::stringstream ss;
    ss << value;
    std::cout << message << " " << ss.str() << std::endl;
    }
}

