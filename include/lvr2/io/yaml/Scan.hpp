
#ifndef LVR2_IO_YAML_SCANMETA_IO_HPP
#define LVR2_IO_YAML_SCANMETA_IO_HPP

#include <yaml-cpp/yaml.h>
#include "lvr2/types/ScanTypes.hpp"
#include "lvr2/io/Timestamp.hpp"
#include "Matrix.hpp"

namespace YAML {  

/**
 * YAML-CPPs convert specialization
 * 
 * example: 
 */

// WRITE SCAN PARTIALLY
template <>
struct convert<lvr2::Scan> 
{

    /**
     * Encode Eigen matrix to yaml. 
     */
    static Node encode(const lvr2::Scan& scan) {
        Node node;
        node["type"] = lvr2::Scan::type;
        node["kind"] = boost::typeindex::type_id<lvr2::Scan>().pretty_name();

        node["start_time"]  = scan.startTime;
        node["end_time"] = scan.endTime;

        node["pose_estimation"] = scan.poseEstimation;
        node["transformation"] = scan.transformation;

        Node config;
        config["theta"] = Load("[]");
        config["theta"].push_back(scan.thetaMin);
        config["theta"].push_back(scan.thetaMax);

        config["phi"] = Load("[]");
        config["phi"].push_back(scan.phiMin);
        config["phi"].push_back(scan.phiMax);

        config["v_res"] = scan.vResolution;
        config["h_res"] = scan.hResolution;

        config["num_points"] = scan.numPoints;
        node["config"] = config;


        if(scan.points)
        {
            node["channels"] = Load("[]");
            for(auto elem : *scan.points)
            {
                node["channels"].push_back(elem.first);
            }
        }
        

        return node;
    }

    static bool decode(const Node& node, lvr2::Scan& scan) 
    {
        if(!node["type"])
        {
            std::cout << "[YAML::convert<Scan> - decode] 'type' Tag not found." << std::endl;
            return false;
        }

        if(node["type"].as<std::string>() != lvr2::Scan::type) 
        {
            // TODO: proper  warning or error?
            std::cout << "[YAML::convert<Scan> - decode] Try to load " << node["type"].as<std::string>() << " as " << lvr2::Scan::type << std::endl;
            return false;
        }

        if(node["start_time"])
        {
            scan.startTime = node["start_time"].as<double>();
        } else {
            scan.startTime = -1.0;
        }

        if(node["end_time"])
        {
            scan.endTime = node["end_time"].as<double>();
        } else {
            scan.endTime = -1.0;
        }
        
        if(node["pose_estimate"])
        {
            scan.poseEstimation = node["pose_estimate"].as<lvr2::Transformd>();
        } else {
            scan.poseEstimation = lvr2::Transformd::Identity();
        }

        if(node["transformation"])
        {
            scan.transformation = node["transformation"].as<lvr2::Transformd>();
        } else {
            scan.transformation = lvr2::Transformd::Identity();
        }
        
        
        if(node["config"])
        {
            const Node& config = node["config"];
        
            if(config["theta"])
            {
                scan.thetaMin = config["theta"][0].as<double>();
                scan.thetaMax = config["theta"][1].as<double>();
            }
            
            if(config["phi"])
            {
                scan.phiMin = config["phi"][0].as<double>();
                scan.phiMax = config["phi"][1].as<double>();
            }

            if(config["v_res"])
            {
                scan.vResolution = config["v_res"].as<double>();
            }            

            if(config["h_res"])
            {
                scan.hResolution = config["h_res"].as<double>();
            }
            
            if(config["num_points"])
            {
                scan.numPoints = config["num_points"].as<size_t>();
            }   
        }

        return true;
    }

};

}  // namespace YAML

#endif // LVR2_IO_YAML_SCANMETA_IO_HPP

