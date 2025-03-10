/**
 * Copyright (c) 2018, University Osnabrück
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University Osnabrück nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL University Osnabrück BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Debug.tcc
 *
 *  @date 18.07.2017
 *  @author Johan M. von Behren <johan@vonbehren.eu>
 */

#include <unordered_map>

using std::unordered_map;

#include "lvr2/algorithm/Materializer.hpp"
#include "lvr2/types/MeshBuffer.hpp"
#include "lvr2/io/ModelFactory.hpp"
#include "lvr2/algorithm/FinalizeAlgorithms.hpp"


namespace lvr2
{

template<typename BaseVecT>
void writeDebugMesh(
    const BaseMesh<BaseVecT>& mesh,
    string filename,
    RGB8Color color
)
{
    // Generate color map
    // TODO: replace with better impl of attr map
    DenseVertexMap<RGB8Color> colorMap;
    colorMap.reserve(mesh.numVertices());

    for (auto vH: mesh.vertices())
    {
        colorMap.insert(vH, color);
    }

    // Set color to mesh
    SimpleFinalizer<BaseVecT> finalize;
    finalize.setColorData(colorMap);

    // Get buffer
    auto buffer = finalize.apply(mesh);

    // Save mesh
//    auto m = boost::make_shared<Model>(buffer);
//    ModelFactory::saveModel(m, filename);
}

template<typename BaseVecT>
vector<vector<VertexHandle>> getDuplicateVertices(const BaseMesh<BaseVecT>& mesh)
{
    // Save vertex handles "behind" equal points
    unordered_map<BaseVecT, vector<VertexHandle>> uniquePoints;
    for (auto vH: mesh.vertices())
    {
        auto point = mesh.getVertexPosition(vH);
        uniquePoints[point].push_back(vH);
    }

    // Extract all vertex handles, where one point has more than one vertex handle
    vector<vector<VertexHandle>> duplicateVertices;
    for (auto elem: uniquePoints)
    {
        auto vec = elem.second;
        if (vec.size() > 1)
        {
            duplicateVertices.push_back(vec);
        }
    }

    return duplicateVertices;
}

template<typename BaseVecT>
void
writeDebugContourMesh(
    const BaseMesh<BaseVecT>& mesh,
    string filename,
    RGB8Color connectedColor,
    RGB8Color contourColor,
    RGB8Color bugColor
)
{
    DenseVertexMap<RGB8Color> color_vertices(mesh.numVertices(), connectedColor);
    for (auto eH: mesh.edges())
    {
        auto vertices = mesh.getVerticesOfEdge(eH);

        // found a contour
        auto numFaces = mesh.numAdjacentFaces(eH);
        if (1 == numFaces)
        {
            color_vertices[vertices[0]] = contourColor;
            color_vertices[vertices[1]] = contourColor;
        }
            // found something weird
        else if (2 != numFaces)
        {
            color_vertices[vertices[0]] = bugColor;
            color_vertices[vertices[1]] = bugColor;
        }
    }

    // Finalize mesh (convert it to simple `MeshBuffer`)
    SimpleFinalizer<BaseVecT> finalize;
    finalize.setColorData(color_vertices);
    auto buffer = finalize.apply(mesh);

    // Save mesh
    std::cout << "IMPLEMENT ME " << std::endl;

    //auto m = boost::make_shared<Model>(buffer);
    //ModelFactory::saveModel(m, filename);
}

} // namespace lvr2
