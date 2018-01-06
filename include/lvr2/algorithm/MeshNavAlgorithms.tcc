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
 * MeshNavAlgorithms.tcc
 */

#include <algorithm>
#include <limits>

#include <lvr2/attrmaps/AttrMaps.hpp>


namespace lvr2
{

template <typename BaseVecT>
void calcVertexLocalNeighborhood(
    const BaseMesh<BaseVecT>& mesh,
    VertexHandle vH,
    double radius,
    vector<VertexHandle>& neighborsOut
)
{
    auto vPos = mesh.getVertexPosition(vH);

    radius *= radius;
    //Store vertices to visit
    vector<VertexHandle> stack;
    stack.push_back(vH);
    //Save visited vertices
    SparseVertexMap<bool> usedVertices(false);
    vector<VertexHandle> directNeighbors;

    //As long as there are vertices to visit
    while (!stack.empty())
    {
        // Visit the next vertex
        auto curVH = stack.back();
        stack.pop_back();

        usedVertices.insert(curVH, true);

        directNeighbors.clear();
        mesh.getNeighboursOfVertex(curVH, directNeighbors);
        for (auto newVH: directNeighbors)
        {
            // Add vertices within the radius to the local neighborhood
            auto dist = mesh.getVertexPosition(newVH).squaredDistanceFrom(vPos);
            if (!usedVertices[newVH] && dist < radius)
            {
                stack.push_back(newVH);
                neighborsOut.push_back(newVH);
            }
        }
    }
}

template <typename BaseVecT>
DenseVertexMap<float> calcVertexHeightDiff(const BaseMesh<BaseVecT>& mesh, double radius)
{
    DenseVertexMap<float> heightDiff;
    // Get neighbored vertices
    vector<VertexHandle> neighbors;

    // Calculate height difference for each vertex
    for (auto vH: mesh.vertices())
    {
        neighbors.clear();
        calcVertexLocalNeighborhood(mesh, vH, radius, neighbors);

        // Store initial values for min and max height
        float minHeight = std::numeric_limits<float>::max();
        float maxHeight = -std::numeric_limits<float>::max();

        // Adjust the min and max height values, according to the neighborhood
        for (auto neighbor: neighbors)
        {
            auto cur_pos = mesh.getVertexPosition(neighbor);
            minHeight = std::min(cur_pos.z, minHeight);
            maxHeight = std::max(cur_pos.z, maxHeight);
        }

        // Calculate the final height difference
        heightDiff.insert(vH, maxHeight - minHeight);
    }

    return heightDiff;
}

template<typename BaseVecT>
DenseEdgeMap<float> calcVertexAngleEdges(const BaseMesh<BaseVecT>& mesh, const VertexMap<Normal<BaseVecT>>& normals)
{
    DenseEdgeMap<float> edgeAngle;

    for (auto eH: mesh.edges())
    {
        auto vHVector = mesh.getVerticesOfEdge(eH);
        edgeAngle.insert(eH, acos(normals[vHVector[0]].dot(normals[vHVector[1]].asVector())));
        if(isnan(edgeAngle[eH]))
        {
                edgeAngle[eH] = 0;
        }
    }
    return edgeAngle;
}

template<typename BaseVecT>
DenseVertexMap<float> calcAverageVertexAngles(
    const BaseMesh<BaseVecT>& mesh,
    const VertexMap<Normal<BaseVecT>>& normals
)
{
    DenseVertexMap<float> vertexAngles;
    auto edgeAngles = calcVertexAngleEdges(mesh, normals);

    for (auto vH: mesh.vertices())
    {
        float angleSum = 0;
        auto edgeVec = mesh.getEdgesOfVertex(vH);
        int degree = edgeVec.size();
        for(auto eH: edgeVec)
        {
            angleSum += edgeAngles[eH];
        }
        vertexAngles.insert(vH, angleSum / degree);
    }
    return vertexAngles;
}


template<typename BaseVecT>
DenseVertexMap<float> calcVertexRoughness(
    const BaseMesh<BaseVecT>& mesh,
    double radius,
    const VertexMap<Normal<BaseVecT>>& normals
)
{
    DenseVertexMap<float> roughness;

    // Get neighbored vertices
    vector<VertexHandle> neighbors;
    auto averageAngles = calcAverageVertexAngles(mesh, normals);

    // Calculate roughness for each vertex
    for (auto vH: mesh.vertices())
    {
        double sum = 0.0;

        neighbors.clear();
        calcVertexLocalNeighborhood(mesh, vH, radius, neighbors);

        // Adjust sum values, according to the neighborhood
        for (auto neighbor: neighbors)
        {
           sum += averageAngles[neighbor];
        }

        // Calculate the final roughness
        roughness.insert(vH, sum / neighbors.size());

    }
    return roughness;

}

template<typename BaseVecT>
void calcVertexRoughnessAndHeightDiff(const BaseMesh<BaseVecT>& mesh, double radius, const VertexMap<Normal<BaseVecT>>& normals, DenseVertexMap<float>& roughness, DenseVertexMap<float>& heightDiff)
{
    roughness.clear();
    heightDiff.clear();

    // Get neighbored vertices
    vector<VertexHandle> neighbors;
    auto averageAngles = calcAverageVertexAngles(mesh, normals);

    // Calculate roughness and height difference for each vertex
    for (auto vH: mesh.vertices())
    {
        double sum = 0.0;

        neighbors.clear();
        calcVertexLocalNeighborhood(mesh, vH, radius, neighbors);

        // Adjust sum values, according to the neighborhood
        for (auto neighbor: neighbors)
        {
            sum += averageAngles[neighbor];
        }

        // Calculate the final roughness
        roughness.insert(vH, sum / neighbors.size());

        // Store initial values for min and max height
        float minHeight = std::numeric_limits<float>::max();
        float maxHeight = -std::numeric_limits<float>::max();

        // Adjust the min and max height values, according to the neighborhood
        for (auto neighbor: neighbors)
        {
            auto cur_pos = mesh.getVertexPosition(neighbor);
            minHeight = std::min(cur_pos.z, minHeight);
            maxHeight = std::max(cur_pos.z, maxHeight);
        }

        // Calculate the final height difference
        heightDiff.insert(vH, maxHeight - minHeight);
    }
}


} // namespace lvr2
