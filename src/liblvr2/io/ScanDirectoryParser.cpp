#include <iomanip>
#include <sstream>
#include <boost/format.hpp>

#include "lvr2/io/ScanDirectoryParser.hpp"
#include "lvr2/io/IOUtils.hpp"
#include "lvr2/io/ModelFactory.hpp"

using namespace boost::filesystem;

namespace lvr2
{

ScanDirectoryParser::ScanDirectoryParser(const std::string& directory) noexcept
{
    // Check if directory exists and save path
    Path dir(directory);
    if(!exists(directory))
    {
        std::cout << timestamp << "Directory " << directory << " does not exist." << std::endl;
    }
    else
    {
        m_directory = directory;
    }

    // Set default prefixes and extension
    m_pointExtension = ".txt";
    m_poseExtension = ".dat";
    m_pointPrefix = "scan";
    m_posePrefix = "scan";

    m_start = 0;
    m_end = 0;
}

void ScanDirectoryParser::setPointCloudPrefix(const std::string& prefix)
{
    m_pointPrefix = prefix;
}
void ScanDirectoryParser::setPointCloudExtension(const std::string& extension)
{
    m_pointExtension = extension;
}
void ScanDirectoryParser::setPosePrefix(const std::string& prefix)
{
    m_posePrefix = prefix;
}   
void ScanDirectoryParser::setPoseExtension(const std::string& extension)
{
    m_poseExtension = extension;
}

void ScanDirectoryParser::setStart(int s)
{
    m_start = s;
}
void ScanDirectoryParser::setEnd(int e)
{
    m_end = e;
}

void ScanDirectoryParser::setTargetSize(const size_t& size)
{
    m_targetSize = size;
}

size_t ScanDirectoryParser::examinePLY(const std::string& filename)
{
    return getNumberOfPointsInPLY(filename);
}

size_t ScanDirectoryParser::examineASCII(const std::string& filename)
{
    Path p(filename);
    return countPointsInFile(p);
} 

Eigen::Matrix4d ScanDirectoryParser::getPose(const Path& poseFile)
{
    if(m_poseExtension == ".dat")
    {
        return getTransformationFromDat(poseFile);
    }
    else if(m_poseExtension == ".pose")
    {
        return getTransformationFromPose(poseFile);
    }
    else if(m_poseExtension == ".frames")
    {
        return getTransformationFromFrames(poseFile);
    }
    
    return Eigen::Matrix4d::Identity();
}

PointBufferPtr ScanDirectoryParser::subSample()
{
    ModelPtr out_model(new Model);
    for(auto i : m_scans)
    {
        ModelPtr model = ModelFactory::readModel(i.m_filename);
        if(model)
        {
            PointBufferPtr buffer = model->m_pointCloud;
            if(buffer)
            {
                PointBufferPtr reduced = subSamplePointBuffer(buffer, 1000000);
                out_model->m_pointCloud = reduced;

                Path p(i.m_filename);
                std::stringstream name_stream;
                name_stream << p.stem().string() << "_reduced" << ".ply";
                ModelFactory::saveModel(out_model, name_stream.str());
            }
        }
    }
}

void ScanDirectoryParser::parseDirectory()
{
    std::cout << timestamp << "Parsing directory" << m_directory << std::endl;
    m_numPoints = 0;
    for(size_t i = m_start; i <= m_end; i++)
    {
        // Construct name of current file
        std::stringstream point_ss;
        point_ss << m_pointPrefix << boost::format("%03d") % i << m_pointExtension;
        std::string pointFileName = point_ss.str();
        Path pointPath = Path(m_directory)/Path(pointFileName);

        // Construct name of transformation file
        std::stringstream pose_ss;
        pose_ss << m_posePrefix << boost::format("%03d") % i << m_poseExtension;
        std::string poseFileName = pose_ss.str();
        Path posePath = Path(m_directory)/Path(poseFileName);

        // Check for file and get number of points
        size_t n = 0;
        if(exists(pointPath))
        {
            std::cout << timestamp << "Counting points in file " << pointPath << std::endl;
            if(pointPath.extension() == ".3d" || pointPath.extension() == ".txt" || pointPath.extension() == ".pts")
            {
                n = examineASCII(pointPath.string());
            }
            else if(pointPath.extension() == ".ply")
            {
                n = examinePLY(pointPath.string());
            }
            m_numPoints += n;
        }
        else
        {
            std::cout << timestamp << "Point cloud file " << pointPath << " does not exist." << std::endl;
        }

        // Check for pose information file
        Eigen::Matrix4d matrix = Eigen::Matrix4d::Identity();

        if(exists(posePath))
        {
            matrix = getPose(posePath.string());
            std::cout << timestamp << "Found transformation: " << posePath << " @ " << std::endl << matrix << std::endl;
        }
        else
        {
            std::cout << timestamp << "Scan pose file " << posePath << "does not exist. Will not transfrom." << std::endl;
        }

        ScanInfo info;
        info.m_filename     = pointPath.string();
        info.m_numPoints    = n;
        info.m_pose         = matrix;

        m_scans.push_back(info);
    }
    std::cout << timestamp << "Finished parsing. Directory contains " << m_scans.size() << " scans with " << m_numPoints << " points." << std::endl;
}

 

} // namespace lvr2