#pragma once
#include "query.cuh"
#include "static_priorityqueue.cuh"

// default is one nearest neighbor
// #ifndef K
// #define K 50
// #endif

namespace lvr2
{

namespace lbvh 
{
    __forceinline__ __device__ void query_knn(
        const BVHNode* __restrict__ nodes,
        const float* __restrict__ points,          // Changed from float3* to float*
        const unsigned int* __restrict__ sorted_indices,
        unsigned int root_index,
        const float3* __restrict__ query_point,
        StaticPriorityQueue<float, K>& queue)
    {
        query<StaticPriorityQueue<float, K>>(nodes, points, sorted_indices, root_index, query_point, queue);
    }

    __forceinline__ __device__ StaticPriorityQueue<float, K> query_knn(
        const BVHNode* __restrict__ nodes,
        const float* __restrict__ points,          // Changed from float3* to float*
        const unsigned int* __restrict__ sorted_indices,
        unsigned int root_index,
        const float3* __restrict__ query_point,
        const float max_radius
    )
    {
        StaticPriorityQueue<float, K> queue(max_radius);
        query_knn(nodes, points, sorted_indices, root_index, query_point, queue);
        return queue;
        // std::move(queue);
    }
} // namespace lbvh

} // namespace lvr2

