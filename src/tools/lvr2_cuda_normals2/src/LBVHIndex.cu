#include "LBVHIndex.cuh"
#include "lbvh_kernels.cuh"

#include <stdio.h>
#include <thrust/sort.h>

#include "GPUErrorCheck.h"

using namespace lbvh;

__host__
LBVHIndex::LBVHIndex(int leaf_size, bool sort_queries, bool compact, bool shrink_to_fit)
{
    m_num_objects = 0;
    m_num_nodes = 0;
    m_leaf_size = leaf_size;
    m_sort_queries = sort_queries;
    m_compact = compact;
    m_shrink_to_fit = shrink_to_fit;

}

__host__
void LBVHIndex::build(float* points, size_t num_points)
{
    m_points = points;

    m_num_objects = num_points;
    m_num_nodes = 2 * m_num_objects - 1;

    // initialize AABBs
    AABB* aabbs = (struct AABB*) malloc(sizeof(struct AABB) * num_points);

    // Initial bounding boxes are the points
    for(int i = 0; i < m_num_objects; i ++)
    {
        aabbs[i].min.x = points[3 * i + 0];
        aabbs[i].max.x = points[3 * i + 0];
        aabbs[i].min.y = points[3 * i + 1];
        aabbs[i].max.y = points[3 * i + 1];
        aabbs[i].min.z = points[3 * i + 2];
        aabbs[i].max.z = points[3 * i + 2];
    }
    // Get the extent
    AABB* extent = (struct AABB*) malloc(sizeof(struct AABB)); 
    getExtent(extent, points, m_num_objects);

    AABB* d_extent;
    gpuErrchk(cudaMalloc(&d_extent, sizeof(struct AABB)));
    //cudaMalloc(&d_extent, sizeof(struct AABB));

    gpuErrchk(cudaMemcpy(d_extent, extent, sizeof(struct AABB), cudaMemcpyHostToDevice));
    //cudaMemcpy(d_extent, extent, sizeof(struct AABB), cudaMemcpyHostToDevice);
    
    AABB* d_aabbs;
    gpuErrchk(cudaMalloc(&d_aabbs, sizeof(struct AABB) * num_points));
    //cudaMalloc(&d_aabbs, sizeof(struct AABB) * num_points);
    
    gpuErrchk(cudaMemcpy(d_aabbs, aabbs, sizeof(struct AABB) * num_points, cudaMemcpyHostToDevice));
    //cudaMemcpy(d_aabbs, aabbs, sizeof(struct AABB) * num_points, cudaMemcpyHostToDevice);

    int size_morton = num_points * sizeof(unsigned long long int);

    int threadsPerBlock = 256;
    int blocksPerGrid = (num_points + threadsPerBlock - 1) 
                        / threadsPerBlock;

    // Get the morton codes of the points
    unsigned long long int* d_morton_codes;
    gpuErrchk(cudaMalloc(&d_morton_codes, size_morton));
    // cudaMalloc(&d_morton_codes, size_morton);

    compute_morton_kernel<<<blocksPerGrid, threadsPerBlock>>>
            (d_aabbs, d_extent, d_morton_codes, num_points);
    
    gpuErrchk(cudaPeekAtLastError());
    
    cudaFree(d_aabbs);
    cudaFree(d_extent);

    gpuErrchk(cudaDeviceSynchronize());
    // cudaDeviceSynchronize();

    unsigned long long int* h_morton_codes = (unsigned long long int*)
                    malloc(sizeof(unsigned long long int) * num_points);

    // printf("Code: %llu \n", h_morton_codes[0]);

    cudaMemcpy(h_morton_codes, d_morton_codes, size_morton, cudaMemcpyDeviceToHost);
    


    // printf("Code: %llu \n", h_morton_codes[0]);

    // thrust::sort_by_key(keys, keys + num_points, values);
    thrust::sort_by_key(h_morton_codes, h_morton_codes + num_points, 
                        aabbs);

    for(int i = 0; i < num_points - 1; i++)
    {
        if(h_morton_codes[i] > h_morton_codes[i + 1])
        {
            printf("Error in sorting \n");
            break;
        }
    }
    
    

    cudaFree(d_morton_codes);
    
    free(aabbs);
    free(extent);
    free(h_morton_codes);

    return;
}

// Get the extent of the points 
// (minimum and maximum values in each dimension)
__host__ 
AABB* LBVHIndex::getExtent(AABB* extent, float* points, size_t num_points)
{
    float min_x = INT_MAX;
    float min_y = INT_MAX;
    float min_z = INT_MAX;

    float max_x = INT_MIN; 
    float max_y = INT_MIN; 
    float max_z = INT_MIN;

    for(int i = 0; i < num_points; i++)
    {
        if(points[3 * i + 0] < min_x)
        {
            min_x = points[3 * i + 0];
        }

        if(points[3 * i + 1] < min_y)
        {
            min_y = points[3 * i + 1];
        }

        if(points[3 * i + 2] < min_z)
        {
            min_z = points[3 * i + 2];
        }

        if(points[3 * i + 0] > max_x)
        {
            max_x = points[3 * i + 0];
        }

        if(points[3 * i + 1] > max_y)
        {
            max_y = points[3 * i + 1];
        }

        if(points[3 * i + 2] > max_z)
        {
            max_z = points[3 * i + 2];
        }
    }
    
    extent->min.x = min_x;
    extent->min.y = min_y;
    extent->min.z = min_z;
    
    extent->max.x = max_x;
    extent->max.y = max_y;
    extent->max.z = max_z;
    
    return extent;
}