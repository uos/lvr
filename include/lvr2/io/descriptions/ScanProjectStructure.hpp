#ifndef SCANPROJECTPARSER_HPP_
#define SCANPROJECTPARSER_HPP_

#include <string>
#include <tuple>

#include <boost/optional.hpp>

#include <yaml-cpp/yaml.h>
namespace lvr2
{

using StringOptional = boost::optional<std::string>;
using NodeOptional = boost::optional<YAML::Node>;
struct Description
{
    StringOptional groupName;
    StringOptional dataSetName;
    StringOptional metaName;
    NodeOptional metaData;
};

class ScanProjectStructure 
{
public:
    ScanProjectStructure() = delete;

    ScanProjectStructure(const std::string& root) 
        : m_root(root) {};

    ~ScanProjectStructure() = default;

    virtual Description scanProject() const = 0;
    virtual Description position(const size_t& scanPosNo) const = 0;
    virtual Description scan(const size_t& scanPosNo, const size_t& scanNo) const = 0;
    virtual Description scan(const std::string& scanPositionPath, const size_t& scanNo) const = 0;
    
    virtual Description scanCamera(const size_t& scanPositionNo, const size_t& camNo) const = 0;
    virtual Description scanCamera(const std::string& scanPositionPath, const size_t& camNo) const = 0;
 
    virtual Description scanImage(
        const size_t& scanPosNo, const size_t& scanNo,
        const size_t& scanCameraNo, const size_t& scanImageNo) const = 0;

    virtual Description scanImage(
        const std::string& scanImagePath, const size_t& scanImageNo) const = 0;

    virtual Description hyperspectralCamera(const size_t& position) const
    {
        /// TODO: IMPLEMENT ME!!!
        return Description();
    }

    virtual Description hyperSpectralTimestamps(const std::string& group) const
    {
        Description d;
        // Timestamps should be in the same group as the 
        d.groupName = group;
        d.dataSetName = "timestamps";
        d.metaData = boost::none; 
    }

    virtual Description hyperSpectralFrames(const std::string& group) const
    {
        Description d;
        // Timestamps should be in the same group as the 
        d.groupName = group;
        d.dataSetName = "frames";
        d.metaData = boost::none; 
    }
protected:
    std::string     m_root;
};


} // namespace lvr2

#endif