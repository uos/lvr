#ifndef SCANPROJECTPARSER_HPP_
#define SCANPROJECTPARSER_HPP_

#include <string>
#include <tuple>

#include <boost/optional.hpp>
#include <boost/filesystem.hpp>

#include <yaml-cpp/yaml.h>
namespace lvr2
{

using StringOptional = boost::optional<std::string>;
using NodeOptional = boost::optional<YAML::Node>;

struct Description
{
    // path to relative to project root: is built recursively
    StringOptional groupName;
    // if group contains data (relative to group)
    StringOptional dataSetName;
    // if group contains meta (relative to group)
    StringOptional metaName;
};

std::ostream& operator<<(std::ostream& os, const Description& desc);

std::pair<std::string, std::string> getNames(
    const std::string& defaultGroup, 
    const std::string& defaultContainer, 
    const Description& d);

class ScanProjectSchema
{
public:
    ScanProjectSchema() {}

    ~ScanProjectSchema() = default;

    virtual Description scanProject() const = 0;

    virtual Description position(
        const size_t& scanPosNo) const = 0;

    virtual Description lidar(
        const Description& scanpos_descr,
        const size_t& lidarNo) const = 0;
    
    virtual Description scan(
        const Description& lidar_descr, 
        const size_t& scanNo) const = 0;

    virtual Description camera(
        const Description& scanpos_descr, 
        const size_t& camNo) const = 0;
 
    virtual Description cameraImage(
        const Description& camera_descr, 
        const size_t& cameraImageNo) const = 0;

    virtual Description hyperspectralCamera(
        const Description& scanpos_descr,
        const size_t& camNo) const = 0;

    virtual Description hyperspectralPanorama(
        const Description& hcam_descr,
        const size_t& panoNo
    ) const = 0;

    virtual Description hyperspectralPanoramaChannel(
        const Description& hpano_descr,
        const size_t& channelNo
    ) const = 0;

    // virtual Description hyperspectralCamera(const size_t& position) const
    // {
    //     /// TODO: IMPLEMENT ME!!!
    //     return Description();
    // }
    
    // virtual Description hyperSpectralTimestamps(const std::string& group) const
    // {
    //     Description d;
    //     // Timestamps should be in the same group as the 
    //     d.groupName = group;
    //     d.dataSetName = "timestamps";
    //     d.metaData = boost::none; 
    // }

    // virtual Description hyperSpectralFrames(const std::string& group) const
    // {
    //     Description d;
    //     // Timestamps should be in the same group as the 
    //     d.groupName = group;
    //     d.dataSetName = "frames";
    //     d.metaData = boost::none; 
    // }
// protected:
    
};

/// Marker interface for HDF5 schemas
class HDF5Schema : public ScanProjectSchema 
{
public:
    HDF5Schema() {}
};

/// Marker interface for HDF5 schemas
class LabelHDF5Schema : public HDF5Schema
{
public:
    LabelHDF5Schema() {}
};

/// Marker interface for directory schemas
class DirectorySchema : public ScanProjectSchema
{
public:
    DirectorySchema(const std::string& root) : m_rootPath(root) {}

protected:
    boost::filesystem::path m_rootPath;
};

using ScanProjectSchemaPtr = std::shared_ptr<ScanProjectSchema>;
using DirectorySchemaPtr = std::shared_ptr<DirectorySchema>;
using HDF5SchemaPtr = std::shared_ptr<HDF5Schema>;
using LabelHDF5SchemaPtr = std::shared_ptr<LabelHDF5Schema>;

} // namespace lvr2

#endif