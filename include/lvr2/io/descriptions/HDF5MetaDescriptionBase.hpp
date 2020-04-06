#ifndef HDF5_META_DESCRIPTION
#define HDF5_META_DESCRIPTION

#include <yaml-cpp/yaml.h>
#include <hdf5_hl.h>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5File.hpp>

#include <tuple>

namespace lvr2
{


class HDF5MetaDescriptionBase
{
public:
    HDF5MetaDescriptionBase() = default;
    virtual ~HDF5MetaDescriptionBase() = default;

      virtual YAML::Node hyperspectralCamera(const HighFive::Group& g) const = 0;
    virtual YAML::Node hyperspectralPanoramaChannel(const HighFive::Group& g) const = 0;
    virtual YAML::Node scan(const HighFive::Group& g) const = 0;
    virtual YAML::Node scanPosition(const HighFive::Group& g) const = 0;
    virtual YAML::Node scanProject(const HighFive::Group& g) const = 0;
    virtual YAML::Node scanCamera(const HighFive::Group& g) const = 0;
    virtual YAML::Node scanImage(const HighFive::Group& g) const = 0;

    
};

} // namespace lvr2

#endif