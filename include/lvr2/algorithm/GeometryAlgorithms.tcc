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
 * GeometryAlgorithms.tcc
 */

#include <algorithm>
#include <limits>
#include <queue>
#include <set>
#include <list>

#include "lvr2/attrmaps/AttrMaps.hpp"
#include "lvr2/util/Progress.hpp"
#include "lvr2/algorithm/FinalizeAlgorithms.hpp"
#include "lvr2/util/Util.hpp"
#include "lvr2/geometry/BaseMesh.hpp"
#include <iostream>

#ifdef LVR2_USE_EMBREE
#include "lvr2/algorithm/raycasting/EmbreeRaycaster.hpp"
#else
#include "lvr2/algorithm/raycasting/BVHRaycaster.hpp"
#endif


namespace lvr2
{

template <typename BaseVecT>
void calcVertexLocalNeighborhood(
    const BaseMesh<BaseVecT> &mesh,
    VertexHandle vH,
    double radius,
    vector<VertexHandle> &neighborsOut)
{
    visitLocalNeighborhoodOfVertex(mesh, vH, radius, [&](auto newVH) {
        neighborsOut.push_back(newVH);
    });
}

template <typename BaseVecT, typename VisitorF>
void visitLocalVertexNeighborhood(
    const BaseMesh<BaseVecT> &mesh,
    std::set<VertexHandle> &invalid,
    VertexHandle vH,
    double radius,
    VisitorF visitor)
{
    // Prepare values for the radius test
    auto vPos = mesh.getVertexPosition(vH);
    const double radiusSquared = radius * radius;

    // Store the vertices we want to expand. We reserve memory for 8 vertices
    // already, since it's rather likely to have at least that many vertices
    // in the stack. In the beginning, the stack only contains the original
    // vertex we were given.
    vector<VertexHandle> stack;
    stack.reserve(8);
    stack.push_back(vH);

    // In this map we store whether or not we have already visited a vertex,
    // where visiting means: calling the visitor with it and pushing it on
    // the stack of vertices we still need to expand.
    //
    // It would be more appropriate to use a set instead of a map, but there
    // are no attribute-sets...
    SparseVertexMap<bool> visited(8, false);
    visited.insert(vH, true);

    // This vector is later used to store the neighbors of a vertex. It's
    // created here to reduce the amount of heap allocations.
    vector<VertexHandle> directNeighbors;

    // As long as there are vertices we want to expand...
    while (!stack.empty())
    {
        // Get the next vertex and remove it from the stack.
        auto curVH = stack.back();
        stack.pop_back();

        // Expand current vertex: add visit its direct neighbors.
        directNeighbors.clear();
        try
        {
            mesh.getNeighboursOfVertex(curVH, directNeighbors);
        }
        catch (lvr2::PanicException exception)
        {
            invalid.insert(curVH);
        }
        for (auto newVH : directNeighbors)
        {
            // If this vertex is within the radius of the original vertex, we
            // want to visit it later, thus pushing it onto the stack. But we
            // only do that if we haven't visited the vertex before. We use
            // `containsKey` here because we only insert `true` anyway.
            auto distSquared = mesh.getVertexPosition(newVH).squaredDistanceFrom(vPos);
            if (!visited.containsKey(newVH) && distSquared < radiusSquared)
            {
                visitor(newVH);
                stack.push_back(newVH);
                visited.insert(newVH, true);
            }
        }
    }
}


template <typename BaseVecT, typename VisitorF>
void visitLocalVertexNeighborhoodPlane(
    const BaseMesh<BaseVecT> &mesh,
    std::set<VertexHandle> &invalid,
    VertexHandle vH,
    BaseVecT normal,
    double radius,
    VisitorF visitor)
{
    // Prepare values for the radius test
    const BaseVecT& vPos = mesh.getVertexPosition(vH);
    const BaseVecT vPosProj = vPos - normal * vPos.dot(normal);
    const double radiusSquared = radius * radius;

    // Store the vertices we want to expand. We reserve memory for 8 vertices
    // already, since it's rather likely to have at least that many vertices
    // in the stack. In the beginning, the stack only contains the original
    // vertex we were given.
    vector<VertexHandle> stack;
    stack.reserve(8);
    stack.push_back(vH);

    // In this map we store whether or not we have already visited a vertex,
    // where visiting means: calling the visitor with it and pushing it on
    // the stack of vertices we still need to expand.
    //
    // It would be more appropriate to use a set instead of a map, but there
    // are no attribute-sets...
    SparseVertexMap<bool> visited(8, false);
    visited.insert(vH, true);

    // This vector is later used to store the neighbors of a vertex. It's
    // created here to reduce the amount of heap allocations.
    vector<VertexHandle> directNeighbors;

    // As long as there are vertices we want to expand...
    while (!stack.empty())
    {
        // Get the next vertex and remove it from the stack.
        auto curVH = stack.back();
        stack.pop_back();

        // Expand current vertex: add visit its direct neighbors.
        directNeighbors.clear();
        try
        {
            mesh.getNeighboursOfVertex(curVH, directNeighbors);
        }
        catch (lvr2::PanicException exception)
        {
            invalid.insert(curVH);
        }
        for (auto newVH : directNeighbors)
        {
            // If this vertex is within the radius on plane of the original vertex, we
            // want to visit it later, thus pushing it onto the stack. But we
            // only do that if we haven't visited the vertex before. We use
            // `containsKey` here because we only insert `true` anyway.
            const BaseVecT& vPosNew = mesh.getVertexPosition(newVH);
            const BaseVecT vPosNewProj = vPosNew - normal * vPosNew.dot(normal);

            auto distSquared = vPosNewProj.squaredDistanceFrom(vPosProj);
            if (!visited.containsKey(newVH) && distSquared < radiusSquared)
            {
                visitor(newVH);
                stack.push_back(newVH);
                visited.insert(newVH, true);
            }
        }
    }
}

template <typename BaseVecT>
DenseVertexMap<float> calcVertexHeightDifferences(
  const BaseMesh<BaseVecT> &mesh, double radius)
{
    // We create a map to store a height-diff for each vertex. We preallocate
    // memory for all vertices. This is not only an optimization, but more
    // importantly to avoid multithreading crashes. The parallelized loop
    // further down inserts into this data structure; if there is no free
    // memory left, it will reallocate which will crash horribly when multiple
    // threads are involved.
    DenseVertexMap<float> heightDiff;
    heightDiff.reserve(mesh.nextVertexIndex());

    // Output
    string msg = timestamp.getElapsedTime() + "Computing height differences...";
    ProgressBar progress(mesh.numVertices(), msg);
    ++progress;

    std::set<VertexHandle> invalid;

    // Calculate height difference for each vertex
    // for(auto vH : mesh.vertices())
    #pragma omp parallel for
    for (size_t i = 0; i < mesh.nextVertexIndex(); i++)
    {
        auto vH = VertexHandle(i);
        if (!mesh.containsVertex(vH))
        {
          continue;
        }

        // this was wrong:
        // this neighborhood rejects connections outside a maximum radius
        // however, for a flat wall this would give wrong results
        // -> solution: finish at last neighbor outside the radius 
        //  but still consider it for height diff computation
        // other solution: limit the search only along the xy plane
        // - could be the best solution though
        // Alex: Second thought
        // even after these fixes it would be wrong for some cases
        // e.g. if the vertex lies outside of the cylinder, and the only connection 
        // to it has a steep edge. Then it is discared from height diff computation
        // however, the correct solution would be to find the intersection of the edge
        // with the limiting geometry (sphere/cylinder)
        
        BaseVecT normal;
        normal.x = 0.0;
        normal.y = 0.0;
        normal.z = 1.0;
        // this would also work for rotated meshes, if you define an up vector

        // forgot own position!
        const BaseVecT& vPos = mesh.getVertexPosition(vH);
        float height = normal.dot(vPos);
        float minHeight = height;
        float maxHeight = height;

        visitLocalVertexNeighborhoodPlane(mesh, invalid, vH, normal, radius, [&](auto neighbor) 
        {
            auto curPos = mesh.getVertexPosition(neighbor);

            // height w.r.t. plane
            float height = normal.dot(curPos);

            if (height < minHeight)
            {
                minHeight = height;
            }
            if (height > maxHeight)
            {
                maxHeight = height;
            }
        });

        // Calculate the final height difference
        #pragma omp critical
        {
            heightDiff.insert(vH, maxHeight - minHeight);
            ++progress;
        }
    }

    if(!timestamp.isQuiet())
    {
      std::cout << std::endl;
    }

    if(!invalid.empty())
    {
        std::cerr << "Found " << invalid.size() << " invalid, non manifold "
            << "vertices." << std::endl;
    }

    return heightDiff;
}

template <typename BaseVecT>
DenseEdgeMap<float> calcVertexAngleEdges(const BaseMesh<BaseVecT> &mesh, const VertexMap<Normal<typename BaseVecT::CoordType>> &normals)
{
    DenseEdgeMap<float> edgeAngle(mesh.nextEdgeIndex(), 0);
    for (auto eH : mesh.edges())
    {
        auto vHVector = mesh.getVerticesOfEdge(eH);
        edgeAngle.insert(eH, acos(normals[vHVector[0]].dot(normals[vHVector[1]])));
        if (isnan(edgeAngle[eH]))
        {
            edgeAngle[eH] = 0;
        }
    }
    return edgeAngle;
}

template <typename BaseVecT>
DenseVertexMap<float> calcAverageVertexAngles(
    const BaseMesh<BaseVecT> &mesh,
    const VertexMap<Normal<typename BaseVecT::CoordType>> &normals)
{
    DenseVertexMap<float> vertexAngles(mesh.nextVertexIndex(), 0);
    auto edgeAngles = calcVertexAngleEdges(mesh, normals);
    std::set<VertexHandle> invalid;

    for (auto vH : mesh.vertices())
    {
        float angleSum = 0;
        try
        {
            auto edgeVec = mesh.getEdgesOfVertex(vH);
            int degree = edgeVec.size();
            for (auto eH : edgeVec)
            {
                angleSum += edgeAngles[eH];
            }
            vertexAngles.insert(vH, angleSum / degree);
        }
        catch (lvr2::PanicException exception)
        {
            vertexAngles.insert(vH, M_PI);
            invalid.insert(vH);
        }
        catch (VertexLoopException exception)
        {
            vertexAngles.insert(vH, M_PI);
            invalid.insert(vH);
        }
    }
    if (!invalid.empty())
    {
        std::cerr << std::endl << "Found " << invalid.size()
            << " invalid, non manifold vertices." << std::endl
            << "The average vertex angle of the invalid vertices has been set"
            << " to Pi." << std::endl;
    }
    return vertexAngles;
}

template <typename BaseVecT>
DenseVertexMap<float> calcVertexRoughness(
    const BaseMesh<BaseVecT> &mesh,
    double radius,
    const VertexMap<Normal<typename BaseVecT::CoordType>> &normals)
{
    // We create a map to store the roughness for each vertex. We preallocate
    // memory for all vertices. This is not only an optimization, but more
    // importantly to avoid multithreading crashes. The parallelized loop
    // further down inserts into this data structure; if there is no free
    // memory left, it will reallocate which will crash horribly when multiple
    // threads are involved.
    DenseVertexMap<float> roughness;
    roughness.reserve(mesh.nextVertexIndex());

    auto averageAngles = calcAverageVertexAngles(mesh, normals);

    // Output
    string msg = timestamp.getElapsedTime() + "Computing roughness";
    ProgressBar progress(mesh.numVertices(), msg);
    ++progress;

    std::set<VertexHandle> invalid;

    // Calculate roughness for each vertex
    #pragma omp parallel for
    for (size_t i = 0; i < mesh.nextVertexIndex(); i++)
    {
        auto vH = VertexHandle(i);
        if (!mesh.containsVertex(vH))
        {
            continue;
        }

        // forgot to add self
        float sum = averageAngles[vH];
        size_t count = 1;

        visitLocalVertexNeighborhood(mesh, invalid, vH, radius, [&](auto neighbor) 
        {
            sum += averageAngles[neighbor];
            count += 1;
        });

        #pragma omp critical
        {
            // Calculate the final roughness
            roughness.insert(vH, count ? sum / count : 0);
            ++progress;
        }
    }
    if(!timestamp.isQuiet())
    {
        std::cout << std::endl;
    }

    if (!invalid.empty())
    {
        std::cerr << "Found " << invalid.size() << " invalid, non manifold "
            << "vertices." << std::endl;
    }
    
    return roughness;
}

template <typename BaseVecT>
void calcVertexRoughnessAndHeightDifferences(
    const BaseMesh<BaseVecT> &mesh,
    double radius,
    const VertexMap<Normal<typename BaseVecT::CoordType>> &normals,
    DenseVertexMap<float> &roughness,
    DenseVertexMap<float> &heightDiff)
{
    // Reserving memory in those maps is important to avoid multi threading
    // related crashes.
    roughness.clear();
    roughness.reserve(mesh.nextVertexIndex());
    heightDiff.clear();
    heightDiff.reserve(mesh.nextVertexIndex());

    std::set<VertexHandle> invalid;
    auto averageAngles = calcAverageVertexAngles(mesh, normals);

    // Calculate roughness and height difference for each vertex
    #pragma omp parallel for
    for (size_t i = 0; i < mesh.nextVertexIndex(); i++)
    {
        auto vH = VertexHandle(i);
        if (!mesh.containsVertex(vH))
        {
            continue;
        }

        double sum = 0.0;
        uint32_t count = 0;
        float minHeight = std::numeric_limits<float>::max();
        float maxHeight = std::numeric_limits<float>::lowest();

        visitLocalVertexNeighborhood(mesh, invalid, vH, radius, [&](auto neighbor) 
        {
            sum += averageAngles[neighbor];
            count += 1;

            auto curPos = mesh.getVertexPosition(neighbor);
            if (curPos.z < minHeight)
            {
                minHeight = curPos.z;
            }
            if (curPos.z > maxHeight)
            {
                maxHeight = curPos.z;
            }
        });

        #pragma omp critical
        {
            // Calculate the final roughness
            roughness.insert(vH, count ? sum / count : 0);

            // Calculate the final height difference
            heightDiff.insert(vH, maxHeight - minHeight);
        }
    }
    if (!invalid.empty())
    {
        std::cerr << "Found " << invalid.size() << " invalid, non manifold "
            << "vertices." << std::endl;
    }
}

template <typename BaseVecT>
DenseEdgeMap<float> calcVertexDistances(const BaseMesh<BaseVecT> &mesh)
{
    DenseEdgeMap<float> distances;

    distances.clear();
    distances.reserve(mesh.nextEdgeIndex());
    for (auto eH : mesh.edges())
    {
        auto vertices = mesh.getVerticesOfEdge(eH);
        const float dist = mesh.getVertexPosition(vertices[0]).distance(mesh.getVertexPosition(vertices[1]));
        distances.insert(eH, dist);
    }
    return distances;
}


template <typename BaseVecT>
DenseVertexMap<float> calcBorderCosts(
  const BaseMesh<BaseVecT> &mesh, 
  double border_cost)
{
    DenseVertexMap<float> borderCosts;
    borderCosts.reserve(mesh.nextVertexIndex());

    // Output
    string msg = timestamp.getElapsedTime() + "Computing border weights...";
    ProgressBar progress(mesh.numVertices(), msg);
    ++progress;

    // Calculate height difference for each vertex
    #pragma omp parallel for
    for (size_t i = 0; i < mesh.nextVertexIndex(); i++)
    {
        auto vH = VertexHandle(i);
        if (!mesh.containsVertex(vH))
        {
            continue;
        }

        bool is_border_vertex = false;
        for(auto edge : mesh.getEdgesOfVertex(vH))
        {
            if(mesh.isBorderEdge(edge))
            {
                is_border_vertex = true;
                // it's a border vertex. stop searching. early finish
                break;
            }
        }

        // Calculate the final border weight
        #pragma omp critical
        {
            if(is_border_vertex)
            {
                borderCosts.insert(vH, border_cost);
            } else {
                borderCosts.insert(vH, 0.0);
            }
            
            ++progress;
        }
    }

    if(!timestamp.isQuiet())
    {
      std::cout << std::endl;
    }

    return borderCosts;
}


template <typename BaseVecT>
DenseVertexMap<float> calcNormalClearance(
    const BaseMesh<BaseVecT>& mesh,
    const DenseVertexMap<Normal<typename BaseVecT::CoordType>>& normals
)
{
    // Create a MeshBufferPtr to pass to raycaster implementations
    SimpleFinalizer<BaseVecT> fin;
    MeshBufferPtr buffer = fin.apply(mesh);

    // Create a raycaster implementation
#ifdef LVR2_USE_EMBREE
    auto raycaster = EmbreeRaycaster<DistInt>(buffer);
#else
    auto raycaster = BVHRaycaster<DistInt>(buffer);
#endif

    // Reserve enough memory for all vertices to prevent unnecessary reallocations
    DenseVertexMap<float> freespace;
    freespace.reserve(mesh.nextVertexIndex());
    
    std::stringstream msg;
    msg << timestamp << "[calcNormalClearance] Calculating free space along vertex normals";
    ProgressBar progress(mesh.numVertices(), msg.str());

    // Cast rays for each vertex in parallel
    #pragma omp parallel for
    for (auto i = 0; i < mesh.nextVertexIndex(); i++)
    {
        // Create a vertex handle and check if the mesh contains a vertex with index i
        auto vertexH = VertexHandle(i);
        if (!mesh.containsVertex(vertexH))
        {
            continue;
        }
        
        float distance = std::numeric_limits<float>::infinity();
        DistInt result;
        const Vector3f origin = Util::to_eigen(mesh.getVertexPosition(vertexH));
        const Vector3f normal = Util::to_eigen(normals[vertexH]);
        // Add a small offset to the vertex position to avoid intersections with its incident faces
        if (raycaster.castRay(
            origin + normal * 0.001,
            normal,
            result
        ))
        {
            // Add the same offset to the distance result
            distance = result.dist + 0.001;
        }
        
        #pragma omp critical
        {
            freespace.insert(vertexH, distance);
            ++progress;
        }
    }

    if (!timestamp.isQuiet())
    {
        std::cout << std::endl;
    }
    
    return freespace;
}


class CompareDist
{
public:
    bool operator()(pair<lvr2::VertexHandle, float> n1, pair<lvr2::VertexHandle, float> n2)
    {
        return n1.second > n2.second;
    }
};

template <typename BaseVecT>
bool Dijkstra(
    const BaseMesh<BaseVecT> &mesh,
    const VertexHandle &start,
    const VertexHandle &goal,
    const DenseEdgeMap<float> &edgeCosts,
    std::list<VertexHandle> &path,
    DenseVertexMap<float> &distances,
    DenseVertexMap<VertexHandle> &predecessors,
    DenseVertexMap<bool> &seen,
    DenseVertexMap<float> &vertex_costs)
{
    path.clear();
    distances.clear();
    predecessors.clear();

    // initialize distances with infinity
    // initialize predecessor of each vertex with itself
    for (auto const &vH : mesh.vertices())
    {
        distances.insert(vH, std::numeric_limits<float>::infinity());
        predecessors.insert(vH, vH);
    }

    distances[start] = 0;

    if (goal == start)
    {
        return true;
    }

    std::priority_queue<pair<VertexHandle, float>, vector<pair<VertexHandle, float>>, CompareDist> pq;

    pair<VertexHandle, float> first_pair(start, 0);
    pq.push(first_pair);

    while (!pq.empty())
    {
        pair<VertexHandle, float> pair = pq.top();
        VertexHandle current_vh = pair.first;
        float current_dist = pair.second;
        pq.pop();

        // Check if the current Vertex was seen already
        if (seen[current_vh])
        {
            // Skip to while
            continue;
        }
        // Set the seen vertex to True
        seen[current_vh] = true;

        // Get all edges from the current Vertex
        std::vector<VertexHandle> neighbours;
        mesh.getNeighboursOfVertex(current_vh, neighbours);

        for (auto neighbour_vh : neighbours)
        {
            if (seen[neighbour_vh] || vertex_costs[neighbour_vh] >= 1)
                continue;

            float edge_cost = edgeCosts[mesh.getEdgeBetween(current_vh, neighbour_vh).unwrap()];

            float tmp_neighbour_cost = distances[current_vh] + edge_cost;
            if (distances[neighbour_vh] > tmp_neighbour_cost)
            {
                distances[neighbour_vh] = tmp_neighbour_cost;
                predecessors[neighbour_vh] = current_vh;
                pq.push(std::pair<VertexHandle, float>(neighbour_vh, tmp_neighbour_cost));
            }
        }
    }

    VertexHandle prev = goal;

    if (prev == predecessors[goal])
        return false;

    do
    {
        path.push_front(prev);
        prev = predecessors[prev];
    } while (prev != start);

    path.push_front(start);

    return true;
}

} // namespace lvr2
