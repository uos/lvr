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

#include <random>
#include <string>
#include <algorithm>
#include <iostream>

#include <boost/filesystem.hpp>

#include "lvr2/reconstruction/SearchTreeFlann.hpp"
#include "lvr2/reconstruction/LargeScaleReconstruction.hpp"
#include "lvr2/io/kernels/HDF5Kernel.hpp"
#include "lvr2/io/kernels/DirectoryKernel.hpp"
#include "lvr2/io/scanio/ScanProjectIO.hpp"
#include "lvr2/io/scanio/DirectoryIO.hpp"
#include "lvr2/io/scanio/HDF5IO.hpp"
#include "lvr2/io/schema/ScanProjectSchemaHDF5.hpp"
#include "lvr2/io/schema/ScanProjectSchemaRaw.hpp"
#include "lvr2/util/IOUtils.hpp"

#include "Options.hpp"


using std::cout;
using std::endl;
using namespace lvr2;
using namespace lvr2::scanio;

#if defined CUDA_FOUND
#define GPU_FOUND
#include "lvr2/reconstruction/cuda/CudaSurface.hpp"
typedef CudaSurface GpuSurface;
#elif defined OPENCL_FOUND
#define GPU_FOUND
#include "lvr2/reconstruction/opencl/ClSurface.hpp"
typedef ClSurface GpuSurface;
#endif

using Vec = lvr2::BaseVector<float>;

// using BaseHDF5IO = lvr2::Hdf5IO<>;

// Extend IO with features (dependencies are automatically fetched)
// using HDF5IO = BaseHDF5IO::AddFeatures<lvr2::hdf5features::ScanProjectIO>;

int main(int argc, char** argv)
{
    // =======================================================================
    // Parse and print command line parameters
    // =======================================================================
    // Parse command line arguments
    LargeScaleOptions::Options options(argc, argv);

    // Exit if options had to generate a usage message
    // (this means required parameters are missing)
    if (options.printUsage())
    {
        return EXIT_SUCCESS;
    }

    options.printLogo();

    fs::path selectedFile = options.m_inputFile;
    std::string input = selectedFile.string();

    std::string extension = selectedFile.extension().string();

    LargeScaleReconstruction<Vec> lsr(options.m_options);


    ScanProjectEditMarkPtr project(new ScanProjectEditMark);

    //reconstruction from hdf5
    if (extension == ".h5")
    {
        std::cout << timestamp << "Reading project from HDF5 file" << std::endl;
        HDF5KernelPtr hdf5kernel(new HDF5Kernel(input));
        HDF5SchemaPtr schema(new ScanProjectSchemaHDF5());
        HDF5IOPtr hdf5io(new HDF5IO(hdf5kernel, schema));
        project->kernel = hdf5kernel;
        project->schema = schema;

        project->project = hdf5io->ScanProjectIO::load();

        project->changed.resize(project->project->positions.size(), true);
    }
    else
    {

        ScanProjectPtr dirScanProject;
        
        DirectoryKernelPtr dirKernel(new DirectoryKernel(input));
        DirectorySchemaPtr dirSchema(new ScanProjectSchemaRaw(input));
        DirectoryIOPtr dirio(new DirectoryIO(dirKernel, dirSchema));
        dirScanProject = dirio->ScanProjectIO::load();
        project->kernel = dirKernel;
        project->schema = dirSchema;

        //reconstruction from ScanProject Folder
        if(dirScanProject) 
        {
            project->project = dirScanProject;
            project->changed.resize(dirScanProject->positions.size(), true);
        }
        //reconstruction from a .ply file
        else if(!boost::filesystem::is_directory(selectedFile))
        {
            std::cout << timestamp << "Reading single file: " << selectedFile << std::endl;
            ModelPtr model = ModelFactory::readModel(input);

            // Create new scan object and mark scan data as loaded
            ScanPtr scan(new Scan());
            scan->points = model->m_pointCloud;

            // Create new lidar object
            LIDARPtr lidar(new LIDAR());
            lidar->scans.push_back(scan);

            // Create new scan position
            ScanPositionPtr scanPosPtr(new ScanPosition());
            scanPosPtr->lidars.push_back(lidar);

            // Create new scan project
            project->project.reset(new ScanProject());
            project->project->positions.push_back(scanPosPtr);

            project->changed.push_back(true);
        }
        else
        {
            // Reconstruction from a folder of .ply files

            // Setup basic scan project structure
            project->project.reset(new ScanProject);
            for (auto file : boost::filesystem::directory_iterator(selectedFile))
            {
                auto path = file.path();
                if(path.extension() != ".ply")
                {
                    std::cout << timestamp << "Skipping file: " << path << std::endl;
                    continue;
                }

                std::cout << timestamp << "Using file: " << path << std::endl;

                // Create new Scan
                ScanPtr scan(new Scan);
                scan->points_loader = [path](){ return ModelFactory::readModel(path.string())->m_pointCloud; };

                // Wrap scan into lidar object
                LIDARPtr lidar(new LIDAR);
                lidar->scans.push_back(scan);

                // Put lidar into new scan position
                ScanPositionPtr position(new ScanPosition);
                position->lidars.push_back(lidar);

                // Add new scan position to scan project
                project->project->positions.push_back(position);
                project->changed.push_back(true);
            }
        }
    }

    BoundingBox<Vec> boundingBox;
    std::shared_ptr<ChunkHashGrid> cm = nullptr;
    fs::path chunkFile = options.m_options.tempDir / "chunk_manager.h5";
    if (options.m_options.partMethod == 1)
    {
        cm = std::shared_ptr<ChunkHashGrid>(new ChunkHashGrid(chunkFile.string(), 10, boundingBox, options.m_options.bgVoxelSize));
    }

    BoundingBox<Vec> bb;
    lsr.chunkAndReconstruct(project, bb, cm);

    project.reset();
    cm.reset();
    fs::remove_all(options.m_options.tempDir);

    std::cout << timestamp << "Program end." << std::endl;

    return 0;
}
