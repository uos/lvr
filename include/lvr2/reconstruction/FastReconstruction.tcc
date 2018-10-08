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
 * FastReconstruction.cpp
 *
 *  Created on: 16.02.2011
 *      Author: Thomas Wiemann
 */
#include <lvr2/geometry/BaseMesh.hpp>
#include <lvr2/reconstruction/FastReconstructionTables.hpp>
#include <lvr2/io/Progress.hpp>

namespace lvr2
{

template<typename BaseVecT, typename BoxT>
FastReconstruction<BaseVecT, BoxT>::FastReconstruction(shared_ptr<HashGrid<BaseVecT, BoxT>> grid)
{
    m_grid = grid;
}

template<typename BaseVecT, typename BoxT>
void FastReconstruction<BaseVecT, BoxT>::getMesh(BaseMesh<BaseVecT> &mesh)
{
    // Status message for mesh generation
    string comment = timestamp.getElapsedTime() + "Creating mesh ";
    ProgressBar progress(m_grid->getNumberOfCells(), comment);

    // Some pointers
    BoxT* b;
    unsigned int global_index = mesh.numVertices();

    // Iterate through cells and calculate local approximations
    typename HashGrid<BaseVecT, BoxT>::box_map_it it;
    for(it = m_grid->firstCell(); it != m_grid->lastCell(); it++)
    {
        b = it->second;
        b->getSurface(mesh, m_grid->getQueryPoints(), global_index);
        if(!timestamp.isQuiet())
            ++progress;
    }

    if(!timestamp.isQuiet())
        cout << endl;

    BoxTraits<BoxT> traits;

    if(traits.type == "SharpBox")  // Perform edge flipping for extended marching cubes
    {
        string SFComment = timestamp.getElapsedTime() + "Flipping edges  ";
        ProgressBar SFProgress(this->m_grid->getNumberOfCells(), SFComment);
        for(it = this->m_grid->firstCell(); it != this->m_grid->lastCell(); it++)
        {

            SharpBox<BaseVecT>* sb;
            sb = reinterpret_cast<SharpBox<BaseVecT>* >(it->second);
            if(sb->m_containsSharpFeature)
            {
                OptionalVertexHandle v1;
                OptionalVertexHandle v2;
                OptionalEdgeHandle e;

                if(sb->m_containsSharpCorner)
                {
                    // 1
                    v1 = sb->m_intersections[ExtendedMCTable[sb->m_extendedMCIndex][0]];
                    v2 = sb->m_intersections[ExtendedMCTable[sb->m_extendedMCIndex][1]];

                    if(v1 && v2)
                    {
                        e = mesh.getEdgeBetween(v1.unwrap(), v2.unwrap());
                        if(e)
                        {
                            mesh.flipEdge(e.unwrap());
                        }
                    }

                    // 2
                    v1 = sb->m_intersections[ExtendedMCTable[sb->m_extendedMCIndex][2]];
                    v2 = sb->m_intersections[ExtendedMCTable[sb->m_extendedMCIndex][3]];

                    if(v1 && v2)
                    {
                        e = mesh.getEdgeBetween(v1.unwrap(), v2.unwrap());
                        if(e)
                        {
                            mesh.flipEdge(e.unwrap());
                        }
                    }

                    // 3
                    v1 = sb->m_intersections[ExtendedMCTable[sb->m_extendedMCIndex][4]];
                    v2 = sb->m_intersections[ExtendedMCTable[sb->m_extendedMCIndex][5]];

                    if(v1 && v2)
                    {
                        e = mesh.getEdgeBetween(v1.unwrap(), v2.unwrap());
                        if(e)
                        {
                            mesh.flipEdge(e.unwrap());
                        }
                    }

                }
                else
                {
                    // 1
                    v1 = sb->m_intersections[ExtendedMCTable[sb->m_extendedMCIndex][0]];
                    v2 = sb->m_intersections[ExtendedMCTable[sb->m_extendedMCIndex][1]];

                    if(v1 && v2)
                    {
                        e = mesh.getEdgeBetween(v1.unwrap(), v2.unwrap());
                        if(e)
                        {
                            mesh.flipEdge(e.unwrap());
                        }
                    }

                    // 2
                    v1 = sb->m_intersections[ExtendedMCTable[sb->m_extendedMCIndex][4]];
                    v2 = sb->m_intersections[ExtendedMCTable[sb->m_extendedMCIndex][5]];

                    if(v1 && v2)
                    {
                        e = mesh.getEdgeBetween(v1.unwrap(), v2.unwrap());
                        if(e)
                        {
                            mesh.flipEdge(e.unwrap());
                        }
                    }
                }
            }
            ++SFProgress;
        }
        cout << endl;
    }

     if(traits.type == "BilinearFastBox")
     {
         string comment = timestamp.getElapsedTime() + "Optimizing plane contours  ";
         ProgressBar progress(this->m_grid->getNumberOfCells(), comment);
         for(it = this->m_grid->firstCell(); it != this->m_grid->lastCell(); it++)
         {
          // F... type safety. According to traits object this is OK!
             BilinearFastBox<BaseVecT>* box = reinterpret_cast<BilinearFastBox<BaseVecT>*>(it->second);
             box->optimizePlanarFaces(mesh, 5);
             ++progress;
         }
         cout << endl;
     }

}

template<typename BaseVecT, typename BoxT>
void FastReconstruction<BaseVecT, BoxT>::getMesh(
    BaseMesh<BaseVecT>& mesh,
    BoundingBox<BaseVecT>& bb,
    vector<unsigned int>& duplicates,
    float comparePrecision
)
{
//    // Status message for mesh generation
//    string comment = timestamp.getElapsedTime() + "Creating Mesh ";
//    ProgressBar progress(m_grid->getNumberOfCells(), comment);

//    // Some pointers
//    BoxT* b;
//    unsigned int global_index = mesh.numVertices();

//    // Iterate through cells and calculate local approximations
//    typename HashGrid<BaseVecT, BoxT>::box_map_it it;
//    for(it = m_grid->firstCell(); it != m_grid->lastCell(); it++)
//    {
//        b = it->second;
//        b->getSurface(mesh, m_grid->getQueryPoints(), global_index, bb, duplicates, comparePrecision);
//        if(!timestamp.isQuiet())
//            ++progress;
//    }

//    if(!timestamp.isQuiet())
//        cout << endl;
}

} // namespace lvr2
