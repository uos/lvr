// #include "lvr2/reconstruction/cuda/lbvh/normals_kernel.cuh"

// #include <stdio.h>

// using namespace lvr2;
// using namespace lbvh;

// namespace lvr2
// {

// __global__ void lbvh::calculate_normals_kernel(float* points, 
//     float* queries, size_t num_queries, 
//     int K,
//     unsigned int* n_neighbors_out, unsigned int* indices_out, 
//     // unsigned int* neigh_sum,
//     float* normals,
//     float flip_x, float flip_y, float flip_z)
// {
//     unsigned int tid = blockIdx.x * blockDim.x + threadIdx.x;

//     if(tid >= num_queries)
//     {
//         return;
//     }

//     // http://www.ilikebigbits.com/2017_09_25_plane_from_points_2.html

//     unsigned int n = n_neighbors_out[tid];
//     const float nf = (float) n;

//     if(n < 3)
//     {
//         normals[3 * tid + 0] = 0.0f;
//         normals[3 * tid + 1] = 0.0f;
//         normals[3 * tid + 2] = 0.0f;
//         return; // Not enough neighbors
//     }

//     // Get the centroid
//     float3 sum = {0.0f, 0.0f, 0.0f};

//     for(int i = 0; i < n; i++)
//     {
//         // Coordinates of point which is the i-th neighbor
//         sum.x += points[ 3 * indices_out[K * tid + i] + 0];
//         sum.y += points[ 3 * indices_out[K * tid + i] + 1];
//         sum.z += points[ 3 * indices_out[K * tid + i] + 2];
//     }

//     sum.x /= nf; // x,y,z coordinates of centroid
//     sum.y /= nf;
//     sum.z /= nf;

//     // Calculate the covariance matrix
//     float xx = 0.0f; float xy = 0.0f; float xz = 0.0f;
//     float yy = 0.0f; float yz = 0.0f;
//     float zz = 0.0f;

//     for(int i = 0; i < n; i++)
//     {
//         float3 r =
//         {
//             points[ 3 * indices_out[K * tid + i] + 0] - sum.x,
//             points[ 3 * indices_out[K * tid + i] + 1] - sum.y,
//             points[ 3 * indices_out[K * tid + i] + 2] - sum.z
//         };
       
//         xx += r.x * r.x;
//         xy += r.x * r.y;
//         xz += r.x * r.z;
//         yy += r.y * r.y;
//         yz += r.y * r.z;
//         zz += r.z * r.z;
        
//     }

//     xx /= nf;
//     xy /= nf;
//     xz /= nf;
//     yy /= nf;
//     yz /= nf;
//     zz /= nf;
    
//     float3 weighted_dir = {0.0f, 0.0f, 0.0f};
//     float3 axis_dir;

//     // For x
//     float det_x = yy*zz - yz*yz;

//     axis_dir.x = det_x;
//     axis_dir.y = xz*yz - xy*zz;
//     axis_dir.z = xy*yz - xz*yy;

//     float weight = det_x * det_x;

//     if(weighted_dir.x * axis_dir.x + weighted_dir.y * weighted_dir.y + weighted_dir.z * weighted_dir.z < 0.0f)
//     {
//         weight *= -1;
//     }
    
//     weighted_dir.x += axis_dir.x * weight;
//     weighted_dir.y += axis_dir.y * weight;
//     weighted_dir.z += axis_dir.z * weight;


//     // For y
//     float det_y = xx*zz - xz*xz;

//     axis_dir.x = xz*yz - xy*zz;
//     axis_dir.y = det_y;
//     axis_dir.z = xy*xz - yz*xx;

//     weight = det_y * det_y;

//     if(weighted_dir.x * axis_dir.x + weighted_dir.y * weighted_dir.y + weighted_dir.z * weighted_dir.z < 0.0f)
//     {
//         weight *= -1;
//     }
    
//     weighted_dir.x += axis_dir.x * weight;
//     weighted_dir.y += axis_dir.y * weight;
//     weighted_dir.z += axis_dir.z * weight;


//     // For z
//     float det_z = xx*yy - xy*xy;

//     axis_dir.x = xy*yz - xz*yy;
//     axis_dir.y = xy*xz - yz*xx;
//     axis_dir.z = det_z;

//     weight = det_z * det_z;

//     if(weighted_dir.x * axis_dir.x + weighted_dir.y * weighted_dir.y + weighted_dir.z * weighted_dir.z < 0.0f)
//     {
//         weight *= -1;
//     }
    
//     weighted_dir.x += axis_dir.x * weight;
//     weighted_dir.y += axis_dir.y * weight;
//     weighted_dir.z += axis_dir.z * weight;

//     // Create the normal
//     float3 normal = weighted_dir;

//     // Normalize normal
//     float mag = sqrt((normal.x * normal.x) + (normal.y * normal.y) + (normal.z * normal.z));

//     normal.x /= mag;
//     normal.y /= mag;
//     normal.z /= mag;

//     // Check if the normals need to be flipped
//     float vertex_x = queries[3 * tid + 0];
//     float vertex_y = queries[3 * tid + 1];
//     float vertex_z = queries[3 * tid + 2];

//     // flip the normals
//     float x_dir = flip_x - vertex_x;
//     float y_dir = flip_y - vertex_y;
//     float z_dir = flip_z - vertex_z;

//     float scalar = x_dir * normal.x + y_dir * normal.y + z_dir * normal.z;

//     // TODO < or > ?
//     if(scalar < 0)
//     {
//         normal.x = -normal.x;
//         normal.y = -normal.y;
//         normal.z = -normal.z;
//     }

//     // TODO Set normals to zero if nan or inf values occur
//     // Done?
//     if(!(scalar <= 0 || scalar >= 0) || isinf(scalar))
//     {
//         normal.x = 0.0f;
//         normal.y = 0.0f;
//         normal.z = 0.0f;
//     }

//     // TODO Is the indexing here correct?
//     // Set the normal in the normal array
//     normals[3 * tid + 0] = normal.x;
//     normals[3 * tid + 1] = normal.y;
//     normals[3 * tid + 2] = normal.z;
// }

// } // namespace lvr2


#include "lvr2/reconstruction/cuda/lbvh/normals_kernel.cuh"

#include <stdio.h>

using namespace lvr2;
using namespace lbvh;

namespace lvr2
{

__global__ void lbvh::calculate_normals_kernel(
    float* points, 
    float* queries, 
    size_t num_queries, 
    int K,
    unsigned int* n_neighbors_in, 
    unsigned int* indices_in, 
    float* normals,
    float flip_x, 
    float flip_y, 
    float flip_z
)
{
    unsigned int tid = blockIdx.x * blockDim.x + threadIdx.x;

    if(tid >= num_queries)
    {
        return;
    }
   
    // http://www.ilikebigbits.com/2017_09_25_plane_from_points_2.html

    unsigned int n = n_neighbors_in[tid];
    const double nf = (double) n - 1;

    if(n < 3)
    {
        normals[3 * tid + 0] = 0.0f;
        normals[3 * tid + 1] = 0.0f;
        normals[3 * tid + 2] = 0.0f;
        return; // Not enough neighbors
    }

    // Get the centroid
    double3 sum = {0.0, 0.0, 0.0};

    for(int i = 1; i < n; i++)
    {
        // Coordinates of point which is the i-th neighbor
        sum.x += (double) points[ 3 * indices_in[K * tid + i] + 0] / nf;
        sum.y += (double) points[ 3 * indices_in[K * tid + i] + 1] / nf;
        sum.z += (double) points[ 3 * indices_in[K * tid + i] + 2] / nf;
    }

    // Doing this in the for loop above leads to less uninitialised normals
    // sum.x /= nf; // x,y,z coordinates of centroid
    // sum.y /= nf;
    // sum.z /= nf;

    // Calculate the covariance matrix
    double xx = 0.0f; double xy = 0.0f; double xz = 0.0f;
    double yy = 0.0f; double yz = 0.0f;
    double zz = 0.0f;

    for(int i = 1; i < n; i++)
    {
        double3 r =
        {
            (double) points[ 3 * indices_in[K * tid + i] + 0] - sum.x,
            (double) points[ 3 * indices_in[K * tid + i] + 1] - sum.y,
            (double) points[ 3 * indices_in[K * tid + i] + 2] - sum.z
        };
       
        xx += r.x * r.x / nf;
        xy += r.x * r.y / nf;
        xz += r.x * r.z / nf;
        yy += r.y * r.y / nf;
        yz += r.y * r.z / nf;
        zz += r.z * r.z / nf;
        
    }

    // Doing this in the for loop above leads to less uninitialised normals
    // xx /= nf;
    // xy /= nf;
    // xz /= nf;
    // yy /= nf;
    // yz /= nf;
    // zz /= nf;
    
    double3 weighted_dir = {0.0f, 0.0f, 0.0f};
    double3 axis_dir;

    // For x
    double det_x = yy*zz - yz*yz;

    axis_dir.x = det_x;
    axis_dir.y = xz*yz - xy*zz;
    axis_dir.z = xy*yz - xz*yy;

    double weight = det_x * det_x;

    if(weighted_dir.x * axis_dir.x + weighted_dir.y * axis_dir.y + weighted_dir.z * axis_dir.z < 0.0f)
    {
        weight *= -1;
    }
    
    weighted_dir.x += axis_dir.x * weight;
    weighted_dir.y += axis_dir.y * weight;
    weighted_dir.z += axis_dir.z * weight;


    // For y
    double det_y = xx*zz - xz*xz;

    axis_dir.x = xz*yz - xy*zz;
    axis_dir.y = det_y;
    axis_dir.z = xy*xz - yz*xx;

    weight = det_y * det_y;

    if(weighted_dir.x * axis_dir.x + weighted_dir.y * axis_dir.y + weighted_dir.z * axis_dir.z < 0.0f)
    {
        weight *= -1;
    }
    
    weighted_dir.x += axis_dir.x * weight;
    weighted_dir.y += axis_dir.y * weight;
    weighted_dir.z += axis_dir.z * weight;


    // For z
    double det_z = xx*yy - xy*xy;

    axis_dir.x = xy*yz - xz*yy;
    axis_dir.y = xy*xz - yz*xx;
    axis_dir.z = det_z;

    weight = det_z * det_z;

    if(weighted_dir.x * axis_dir.x + weighted_dir.y * axis_dir.y + weighted_dir.z * axis_dir.z < 0.0f)
    {
        weight *= -1;
    }
    
    weighted_dir.x += axis_dir.x * weight;
    weighted_dir.y += axis_dir.y * weight;
    weighted_dir.z += axis_dir.z * weight;

    // Create the normal
    double3 normal = weighted_dir;

    // Normalize normal
    double mag = sqrt((normal.x * normal.x) + (normal.y * normal.y) + (normal.z * normal.z));

    normal.x /= mag;
    normal.y /= mag;
    normal.z /= mag;

    // Check if the normals need to be flipped
    double vertex_x = queries[3 * tid + 0];
    double vertex_y = queries[3 * tid + 1];
    double vertex_z = queries[3 * tid + 2];

    double x_dir = flip_x - vertex_x;
    double y_dir = flip_y - vertex_y;
    double z_dir = flip_z - vertex_z;

    double scalar = x_dir * normal.x + y_dir * normal.y + z_dir * normal.z;

    // Set normals to zero if nan or inf values occur
    if(!(scalar <= 0 || scalar >= 0) || isinf(scalar))
    {
        normal.x = 0.0f;
        normal.y = 0.0f;
        normal.z = 0.0f;
    }

    // flip the normals
    if(scalar < 0)
    {
        normal.x = -normal.x;
        normal.y = -normal.y;
        normal.z = -normal.z;
    }

    // Set the normal in the normal array
    normals[3 * tid + 0] = (float) normal.x;
    normals[3 * tid + 1] = (float) normal.y;
    normals[3 * tid + 2] = (float) normal.z;

    return;
}

} // namespace lvr2
