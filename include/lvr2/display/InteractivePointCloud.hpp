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
 * InteractivePointCloud.hpp
 *
 *  Created on: 02.04.2012
 *      Author: Thomas Wiemann
 */

#ifndef INTERACTIVEPOINTCLOUD_HPP_
#define INTERACTIVEPOINTCLOUD_HPP_

#include <lvr2/display/Renderable.hpp>

#include <lvr2/io/Model.hpp>

namespace lvr2
{

class InteractivePointCloud : public Renderable
{
public:
	InteractivePointCloud();
	InteractivePointCloud(PointBuffer2Ptr buffer);
	virtual ~InteractivePointCloud();

	virtual void render();

	void updateBuffer(PointBuffer2Ptr buffer);


private:

	PointBuffer2Ptr			m_buffer;
};

} /* namespace lvr2 */

#include "InteractivePointCloud.cpp"

#endif /* INTERACTIVEPOINTCLOUD_HPP_ */
