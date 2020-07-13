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
 * BVHRaycaster.hpp
 *
 *  @date 25.01.2019
 *  @author Johan M. von Behren <johan@vonbehren.eu>
 *  @author Alexander Mock <amock@uos.de>
 */

#pragma once
#ifndef LVR2_ALGORITHM_RAYCASTING_BVHRAYCASTER
#define LVR2_ALGORITHM_RAYCASTING_BVHRAYCASTER


#include "lvr2/io/MeshBuffer.hpp"
#include "lvr2/types/MatrixTypes.hpp"
#include "lvr2/geometry/BVH.hpp"
#include "lvr2/algorithm/raycasting/RaycasterBase.hpp"

#define EPSILON 0.0000001
#define PI 3.14159265
#define BVH_STACK_SIZE 128

namespace lvr2
{

/**
 *  @brief BVHRaycaster: CPU version of BVH Raycasting: WIP
 */
class BVHRaycaster : public RaycasterBase {
public:
    /**
     * @brief Constructor: Stores mesh as member
     */
    BVHRaycaster(const MeshBufferPtr mesh);

    bool castRay(
        const Vector3f& origin,
        const Vector3f& direction,
        Vector3f& intersection
    );

    void castRays(
        const Vector3f& origin,
        const std::vector<Vector3f >& directions,
        std::vector<Vector3f >& intersections,
        std::vector<uint8_t>& hits
    );

    void castRays(
        const std::vector<Vector3f>& origins,
        const std::vector<Vector3f>& directions,
        std::vector<Vector3f>& intersections,
        std::vector<uint8_t>& hits
    );


    /**
     * @struct Ray
     * @brief Data type to store information about a ray
     */

    struct Ray {
        Vector3f dir;
        Vector3f invDir;
        Vector3i rayDirSign;
    };


    /**
     * @struct TriangleIntersectionResult
     * @brief A struct to return the calculation results of a triangle intersection
     */
    struct TriangleIntersectionResult {
        bool hit;
        unsigned int pBestTriId;
        Vector3f pointHit;
        float hitDist;
    };
    

protected:
    BVHTree<BaseVector<float> > m_bvh;

private:


    /**
     * @brief Calculates the squared distance of two vectors
     * @param a First vector
     * @param b Second vector
     * @return The square distance
     */
    inline float distanceSquare(const Vector3f& a, const Vector3f& b) const
    {
        return (a - b).squaredNorm();
    }

    /**
     * @brief Calculates whether a ray intersects a box
     * @param origin    The origin of the ray
     * @param ray       The ray
     * @param boxPtr    A pointer to the box data
     * @return          A boolean indicating whether the ray hits the box
     */
    bool rayIntersectsBox(Vector3f origin, Ray ray, const float* boxPtr);

    /**
     * @brief Calculates the closest intersection of a raycast into a scene of triangles, given a bounding volume hierarchy
     *
     * @param clBVHindicesOrTriLists        Compressed BVH Node data, that stores for each node, whether it is a leaf node
     *                                      and triangle indices lists for leaf nodes and the indices of their child nodes
     *                                      for inner nodes
     * @param origin                        Origin of the ray
     * @param ray                           Direction of the ray
     * @param clBVHlimits                   3d upper and lower limits for each bounding box in the BVH
     * @param clTriangleIntersectionData    Precomputed intersection data for each triangle
     * @param clTriIdxList                  List of triangle indices
     * @return The TriangleIntersectionResult, containing information about the triangle intersection
     */
    TriangleIntersectionResult intersectTrianglesBVH(
        const unsigned int* clBVHindicesOrTriLists,
        Vector3f origin,
        Ray ray,
        const float* clBVHlimits,
        const float* clTriangleIntersectionData,
        const unsigned int* clTriIdxList
    );

    /**
     * @brief Casts one ray from one origin into a scene of triangles, given a bounding volume hierarchy
     *
     * @param rays_origin                   Origins of the rays
     * @param rays                          Forward directions of each pose
     * @param clBVHindicesOrTriLists        Compressed BVH Node data, that stores for each node, whether it is a leaf node
     *                                      and triangle indices lists for leaf nodes and the indices of their child nodes
     *                                      for inner nodes
     * @param clBVHlimits                   3d upper and lower limits for each bounding box in the BVH
     * @param clTriangleIntersectionData    Precomputed intersection data for each triangle
     * @param clTriIdxList                  List of triangle indices
     * @param result                        Result point positions
     * @param result_hits                   Result hits, where for each ray is stored whether it has hit a triangle
     */
    void cast_rays_one_one(
        const float* ray_origin,
        const float* rays,
        const unsigned int* clBVHindicesOrTriLists,
        const float* clBVHlimits,
        const float* clTriangleIntersectionData,
        const unsigned int* clTriIdxList,
        float* result,
        uint8_t* result_hits
    );

    /**
     * @brief Casts multiple rays from one origin into a scene of triangles, given a bounding volume hierarchy
     *
     * @param rays_origin                   Origins of the rays
     * @param rays                          Forward directions of each pose
     * @param clBVHindicesOrTriLists        Compressed BVH Node data, that stores for each node, whether it is a leaf node
     *                                      and triangle indices lists for leaf nodes and the indices of their child nodes
     *                                      for inner nodes
     * @param clBVHlimits                   3d upper and lower limits for each bounding box in the BVH
     * @param clTriangleIntersectionData    Precomputed intersection data for each triangle
     * @param clTriIdxList                  List of triangle indices
     * @param result                        Result point positions
     * @param result_hits                   Result hits, where for each ray is stored whether it has hit a triangle
     */

    void cast_rays_one_multi(
        const float* ray_origin,
        const float* rays,
        size_t num_rays,
        const unsigned int* clBVHindicesOrTriLists,
        const float* clBVHlimits,
        const float* clTriangleIntersectionData,
        const unsigned int* clTriIdxList,
        float* result,
        uint8_t* result_hits
    );

    /**
     * @brief Casts multiple rays from multiple origins into a scene of triangles, given a bounding volume hierarchy
     *
     * @param rays_origin                   Origins of the rays
     * @param rays                          Forward directions of each pose
     * @param clBVHindicesOrTriLists        Compressed BVH Node data, that stores for each node, whether it is a leaf node
     *                                      and triangle indices lists for leaf nodes and the indices of their child nodes
     *                                      for inner nodes
     * @param clBVHlimits                   3d upper and lower limits for each bounding box in the BVH
     * @param clTriangleIntersectionData    Precomputed intersection data for each triangle
     * @param clTriIdxList                  List of triangle indices
     * @param result                        Result point positions
     * @param result_hits                   Result hits, where for each ray is stored whether it has hit a triangle
     */
    void cast_rays_multi_multi(
        const float* ray_origin,
        const float* rays,
        size_t num_rays,
        const unsigned int* clBVHindicesOrTriLists,
        const float* clBVHlimits,
        const float* clTriangleIntersectionData,
        const unsigned int* clTriIdxList,
        float* result,
        uint8_t* result_hits
    );


};

} // namespace lvr2

#endif // LVR2_ALGORITHM_RAYCASTING_BVHRAYCASTER