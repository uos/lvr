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
 * StaticMesh.h
 *
 *  Created on: 12.11.2008
 *      Author: Thomas Wiemann
 */

#ifndef STATICMESH_H_
#define STATICMESH_H_

#ifdef WIN32
#pragma warning(disable: 4996)
#endif

#include <stdlib.h>
#include <string>
#include <string.h>
#include <sstream>

#ifdef _MSC_VER
#include <Windows.h>
#endif

#ifndef __APPLE__
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif

#include "Renderable.hpp"
#include <lvr/geometry/BoundingBox.hpp>

using namespace std;


namespace lvr
{

enum
{
    RenderSurfaces            = 0x01,
    RenderTriangles           = 0x02,
    RenderSurfaceNormals      = 0x04,
    RenderVertexNormals       = 0x08,
    RenderColors              = 0x10,
};

class StaticMesh : public Renderable
{
public:
    StaticMesh();
    StaticMesh( ModelPtr model, string name=""); // <unnamed static mesh>" );
    StaticMesh( MeshBufferPtr buffer, string name=""); //<unnamed static mesh>" );

    StaticMesh(const StaticMesh &o);
    ~StaticMesh();
    inline void render();

    virtual void finalize();
    virtual void savePLY(string filename);
    virtual void setName(string name);

    floatArr        getVertices();
    uintArr         getIndices();
    float*          getNormals();

    size_t            getNumberOfVertices();
    size_t            getNumberOfFaces();

    void setRenderMode(int mode) { m_renderMode = mode;}
    int  getRenderMode() { return m_renderMode;}

private:

    void init( MeshBufferPtr mesh );

    void interpolateNormals();
    void setDefaultColors();
    void calcBoundingBox();

protected:

    //void compileDisplayLists();
    void compileColoredMeshList();
    void compileWireframeList();
    void compileNameList();

    void setColorMaterial(float r, float g, float b);


    void readPly(string filename);

    floatArr        m_normals;
    float*          m_faceNormals;
    floatArr        m_vertices;
    ucharArr        m_colors;
    unsigned char*  m_blackColors;

    uintArr         m_faces;

    bool            m_finalized;
    bool            m_haveMaterials;

    size_t          m_numVertices;
    size_t          m_numFaces;
    size_t          m_numMaterials;

    int             m_renderMode;


    int             m_coloredMeshList;
    int             m_wireframeList;
    int                m_nameList;

};

void StaticMesh::render(){

    if(m_active)
    {
        if(m_finalized){
            glPushMatrix();
            glMultMatrixf(m_transformation.getData());
            if(m_renderMode & RenderSurfaces)
            {
                glEnable(GL_LIGHTING);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glCallList(m_coloredMeshList);
                glCallList(m_nameList);
            }

            if(m_renderMode & RenderTriangles)
            {
                glDisable(GL_LIGHTING);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glLineWidth(m_lineWidth);
                glColor3f(0.0, 0.0, 0.0);
                glCallList(m_wireframeList);
                glEnable(GL_LIGHTING);
            }
            glPopMatrix();

         }
    }
}

} // namespace lvr

#endif /* STATICMESH_H_ */
