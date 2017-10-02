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
 * Grid.hpp
 *
 *  @date 10.01.2012
 *  @author Thomas Wiemann
 */

#ifndef GRID_HPP_
#define GRID_HPP_

#include "Renderable.hpp"
#include <lvr/io/DataStruct.hpp>

namespace lvr
{

class Grid : public Renderable
{
public:
    Grid(floatArr vertices, uintArr boxes, uint numPoints, uint numBoxes);
    virtual ~Grid();
    virtual void render();

private:
    floatArr        m_vertices;
    uintArr         m_boxes;
    uint            m_numPoints;
    uint            m_numBoxes;

    GLuint          m_pointDisplayList;
    GLuint          m_gridDisplayList;
};

} /* namespace lvr */
#endif /* GRID_HPP_ */