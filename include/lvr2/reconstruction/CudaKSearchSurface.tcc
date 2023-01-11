#include "SearchTreeLBVH.hpp"

#include <iostream>

#define NUM 1000000
namespace lvr2
{

template<typename BaseVecT>
CudaKSearchSurface<BaseVecT>::CudaKSearchSurface()
{
    this->setKn(50);

}

template<typename BaseVecT>
CudaKSearchSurface<BaseVecT>::CudaKSearchSurface(
    PointBufferPtr pbuffer,
    size_t k
) : PointsetSurface<BaseVecT>(pbuffer), m_tree(1, true, true)
{
    this->setKn(k);

    // floatArr points = this->m_pointBuffer->getPointArray();
    // float* points_raw = &points[0];
    // int num_points = this->m_pointBuffer->numPoints();
    int num_points = NUM;

    float* points_raw = (float*) malloc(sizeof(float) * num_points);

    for(int i = 0; i < num_points; i++)
    {
        int n = i % 100;
        float j = (float) n;
        points_raw[i] = j + 0.001 * j;
    }
    std::cout << "Building Tree..." << std::endl;
    this->m_tree.build(points_raw, num_points);
    std::cout << "Tree Done!" << std::endl;

}

template<typename BaseVecT>
std::pair<typename BaseVecT::CoordType, typename BaseVecT::CoordType>
    CudaKSearchSurface<BaseVecT>::distance(BaseVecT p) const
{
    // Not implemented here
    throw std::runtime_error("Do not use this function!");
    return std::pair<typename BaseVecT::CoordType, typename BaseVecT::CoordType>(0.0, 0.0);
}

template<typename BaseVecT>
void CudaKSearchSurface<BaseVecT>::calculateSurfaceNormals()
{
    // floatArr points = this->m_pointBuffer->getPointArray();
    
    // float* points_raw = &points[0];
    int num_points = NUM;
    // int num_points = this->m_pointBuffer->numPoints();
    float* points_raw = (float*) malloc(sizeof(float) * num_points);

    for(int i = 0; i < num_points; i++)
    {
        int n = i % 100;
        float j = (float) n;
        points_raw[i] = j + 0.001 * j;
    }
    // TODO Check if 3 floats long
    BaseVecT* query = reinterpret_cast<BaseVecT*>(points_raw);


    int K = this->m_kn;

    size_t size =  3 * num_points;

    // Get the queries
    size_t num_queries = num_points;

    float* queries = points_raw;

    // Create the normal array
    float* normals = (float*) malloc(sizeof(float) * num_queries * 3);

    int mode = 0;
    // #########################################################################################
    if(mode == 0)
    {

        std::cout << "KNN & Normals..." << std::endl;
        this->m_tree.knn_normals(
            queries, 
            num_queries, 
            K,
            normals,
            num_queries
        );
        std::cout << "Done..." << std::endl;
    }

    //##########################################################################################
    if(mode == 1)
    {
        // Create the return arrays
        unsigned int* n_neighbors_out;
        unsigned int* indices_out;
        float* distances_out;

        // Malloc the output arrays here
        n_neighbors_out = (unsigned int*) malloc(sizeof(unsigned int) * num_queries);
        indices_out = (unsigned int*) malloc(sizeof(unsigned int) * num_queries * K);
        distances_out = (float*) malloc(sizeof(float) * num_queries * K);


        std::cout << "KNN Search..." << std::endl;
        // Process the queries 
        this->m_tree.kSearch(queries, num_queries,
                    K,
                    n_neighbors_out, indices_out, distances_out);


        std::cout << "Normals..." << std::endl;
        // Calculate the normals
        this->m_tree.calculate_normals(normals, num_queries,
                    queries, num_queries, K,
                    n_neighbors_out, indices_out);

        std::cout << "Done..." << std::endl;

    }

    // ########################################################################################
    // Set the normals in the point buffer
    floatArr new_normals = floatArr(&normals[0]);

    this->m_pointBuffer->setNormalArray(new_normals, num_points);

}

} // namespace lvr2