#ifndef LBVH_NORMALS_KERNEL
#define LBVH_NORMALS_KERNEL

namespace lbvh 
{

__global__
void calculate_normals_kernel
    (float* points,
    float* queries, size_t num_queries, 
    unsigned int* n_neighbors_out, unsigned int* indices_out, 
    unsigned int* neigh_sum,
    float* normals);
}

#endif // LBVH_NORMALS_KERNEL