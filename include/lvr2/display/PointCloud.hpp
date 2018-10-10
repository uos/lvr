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
 * PointCloud.hpp
 *
 *  Created on: 20.08.2011
 *      Author: Thomas Wiemann
 */

#ifndef POINTCLOUD_H_
#define POINTCLOUD_H_

#include <lvr2/display/Renderable.hpp>

#include <lvr2/geometry/ColorVertex.hpp>

#include <vector>
#include <string>
#include <fstream>


using namespace std;


namespace lvr2
{

enum
{
    RenderPoints                = 0x01,
    RenderNormals               = 0x02,
};

class PointCloud : public Renderable{

    using uColorVertex = ColorVertex<Vec, unsigned char>;
public:

    PointCloud();
    PointCloud(ModelPtr loader, string name = "<unamed cloud>");
    PointCloud(PointBuffer2Ptr buffer, string name = "<unamed cloud>");

    virtual ~PointCloud();
    virtual inline void render();

    vector<uColorVertex> getPoints(){return m_points;};
    void setPoints(){};

    void addPoint(float x, float y, float z, unsigned char r, unsigned char g, unsigned char b){
        m_boundingBox->expand(uColorVertex(x, y, z, r, g, b));
        m_points.push_back(uColorVertex(x, y, z, r, g, b));
    };

    void addPoint(const uColorVertex v) {
        m_boundingBox->expand(Vector<Vec>(v.x, v.y, v.z));
        m_points.push_back(v);
    };

    void clear(){
        delete m_boundingBox;
        m_boundingBox = new BoundingBox<Vec>;
        m_points.clear();
    };

    void updateBuffer(PointBuffer2Ptr buffer);

    void updateDisplayLists();
//private:
    vector<uColorVertex> m_points;

    void setRenderMode(int mode) {m_renderMode = mode;}


private:
    int getFieldsPerLine(string filename);
    void init(PointBuffer2Ptr buffer);

    int                        m_renderMode;
    GLuint                     m_normalListIndex;
    floatArr                   m_normals;
    size_t                     m_numNormals;

};

inline void PointCloud::render()
{
    //cout << name << " : Active: " << " " << active << " selected : " << selected << endl;
    if(m_listIndex != -1 && m_active)
    {
        // Increase point size if normal rendering is enabled
        if(m_renderMode & RenderNormals)
        {
            glPointSize(5.0);
        }
        else
        {
            glPointSize(m_pointSize);
        }
        glDisable(GL_LIGHTING);
        glPushMatrix();
        glMultMatrixf(m_transformation.getData());

        // Render points
        if(m_selected)
        {
            glCallList(m_activeListIndex);
        }
        else
        {
            glCallList(m_listIndex);
        }

        // Render normals
        if(m_renderMode & RenderNormals)
        {
            glCallList(m_normalListIndex);
        }
        glPointSize(1.0);
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }
}

} // namespace lvr2

#endif /* POINTCLOUD_H_ */
