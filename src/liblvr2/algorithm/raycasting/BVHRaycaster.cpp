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
#include "lvr2/algorithm/raycasting/BVHRaycaster.hpp"

namespace lvr2 
{

BVHRaycaster::BVHRaycaster(const MeshBufferPtr mesh)
:RaycasterBase(mesh)
,m_bvh(mesh)
{
    
}

bool BVHRaycaster::castRay(
    const Vector3f& origin,
    const Vector3f& direction,
    Vector3f& intersection
)
{
    // Cast one ray from one origin
    std::vector<uint8_t> tmp(1);

    const float *origin_f = reinterpret_cast<const float*>(&origin.coeffRef(0));
    const float *direction_f = reinterpret_cast<const float*>(&direction.coeffRef(0));
    const unsigned int* clBVHindicesOrTriLists = m_bvh.getIndexesOrTrilists().data();
    const float* clBVHlimits = m_bvh.getLimits().data();
    const float* clTriangleIntersectionData = m_bvh.getTrianglesIntersectionData().data();
    const unsigned int* clTriIdxList = m_bvh.getTriIndexList().data();
    float* result = reinterpret_cast<float*>(&intersection.coeffRef(0));
    uint8_t* result_hits = tmp.data();

    cast_rays_one_one(origin_f, 
        direction_f, 
        clBVHindicesOrTriLists,
        clBVHlimits,
        clTriangleIntersectionData,
        clTriIdxList,
        result,
        result_hits);

    bool success = tmp[0];

    return success;
}

void BVHRaycaster::castRays(
    const Vector3f& origin,
    const std::vector<Vector3f>& directions,
    std::vector<Vector3f>& intersections,
    std::vector<uint8_t>& hits)
{
    intersections.resize(directions.size());
    hits.resize(directions.size());


    const float *origin_f = reinterpret_cast<const float*>(&origin.coeffRef(0));
    const float *direction_f = reinterpret_cast<const float*>(directions.data());
    const unsigned int* clBVHindicesOrTriLists = m_bvh.getIndexesOrTrilists().data();
    const float* clBVHlimits = m_bvh.getLimits().data();
    const float* clTriangleIntersectionData = m_bvh.getTrianglesIntersectionData().data();
    const unsigned int* clTriIdxList = m_bvh.getTriIndexList().data();
    float* result = reinterpret_cast<float*>(intersections.data());
    uint8_t* result_hits = hits.data();

    size_t num_rays = directions.size();

    cast_rays_one_multi(origin_f, 
        direction_f, 
        num_rays,
        clBVHindicesOrTriLists,
        clBVHlimits,
        clTriangleIntersectionData,
        clTriIdxList,
        result,
        result_hits);

    // Cast multiple rays from one origin
}

void BVHRaycaster::castRays(
    const std::vector<Vector3f>& origins,
    const std::vector<Vector3f>& directions,
    std::vector<Vector3f>& intersections,
    std::vector<uint8_t>& hits
)
{
    intersections.resize(directions.size());
    hits.resize(directions.size());

    const float *origin_f = reinterpret_cast<const float*>(origins.data());
    const float *direction_f = reinterpret_cast<const float*>(directions.data());
    const unsigned int* clBVHindicesOrTriLists = m_bvh.getIndexesOrTrilists().data();
    const float* clBVHlimits = m_bvh.getLimits().data();
    const float* clTriangleIntersectionData = m_bvh.getTrianglesIntersectionData().data();
    const unsigned int* clTriIdxList = m_bvh.getTriIndexList().data();
    float* result = reinterpret_cast<float*>(intersections.data());
    uint8_t* result_hits = hits.data();

    size_t num_rays = directions.size();

    cast_rays_multi_multi(origin_f, 
        direction_f, 
        num_rays,
        clBVHindicesOrTriLists,
        clBVHlimits,
        clTriangleIntersectionData,
        clTriIdxList,
        result,
        result_hits);

}


// PRIVATE FUNCTIONS
bool BVHRaycaster::rayIntersectsBox(
    Vector3f origin,
    Ray ray,
    const float* boxPtr)
{
    const float* limitsX2 = boxPtr;
    const float* limitsY2 = boxPtr+2;
    const float* limitsZ2 = boxPtr+4;

    float tmin, tmax, tymin, tymax, tzmin, tzmax;

    tmin =  (limitsX2[    ray.rayDirSign.x()] - origin.x()) * ray.invDir.x();
    tmax =  (limitsX2[1 - ray.rayDirSign.x()] - origin.x()) * ray.invDir.x();
    tymin = (limitsY2[    ray.rayDirSign.y()] - origin.y()) * ray.invDir.y();
    tymax = (limitsY2[1 - ray.rayDirSign.y()] - origin.y()) * ray.invDir.y();

    if ((tmin > tymax) || (tymin > tmax))
    {
        return false;
    }
    if (tymin >tmin)
    {
        tmin = tymin;
    }
    if (tymax < tmax)
    {
        tmax = tymax;
    }

    tzmin = (limitsZ2[    ray.rayDirSign.z()] - origin.z()) * ray.invDir.z();
    tzmax = (limitsZ2[1 - ray.rayDirSign.z()] - origin.z()) * ray.invDir.z();

    if ((tmin > tzmax) || (tzmin > tmax))
    {
        return false;
    }
    if (tzmin > tmin)
    {
        tmin = tzmin;
    }
    if (tzmax < tmax)
    {
        tmax = tzmax;
    }

    return true;
}


typename BVHRaycaster::TriangleIntersectionResult BVHRaycaster::intersectTrianglesBVH(
    const unsigned int* clBVHindicesOrTriLists,
    Vector3f origin,
    Ray ray,
    const float* clBVHlimits,
    const float* clTriangleIntersectionData,
    const unsigned int* clTriIdxList
)
{

    int tid_scale = 4;
    int bvh_limits_scale = 2;

    TriangleIntersectionResult result;
    result.hit = false;
    unsigned int pBestTriId = 0;
    float bestTriDist = std::numeric_limits<float>::max();

    unsigned int stack[BVH_STACK_SIZE];

    int stackId = 0;
    stack[stackId++] = 0;
    Vector3f hitpoint;

    // while stack is not empty
    while (stackId)
    {
        
        unsigned int boxId = stack[stackId - 1];

        stackId--;

        // the first bit of the data of a bvh node indicates whether it is a leaf node, by performing a bitwise and
        // with 0x80000000 all other bits are set to zero and the value of that one bit can be checked
        if (!(clBVHindicesOrTriLists[4 * boxId + 0] & 0x80000000)) // inner node
        {
            
            // if ray intersects inner node, push indices of left and right child nodes on the stack
            if (rayIntersectsBox(origin, ray, &clBVHlimits[bvh_limits_scale * 3 * boxId]))
            {
                stack[stackId++] = clBVHindicesOrTriLists[4 * boxId + 1];
                stack[stackId++] = clBVHindicesOrTriLists[4 * boxId + 2];

                // return if stack size is exceeded
                if ( stackId > BVH_STACK_SIZE)
                {
                    printf("BVH stack size exceeded!\n");
                    result.hit = 0;
                    return result;
                }
            }
        }
        else // leaf node
        {
            
            // iterate over all triangles in this leaf node
            for (
                unsigned int i = clBVHindicesOrTriLists[4 * boxId + 3];
                i < (clBVHindicesOrTriLists[4 * boxId + 3] + (clBVHindicesOrTriLists[4* boxId + 0] & 0x7fffffff));
                i++
            )
            {
                unsigned int idx = clTriIdxList[i];
                const float* normal = clTriangleIntersectionData + tid_scale * 4 * idx;

                float k = normal[0] * ray.dir[0] + normal[1] * ray.dir[1] + normal[2] * ray.dir[2];
                if (k == 0.0f)
                {
                    continue; // this triangle is parallel to the ray -> ignore it
                }
                float s = (normal[3] - (normal[0] * origin[0] + normal[1] * origin[1] + normal[2] * origin[2])  ) / k;
                if (s <= 0.0f)
                {
                    continue; // this triangle is "behind" the origin
                }
                if (s <= EPSILON)
                {
                    continue; // epsilon
                }
                Vector3f hit = ray.dir * s;
                hit += origin;

                // ray triangle intersection
                // check if the intersection with the triangle's plane is inside the triangle
                const float* ee1 = clTriangleIntersectionData + tid_scale * 4 * idx + tid_scale*1;
                float kt1 = ee1[0] * hit[0] + ee1[1] * hit[1] + ee1[2] * hit[2] - ee1[3];
                if (kt1 < 0.0f)
                {
                    continue;
                }
                const float* ee2 = clTriangleIntersectionData + tid_scale * 4 * idx + tid_scale * 2;
                float kt2 = ee2[0] * hit[0] + ee2[1] * hit[1] + ee2[2] * hit[2] - ee2[3];
                if (kt2 < 0.0f)
                {
                    continue;
                }
                const float* ee3 = clTriangleIntersectionData + tid_scale * 4 * idx + tid_scale * 3;
                float kt3 = ee3[0] * hit[0] + ee3[1] * hit[1] + ee3[2] * hit[2] - ee3[3];
                if (kt3 < 0.0f)
                {
                    continue;
                }

                // ray intersects triangle, "hit" is the coordinate of the intersection
                {
                    // check if this intersection closer than others
                    // use quadratic distance for comparison to save some root calculations
                    float hitZ = distanceSquare(origin, hit);
                    if (hitZ < bestTriDist)
                    {
                        bestTriDist = hitZ;
                        pBestTriId = idx;
                        result.hit = true;
                        hitpoint = hit;
                    }
                }

            }
        }
    }

    result.pBestTriId = pBestTriId ;
    result.pointHit = hitpoint;
    result.hitDist = sqrt(bestTriDist);

    return result;
}

void BVHRaycaster::cast_rays_one_one(
        const float* ray_origin,
        const float* rays,
        const unsigned int* clBVHindicesOrTriLists,
        const float* clBVHlimits,
        const float* clTriangleIntersectionData,
        const unsigned int* clTriIdxList,
        float* result,
        uint8_t* result_hits
    )
{
     // get direction and origin of the ray for the current pose
    Vector3f ray_o = {ray_origin[0], ray_origin[1], ray_origin[2]};
    Vector3f ray_d = {rays[0], rays[1], rays[2]};
    ray_d.normalize();

    // initialize result memory with zeros
    result[0] = 0;
    result[1] = 0;
    result[2] = 0;
    result_hits[0] = 0;

    // precompute ray values to speed up intersection calculation
    Ray ray;
    ray.dir = ray_d;
    ray.invDir = {1.0f / ray_d.x(), 1.0f / ray_d.y(), 1.0f / ray_d.z() };
    ray.rayDirSign.x() = ray.invDir.x() < 0;
    ray.rayDirSign.y() = ray.invDir.y() < 0;
    ray.rayDirSign.z() = ray.invDir.z() < 0;

    // intersect all triangles stored in the BVH
    TriangleIntersectionResult resultBVH = intersectTrianglesBVH(
        clBVHindicesOrTriLists,
        ray_o,
        ray,
        clBVHlimits,
        clTriangleIntersectionData,
        clTriIdxList
    );

    // if a triangle was hit, store the calculated hit point in the result at the current id
    if (resultBVH.hit)
    {
        result[0] = resultBVH.pointHit.x();
        result[1] = resultBVH.pointHit.y();
        result[2] = resultBVH.pointHit.z();
        result_hits[0] = 1;
    }

}

void BVHRaycaster::cast_rays_one_multi(
        const float* ray_origin,
        const float* rays,
        size_t num_rays,
        const unsigned int* clBVHindicesOrTriLists,
        const float* clBVHlimits,
        const float* clTriangleIntersectionData,
        const unsigned int* clTriIdxList,
        float* result,
        uint8_t* result_hits
    )
{
     // get direction and origin of the ray for the current pose
    Vector3f ray_o(ray_origin[0], ray_origin[1], ray_origin[2]);

    #pragma omp parallel for
    for(size_t i=0; i< num_rays; i++)
    {
        Vector3f ray_d(rays[i*3], rays[i*3+1], rays[i*3+2]);
        ray_d.normalize();
        // initialize result memory with zeros
        result[i*3] = 0;
        result[i*3+1] = 0;
        result[i*3+2] = 0;
        result_hits[i] = 0;

        // precompute ray values to speed up intersection calculation
        Ray ray;
        ray.dir = ray_d;
        ray.invDir = {1.0f / ray_d.x(), 1.0f / ray_d.y(), 1.0f / ray_d.z() };
        ray.rayDirSign.x() = ray.invDir.x() < 0;
        ray.rayDirSign.y() = ray.invDir.y() < 0;
        ray.rayDirSign.z() = ray.invDir.z() < 0;

        // intersect all triangles stored in the BVH
        TriangleIntersectionResult resultBVH = intersectTrianglesBVH(
            clBVHindicesOrTriLists,
            ray_o,
            ray,
            clBVHlimits,
            clTriangleIntersectionData,
            clTriIdxList
        );

        // if a triangle was hit, store the calculated hit point in the result at the current id
        if (resultBVH.hit)
        {
            result[i*3] = resultBVH.pointHit.x();
            result[i*3+1] = resultBVH.pointHit.y();
            result[i*3+2] = resultBVH.pointHit.z();
            result_hits[i] = 1;
        }
    }
}

void BVHRaycaster::cast_rays_multi_multi(
        const float* ray_origin,
        const float* rays,
        size_t num_rays,
        const unsigned int* clBVHindicesOrTriLists,
        const float* clBVHlimits,
        const float* clTriangleIntersectionData,
        const unsigned int* clTriIdxList,
        float* result,
        uint8_t* result_hits
    )
{
     // get direction and origin of the ray for the current pose
    #pragma omp parallel for
    for(size_t i=0; i< num_rays; i++)
    {
        Vector3f ray_d(rays[i*3], rays[i*3+1], rays[i*3+2]);
        ray_d.normalize();
        Vector3f ray_o(ray_origin[i*3], ray_origin[i*3+1], ray_origin[i*3+2]);

        // initialize result memory with zeros
        result[i*3] = 0;
        result[i*3+1] = 0;
        result[i*3+2] = 0;
        result_hits[i] = 0;

        // precompute ray values to speed up intersection calculation
        Ray ray;
        ray.dir = ray_d;
        ray.invDir = {1.0f / ray_d.x(), 1.0f / ray_d.y(), 1.0f / ray_d.z() };
        ray.rayDirSign.x() = ray.invDir.x() < 0;
        ray.rayDirSign.y() = ray.invDir.y() < 0;
        ray.rayDirSign.z() = ray.invDir.z() < 0;

        // intersect all triangles stored in the BVH
        TriangleIntersectionResult resultBVH = intersectTrianglesBVH(
            clBVHindicesOrTriLists,
            ray_o,
            ray,
            clBVHlimits,
            clTriangleIntersectionData,
            clTriIdxList
        );

        // if a triangle was hit, store the calculated hit point in the result at the current id
        if (resultBVH.hit)
        {
            result[i*3] = resultBVH.pointHit.x();
            result[i*3+1] = resultBVH.pointHit.y();
            result[i*3+2] = resultBVH.pointHit.z();
            result_hits[i] = 1;
        }
    }

}

} // namespace lvr2