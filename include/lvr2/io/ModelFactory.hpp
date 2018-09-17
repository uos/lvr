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


 /**
 * IOFactory.h
 *
 *  @date 24.08.2011
 *  @author Thomas Wiemann
 */

#ifndef IOFACTORY_H_
#define IOFACTORY_H_

#include <lvr2/io/Model.hpp>

#include <string>
#include <vector>
#include "boost/shared_ptr.hpp"
#include <map>

namespace lvr2
{

/**
 * @brief Struct to define coordinate transformations
 */
struct CoordinateTransform
{
	// x scaling
	float sx;

	// y scaling
	float sy;

	// z scaling
	float sz;

	// Position of the x coordinate in the input data
	int	  x;

	// Position of the x coordinate in the input data
	int   y;

	// Position of the x coordinate in the input data
	int   z;

	// True, if conversion is necessary
	bool  convert;

	CoordinateTransform()
		: sx(1.0), sy(1.0), sz(1.0), x(0), y(1), z(2), convert(false) {}
};

/**
 * @brief Factory class extract point cloud and mesh information
 *        from supported file formats. The instantiated MeshLoader
 *        and PointLoader instances are persistent, i.e. they will
 *        not be freed in the destructor of this class to prevent
 *        side effects.
 */
class ModelFactory
{
    public:

        static ModelPtr readModel( std::string filename );

        static void saveModel( ModelPtr m, std::string file);

        static CoordinateTransform m_transform;

};

typedef boost::shared_ptr<ModelFactory> ModelFactoryPtr;

} // namespace lvr2

#include "ModelFactory.cpp"

#endif /* IOFACTORY_H_ */
