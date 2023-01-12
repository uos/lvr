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


#include <iostream>
#include <memory>
#include <tuple>
#include <stdlib.h>

#include <boost/optional.hpp>
#include <boost/shared_array.hpp>
#include <boost/smart_ptr/make_shared_array.hpp>

#include "lvr2/config/lvropenmp.hpp"

#include "lvr2/geometry/PMPMesh.hpp"
#include "lvr2/geometry/HalfEdgeMesh.hpp"
#include "lvr2/geometry/BaseVector.hpp"
#include "lvr2/geometry/Normal.hpp"
#include "lvr2/attrmaps/StableVector.hpp"
#include "lvr2/attrmaps/VectorMap.hpp"
#include "lvr2/algorithm/FinalizeAlgorithms.hpp"
#include "lvr2/algorithm/NormalAlgorithms.hpp"
#include "lvr2/algorithm/ColorAlgorithms.hpp"
#include "lvr2/geometry/BoundingBox.hpp"
#include "lvr2/algorithm/Tesselator.hpp"
#include "lvr2/algorithm/ClusterPainter.hpp"
#include "lvr2/algorithm/ClusterAlgorithms.hpp"
#include "lvr2/algorithm/CleanupAlgorithms.hpp"
#include "lvr2/algorithm/ReductionAlgorithms.hpp"
#include "lvr2/algorithm/Materializer.hpp"
#include "lvr2/algorithm/Texturizer.hpp"
#include "lvr2/reconstruction/AdaptiveKSearchSurface.hpp" // Has to be included before anything includes opencv stuff, see https://github.com/flann-lib/flann/issues/214 
#include "lvr2/reconstruction/CudaKSearchSurface.hpp"
#include "lvr2/algorithm/SpectralTexturizer.hpp"

#ifdef LVR2_USE_EMBREE
    #include "lvr2/algorithm/RaycastingTexturizer.hpp"
#endif

#include "lvr2/reconstruction/BilinearFastBox.hpp"
#include "lvr2/reconstruction/TetraederBox.hpp"
#include "lvr2/reconstruction/FastReconstruction.hpp"
#include "lvr2/reconstruction/PointsetSurface.hpp"
#include "lvr2/reconstruction/SearchTree.hpp"
#include "lvr2/reconstruction/SearchTreeFlann.hpp"
#include "lvr2/reconstruction/SearchTreeLBVH.hpp"
#include "lvr2/reconstruction/HashGrid.hpp"
#include "lvr2/reconstruction/PointsetGrid.hpp"
#include "lvr2/reconstruction/SharpBox.hpp"
#include "lvr2/types/PointBuffer.hpp"
#include "lvr2/types/MeshBuffer.hpp"
#include "lvr2/io/ModelFactory.hpp"
#include "lvr2/io/PlutoMapIO.hpp"
#include "lvr2/io/meshio/HDF5IO.hpp"
#include "lvr2/io/meshio/DirectoryIO.hpp"
#include "lvr2/util/Factories.hpp"
#include "lvr2/algorithm/GeometryAlgorithms.hpp"
#include "lvr2/algorithm/UtilAlgorithms.hpp"
#include "lvr2/algorithm/KDTree.hpp"
#include "lvr2/io/kernels/HDF5Kernel.hpp"
#include "lvr2/io/scanio/HDF5IO.hpp"
#include "lvr2/io/scanio/ScanProjectIO.hpp"
#include "lvr2/io/schema/ScanProjectSchema.hpp"
#include "lvr2/io/schema/ScanProjectSchemaHDF5.hpp"

#include "lvr2/geometry/BVH.hpp"

#include "lvr2/reconstruction/DMCReconstruction.hpp"

// TODO
#include "lvr2/util/Synthetic.hpp"

#include "Options.hpp"

#if defined LVR2_USE_CUDA
    #define GPU_FOUND

    #include "lvr2/reconstruction/cuda/CudaSurface.hpp"

    typedef lvr2::CudaSurface GpuSurface;
#elif defined LVR2_USE_OPENCL
    #define GPU_FOUND

    #include "lvr2/reconstruction/opencl/ClSurface.hpp"
    typedef lvr2::ClSurface GpuSurface;
#endif

using boost::optional;
using std::unique_ptr;
using std::make_unique;

using namespace lvr2;

using Vec = BaseVector<float>;
using PsSurface = lvr2::PointsetSurface<Vec>;


template <typename IteratorType>
IteratorType concatenate(
    IteratorType output_,
    IteratorType begin0,
    IteratorType end0,
    IteratorType begin1,
    IteratorType end1)
{
    output_ = std::copy(
        begin0,
        end0,
        output_);
    output_ = std::copy(
        begin1,
        end1,
        output_);

    return output_;
}


/**
 * @brief Merges two PointBuffers by copying the data into a new PointBuffer
 * 
 * The function does not modify its arguments, but its not possible to access the PointBuffers data
 * 
 * @param b0 A buffer to copy points from
 * @param b1 A buffer to copy points from
 * @return PointBuffer the merged result of b0 and b1
 */
PointBuffer mergePointBuffers(PointBuffer& b0, PointBuffer& b1)
{
    // number of points in new buffer
    PointBuffer::size_type npoints_total = b0.numPoints() + b1.numPoints();
    // new point array
    floatArr merged_points = floatArr(new float[npoints_total * 3]);

    auto output_it = merged_points.get();
    
    // Copy the points to the new array
    output_it = concatenate(
        output_it,
        b0.getPointArray().get(),
        b0.getPointArray().get() + (b0.numPoints() * 3),
        b1.getPointArray().get(),
        b1.getPointArray().get() + (b1.numPoints() * 3));

    // output iterator should be at the end of the array
    assert(output_it == merged_points.get() + (npoints_total * 3));

    PointBuffer ret(merged_points, npoints_total);

    // Copy colors 
    if (b0.hasColors() && b1.hasColors())
    {
        // nbytes of a color
        size_t w0, w1;
        b0.getColorArray(w0);
        b1.getColorArray(w1);
        if (w0 != w1)
        {
            panic("PointBuffer colors must have the same width!");
        }
        // Number of bytes needed for the colors. Assumes that both color widths are the same
        size_t nbytes = npoints_total * w0;
        ucharArr colors_total = ucharArr(new unsigned char[nbytes]);
        auto output_it = colors_total.get();

        output_it = concatenate(
            output_it,
            b0.getColorArray(w0).get(),
            b0.getColorArray(w0).get() + (b0.numPoints() * w0),
            b1.getColorArray(w1).get(),
            b1.getColorArray(w1).get() + (b1.numPoints() * w1)
        );
        
        ret.setColorArray(colors_total, npoints_total, w0);
    }

    // Copy normals
     if (b0.hasNormals() && b1.hasNormals())
    {
        // Number of bytes needed for the normals
        size_t nbytes = npoints_total * 3;
        floatArr normals_total = floatArr(new float[nbytes]);
        auto output_it = normals_total.get();

        output_it = concatenate(
            output_it,
            b0.getNormalArray().get(),
            b0.getNormalArray().get() + (b0.numPoints() * 3),
            b1.getNormalArray().get(),
            b1.getNormalArray().get() + (b1.numPoints() * 3)
        );
        
        ret.setNormalArray(normals_total,npoints_total);
    }

    return std::move(ret);
}

template <typename BaseVecT>
PointsetSurfacePtr<BaseVecT> loadPointCloud(const reconstruct::Options& options)
{   

    // Create a point loader object
    ModelPtr model = ModelFactory::readModel(options.getInputFileName());
    PointBufferPtr buffer;
    // Parse loaded data
    if (!model)
    {
        boost::filesystem::path selectedFile( options.getInputFileName());
        std::string extension = selectedFile.extension().string();
        std::string filePath = selectedFile.generic_path().string();

        if(selectedFile.extension().string() != ".h5") {
            cout << timestamp << "IO Error: Unable to parse " << options.getInputFileName() << endl;
            return nullptr;
        }
        cout << timestamp << "Loading h5 scanproject from " << filePath << endl;

        // create hdf5 kernel and schema 
        FileKernelPtr kernel = FileKernelPtr(new HDF5Kernel(filePath));
        ScanProjectSchemaPtr schema = ScanProjectSchemaPtr(new ScanProjectSchemaHDF5());
        
        HDF5KernelPtr hdfKernel = std::dynamic_pointer_cast<HDF5Kernel>(kernel);
        HDF5SchemaPtr hdfSchema = std::dynamic_pointer_cast<HDF5Schema>(schema);
        
        // create io object for hdf5 files
        auto scanProjectIO = std::shared_ptr<scanio::HDF5IO>(new scanio::HDF5IO(hdfKernel, hdfSchema));

        ReductionAlgorithmPtr reduction_algorithm;
        // If the user supplied valid octree reduction parameters use octree reduction otherwise use no reduction
        if (options.getOctreeVoxelSize() > 0.0f)
        {
            reduction_algorithm = std::make_shared<OctreeReductionAlgorithm>(options.getOctreeVoxelSize(), options.getOctreeMinPoints());
        }
        else
        {
            reduction_algorithm = std::make_shared<NoReductionAlgorithm>();
        }
        

        if (options.hasScanPositionIndex())
        {
            auto project = scanProjectIO->loadScanProject();
            ModelPtr model = std::make_shared<Model>();

            // Load all given scan positions
            vector<int> scanPositionIndices = options.getScanPositionIndex();
            for(int positionIndex : scanPositionIndices)
            {
                auto pos = scanProjectIO->loadScanPosition(positionIndex);
                auto lidar = pos->lidars.at(0);
                auto scan = lidar->scans.at(0);

                std::cout << timestamp << "Loading scan position " << positionIndex << std::endl;

                // Load scan
                scan->load(reduction_algorithm);

                std::cout << timestamp << "Scan loaded scan has " << scan->numPoints << " points" << std::endl;
                std::cout << timestamp 
                          << "Transforming scan: " << std::endl 
                          <<  (project->transformation * pos->transformation * lidar->transformation * scan->transformation).cast<float>() << std::endl;

                // Transform the new pointcloud
                transformPointCloud<float>(
                    std::make_shared<Model>(scan->points),
                    (project->transformation * pos->transformation * lidar->transformation * scan->transformation).cast<float>());

                // Merge pointcloud and new scan
                // TODO: Maybe merge by allocation all needed memory first instead of constant allocations
                if (model->m_pointCloud)
                {
                    *model->m_pointCloud = mergePointBuffers(*model->m_pointCloud, *scan->points);
                }
                else
                {
                    model->m_pointCloud = std::make_shared<PointBuffer>();
                    *model->m_pointCloud = *scan->points; // Copy the first scan
                }
                scan->release();
            }
            buffer = model->m_pointCloud;
            std::cout << timestamp << "Loaded " << buffer->numPoints() << " points" << std::endl;
        }
        else
        {    
            // === Build the PointCloud ===
            std::cout << timestamp << "Loading scan project" << std::endl;
            ScanProjectPtr project = scanProjectIO->loadScanProject();
            
            std::cout << project->positions.size() << std::endl;
            // The aggregated scans
            ModelPtr model = std::make_shared<Model>();
            model->m_pointCloud = nullptr;
            unsigned ctr = 0;
            for (ScanPositionPtr pos: project->positions)
            {
                std::cout << "Loading scan position " << ctr << " / " << project->positions.size() << std::endl;
                for (LIDARPtr lidar: pos->lidars)
                {
                    for (ScanPtr scan: lidar->scans)
                    {
                        // Load scan
                        bool was_loaded = scan->loaded();
                        if (!scan->loaded())
                        {
                            scan->load();
                        }

                        // Transform the new pointcloud
                        transformPointCloud<float>(
                            std::make_shared<Model>(scan->points),
                            (project->transformation * pos->transformation * lidar->transformation * scan->transformation).cast<float>());
                        
                        // Merge pointcloud and new scan 
                        // TODO: Maybe merge by allocation all needed memory first instead of constant allocations
                        if (model->m_pointCloud)
                        {
                            *model->m_pointCloud = mergePointBuffers(*model->m_pointCloud, *scan->points);
                        }
                        else
                        {
                            model->m_pointCloud = std::make_shared<PointBuffer>();
                            *model->m_pointCloud = *scan->points; // Copy the first scan
                        }
                        
                        
                        
                        // If not previously loaded unload
                        if (!was_loaded)
                        {
                            scan->release();
                        }
                    }
                }
            }

            reduction_algorithm->setPointBuffer(model->m_pointCloud);
            buffer = reduction_algorithm->getReducedPoints();
        }

    }
    else 
    {
        buffer = model->m_pointCloud;
        // TODO 
        // std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << std::endl;
        // buffer = synthetic::genSpherePoints(1000,1000);
    }

    // Create a point cloud manager
    string pcm_name = options.getPCM();
    PointsetSurfacePtr<Vec> surface;

    // Create point set surface object
    if(pcm_name == "PCL")
    {
        cout << timestamp << "Using PCL as point cloud manager is not implemented yet!" << endl;
        panic_unimplemented("PCL as point cloud manager");
    }
    else if(pcm_name == "STANN" || pcm_name == "FLANN" || pcm_name == "NABO" || pcm_name == "NANOFLANN" || pcm_name == "LVR2")
    {
        
        int plane_fit_method = 0;
        
        if(options.useRansac())
        {
            plane_fit_method = 1;
        }

        // plane_fit_method
        // - 0: PCA
        // - 1: RANSAC
        // - 2: Iterative

        surface = make_shared<AdaptiveKSearchSurface<BaseVecT>>(
            buffer,
            pcm_name,
            options.getKn(),
            options.getKi(),
            options.getKd(),
            plane_fit_method,
            options.getScanPoseFile()
        );

    }
    else if(pcm_name == "LBVH_CUDA")
    {
        surface = make_shared<CudaKSearchSurface<BaseVecT>>(
            buffer,
            options.getKn()
        );
    }
    else
    {
        cout << timestamp << "Unable to create PointCloudManager." << endl;
        cout << timestamp << "Unknown option '" << pcm_name << "'." << endl;
        return nullptr;
    }

    // Set search options for normal estimation and distance evaluation
    surface->setKd(options.getKd());
    surface->setKi(options.getKi());
    surface->setKn(options.getKn());

    // Calculate normals if necessary
    if(!buffer->hasNormals() || options.recalcNormals())
    {
        if(options.useGPU())
        {
            #ifdef GPU_FOUND
                std::vector<float> flipPoint = options.getFlippoint();
                size_t num_points = buffer->numPoints();
                floatArr points = buffer->getPointArray();
                floatArr normals = floatArr(new float[ num_points * 3 ]);
                std::cout << timestamp << "Generating GPU kd-tree" << std::endl;
                GpuSurface gpu_surface(points, num_points);
                

                gpu_surface.setKn(options.getKn());
                gpu_surface.setKi(options.getKi());
                gpu_surface.setFlippoint(flipPoint[0], flipPoint[1], flipPoint[2]);

                std::cout << timestamp << "Estimating Normals GPU" << std::endl;
                gpu_surface.calculateNormals();
                gpu_surface.getNormals(normals);

                buffer->setNormalArray(normals, num_points);
                gpu_surface.freeGPU();
            #else
                std::cout << timestamp << "ERROR: GPU Driver not installed" << std::endl;
                surface->calculateSurfaceNormals();
            #endif
        }
        else
        {
            surface->calculateSurfaceNormals();
        }
    }
    else
    {
        cout << timestamp << "Using given normals." << endl;
    }
    if(pcm_name == "LBVH_CUDA")
    {
        std::cout << *buffer << std::endl;
        surface = make_shared<AdaptiveKSearchSurface<BaseVecT>>(
            buffer,
            "FLANN",
            options.getKn(),
            options.getKi(),
            options.getKd(),
            0,
            options.getScanPoseFile()
        );
    }
    return surface;
}

std::pair<shared_ptr<GridBase>, unique_ptr<FastReconstructionBase<Vec>>>
    createGridAndReconstruction(
        const reconstruct::Options& options,
        PointsetSurfacePtr<Vec> surface
    )
{
    // Determine whether to use intersections or voxelsize
    bool useVoxelsize = options.getIntersections() <= 0;
    float resolution = useVoxelsize ? options.getVoxelsize() : options.getIntersections();

    // Create a point set grid for reconstruction
    string decompositionType = options.getDecomposition();

    // Fail safe check
    if(decompositionType != "MT" && decompositionType != "MC" && decompositionType != "DMC" && decompositionType != "PMC" && decompositionType != "SF" )
    {
        cout << "Unsupported decomposition type " << decompositionType << ". Defaulting to PMC." << endl;
        decompositionType = "PMC";
    }

    if(decompositionType == "MC")
    {
        auto grid = std::make_shared<PointsetGrid<Vec, FastBox<Vec>>>(
            resolution,
            surface,
            surface->getBoundingBox(),
            useVoxelsize,
            options.extrude()
        );
        grid->calcDistanceValues();
        auto reconstruction = make_unique<FastReconstruction<Vec, FastBox<Vec>>>(grid);
        return make_pair(grid, std::move(reconstruction));
    }
    else if(decompositionType == "PMC")
    {
        BilinearFastBox<Vec>::m_surface = surface;
        auto grid = std::make_shared<PointsetGrid<Vec, BilinearFastBox<Vec>>>(
            resolution,
            surface,
            surface->getBoundingBox(),
            useVoxelsize,
            options.extrude()
        );
        grid->calcDistanceValues();
        auto reconstruction = make_unique<FastReconstruction<Vec, BilinearFastBox<Vec>>>(grid);
        return make_pair(grid, std::move(reconstruction));
    }
    // else if(decompositionType == "DMC")
    // {
    //     auto reconstruction = make_unique<DMCReconstruction<Vec, FastBox<Vec>>>(
    //         surface,
    //         surface->getBoundingBox(),
    //         options.extrude()
    //     );
    //     return make_pair(nullptr, std::move(reconstruction));
    // }
    else if(decompositionType == "MT")
    {
        auto grid = std::make_shared<PointsetGrid<Vec, TetraederBox<Vec>>>(
            resolution,
            surface,
            surface->getBoundingBox(),
            useVoxelsize,
            options.extrude()
        );
        grid->calcDistanceValues();
        auto reconstruction = make_unique<FastReconstruction<Vec, TetraederBox<Vec>>>(grid);
        return make_pair(grid, std::move(reconstruction));
    }
    else if(decompositionType == "SF")
    {
        SharpBox<Vec>::m_surface = surface;
        auto grid = std::make_shared<PointsetGrid<Vec, SharpBox<Vec>>>(
            resolution,
            surface,
            surface->getBoundingBox(),
            useVoxelsize,
            options.extrude()
        );
        grid->calcDistanceValues();
        auto reconstruction = make_unique<FastReconstruction<Vec, SharpBox<Vec>>>(grid);
        return make_pair(grid, std::move(reconstruction));
    }

    return make_pair(nullptr, nullptr);
}

template <typename Vec>
void addSpectralTexturizers(const reconstruct::Options& options, lvr2::Materializer<Vec>& materializer)
{
    if (!options.hasScanPositionIndex())
    {
        return;
    }

    // TODO: Check if the scanproject has spectral data
    boost::filesystem::path selectedFile( options.getInputFileName());
    std::string filePath = selectedFile.generic_path().string();

    // create hdf5 kernel and schema 
    HDF5KernelPtr hdfKernel = std::make_shared<HDF5Kernel>(filePath);
    HDF5SchemaPtr hdfSchema = std::make_shared<ScanProjectSchemaHDF5>();
    
    // create io object for hdf5 files
    auto hdf5IO = scanio::HDF5IO(hdfKernel, hdfSchema);

    if(options.getScanPositionIndex().size() > 1)
    {
        std::cout 
            << timestamp 
            << "Warning: Spectral texturizing only supports one scan position. Ignoring all but the first." 
            << std::endl;
    }

    // load panorama from hdf5 file
    auto panorama = hdf5IO.HyperspectralPanoramaIO::load(options.getScanPositionIndex()[0], 0, 0);

    

    // If there is no spectral data
    if (!panorama)
    {
        return;
    }

    int texturizer_count = options.getMaxSpectralChannel() - options.getMinSpectralChannel();
    texturizer_count = std::max(texturizer_count, 0);

    // go through all spectralTexturizers of the vector
    for(int i = 0; i < texturizer_count; i++)
    {
        // if the spectralChannel doesnt exist, skip it
        if(panorama->num_channels < options.getMinSpectralChannel() + i)
        {
            continue;
        }

        auto spec_text = std::make_shared<SpectralTexturizer<Vec>>(
            options.getTexelSize(),
            options.getTexMinClusterSize(),
            options.getTexMaxClusterSize()
        );

        // set the spectral texturizer with the current spectral channel
        spec_text->init_image_data(panorama, std::max(options.getMinSpectralChannel(), 0) + i);
        // set the texturizer for the current spectral channel
        materializer.addTexturizer(spec_text);
    }
}

template <typename MeshVec, typename ClusterVec>
void addRaycastingTexturizer(const reconstruct::Options& options, lvr2::Materializer<Vec>& materializer, const BaseMesh<MeshVec>& mesh, const ClusterBiMap<ClusterVec> clusters)
{
#ifdef LVR2_USE_EMBREE
    boost::filesystem::path selectedFile( options.getInputFileName());
    std::string extension = selectedFile.extension().string();
    std::string filePath = selectedFile.generic_path().string();

    if (extension != ".h5")
    {
        std::cout << timestamp << "Cannot add RGB Texturizer because the scanproject is not a HDF5 file" << std::endl;
        return;
    }

    HDF5KernelPtr kernel = std::make_shared<HDF5Kernel>(filePath);
    HDF5SchemaPtr schema = std::make_shared<ScanProjectSchemaHDF5>();
    
    // create io object for hdf5 files
    auto hdf5IO = scanio::HDF5IO(kernel, schema);

    ScanProjectPtr project = hdf5IO.loadScanProject();

    // If only one scan position is used for reconstruction use only the images associated with that position
    if (options.hasScanPositionIndex())
    {
        project->positions.clear();
        auto scanPositions = options.getScanPositionIndex();
        for (int positionIndex : scanPositions)
        {
            ScanPositionPtr pos = hdf5IO.loadScanPosition(positionIndex);
            // Check if position exists
            if (!pos)
            {
                std::cout << timestamp << "Cannot add ScanPosition " << positionIndex << " to scan project." << std::endl;
                return;
            }

            // If the single scan position was not transformed from position to world space
            // remove the transformation from the project and position
            if (!options.transformScanPosition())
            {
                project->transformation = Transformd::Identity(); // Project -> GPS
                pos->transformation = Transformd::Identity();     // Position -> Project
            }

            project->positions.push_back(pos);
        }
    }

    auto texturizer = std::make_shared<RaycastingTexturizer<Vec>>(
        options.getTexelSize(),
        options.getTexMinClusterSize(),
        options.getTexMaxClusterSize(),
        mesh,
        clusters,
        project
    );

    materializer.addTexturizer(texturizer);
#else
    std::cout << timestamp << "This software was compiled without support for Embree!\n";
    std::cout << timestamp << "The RaycastingTexturizer needs the Embree library." << std::endl;
#endif
}

template <typename BaseMeshT, typename BaseVecT>
BaseMeshT reconstructMesh(reconstruct::Options options, PointsetSurfacePtr<BaseVecT> surface)
{
    // =======================================================================
    // Reconstruct mesh from point cloud data
    // =======================================================================
    // Create an empty mesh
    BaseMeshT mesh;

    shared_ptr<GridBase> grid;
    unique_ptr<FastReconstructionBase<Vec>> reconstruction;
    std::tie(grid, reconstruction) = createGridAndReconstruction(options, surface);

    // Reconstruct mesh
    reconstruction->getMesh(mesh);

    // Save grid to file
    if(options.saveGrid() && grid)
    {
        grid->saveGrid("fastgrid.grid");
    }

    return std::move(mesh);
}

template <typename BaseMeshT>
void optimizeMesh(reconstruct::Options options, BaseMeshT& mesh)
{
    // =======================================================================
    // Optimize mesh
    // =======================================================================
    if(options.getDanglingArtifacts())
    {
        cout << timestamp << "Removing dangling artifacts" << endl;
        removeDanglingCluster(mesh, static_cast<size_t>(options.getDanglingArtifacts()));
    }

    cleanContours(mesh, options.getCleanContourIterations(), 0.0001);

    if(options.getFillHoles())
    {
        mesh.fillHoles(options.getFillHoles());
    }

    // Reduce mesh complexity
    const auto reductionRatio = options.getEdgeCollapseReductionRatio();
    if (reductionRatio > 0.0)
    {
        if (reductionRatio > 1.0)
        {
            throw "The reduction ratio needs to be between 0 and 1!";
        }

        size_t old = mesh.numVertices();
        size_t target = old * (1.0 - reductionRatio);
        std::cout << timestamp << "Trying to remove " << old - target << " / " << old << " vertices." << std::endl;
        mesh.simplify(target);
        std::cout << timestamp << "Removed " << old - mesh.numVertices() << " vertices." << std::endl;
    }

    auto faceNormals = calcFaceNormals(mesh);


    if (options.optimizePlanes())
    {
        ClusterBiMap<FaceHandle> clusterBiMap = iterativePlanarClusterGrowingRANSAC(
            mesh,
            faceNormals,
            options.getNormalThreshold(),
            options.getPlaneIterations(),
            options.getMinPlaneSize()
        );

        if(options.getSmallRegionThreshold() > 0)
        {
            deleteSmallPlanarCluster(mesh, clusterBiMap, static_cast<size_t>(options.getSmallRegionThreshold()));
        }

        cleanContours(mesh, options.getCleanContourIterations(), 0.0001);

        if(options.getFillHoles())
        {
            mesh.fillHoles(options.getFillHoles());
        }
    
        // Recalculate the face normals because the faces were modified previously
        faceNormals = calcFaceNormals(mesh);
        // Regrow clusters after hole filling and small region removal
        clusterBiMap = planarClusterGrowing(mesh, faceNormals, options.getNormalThreshold());

        if (options.retesselate())
        {
            Tesselator<Vec>::apply(mesh, clusterBiMap, faceNormals, options.getLineFusionThreshold());
        }
    }

}

template <typename BaseVecT>
struct cmpBaseVecT
{
    bool operator()(const BaseVecT& lhs, const BaseVecT& rhs) const
    {
        return (lhs[0] < rhs[0])
            || (lhs[0] == rhs[0] && lhs[1] < rhs[1])
            || (lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] < rhs[2]);

    }
};

template <typename BaseMeshT, typename BaseVecT>
auto loadExistingMesh(reconstruct::Options options)
{
    meshio::HDF5IO io(
        std::make_shared<HDF5Kernel>(options.getInputMeshFile()),
        std::make_shared<MeshSchemaHDF5>()
    );
    MeshBufferPtr mesh_buffer = io.loadMesh(options.getInputMeshName());


    // Handle Maps needed during mesh construction
    std::map<size_t, VertexHandle> indexToVertexHandle;
    std::map<BaseVecT, VertexHandle, cmpBaseVecT<BaseVecT>> positionToVertexHandle;
    std::map<size_t, FaceHandle> indexToFaceHandle;
    // Create all this stuff manually instead of using the constructors to
    // ensure the Handles are correct.
    BaseMeshT mesh;
    DenseFaceMap<Normal<float>> faceNormalMap;
    ClusterBiMap<FaceHandle> clusterBiMap;

    // Add vertices
    floatArr vertices = mesh_buffer->getVertices();
    for (size_t i = 0; i < mesh_buffer->numVertices(); i++)
    {
        BaseVecT vertex_pos(
            vertices[i * 3 + 0],
            vertices[i * 3 + 1],
            vertices[i * 3 + 2]);

        // If the vertex position already exists do not add new vertex
        if (positionToVertexHandle.count(vertex_pos))
        {
            VertexHandle vertexH = positionToVertexHandle.at(vertex_pos);
            indexToVertexHandle.insert(std::pair(i, vertexH));
        }
        else
        {
            VertexHandle vertexH = mesh.addVertex(vertex_pos);
            indexToVertexHandle.insert(std::pair(i, vertexH));
        }
        
        
    }

    // Add faces
    indexArray faces = mesh_buffer->getFaceIndices();
    floatArr faceNormals = mesh_buffer->getFaceNormals();
    for (size_t i = 0; i < mesh_buffer->numFaces(); i++)
    {
        VertexHandle v0 = indexToVertexHandle.at(faces[i * 3 + 0]);
        VertexHandle v1 = indexToVertexHandle.at(faces[i * 3 + 1]);
        VertexHandle v2 = indexToVertexHandle.at(faces[i * 3 + 2]);
        // Add face
        FaceHandle faceH = mesh.addFace(v0, v1, v2);
        indexToFaceHandle.insert(std::pair(i, faceH));

        if (faceNormals)
        {
            // Add normal
            Normal<float> normal(
                faceNormals[i * 3 + 0],
                faceNormals[i * 3 + 1],
                faceNormals[i * 3 + 2]
            );
            
            faceNormalMap.insert(faceH, normal);
        }
        
    }

    if (!faceNormals)
    {
        std::cout << timestamp << "Calculating face normals" << std::endl;
        faceNormalMap = calcFaceNormals(mesh);
    }

    // Add clusters
    for (size_t i = 0;; i++)
    {
        std::string clusterString =  "cluster" + std::to_string(i) + "_face_indices";
        auto clusterIndicesOptional = mesh_buffer->getIndexChannel(clusterString);
        // If the cluster does not exist break
        if (!clusterIndicesOptional) break;

        ClusterHandle clusterH = clusterBiMap.createCluster();
        for (size_t j = 0; j < clusterIndicesOptional->numElements(); j++)
        {
            FaceHandle faceH = indexToFaceHandle.at(j);
            clusterBiMap.addToCluster(clusterH, faceH);
        }
    }

    // Load the pointcloud
    PointsetSurfacePtr<Vec> surface = loadPointCloud<BaseVecT>(options);

    return std::make_tuple(std::move(mesh), std::move(surface), std::move(faceNormalMap), std::move(clusterBiMap));
}

int main(int argc, char** argv)
{
    // =======================================================================
    // Parse and print command line parameters
    // =======================================================================
    // Parse command line arguments
    reconstruct::Options options(argc, argv);

    options.printLogo();

    // Exit if options had to generate a usage message
    // (this means required parameters are missing)
    if (options.printUsage())
    {
        return EXIT_SUCCESS;
    }

    std::cout << options << std::endl;

    // =======================================================================
    // Load (and potentially store) point cloud
    // =======================================================================
    OpenMPConfig::setNumThreads(options.getNumThreads());

    lvr2::PMPMesh<Vec> mesh;
    PointsetSurfacePtr<Vec> surface;
    DenseFaceMap<Normal<float>> faceNormals;
    ClusterBiMap<FaceHandle> clusterBiMap;

    if (options.useExistingMesh())
    {
        std::cout << timestamp << "Loading existing mesh '" << options.getInputMeshName() << "' from file '" << options.getInputMeshFile() << "'" << std::endl;
        std::tie(mesh, surface, faceNormals, clusterBiMap) = loadExistingMesh<lvr2::PMPMesh<Vec>, Vec>(options);
    }
    else
    {
        // Load PointCloud
        surface = loadPointCloud<Vec>(options);
        if (!surface)
        {
            cout << "Failed to create pointcloud. Exiting." << endl;
            exit(EXIT_FAILURE);
        }
        
        // Reconstruct simple mesh
        mesh = reconstructMesh<lvr2::PMPMesh<Vec>>(options, surface);
    }

    // Save points and normals only
    if(options.savePointNormals())
    {
        ModelPtr pn(new Model(surface->pointBuffer()));
        ModelFactory::saveModel(pn, "pointnormals.ply");
    }

    // Optimize the mesh if requested
    optimizeMesh(options, mesh);
    

    // Calc normals and clusters
    faceNormals = calcFaceNormals(mesh);
    clusterBiMap = planarClusterGrowing(mesh, faceNormals, options.getNormalThreshold());

    // =======================================================================
    // Finalize mesh
    // =======================================================================
    // Prepare color data for finalizing

    ColorGradient::GradientType t = ColorGradient::gradientFromString(options.getClassifier());

    ClusterPainter painter(clusterBiMap);
    auto clusterColors = boost::optional<DenseClusterMap<RGB8Color>>(painter.colorize(mesh, t));
    auto vertexColors = calcColorFromPointCloud(mesh, surface);

    // Calc normals for vertices
    auto vertexNormals = calcVertexNormals(mesh, faceNormals, *surface);

    // Prepare finalize algorithm
    TextureFinalizer<Vec> finalize(clusterBiMap);
    finalize.setVertexNormals(vertexNormals);

    // Vertex colors:
    // If vertex colors should be generated from pointcloud:
    if (options.vertexColorsFromPointcloud())
    {
        // set vertex color data from pointcloud
        finalize.setVertexColors(*vertexColors);
    }
    else if (clusterColors)
    {
        // else: use simpsons painter for vertex coloring
        finalize.setClusterColors(*clusterColors);
    }

    // Materializer for face materials (colors and/or textures)
    Materializer<Vec> materializer(
        mesh,
        clusterBiMap,
        faceNormals,
        *surface
    );

    auto texturizer = std::make_shared<Texturizer<Vec>>(
        options.getTexelSize(),
        options.getTexMinClusterSize(),
        options.getTexMaxClusterSize()
    );




    // When using textures ...
    if (options.generateTextures())
    {

        boost::filesystem::path selectedFile( options.getInputFileName());
        std::string filePath = selectedFile.generic_path().string();

        if(selectedFile.extension().string() != ".h5") {
            materializer.setTexturizer(texturizer);
        } 
        else 
        {
            addSpectralTexturizers(options, materializer);

#ifdef LVR2_USE_EMBREE
            if (options.useRaycastingTexturizer())
            {
                addRaycastingTexturizer(options, materializer, mesh, clusterBiMap);
            }
            else
            {
                materializer.addTexturizer(texturizer);
            }
#else
            materializer.addTexturizer(texturizer);
#endif
            
        }
    }

    // Generate materials
    MaterializerResult<Vec> matResult = materializer.generateMaterials();

    // Add material data to finalize algorithm
    finalize.setMaterializerResult(matResult);
    // Run finalize algorithm
    auto buffer = finalize.apply(mesh);

    // When using textures ...
    if (options.generateTextures())
    {
        // Set optioins to save them to disk
        //materializer.saveTextures();
        buffer->addIntAtomic(1, "mesh_save_textures");
        buffer->addIntAtomic(1, "mesh_texture_image_extension");
    }

    // =======================================================================
    // Write all results (including the mesh) to file
    // =======================================================================
    // Create output model and save to file
    auto m = ModelPtr( new Model(buffer));

    if(options.saveOriginalData())
    {
        m->m_pointCloud = surface->pointBuffer();

        cout << "REPAIR SAVING" << endl;
    }

    for(const std::string& output_filename : options.getOutputFileNames())
    {
        boost::filesystem::path outputDir(options.getOutputDirectory());
        boost::filesystem::path selectedFile( output_filename );
        boost::filesystem::path outputFile = outputDir/selectedFile;
        std::string extension = selectedFile.extension().string();

        cout << timestamp << "Saving mesh to "<< output_filename << "." << endl;

        if (extension == ".h5")
        {

            HDF5KernelPtr kernel = HDF5KernelPtr(new HDF5Kernel(outputFile.string()));
            MeshSchemaHDF5Ptr schema = MeshSchemaHDF5Ptr(new MeshSchemaHDF5());
            auto mesh_io = meshio::HDF5IO(kernel, schema);

            mesh_io.saveMesh(
                options.getMeshName(),
                buffer
                );

            continue;
        }

        if (extension == "")
        {
            DirectoryKernelPtr kernel = DirectoryKernelPtr(new DirectoryKernel(outputFile.string()));
            MeshSchemaDirectoryPtr schema = MeshSchemaDirectoryPtr(new MeshSchemaDirectory());
            auto mesh_io = meshio::DirectoryIO(kernel, schema);

            mesh_io.saveMesh(
                options.getMeshName(),
                buffer
                );

            continue;
        }

        ModelFactory::saveModel(m, outputFile.string());
    }

    if (matResult.m_keypoints)
    {
        // save materializer keypoints to hdf5 which is not possible with ModelFactory
        //PlutoMapIO map_io("triangle_mesh.h5");
        //map_io.addTextureKeypointsMap(matResult.m_keypoints.get());
    }

    cout << timestamp << "Program end." << endl;

    return 0;
}
