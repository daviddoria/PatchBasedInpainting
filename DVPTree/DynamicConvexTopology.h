#ifndef DynamicConvexTopology_h
#define DynamicConvexTopology_h

class DynamicConvexTopology 
{
  public: // For VisualAge C++
  struct point 
  {
    //BOOST_STATIC_CONSTANT(std::size_t, dimensions = Dims);
    point() { }
    point(const unsigned int dims) { this->Dims = dims;}
    void SetDims(const unsigned int dims) { this->Dims = dims;}

    double& operator[](std::size_t i) {return values[i];}
    const double& operator[](std::size_t i) const {return values[i];}

  private:
    //double values[Dims];
    double* values;
    unsigned int Dims;
  };

  public: // For VisualAge C++
  struct point_difference
  {
    BOOST_STATIC_CONSTANT(std::size_t, dimensions = Dims);
    point_difference() {
      for (std::size_t i = 0; i < Dims; ++i) values[i] = 0.;
    }
    double& operator[](std::size_t i) {return values[i];}
    const double& operator[](std::size_t i) const {return values[i];}

    friend point_difference operator+(const point_difference& a, const point_difference& b) {
      point_difference result;
      for (std::size_t i = 0; i < Dims; ++i)
        result[i] = a[i] + b[i];
      return result;
    }

    friend point_difference& operator+=(point_difference& a, const point_difference& b) {
      for (std::size_t i = 0; i < Dims; ++i)
        a[i] += b[i];
      return a;
    }

    friend point_difference operator-(const point_difference& a) {
      point_difference result;
      for (std::size_t i = 0; i < Dims; ++i)
        result[i] = -a[i];
      return result;
    }

    friend point_difference operator-(const point_difference& a, const point_difference& b) {
      point_difference result;
      for (std::size_t i = 0; i < Dims; ++i)
        result[i] = a[i] - b[i];
      return result;
    }

    friend point_difference& operator-=(point_difference& a, const point_difference& b) {
      for (std::size_t i = 0; i < Dims; ++i)
        a[i] -= b[i];
      return a;
    }

    friend point_difference operator*(const point_difference& a, const point_difference& b) {
      point_difference result;
      for (std::size_t i = 0; i < Dims; ++i)
        result[i] = a[i] * b[i];
      return result;
    }

    friend point_difference operator*(const point_difference& a, double b) {
      point_difference result;
      for (std::size_t i = 0; i < Dims; ++i)
        result[i] = a[i] * b;
      return result;
    }

    friend point_difference operator*(double a, const point_difference& b) {
      point_difference result;
      for (std::size_t i = 0; i < Dims; ++i)
        result[i] = a * b[i];
      return result;
    }

    friend point_difference operator/(const point_difference& a, const point_difference& b) {
      point_difference result;
      for (std::size_t i = 0; i < Dims; ++i)
        result[i] = (b[i] == 0.) ? 0. : a[i] / b[i];
      return result;
    }

    friend double dot(const point_difference& a, const point_difference& b) {
      double result = 0;
      for (std::size_t i = 0; i < Dims; ++i)
        result += a[i] * b[i];
      return result;
    }

  private:
    double values[Dims];
  };

 public:
  typedef point point_type;
  typedef point_difference point_difference_type;

  double distance(point a, point b) const 
  {
    double dist = 0.;
    for (std::size_t i = 0; i < Dims; ++i) {
      double diff = b[i] - a[i];
      dist = boost::math::hypot(dist, diff);
    }
    // Exact properties of the distance are not important, as long as
    // < on what this returns matches real distances; l_2 is used because
    // Fruchterman-Reingold also uses this code and it relies on l_2.
    return dist;
  }

  point move_position_toward(point a, double fraction, point b) const 
  {
    point result;
    for (std::size_t i = 0; i < Dims; ++i)
      result[i] = a[i] + (b[i] - a[i]) * fraction;
    return result;
  }

  point_difference difference(point a, point b) const {
    point_difference result;
    for (std::size_t i = 0; i < Dims; ++i)
      result[i] = a[i] - b[i];
    return result;
  }

  point adjust(point a, point_difference delta) const {
    point result;
    for (std::size_t i = 0; i < Dims; ++i)
      result[i] = a[i] + delta[i];
    return result;
  }

  point pointwise_min(point a, point b) const {
    BOOST_USING_STD_MIN();
    point result;
    for (std::size_t i = 0; i < Dims; ++i)
      result[i] = min BOOST_PREVENT_MACRO_SUBSTITUTION (a[i], b[i]);
    return result;
  }

  point pointwise_max(point a, point b) const {
    BOOST_USING_STD_MAX();
    point result;
    for (std::size_t i = 0; i < Dims; ++i)
      result[i] = max BOOST_PREVENT_MACRO_SUBSTITUTION (a[i], b[i]);
    return result;
  }

  double norm(point_difference delta) const {
    double n = 0.;
    for (std::size_t i = 0; i < Dims; ++i)
      n = boost::math::hypot(n, delta[i]);
    return n;
  }

  double volume(point_difference delta) const {
    double n = 1.;
    for (std::size_t i = 0; i < Dims; ++i)
      n *= delta[i];
    return n;
  }

};

#endif
