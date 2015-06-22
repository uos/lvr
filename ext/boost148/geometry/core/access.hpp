// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2008-2011 Bruno Lalande, Paris, France.
// Copyright (c) 2008-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2009-2011 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_CORE_ACCESS_HPP
#define BOOST_GEOMETRY_CORE_ACCESS_HPP


#include <cstddef>

#include <boost/mpl/assert.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/concept_check.hpp>

#include <boost148/geometry/core/coordinate_type.hpp>
#include <boost148/geometry/core/point_type.hpp>
#include <boost148/geometry/core/tag.hpp>


namespace boost { namespace geometry
{

/// Index of minimum corner of the box.
int const min_corner = 0;

/// Index of maximum corner of the box.
int const max_corner = 1;

namespace traits
{

/*!
\brief Traits class which gives access (get,set) to points.
\ingroup traits
\par Geometries:
///     @li point
\par Specializations should provide, per Dimension
///     @li static inline T get(G const&)
///     @li static inline void set(G&, T const&)
\tparam Geometry geometry-type
\tparam Dimension dimension to access
*/
template <typename Geometry, std::size_t Dimension, typename Enable = void>
struct access
{
   BOOST_MPL_ASSERT_MSG
        (
            false, NOT_IMPLEMENTED_FOR_THIS_POINT_TYPE, (types<Geometry>)
        );
};


/*!
\brief Traits class defining "get" and "set" to get
    and set point coordinate values
\tparam Geometry geometry (box, segment)
\tparam Index index (min_corner/max_corner for box, 0/1 for segment)
\tparam Dimension dimension
\par Geometries:
    - box
    - segment
\par Specializations should provide:
    - static inline T get(G const&)
    - static inline void set(G&, T const&)
\ingroup traits
*/
template <typename Geometry, std::size_t Index, std::size_t Dimension>
struct indexed_access {};


} // namespace traits


#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

template
<
    typename Tag,
    typename Geometry,
    typename
    CoordinateType, std::size_t Dimension
>
struct access
{
    //static inline T get(G const&) {}
    //static inline void set(G& g, T const& value) {}
};

template
<
    typename Tag,
    typename Geometry,
    typename CoordinateType,
    std::size_t Index,
    std::size_t Dimension
>
struct indexed_access
{
    //static inline T get(G const&) {}
    //static inline void set(G& g, T const& value) {}
};

template <typename Point, typename CoordinateType, std::size_t Dimension>
struct access<point_tag, Point, CoordinateType, Dimension>
{
    static inline CoordinateType get(Point const& point)
    {
        return traits::access<Point, Dimension>::get(point);
    }
    static inline void set(Point& p, CoordinateType const& value)
    {
        traits::access<Point, Dimension>::set(p, value);
    }
};

template
<
    typename Box,
    typename CoordinateType,
    std::size_t Index,
    std::size_t Dimension
>
struct indexed_access<box_tag, Box, CoordinateType, Index, Dimension>
{
    static inline CoordinateType get(Box const& box)
    {
        return traits::indexed_access<Box, Index, Dimension>::get(box);
    }
    static inline void set(Box& b, CoordinateType const& value)
    {
        traits::indexed_access<Box, Index, Dimension>::set(b, value);
    }
};

template
<
    typename Segment,
    typename CoordinateType,
    std::size_t Index,
    std::size_t Dimension
>
struct indexed_access<segment_tag, Segment, CoordinateType, Index, Dimension>
{
    static inline CoordinateType get(Segment const& segment)
    {
        return traits::indexed_access<Segment, Index, Dimension>::get(segment);
    }
    static inline void set(Segment& segment, CoordinateType const& value)
    {
        traits::indexed_access<Segment, Index, Dimension>::set(segment, value);
    }
};

} // namespace core_dispatch
#endif // DOXYGEN_NO_DISPATCH


#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

// Two dummy tags to distinguish get/set variants below.
// They don't have to be specified by the user. The functions are distinguished
// by template signature also, but for e.g. GCC this is not enough. So give them
// a different signature.
struct signature_getset_dimension {};
struct signature_getset_index_dimension {};

} // namespace detail
#endif // DOXYGEN_NO_DETAIL


/*!
\brief Get coordinate value of a geometry (usually a point)
\details \details_get_set
\ingroup get
\tparam Dimension \tparam_dimension_required
\tparam Geometry \tparam_geometry (usually a Point Concept)
\param geometry \param_geometry (usually a point)
\param dummy \qbk_skip
\return The coordinate value of specified dimension of specified geometry
\qbk{[include reference/core/get_point.qbk]}
*/
template <std::size_t Dimension, typename Geometry>
inline typename coordinate_type<Geometry>::type get(Geometry const& geometry
        , detail::signature_getset_dimension* dummy = 0
        )
{
    boost::ignore_unused_variable_warning(dummy);

    typedef typename boost::remove_const<Geometry>::type ncg_type;

    typedef core_dispatch::access
        <
            typename tag<Geometry>::type,
            ncg_type,
            typename coordinate_type<ncg_type>::type,
            Dimension
        > coord_access_type;

    return coord_access_type::get(geometry);
}


/*!
\brief Set coordinate value of a geometry (usually a point)
\details \details_get_set
\tparam Dimension \tparam_dimension_required
\tparam Geometry \tparam_geometry (usually a Point Concept)
\param geometry geometry to assign coordinate to
\param geometry \param_geometry (usually a point)
\param value The coordinate value to set
\param dummy \qbk_skip
\ingroup set

\qbk{[include reference/core/set_point.qbk]}
*/
template <std::size_t Dimension, typename Geometry>
inline void set(Geometry& geometry
        , typename coordinate_type<Geometry>::type const& value
        , detail::signature_getset_dimension* dummy = 0
        )
{
    boost::ignore_unused_variable_warning(dummy);

    typedef typename boost::remove_const<Geometry>::type ncg_type;

    typedef core_dispatch::access
        <
            typename tag<Geometry>::type,
            ncg_type,
            typename coordinate_type<ncg_type>::type,
            Dimension
        > coord_access_type;

    coord_access_type::set(geometry, value);
}


/*!
\brief get coordinate value of a Box or Segment
\details \details_get_set
\tparam Index \tparam_index_required
\tparam Dimension \tparam_dimension_required
\tparam Geometry \tparam_box_or_segment
\param geometry \param_geometry
\param dummy \qbk_skip
\return coordinate value
\ingroup get

\qbk{distinguish,with index}
\qbk{[include reference/core/get_box.qbk]}
*/
template <std::size_t Index, std::size_t Dimension, typename Geometry>
inline typename coordinate_type<Geometry>::type get(Geometry const& geometry
        , detail::signature_getset_index_dimension* dummy = 0
        )
{
    boost::ignore_unused_variable_warning(dummy);

    typedef typename boost::remove_const<Geometry>::type ncg_type;

    typedef core_dispatch::indexed_access
        <
            typename tag<Geometry>::type,
            ncg_type,
            typename coordinate_type<ncg_type>::type,
            Index,
            Dimension
        > coord_access_type;

    return coord_access_type::get(geometry);
}

/*!
\brief set coordinate value of a Box / Segment
\details \details_get_set
\tparam Index \tparam_index_required
\tparam Dimension \tparam_dimension_required
\tparam Geometry \tparam_box_or_segment
\param geometry geometry to assign coordinate to
\param geometry \param_geometry
\param value The coordinate value to set
\param dummy \qbk_skip
\ingroup set

\qbk{distinguish,with index}
\qbk{[include reference/core/set_box.qbk]}
*/
template <std::size_t Index, std::size_t Dimension, typename Geometry>
inline void set(Geometry& geometry
        , typename coordinate_type<Geometry>::type const& value
        , detail::signature_getset_index_dimension* dummy = 0
        )
{
    boost::ignore_unused_variable_warning(dummy);

    typedef typename boost::remove_const<Geometry>::type ncg_type;

    typedef core_dispatch::indexed_access
        <
            typename tag<Geometry>::type, ncg_type,
            typename coordinate_type<ncg_type>::type,
            Index,
            Dimension
        > coord_access_type;

    coord_access_type::set(geometry, value);
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_CORE_ACCESS_HPP
