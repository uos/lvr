#include <iostream>
#include <string>
#include <lvr/io/Model.hpp>
#include "BigGrid.hpp"
#include "lvr/io/DataStruct.hpp"
#include "lvr/io/PointBuffer.hpp"
#include "lvr/io/Model.hpp"
#include "lvr/io/PLYIO.hpp"
#include "lvr/geometry/BoundingBox.hpp"
#include "BigGridKdTree.hpp"
#include <fstream>
#include <sstream>
#include <lvr/io/Timestamp.hpp>
#include <lvr/reconstruction/PointsetSurface.hpp>
#include <lvr/geometry/ColorVertex.hpp>
#include <lvr/reconstruction/AdaptiveKSearchSurface.hpp>
#include <lvr/geometry/ColorVertex.hpp>
#include <lvr/geometry/Normal.hpp>
#include <lvr/reconstruction/HashGrid.hpp>
#include <lvr/reconstruction/FastReconstruction.hpp>
#include <lvr/reconstruction/PointsetGrid.hpp>
#include <lvr/geometry/QuadricVertexCosts.hpp>
#include <lvr/reconstruction/FastBox.hpp>
#include <lvr/geometry/HalfEdgeMesh.hpp>
#include "lvr/reconstruction/QueryPoint.hpp"
#include <fstream>
#include <sstream>
#include "LargeScaleOptions.hpp"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/algorithm/string/replace.hpp>
#include "LineReader.hpp"
#include <random>
using std::cout;
using std::endl;
using namespace lvr;

#if defined CUDA_FOUND
    #define GPU_FOUND

    #include <lvr/reconstruction/cuda/CudaSurface.hpp>

    typedef CudaSurface GpuSurface;
#elif defined OPENCL_FOUND
    #define GPU_FOUND

    #include <lvr/reconstruction/opencl/ClSurface.hpp>
    typedef ClSurface GpuSurface;
#endif


typedef lvr::PointsetSurface<lvr::ColorVertex<float, unsigned char> > psSurface;
typedef lvr::AdaptiveKSearchSurface<lvr::ColorVertex<float, unsigned char>, lvr::Normal<float> > akSurface;

int main(int argc, char** argv)
{
//make sure to include the random number generators and such

//    std::random_device seeder;
//    std::mt19937 engine(seeder());
//    std::uniform_real_distribution<float> dist(10, 10.05);
//    std::random_device seeder2;
//    std::mt19937 engine2(seeder());
//    std::uniform_real_distribution<float> dist2(0, 10);
//    ofstream testofs("plane.pts");
//    for(float x = 0; x < 10; x+=0.05)
//    {
//        for(float y = 0 ; y < 10 ; y+=0.05)
//        {
//            testofs << dist2(engine2) << " " << dist2(engine2) << " " << dist(engine) << endl;
//        }
//    }


    LargeScaleOptions::Options options(argc, argv);

    string filePath = options.getInputFileName();
    float voxelsize = options.getVoxelsize();
    float scale = options.getScaling();
    std::vector<float> flipPoint = options.getFlippoint();
    cout << lvr::timestamp << "Starting grid" << endl;
    BigGrid bg(filePath, voxelsize, scale);
    cout << lvr::timestamp << "grid finished " << endl;
    lvr::BoundingBox<lvr::Vertexf> bb = bg.getBB();
    cout << bb << endl;


    //lvr::floatArr points = bg.getPointCloud(numPoints);

    cout << lvr::timestamp << "making tree" << endl;
    BigGridKdTree gridKd(bg.getBB(),options.getNodeSize(),&bg, voxelsize);
    gridKd.insert(bg.pointSize(),bg.getBB().getCentroid());
    cout << lvr::timestamp << "finished tree" << endl;

    std::cout << lvr::timestamp << "got: " << gridKd.getLeafs().size() << " leafs, saving leafs" << std::endl;



    BoundingBox<ColorVertex<float,unsigned char> > cbb(bb.getMin().x, bb.getMin().y, bb.getMin().z,
                                                       bb.getMax().x, bb.getMax().y, bb.getMax().z);

    vector<string> grid_files;

    for(int i = 0 ; i < gridKd.getLeafs().size() ; i++)
    {
        size_t numPoints;

        //todo: okay?
        lvr::floatArr points = bg.points(gridKd.getLeafs()[i]->getBB().getMin().x - voxelsize*3, gridKd.getLeafs()[i]->getBB().getMin().y - voxelsize*3, gridKd.getLeafs()[i]->getBB().getMin().z - voxelsize*3 ,
                                         gridKd.getLeafs()[i]->getBB().getMax().x + voxelsize*3, gridKd.getLeafs()[i]->getBB().getMax().y + voxelsize*3, gridKd.getLeafs()[i]->getBB().getMax().z + voxelsize*3,numPoints);

        //std::cout << "i: " << std::endl << bb << std::endl << "got : " << numPoints << std::endl;
        if(numPoints<=50) continue;

        BoundingBox<ColorVertex<float,unsigned char> > gridbb(gridKd.getLeafs()[i]->getBB().getMin().x, gridKd.getLeafs()[i]->getBB().getMin().y, gridKd.getLeafs()[i]->getBB().getMin().z,
                                                              gridKd.getLeafs()[i]->getBB().getMax().x, gridKd.getLeafs()[i]->getBB().getMax().y, gridKd.getLeafs()[i]->getBB().getMax().z);
        cout << "grid: " << i << "/" << gridKd.getLeafs().size()-1 << endl;
        cout << "grid has " << numPoints << " points" << endl;
        cout << "kn=" << options.getKn() << endl;
        cout << "ki=" << options.getKi() << endl;
        cout << "kd=" << options.getKd() << endl;
        cout << gridbb << endl;
        lvr::PointBufferPtr p_loader(new lvr::PointBuffer);
        p_loader->setPointArray(points, numPoints);

        if(bg.hasNormals())
        {

            size_t numNormals;
            lvr::floatArr normals = bg.normals(gridKd.getLeafs()[i]->getBB().getMin().x, gridKd.getLeafs()[i]->getBB().getMin().y, gridKd.getLeafs()[i]->getBB().getMin().z ,
                                              gridKd.getLeafs()[i]->getBB().getMax().x, gridKd.getLeafs()[i]->getBB().getMax().y, gridKd.getLeafs()[i]->getBB().getMax().z,numNormals);


            p_loader->setPointNormalArray(normals, numNormals);
        } else {
            #ifdef GPU_FOUND
            if( options.useGPU() )
            {

                floatArr normals = floatArr(new float[ numPoints * 3 ]);
                cout << timestamp << "Constructing kd-tree..." << endl;
                GpuSurface gpu_surface(points, numPoints);
                cout << timestamp << "Finished kd-tree construction." << endl;
                gpu_surface.setKn( options.getKn() );
                gpu_surface.setKi( options.getKi() );
                gpu_surface.setFlippoint(flipPoint[0], flipPoint[1], flipPoint[2]);
                cout << timestamp << "Start Normal Calculation..." << endl;
                gpu_surface.calculateNormals();
                gpu_surface.getNormals(normals);
                cout << timestamp << "Finished Normal Calculation. " << endl;
                p_loader->setPointNormalArray(normals, numPoints);
                gpu_surface.freeGPU();
            }
            #else
                cout << "ERROR: OpenCl not found" << endl;
                exit(-1);
            #endif
        }

        psSurface::Ptr surface = psSurface::Ptr(new akSurface(
            p_loader,
            "FLANN",
            options.getKn(),
            options.getKi(),
            options.getKd(),
            options.useRansac()

//                p_loader, pcm_name,
//                options.getKn(),
//                options.getKi(),
//                options.getKd(),
//                options.useRansac(),
//                options.getScanPoseFile(),
//                center

        ));



        if(! bg.hasNormals() && !options.useGPU()) {
            surface->calculateSurfaceNormals();
        }

        lvr::GridBase* grid;
        lvr::FastReconstructionBase<lvr::ColorVertex<float, unsigned char>, lvr::Normal<float> >* reconstruction;

        grid = new PointsetGrid<ColorVertex<float, unsigned char>, FastBox<ColorVertex<float, unsigned char>, Normal<float> > >(voxelsize, surface,gridbb , true, options.extrude());
//        FastBox<ColorVertex<float, unsigned char>, Normal<float> >::m_surface = surface;
        PointsetGrid<ColorVertex<float, unsigned char>, FastBox<ColorVertex<float, unsigned char>, Normal<float> > >* ps_grid = static_cast<PointsetGrid<ColorVertex<float, unsigned char>, FastBox<ColorVertex<float, unsigned char>, Normal<float> > > *>(grid);
        ps_grid->setBB(gridbb);
        ps_grid->calcIndices();
        ps_grid->calcDistanceValues();

        reconstruction = new FastReconstruction<ColorVertex<float, unsigned char> , Normal<float>, FastBox<ColorVertex<float, unsigned char>, Normal<float> >  >(ps_grid);
        HalfEdgeMesh<ColorVertex<float, unsigned char> , Normal<float> > mesh;

        vector<unsigned int> duplicates;
        reconstruction->getMesh(mesh, gridbb, duplicates, voxelsize/10);
        mesh.finalize();

        std::stringstream ss_mesh;
        ss_mesh << "part-" << i << "-mesh.ply";
        ModelPtr m( new Model( mesh.meshBuffer() ) );
        ModelFactory::saveModel( m, ss_mesh.str());
        delete reconstruction;

        std::stringstream ss_grid;
        ss_grid << "part-" << i << "-grid.ser";
        ps_grid->saveCells(ss_grid.str());
        grid_files.push_back(ss_grid.str());

        std::stringstream ss_duplicates;
        ss_duplicates << "part-" << i << "-duplicates.ser";
        std::ofstream ofs(ss_duplicates.str());
        boost::archive::text_oarchive oa(ofs);
        oa & duplicates;

        delete ps_grid;




    }

    vector<size_t> offsets;
    offsets.push_back(0);
    vector<unsigned int> all_duplicates;
    vector<float> duplicateVertices;
    for(int i = 0 ; i <grid_files.size() ; i++)
    {
        string duplicate_path = grid_files[i];
        string ply_path = grid_files[i];
        boost::algorithm::replace_last(duplicate_path, "-grid.ser", "-duplicates.ser");
        boost::algorithm::replace_last(ply_path, "-grid.ser", "-mesh.ply");
        std::ifstream ifs(duplicate_path);
        boost::archive::text_iarchive ia(ifs);
        std::vector<unsigned int> duplicates;
        ia & duplicates;
        LineReader lr(ply_path);
        lvr::PLYIO io;
        ModelPtr modelPtr = io.read(ply_path);
        //ModelPtr modelPtr = ModelFactory::readModel(ply_path);
        size_t numVertices;
        floatArr modelVertices = modelPtr->m_mesh->getVertexArray(numVertices);
        for(size_t j  = 0 ; j<duplicates.size() ; j++)
        {
            duplicateVertices.push_back(modelVertices[duplicates[j]*3]);
            duplicateVertices.push_back(modelVertices[duplicates[j]*3+1]);
            duplicateVertices.push_back(modelVertices[duplicates[j]*3+2]);
        }
        size_t numPoints = lr.getNumPoints();
        offsets.push_back(numPoints+offsets[i]);
        std::transform (duplicates.begin(), duplicates.end(), duplicates.begin(), [&](unsigned int x){return x+offsets[i];});
        all_duplicates.insert(all_duplicates.end(),duplicates.begin(), duplicates.end());

    }
    std::unordered_map<unsigned int, unsigned int> oldToNew;
    float dist_epsilon_squared = (voxelsize/10)*(voxelsize/10);
    for(size_t i = 0 ; i < duplicateVertices.size()/3 ; i+=3)
    {
        float ax = duplicateVertices[i];
        float ay = duplicateVertices[i+1];
        float az = duplicateVertices[i+2];
        for(size_t j = 0 ; j < duplicateVertices.size()/3 ; j+=3)
        {
            if(i==j) continue;
            float bx = duplicateVertices[j];
            float by = duplicateVertices[j+1];
            float bz = duplicateVertices[j+2];
            float dist_squared = (ax-bx)*(ax-bx) + (ay-by)*(ay-by) + (az-bz)*(az-bz);
            if(dist_squared < dist_epsilon_squared)
            {
                if(oldToNew.find(all_duplicates[i]) == oldToNew.end())
                {
                    oldToNew[all_duplicates[j]] = all_duplicates[i];
                }
            }
        }
    }

    ofstream ofs_vertices("largeVertices.bin");
    ofstream ofs_faces("largeFaces.bin");
    size_t increment=0;

    size_t newNumVertices = 0;
    size_t newNumFaces = 0;
    for(int i = 0 ; i <grid_files.size() ; i++)
    {
        vector<size_t> increments;
        string ply_path = grid_files[i];
        boost::algorithm::replace_last(ply_path, "-grid.ser", "-mesh.ply");
        lvr::PLYIO io;
        ModelPtr modelPtr = io.read(ply_path);
        size_t numVertices;
        size_t numFaces;
        size_t offset = offsets[i];
        floatArr modelVertices = modelPtr->m_mesh->getVertexArray(numVertices);
        uintArr modelFaces = modelPtr->m_mesh->getFaceArray(numFaces);
        newNumFaces+=numFaces;
        for(int j = 0; j<numVertices ; j++)
        {
            float x = modelVertices[j*3];
            float y = modelVertices[j*3+1];
            float z = modelVertices[j*3+2];
            if(oldToNew.find(j+offset)==oldToNew.end())
            {
                ofs_vertices << x << " " << y << " " << z << endl;
                newNumVertices++;
            }
            else
            {
                increment++;
            }
            increments.push_back(increment);

        }
        for(int j = 0 ; j<numFaces; j++)
        {
            unsigned int f[3];
            f[0] = modelFaces[j*3] + offset;
            f[1] = modelFaces[j*3+1] + offset;
            f[2] = modelFaces[j*3+2] + offset;
            ofs_faces << "3 ";
            for(int k = 0 ; k < 3; k++)
            {
                if(oldToNew.find(f[k]) == oldToNew.end())
                {
                    ofs_faces << f[k] - increments[f[k]-offset];
                }
                else
                {
                    ofs_faces << oldToNew[f[k]]; // todo old2new[f[k]] ??
                }
                if(k!=2) ofs_faces << " ";

            }
            ofs_faces << endl;

        }




    }

    ofstream ofs_ply("bigMesh.ply");
    ifstream ifs_faces("largeFaces.bin");
    ifstream ifs_vertices("largeVertices.bin");
    string line;
    ofs_ply << "ply\n"
            "format ascii 1.0\n"
            "element vertex " << newNumVertices << "\n"
            "property float x\n"
            "property float y\n"
            "property float z\n"
            "element face " << newNumFaces << "\n"
            "property list uchar int vertex_indices\n"
            "end_header" << endl;
    while(std::getline(ifs_vertices,line))
    {
        ofs_ply << line << endl;
    }
    while(std::getline(ifs_faces,line))
    {
        ofs_ply << line << endl;
    }




    cout << lvr::timestamp << "finished" << endl;

//
//    auto vmax = cbb.getMax();
//    auto vmin = cbb.getMin();
//    vmin.x-=voxelsize*2;
//    vmin.y-=voxelsize*2;
//    vmin.z-=voxelsize*2;
//    vmax.x+=voxelsize*2;
//    vmax.y+=voxelsize*2;
//    vmax.z+=voxelsize*2;
//    cbb.expand(vmin);
//    cbb.expand(vmax);
//    HashGrid<ColorVertex<float, unsigned char>, FastBox<ColorVertex<float, unsigned char>, Normal<float> > >
//            hg(grid_files, cbb, voxelsize);
//
//
//
//    //hg.saveGrid("largeScale.grid");
//
//
//
//
//
//    lvr::FastReconstructionBase<lvr::ColorVertex<float, unsigned char>, lvr::Normal<float> >* reconstruction;
//    reconstruction = new FastReconstruction<ColorVertex<float, unsigned char> , Normal<float>, FastBox<ColorVertex<float, unsigned char>, Normal<float> >  >(&hg);
//    std::vector<float> vBuffer;
//    std::vector<unsigned int> fBuffer;
//    size_t vi,fi;
//    reconstruction->getMesh(vBuffer, fBuffer,fi,vi);
////    HalfEdgeMesh<ColorVertex<float, unsigned char> , Normal<float> > mesh;
////    if(options.getDepth())
////    {
////        mesh.setDepth(options.getDepth());
////    }
////    reconstruction->getMesh(mesh);
////    if(options.getDanglingArtifacts())
////    {
////        mesh.removeDanglingArtifacts(options.getDanglingArtifacts());
////    }
////    mesh.cleanContours(options.getCleanContourIterations());
////    mesh.setClassifier(options.getClassifier());
////    mesh.getClassifier().setMinRegionSize(options.getSmallRegionThreshold());
////    if(options.optimizePlanes())
////    {
////        mesh.optimizePlanes(options.getPlaneIterations(),
////                            options.getNormalThreshold(),
////                            options.getMinPlaneSize(),
////                            options.getSmallRegionThreshold(),
////                            true);
////
////        mesh.fillHoles(options.getFillHoles());
////        mesh.optimizePlaneIntersections();
////        mesh.restorePlanes(options.getMinPlaneSize());
////
////        if(options.getNumEdgeCollapses())
////        {
////            QuadricVertexCosts<ColorVertex<float, unsigned char> , Normal<float> > c = QuadricVertexCosts<ColorVertex<float, unsigned char> , Normal<float> >(true);
////            mesh.reduceMeshByCollapse(options.getNumEdgeCollapses(), c);
////        }
////    }
////    else if(options.clusterPlanes())
////    {
////        mesh.clusterRegions(options.getNormalThreshold(), options.getMinPlaneSize());
////        mesh.fillHoles(options.getFillHoles());
////    }
////
////
////    if ( options.retesselate() )
////    {
////        mesh.finalizeAndRetesselate(options.generateTextures(), options.getLineFusionThreshold());
////    }
////    else
////    {
////        mesh.finalize();
////    }
//    lvr::MeshBufferPtr mb(new lvr::MeshBuffer);
//    mb->setFaceArray(fBuffer);
//    mb->setVertexArray(vBuffer);
//    ModelPtr m( new Model( mb ) );
//    ModelFactory::saveModel( m, "largeScale.ply");
//    delete reconstruction;
//
//    lvr::PointBufferPtr pb2(new lvr::PointBuffer);
//    pb2->setPointArray(points2, numPoints2);
//    lvr::ModelPtr m2( new lvr::Model(pb2));
//    lvr::PLYIO io2;
//    io.save(m2,"testPoints2.ply");




    return 0;
}
