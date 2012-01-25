#ifndef LessThanFunctor_HPP
#define LessThanFunctor_HPP

template <typename T>
struct LessThanFunctor
{
  bool operator()(const T& a, const T& b)
  {
    return a < b;
  }
};

#endif
