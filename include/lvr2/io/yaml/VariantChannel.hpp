
#ifndef LVR2_IO_YAML_VARIANT_CHANNEL_HPP
#define LVR2_IO_YAML_VARIANT_CHANNEL_HPP

#include <sstream>

#include <yaml-cpp/yaml.h>
#include "lvr2/types/ScanTypes.hpp"
#include "lvr2/io/yaml/Matrix.hpp"
#include "lvr2/types/MultiChannelMap.hpp"

namespace YAML {

using VChannelT = lvr2::MultiChannel;
template<>
struct convert<VChannelT> 
{
    /**
     * Encode Eigen matrix to yaml. 
     */
    static Node encode(const VChannelT& vchannel) {
        
        Node node;

        node["sensor_type"] = "Channel";
        node["channel_type"] = vchannel.typeName();
        node["dims"] = Load("[]");
        node["dims"].push_back(vchannel.numElements());
        node["dims"].push_back(vchannel.width());

        return node;
    }

    static bool decode(const Node& node, VChannelT& vchannel) 
    {
        if(node["sensor_type"].as<std::string>() != "Channel")
        {
            return false;
        }

        if(node["channel_type"].as<std::string>() != vchannel.typeName())
        {
            return false;
        }
        
        return true;
    }

};

}  // namespace YAML

#endif // LVR2_IO_YAML_VARIANT_CHANNEL_HPP

