
#ifndef LVR2_IO_YAML_HYPERSPECTRALCAMERAMETA_IO_HPP
#define LVR2_IO_YAML_HYPERSPECTRALCAMERAMETA_IO_HPP

#include <yaml-cpp/yaml.h>

#include "Matrix.hpp"
#include "CameraModels.hpp"
#include "lvr2/types/ScanTypes.hpp"

namespace YAML
{

/**
 * YAML-CPPs convert specialization
 *
 * example:
 */

// WRITE HYPERSPECTRALCAMERA PARTIALLY
template <>
struct convert<lvr2::HyperspectralCamera>
{

    /**
     * Encode Eigen matrix to yaml.
     */
    static Node encode(const lvr2::HyperspectralCamera& camera)
    {
        Node node;

        node["type"] = lvr2::HyperspectralCamera::type;
        node["kind"] = lvr2::HyperspectralCamera::kind;
        node["name"] = camera.name;
        node["transformation"] = camera.transformation;

        return node;
    }

    static bool decode(const Node& node, lvr2::HyperspectralCamera& camera)
    {
        // if(!node["type"])
        // {
        //     std::cout << lvr2::timestamp << "[YAML::convert<HyperspectralCamera> - decode] "
        //              << "HyperspectralCamera meta has no key 'type'" << std::endl; 
        //     return false;
        // }
        if(node["entity"] && node["entity"].as<std::string>() != lvr2::HyperspectralCamera::type)
        {
            // different hierarchy level
            std::cout << lvr2::timestamp << "[YAML::convert<HyperspectralCamera> - decode] " 
                        << "Nodes type '" << node["entity"].as<std::string>()
                        << "' is not '" <<  lvr2::HyperspectralCamera::type << "'" << std::endl; 
            return false;
        }

        if(node["type"] && node["type"].as<std::string>() != lvr2::HyperspectralCamera::type)
        {
            // different hierarchy level
            std::cout << lvr2::timestamp << "[YAML::convert<HyperspectralCamera> - decode] " 
                        << "Nodes type '" << node["type"].as<std::string>()
                        << "' is not '" <<  lvr2::HyperspectralCamera::type << "'" << std::endl; 
            return false;
        }

        if(node["kind"])
        {
            if(node["kind"].as<std::string>() != lvr2::HyperspectralCamera::kind)
            {
                std::cout << lvr2::timestamp << "[YAML::convert<HyperspectralCamera> - decode] " 
                            << "Nodes kind '" << node["kind"].as<std::string>()
                            << "' is not '" <<  lvr2::HyperspectralCamera::kind << "'" << std::endl; 
                return false;
            }
        } 
        else if(node["sensor_type"])
        {
            if(node["sensor_type"].as<std::string>() != lvr2::HyperspectralCamera::kind)
            {
                std::cout << lvr2::timestamp << "[YAML::convert<HyperspectralCamera> - decode] " 
                            << "Nodes kind '" << node["sensor_type"].as<std::string>()
                            << "' is not '" <<  lvr2::HyperspectralCamera::kind << "'" << std::endl; 
                return false;
            }
        } else {
            std::cout << lvr2::timestamp << "[YAML::convert<HyperspectralCamera> - decode] "
                     << "WARNING: Cannot detect specialization of sensor. Assuming this sensor to by of kind "  << lvr2::HyperspectralCamera::kind << std::endl;
        }
        std::cout << "load camera???  " << std::endl;

        // if(!node["kind"] && !node["sensor_type"])
        // {
        //     std::cout << lvr2::timestamp << "[YAML::convert<HyperspectralCamera> - decode] "
        //              << "WARNING: Sensor has no key 'kind'. Assuming this sensor to by of kind "  << lvr2::HyperspectralCamera::kind << std::endl;
        // } else {
            
        //     if(node["kind"].as<std::string>() != lvr2::HyperspectralCamera::kind)
        //     {
        //         std::cout << lvr2::timestamp << "[YAML::convert<HyperspectralCamera> - decode] " 
        //                     << "Nodes kind '" << node["kind"].as<std::string>()
        //                     << "' is not '" <<  lvr2::HyperspectralCamera::kind << "'" << std::endl; 
        //         return false;
        //     }
        // }

        if(node["name"])
        {
            camera.name = node["name"].as<decltype(camera.name)>();
            std::cout << "HERE!!!!!!!!!!!!!!!!!!!!" << std::endl;
            std::cout << camera.name << std::endl;
        }

        if(node["sensor_name"])
        {
            camera.name = node["sensor_name"].as<decltype(camera.name)>();
        }

        return true;
    }
};

template <>
struct convert<lvr2::HyperspectralPanorama>
{

    /**
     * Encode Eigen matrix to yaml.
     */
    static Node encode(const lvr2::HyperspectralPanorama& pano)
    {
        Node node;

        node["type"] = lvr2::HyperspectralPanorama::type;
        node["kind"] = lvr2::HyperspectralPanorama::kind;
        node["transformation"] = pano.transformation;
        node["resolution"] = Load("[]");
        node["resolution"].push_back(pano.resolution[0]);
        node["resolution"].push_back(pano.resolution[1]);

        node["wavelength"] = Load("[]");
        node["wavelength"].push_back(pano.wavelength[0]);
        node["wavelength"].push_back(pano.wavelength[1]);

        return node;
    }

    static bool decode(const Node& node, lvr2::HyperspectralPanorama& pano)
    {
        // if(!node["type"])
        // {
        //     std::cout << lvr2::timestamp << "[YAML::convert<HyperspectralPanorama> - decode] "
        //              << "HyperspectralPanorama meta has no key 'type'" << std::endl; 
        //     return false;
        // }

        if(node["entity"] && node["entity"].as<std::string>() != lvr2::HyperspectralPanorama::type)
        {
            if(node["entity"].as<std::string>() == lvr2::HyperspectralPanorama::kind)
            {
                std::cout << "[YAML::convert<HyperspectralPanorama> - decode] " 
                    << "WARNING: 'entity' is '" << node["entity"].as<std::string>() 
                    << "'. 'entity' should be '" << lvr2::HyperspectralPanorama::type
                    << "' and 'sensor_type' should be '" << lvr2::HyperspectralPanorama::kind << "'" << std::endl;
            } else {
                std::cout << lvr2::timestamp << "[YAML::convert<HyperspectralPanorama> - decode] " 
                        << "Nodes entity '" << node["entity"].as<std::string>()
                        << "' is not '" <<  lvr2::HyperspectralPanorama::type << "'" << std::endl; 
                return false;
            }
        }

        if(node["type"] && node["type"].as<std::string>() != lvr2::HyperspectralPanorama::type)
        {
            // different hierarchy level
            std::cout << lvr2::timestamp << "[YAML::convert<HyperspectralPanorama> - decode] " 
                        << "Nodes type '" << node["type"].as<std::string>()
                        << "' is not '" <<  lvr2::HyperspectralPanorama::type << "'" << std::endl; 
            return false;
        }

        if(node["kind"])
        {
            if(node["kind"].as<std::string>() != lvr2::HyperspectralCamera::kind)
            {
                std::cout << lvr2::timestamp << "[YAML::convert<HyperspectralPanorama> - decode] " 
                            << "Nodes kind '" << node["kind"].as<std::string>()
                            << "' is not '" <<  lvr2::HyperspectralPanorama::kind << "'" << std::endl; 
                return false;
            }
        } 
        else if(node["sensor_type"])
        {
            if(node["sensor_type"].as<std::string>() != lvr2::HyperspectralCamera::kind)
            {
                std::cout << lvr2::timestamp << "[YAML::convert<HyperspectralPanorama> - decode] " 
                            << "Nodes kind '" << node["sensor_type"].as<std::string>()
                            << "' is not '" <<  lvr2::HyperspectralPanorama::kind << "'" << std::endl; 
                return false;
            }
        } else {
            std::cout << lvr2::timestamp << "[YAML::convert<HyperspectralPanorama> - decode] "
                     << "WARNING: Cannot detect specialization of SensorData. Assuming this SensorData to be of kind "  << lvr2::HyperspectralPanorama::kind << std::endl;
        }

        if(node["width"]) {
            pano.resolution[0] = node["width"].as<size_t>();
        }

        if(node["height"]) {
            pano.resolution[1] = node["height"].as<size_t>();
        }
    

        return true;
    }
};

template <>
struct convert<lvr2::HyperspectralPanoramaChannel>
{

    /**
     * Encode Eigen matrix to yaml.
     */
    static Node encode(const lvr2::HyperspectralPanoramaChannel& hchannel)
    {
        Node node;

        node["type"] = lvr2::HyperspectralPanoramaChannel::type;
        node["kind"] = lvr2::HyperspectralPanoramaChannel::kind;

        return node;
    }

    static bool decode(const Node& node, lvr2::HyperspectralPanoramaChannel& hchannel)
    {
        // if(!node["type"])
        // {
        //     std::cout << lvr2::timestamp << "[YAML::convert<HyperspectralPanorama> - decode] "
        //              << "HyperspectralPanorama meta has no key 'type'" << std::endl; 
        //     return false;
        // }

        // if (node["type"].as<std::string>() != lvr2::HyperspectralPanorama::type)
        // {
        //     // different hierarchy level
        //     std::cout << lvr2::timestamp << "[YAML::convert<HyperspectralPanorama> - decode] " 
        //                 << "Nodes type '" << node["type"].as<std::string>()
        //                 << "' is not '" <<  lvr2::HyperspectralPanorama::type << "'" << std::endl; 
        //     return false;
        // }

        // if(!node["kind"])
        // {
        //     std::cout << lvr2::timestamp << "[YAML::convert<HyperspectralPanorama> - decode] "
        //              << "WARNING: Sensor has no key 'kind'. Assuming this sensor to by of kind "  << lvr2::Camera::kind << std::endl;
        // } else {
        //     if(node["kind"].as<std::string>() != lvr2::HyperspectralPanorama::kind)
        //     {
        //         std::cout << lvr2::timestamp << "[YAML::convert<HyperspectralPanorama> - decode] " 
        //                     << "Nodes kind '" << node["kind"].as<std::string>()
        //                     << "' is not '" <<  lvr2::HyperspectralPanorama::kind << "'" << std::endl; 
        //         return false;
        //     }
        // }

        // if(node["transformation"])
        // {
        //     pano.transformation = node["transformation"].as<lvr2::Transformd>();
        // }

        // if(node["resolution"])
        // {
        //     pano.resolution[0] = node["resolution"][0].as<unsigned int>();
        //     pano.resolution[1] = node["resolution"][1].as<unsigned int>();
        // }

        // if(node["wavelength"])
        // {
        //     pano.wavelength[0] = node["wavelength"][0].as<double>();
        //     pano.wavelength[1] = node["wavelength"][0].as<double>();
        // }

        return true;
    }
};

} // namespace YAML

#endif // LVR2_IO_YAML_HYPERSPECTRALCAMERAMETA_IO_HPP
