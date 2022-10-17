#ifndef LBVHINDEX_CUH
#define LBVHINDEX_CUH

#include "aabb.cuh"
// #include "lbvh_kernels.cuh"

namespace lbvh
{
class LBVHIndex
{
public:
    unsigned int m_num_objects;
    unsigned int m_num_nodes;
    unsigned int m_leaf_size;
    bool m_sort_queries;
    bool m_compact;
    bool m_shrink_to_fit;

    float* m_points;
    int* m_sorted_indices;
    
    __host__
    LBVHIndex(int leaf_size, bool sort_queries, bool compact, bool shrink_to_fit);

    __host__
    void build(float* points, size_t num_points);

    __host__ 
    AABB* getExtent(AABB* extent, float* points, size_t num_points);
};

}   // namespace lbvh

#endif // LBVHINDEX_CUH