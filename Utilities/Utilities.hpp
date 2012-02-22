#ifndef Utilities_HPP
#define Utilities_HPP

template <typename TForwardIt>
bool try_advance(TForwardIt& it,
                 TForwardIt const end,
                 const unsigned int n)
{
  unsigned int i = 0;
  while (i < n && it != end)
  {
    ++i;
    ++it;
  }

  return i == n;
}

#endif
