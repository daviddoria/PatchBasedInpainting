/**
 * \file vector_topology.hpp
 * 
 * This library provides classes that define a vector-topology. A vector-topology is 
 * a simple metric-space where the points are vector values.
 * 
 * \author Sven Mikael Persson <mikael.s.persson@gmail.com>
 * \date October 2011
 */

/*
 *    Copyright 2011 Sven Mikael Persson
 *
 *    THIS SOFTWARE IS DISTRIBUTED UNDER THE TERMS OF THE GNU GENERAL PUBLIC LICENSE v3 (GPLv3).
 *
 *    This file is part of ReaK.
 *
 *    ReaK is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    ReaK is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with ReaK (as LICENSE in the root folder).  
 *    If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef REAK_VECTOR_TOPOLOGY_HPP
#define REAK_VECTOR_TOPOLOGY_HPP


#include <boost/config.hpp> // For BOOST_STATIC_CONSTANT

#include <cmath>

/**
 * This class implements an infinite vector topology. Since the space is 
 * infinite, there is no way to generate random points from it, and thus, 
 * this class does not model the topology concepts, but defines a number 
 * of functions useful to a derived class that can provide the full 
 * model of a topology.
 * \tparam Vector The vector-type for the topology, should model an Arithmetic concept and possess a norm() function.
 */
template <typename Vector>
class vector_topology
{
  public:
    typedef vector_topology<Vector> self;
    
    typedef Vector point_type;
    typedef Vector point_difference_type;
    
    BOOST_STATIC_CONSTANT(std::size_t, dimensions = vect_traits<Vector>::dimensions);
    

    /**
     * Returns a point which is at a fraction between two points a to b.
     */
    point_type move_position_toward(const point_type& a, double fraction, const point_type& b) const 
    {
      return a + (b - a) * vect_traits<Vector>::value_type( fraction );
    }

    /**
     * Returns the difference between two points (a - b).
     */
    point_difference_type difference(const point_type& a, const point_type& b) const {
      return a - b;
    }

    /**
     * Returns the addition of a point-difference to a point.
     */
    point_type adjust(const point_type& a, const point_difference_type& delta) const {
      return a + delta;
    }

};

#endif
