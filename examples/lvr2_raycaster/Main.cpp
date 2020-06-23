#include <iostream>
#include <memory>
#include <tuple>
#include <stdlib.h>

#include <boost/optional.hpp>
#include <chrono>

// lvr2 includes
#include "lvr2/util/Synthetic.hpp"
#include "lvr2/io/MeshBuffer.hpp"
#include "lvr2/io/ModelFactory.hpp"
#include "lvr2/geometry/BaseVector.hpp"

#include "lvr2/algorithm/raycasting/RaycasterBase.hpp"

// LVR2 internal raycaster that is always available
#include "lvr2/algorithm/raycasting/BVHRaycaster.hpp"

#if defined LVR2_USE_OPENCL
#include "lvr2/algorithm/raycasting/CLRaycaster.hpp"
#endif
#if defined LVR2_USE_EMBREE
#include "lvr2/algorithm/raycasting/EmbreeRaycaster.hpp"
#endif


using std::unique_ptr;
using std::make_unique;

using namespace lvr2;


MeshBufferPtr genMesh(){
    MeshBufferPtr dst_mesh = MeshBufferPtr(new MeshBuffer);

    float *vertices = new float[12];
    float *normals = new float[12];
    unsigned int *face_indices = new unsigned int[12];

    // gen vertices
    vertices[0] = 200.0; vertices[1] = -500.0; vertices[2] = -500.0;
    vertices[3] = 200.0; vertices[4] = -500.0; vertices[5] = 500.0;
    vertices[6] = 2000.0; vertices[7] = 500.0; vertices[8] = 500.0;
    vertices[9] = 2000.0; vertices[10] = 500.0; vertices[11] = -500.0;

    // gen normals
    normals[0] = 1.0; normals[1] = 0.0; normals[2] = 0.0;
    normals[3] = 1.0; normals[4] = 0.0; normals[5] = 0.0;
    normals[6] = 1.0; normals[7] = 0.0; normals[8] = 0.0;
    normals[9] = 1.0; normals[10] = 0.0; normals[11] = 0.0;

    // gen faces
    face_indices[0] = 0; face_indices[1] = 1; face_indices[2] = 2;
    face_indices[3] = 0; face_indices[4] = 2; face_indices[5] = 3;

    floatArr vertex_arr(vertices);
    floatArr normal_arr(normals);
    indexArray face_index_arr(face_indices);
    

    // append to mesh
    dst_mesh->setVertices(vertex_arr, 4);
    dst_mesh->setVertexNormals(normal_arr);
    dst_mesh->setFaceIndices(face_index_arr, 2);

    return dst_mesh;
}

void genRays(std::vector<Vector3f>& origins,
std::vector<Vector3f>& directions,
float scale = 1.0,
bool flip_axis = false)
{
    origins.resize(0);
    directions.resize(0);

    // gen origins

    origins.push_back({0.0,0.0,0.0});
    origins.push_back({0.0,1.0,0.0});
    origins.push_back({0.0,2.0,0.0});
    origins.push_back({0.0,3.0,0.0});
    origins.push_back({0.0,4.0,0.0});
    origins.push_back({0.0,5.0,0.0});

    origins.push_back({0.0,6.0,1.0});
    origins.push_back({0.0,6.0,2.0});

    origins.push_back({0.0,7.0,3.0});
    origins.push_back({0.0,7.0,4.0});
    origins.push_back({0.0,7.0,5.0});

    origins.push_back({0.0,8.0,6.0});
    origins.push_back({0.0,8.0,7.0});
    origins.push_back({0.0,8.0,8.0});
    origins.push_back({0.0,8.0,9.0});
    origins.push_back({0.0,8.0,10.0});
    origins.push_back({0.0,8.0,11.0});
    origins.push_back({0.0,8.0,12.0});
    origins.push_back({0.0,8.0,13.0});

    origins.push_back({0.0,7.0,14.0});

    origins.push_back({0.0,6.0,15.0});

    origins.push_back({0.0,5.0,15.0});
    origins.push_back({0.0,5.0,14.0});
    origins.push_back({0.0,5.0,13.0});
    origins.push_back({0.0,5.0,12.0});

    origins.push_back({0.0,4.0,16.0});
    origins.push_back({0.0,3.0,16.0});

    origins.push_back({0.0,2.0,16.0});
    origins.push_back({0.0,2.0,15.0});
    origins.push_back({0.0,2.0,14.0});
    origins.push_back({0.0,2.0,13.0});
    origins.push_back({0.0,2.0,17.0});
    origins.push_back({0.0,2.0,18.0});
    origins.push_back({0.0,2.0,19.0});
    origins.push_back({0.0,2.0,20.0});
    origins.push_back({0.0,2.0,21.0});
    origins.push_back({0.0,2.0,22.0});

    origins.push_back({0.0,1.0,23.0});
    origins.push_back({0.0,0.0,23.0});

    origins.push_back({0.0,-1.0,22.0});
    origins.push_back({0.0,-1.0,21.0});
    origins.push_back({0.0,-1.0,20.0});
    origins.push_back({0.0,-1.0,19.0});
    origins.push_back({0.0,-1.0,18.0});
    origins.push_back({0.0,-1.0,17.0});
    origins.push_back({0.0,-1.0,16.0});
    origins.push_back({0.0,-1.0,15.0});
    origins.push_back({0.0,-1.0,14.0});

    origins.push_back({0.0,-2.0,16.0});
    origins.push_back({0.0,-3.0,16.0});

    origins.push_back({0.0,-4.0,15.0});
    origins.push_back({0.0,-4.0,14.0});
    origins.push_back({0.0,-4.0,13.0});
    origins.push_back({0.0,-4.0,12.0});
    origins.push_back({0.0,-4.0,11.0});

    origins.push_back({0.0,-5.0,12.0});

    origins.push_back({0.0,-6.0,13.0});
    origins.push_back({0.0,-7.0,13.0});

    origins.push_back({0.0,-8.0,12.0});

    origins.push_back({0.0,-9.0,11.0});
    origins.push_back({0.0,-9.0,10.0});

    origins.push_back({0.0,-8.0,9.0});

    origins.push_back({0.0,-7.0,8.0});
    origins.push_back({0.0,-7.0,7.0});

    origins.push_back({0.0,-6.0,6.0});
    origins.push_back({0.0,-6.0,5.0});

    origins.push_back({0.0,-5.0,4.0});
    origins.push_back({0.0,-5.0,3.0});

    origins.push_back({0.0,-4.0,2.0});
    origins.push_back({0.0,-4.0,1.0});

    origins.push_back({0.0,-3.0,0.0});
    origins.push_back({0.0,-2.0,0.0});
    origins.push_back({0.0,-1.0,0.0});




    // same rays and scaling and flipping
    for(int i=0; i<origins.size(); i++)
    {
        origins[i] *= scale;

        if(flip_axis)
        {
            float tmp = origins[i].y();
            origins[i].y() = origins[i].z();
            origins[i].z() = tmp;
        }

        directions.push_back({1.0,0.0,0.0});
    }

}

void test1(RaycasterBasePtr rc)
{
    std::cout << "Raycast Test 1 started" << std::endl;

    Vector3f origin = {20.0,40.0,50.0};
    Vector3f ray = {1.0,0.0,0.0};


    std::vector<Vector3f> origins;
    std::vector<Vector3f> rays;

    for(int i=0; i<100; i++)
    {
        origins.push_back(origin);
        rays.push_back(ray);
    }

    
    std::cout << "TEST 1: one origin, one ray." << std::endl;

    Vector3f intersection;
    bool success = rc->castRay(origin, ray, intersection);
    

    if(success)
    {
        std::cout << "success!" << std::endl;
        std::cout << intersection << std::endl;
    } else {
        std::cout << "NOT succesful!" << std::endl;
    }

    std::cout << "TEST 2: one origin, mulitple rays." << std::endl;

    std::vector<Vector3f> intersections1, intersections2;
    std::vector<uint8_t> hits1, hits2;

    rc->castRays(origin, rays, intersections1, hits1);

    success = true;
    for(int i=0;i<hits1.size(); i++)
    {
        success = hits1[i];
        if(!success)
        {
            break;
        }
    }

    if(intersections1.size() == 0)
    {
        success = false;
    }

    if(success)
    {
        std::cout << "success!" << std::endl;
        std::cout << intersections1[99] << std::endl;
    } else {
        std::cout << "NOT succesful!" << std::endl;
    }


    std::cout << "TEST 3: multiple origins, mulitple rays." << std::endl;

    origins[99].y() = -444;

    rc->castRays(origins, rays, intersections2, hits2);



    success = true;
    for(int i=0;i<hits2.size(); i++)
    {
        if(!hits2[i])
        {
            success = false;
        }
    }

    if(intersections2.size() == 0)
    {
        success = false;
    }

    if(success)
    {
        std::cout << "success!" << std::endl;
        std::cout << intersections2[99] << std::endl;
    } else {
        std::cout << "NOT succesful!" << std::endl;
    }
}

void test2(RaycasterBasePtr rc)
{
    std::vector<Vector3f> origins;
    std::vector<Vector3f> rays;

    genRays(origins, rays, 10.0, true);

    std::vector<Vector3f> intersections;
    std::vector<uint8_t> hits;

    rc->castRays(origins, rays, intersections, hits);

    if(hits.size() == 0)
    {
        return;
    }

    std::vector<Vector3f> results;

    

    for(int i=0; i<hits.size(); i++)
    {
        bool hit = hits[i];
        if(hit)
        {
            results.push_back(intersections[i]);
            
        }
    }

    floatArr points(new float[results.size() * 3]);

    for(int i=0; i<results.size(); i++)
    {
        points[i*3+0] = results[i].x();
        points[i*3+1] = results[i].y();
        points[i*3+2] = results[i].z();
    }


    PointBufferPtr p_buffer(new PointBuffer(points, results.size()));
    ModelPtr model(new Model(p_buffer));

    ModelFactory::saveModel(model, "projected_points.ply");

    // save origins
    floatArr origin_arr(new float[origins.size() * 3]);

    for(int i=0; i<origins.size(); i++)
    {
        origin_arr[i*3+0] = origins[i].x();
        origin_arr[i*3+1] = origins[i].y();
        origin_arr[i*3+2] = origins[i].z();
    }


    PointBufferPtr orig_buffer(new PointBuffer(origin_arr, origins.size()));
    ModelPtr orig_model(new Model(orig_buffer));

    ModelFactory::saveModel(orig_model, "origins.ply");
    
}

double test3(RaycasterBasePtr rc, size_t num_rays=984543)
{
    
    int u_max = int(3027.8730 * 2);
    int v_max = int(2031.0270 * 2);

    Vector3f origin = {0.0,0.0,0.0};
    std::vector<Vector3f > rays(num_rays);

    for(int i=0; i<num_rays; i++)
    {
        Vector3f ray_world = {1.0,0.0,0.0};
        rays[i].x() = 1.0;
        rays[i].y() = 0.0;
        rays[i].z() = 0.0;
    }

    auto start = std::chrono::steady_clock::now();
    std::vector<Vector3f > intersections;
    std::vector<uint8_t> hits;

    rc->castRays(origin, rays, intersections, hits);

    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

float floatInRange(float LO, float HI)
{
    return LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
}

double realTest(RaycasterBasePtr rc, size_t num_rays=984543)
{
    
    // MeshBufferPtr sphere = synthetic::genSphere(10, 10);

    Vector3f origin = {0.0,0.0,0.0};
    std::vector<Vector3f > rays(num_rays);

    for(int i=0; i<num_rays; i++)
    {
        float x = floatInRange(-1.0, 1.0);
        float y = floatInRange(-1.0, 1.0);
        float z = floatInRange(-1.0, 1.0);

        float norm = sqrt(x*x + y*y + z*z);
        x /= norm;
        y /= norm;
        z /= norm;

        rays[i].x() = x;
        rays[i].y() = y;
        rays[i].z() = z;
    }

    auto start = std::chrono::steady_clock::now();
    std::vector<Vector3f > intersections;
    std::vector<uint8_t> hits;

    rc->castRays(origin, rays, intersections, hits);
    auto end = std::chrono::steady_clock::now();
    
    int num_hits = 0;
    for(int i=0; i<hits.size(); i++)
    {
        bool success = hits[i];
        if(success)
        {
            num_hits++;
        }
    }

    std::cout << "hits: " << num_hits << std::endl;

    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

int main(int argc, char** argv)
{
    int num_rays = 1000000;

    if(argc < 2)
    {
        MeshBufferPtr buffer = synthetic::genSphere(50, 50);

        // create a raycaster
        RaycasterBasePtr raycaster;

        // CPU test
        std::cout << "Testing BVHRaycaster" << std::endl;
        raycaster.reset(new BVHRaycaster(buffer));
        std::cout << realTest(raycaster, num_rays) << " ms" << std::endl;

        // GPU test
        #if defined LVR2_USE_OPENCL
        std::cout << "Testing CLRaycaster" << std::endl;
        raycaster.reset(new CLRaycaster(buffer));
        std::cout << realTest(raycaster, num_rays) << " ms" << std::endl;
        #endif

        #if defined LVR2_USE_EMBREE
        std::cout << "Testing EmbreeRaycaster" << std::endl;
        raycaster.reset(new EmbreeRaycaster(buffer));
        std::cout << realTest(raycaster, num_rays) << " ms" << std::endl;
        #endif
    } else {
        std::string filename(argv[1]);

    }

    


    return 0;
}