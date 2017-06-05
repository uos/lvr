/* Copyright (C) 2011 Uni Osnabrück
 * This file is part of the LAS VEGAS Reconstruction Toolkit,
 *
 * LAS VEGAS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * LAS VEGAS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */


/*
 * Vector.hpp
 *
 *  @date 02.06.2017
 *  @author Lukas Kalbertodt <lukas.kalbertodt@gmail.com>
 */

#ifndef LVR2_GEOMETRY_VECTOR_H_
#define LVR2_GEOMETRY_VECTOR_H_


namespace lvr2
{

template <typename> struct Point;

template <typename BaseVecT>
struct Vector : public BaseVecT
{
    Vector() {}
    Vector(BaseVecT base) : BaseVecT(base) {}

    // It doesn't make sense to talk about the distance between two direction
    // vectors. It's the same as asking: "What is the distance between
    // '3 meters north' and '10 cm east'.
    typename BaseVecT::CoordType distance(const BaseVecT &other) const = delete;
    typename BaseVecT::CoordType distance2(const BaseVecT &other) const = delete;

    // The standard operators are deleted and replaced by strongly typed ones.
    BaseVecT operator+(const BaseVecT &other) const = delete;
    BaseVecT operator-(const BaseVecT &other) const = delete;
    BaseVecT& operator-=(const BaseVecT &other) = delete;
    BaseVecT& operator+=(const BaseVecT &other) = delete;

    // Addition/subtraction between two vectors
    Vector<BaseVecT> operator+(const Vector<BaseVecT> &other) const;
    Vector<BaseVecT> operator-(const Vector<BaseVecT> &other) const;
    Vector<BaseVecT>& operator+=(const Vector<BaseVecT> &other);
    Vector<BaseVecT>& operator-=(const Vector<BaseVecT> &other);

    // Addition/subtraction between point and vector
    Point<BaseVecT> operator+(const Point<BaseVecT> &other) const;
    Point<BaseVecT> operator-(const Point<BaseVecT> &other) const;
};

} // namespace lvr

#include "Vector.tcc"

#endif /* LVR2_GEOMETRY_VECTOR_H_ */
