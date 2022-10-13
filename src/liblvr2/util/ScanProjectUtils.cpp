#include "lvr2/types/ScanTypes.hpp"
#include "lvr2/util/Factories.hpp"
#include "lvr2/util/ScanProjectUtils.hpp"
#include "lvr2/util/ScanSchemaUtils.hpp"
#include "lvr2/io/ModelFactory.hpp"
#include "lvr2/io/scanio/HDF5IO.hpp"
#include "lvr2/io/scanio/DirectoryIO.hpp"
#include "lvr2/io/kernels/DirectoryKernel.hpp"
#include "lvr2/io/kernels/HDF5Kernel.hpp"

#include <boost/filesystem.hpp>

namespace lvr2
{

std::pair<ScanPtr, Transformd> scanFromProject(ScanProjectPtr project, size_t scanPositionNo, size_t lidarNo, size_t scanNo)
{
    Transformd transform = Transformd::Identity();

    if(project && scanPositionNo < project->positions.size())
    {
        transform = transform * project->transformation;
        ScanPositionPtr pos = project->positions[scanPositionNo];
        if(pos && lidarNo < pos->lidars.size())
        {
            transform = transform * pos->transformation;
            LIDARPtr lidar =  pos->lidars[lidarNo];
            if(lidar && scanNo < lidar->scans.size())
            {
                transform = transform * lidar->transformation;
                ScanPtr scan = lidar->scans[scanNo];
                return std::make_pair(scan, transform);
            }
        }
    }

    // Something went wrong during access...
    return std::make_pair(nullptr, Transformd::Identity());
}

ScanProjectPtr scanProjectFromHDF5(std::string file, const std::string& schemaName)
{
    HDF5KernelPtr kernel(new HDF5Kernel(file));
    HDF5SchemaPtr schema = hdf5SchemaFromName(schemaName);

    lvr2::scanio::HDF5IO hdf5io(kernel, schema);
    return hdf5io.ScanProjectIO::load();
}

ScanProjectPtr scanProjectFromFile(const std::string& file)
{
    std::cout << timestamp << "Creating scan project from single file: " << file << std::endl;
    ScanProjectPtr project(new ScanProject);
    ModelPtr model = ModelFactory::readModel(file);

    if(model)
    {

        // Create new scan object and mark scan data as
        // loaded
        ScanPtr scan(new Scan);
        scan->points = model->m_pointCloud;

        // Create new lidar object
        LIDARPtr lidar(new LIDAR);

        // Create new scan position
        ScanPositionPtr scanPosPtr = ScanPositionPtr(new ScanPosition());

        // Buildup scan project structure
        project->positions.push_back(scanPosPtr);
        project->positions[0]->lidars.push_back(lidar);
        project->positions[0]->lidars[0]->scans.push_back(scan);

        return project;
    }
    else
    {
        std::cout << timestamp << "Unable to open file '" << file << "' for reading-" << std::endl;
    }
    return nullptr;
}

ScanProjectPtr scanProjectFromPLYFiles(const std::string &dir)
{
    std::cout << timestamp << "Creating scan project from a directory of .ply files..." << std::endl;
    ScanProjectPtr scanProject(new ScanProject);
    boost::filesystem::directory_iterator it{dir};
    while (it != boost::filesystem::directory_iterator{})
    {
        string ext = it->path().extension().string();
        if (ext == ".ply")
        {
            ModelPtr model = ModelFactory::readModel(it->path().string());

            // Create new Scan
            ScanPtr scan(new Scan);
            scan->points = model->m_pointCloud;

            // Wrap scan into lidar object
            LIDARPtr lidar(new LIDAR);
            lidar->scans.push_back(scan);

            // Put lidar into new scan position
            ScanPositionPtr position(new ScanPosition);
            position->lidars.push_back(lidar);

            // Add new scan position to scan project
            scanProject->positions.push_back(position);
        }
        it++;
    }
    if(scanProject->positions.size())
    {
        return scanProject;
    }
    else
    {
        std::cout << timestamp << "Warning: scan project is empty." << std::endl;
        return nullptr;
    }
}

ScanProjectPtr loadScanProject(const std::string& schema, const std::string& source, bool loadData)
{
    boost::filesystem::path sourcePath(source);

    // Check if we try to load from a directory
    if(boost::filesystem::is_directory(sourcePath))
    {
        DirectorySchemaPtr dirSchema = directorySchemaFromName(schema, source);
        DirectoryKernelPtr kernel(new DirectoryKernel(source));

        if(dirSchema && kernel)
        {
            lvr2::scanio::DirectoryIOPtr dirio_in(new lvr2::scanio::DirectoryIO(kernel, dirSchema, loadData));
            return dirio_in->ScanProjectIO::load();
        }
    }
    // Check if we try to load a HDF5 file
    else if(sourcePath.extension() == ".h5")
    {
        HDF5SchemaPtr hdf5Schema = hdf5SchemaFromName(schema);
        HDF5KernelPtr kernel(new HDF5Kernel(source));

        if(hdf5Schema && kernel)
        {
            lvr2::scanio::HDF5IO hdf5io(kernel, hdf5Schema, loadData);
            return hdf5io.ScanProjectIO::load();
        }
    }

    // Loading failed. 
    std::cout << timestamp << "Could not create schema or kernel for loading." << std::endl;
    std::cout << timestamp << "Schema name: " << schema << std::endl;
    std::cout << timestamp << "Source: " << source << std::endl;

    return nullptr;
}

void saveScanProject(ScanProjectPtr& project, const std::string& schema, const std::string target)
{
    if(project)
    {
        boost::filesystem::path targetPath(target);
        if(boost::filesystem::is_directory(target))
        {
            DirectorySchemaPtr dirSchema = directorySchemaFromName(schema, target);
            DirectoryKernelPtr kernel(new DirectoryKernel(target));

            if (dirSchema && kernel)
            {
                lvr2::scanio::DirectoryIOPtr dirio (new lvr2::scanio::DirectoryIO(kernel, dirSchema));
                dirio->ScanProjectIO::save(project);
            }

        }
        else if(targetPath.extension() == ".h5")
        {
            HDF5SchemaPtr hdf5Schema = hdf5SchemaFromName(schema);
            HDF5KernelPtr kernel(new HDF5Kernel(target));

            if (hdf5Schema && kernel)
            {
                lvr2::scanio::HDF5IO hdf5io(kernel, hdf5Schema);
                hdf5io.ScanProjectIO::save(project);
            }
        }

        // Saving failed.
        std::cout << timestamp << "Could not create schema or kernel for saving." << std::endl;
        std::cout << timestamp << "Schema name: " << schema << std::endl;
        std::cout << timestamp << "Target: " << target << std::endl;
    }
    else
    {
        std::cout << timestamp << "Cannot save scan project from null pointer" << std::endl;
    }
}

void printScanProjectStructure(const ScanProjectPtr project)
{
    std::cout << project << std::endl;

    for(size_t i = 0; i < project->positions.size(); i++)
    {
        std::cout << timestamp << "Scan Position: " << i << " / " << project->positions.size() << std::endl;
        printScanPositionStructure(project->positions[i]);
    }
}

void printScanPositionStructure(const ScanPositionPtr p) 
{
    std::cout << p;
    for(size_t i = 0; i < p->lidars.size(); i++)
    {
        std::cout << timestamp << "LiDAR " << i << " / " << p->lidars.size() << std::endl;
        printLIDARStructure(p->lidars[i]);
    }
    for(size_t i = 0; i < p->cameras.size(); i++)
    {
        std::cout << timestamp << "Camera " << i << " / " << p->cameras.size() << std::endl;
        printCameraStructure(p->cameras[i]);
    }
    for(size_t i = 0; i < p->hyperspectral_cameras.size(); i++)
    {
        std::cout << timestamp << "Hyperspectral Camera " << i << " / " << p->hyperspectral_cameras.size() << std::endl;
        printHyperspectralCameraStructure(p->hyperspectral_cameras[i]);
    }

}

void printScanStructure(const ScanPtr p) 
{
    std::cout << p;
    // TODO: Implement output for point buffer
}

void printLIDARStructure(const LIDARPtr p) 
{
    std::cout << p;
    for(size_t i = 0; i < p->scans.size(); i++)
    {
        std::cout << timestamp << "Scan " << i << " / " << p->scans.size() << std::endl;
        printScanStructure(p->scans[i]);
    }
}

void printCameraStructure(const CameraPtr p) 
{
    std::cout << p;

    struct V : public boost::static_visitor<>
    {
        void operator()(const CameraImageGroupPtr p) const {printCameraImageGroupStructure(p);}
        void operator()(const CameraImagePtr p) const { printCameraImageStructure(p);}
    };

    for(size_t i = 0; i < p->images.size(); i++)
    {
        std::cout << timestamp << "Camera image or group " << i << " / " << p->images.size() << std::endl;
        CameraImageOrGroup ciog = p->images[i];
        boost::apply_visitor(V{}, ciog);
    }
}

void printCameraImageGroupStructure(const CameraImageGroupPtr p) 
{
    std::cout << p;
    for(size_t i = 0; i < p->images.size(); i++)
    {
        std::cout << timestamp << "Image " << i << " / " << p->images.size() << std::endl;
        std::cout << p->images[i];
    }
}

void printHyperspectralCameraStructure(const HyperspectralCameraPtr p) 
{
    std::cout << p;
    for(size_t i = 0; i < p->panoramas.size(); i++)
    {
        std::cout << timestamp << "Panorama " << i << " / " << p->panoramas.size() << std::endl;
        printHyperspectralPanoramaStructure(p->panoramas[i]);
    }
}

void printHyperspectralPanoramaStructure(const HyperspectralPanoramaPtr p) 
{
    std::cout << p;
    for(size_t i = 0; i < p->channels.size(); i++)
    {
        std::cout << timestamp << "Channel " << i << " / " << p->channels.size() << std::endl;
        std::cout << p->channels[i];
    }
}

void printCameraImageStructure(const CameraImagePtr p)
{
    std::cout << p;
}



} // namespace lvr2 else
     