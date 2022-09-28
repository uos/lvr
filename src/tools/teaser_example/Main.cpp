//
// Created by praktikum on 13.09.22.
//

#include <chrono>
#include <iostream>
#include <random>
#include "util/pointtovertex.h"
#include <Eigen/Core>

#include <filesystem>

//#include <teaser/ply_io.h>
#include <teaser/registration.h>
//#include <teaser/matcher.h>
#include "open3d/Open3D.h"
#include "lvr2/util/TransformUtils.hpp"

#include "util/MyMatching.h"

#define NOISE_BOUND 0.05

using namespace open3d;
namespace fs = std::filesystem;

auto search_param = open3d::geometry::KDTreeSearchParamKNN(30);

class transformRegistration;
int count=1;
Eigen::Matrix<float_t ,4,4> registration;




open3d::geometry::PointCloud readAndPreprocessPointCloud(std::string path) {
    // Reading PointClouds
    geometry::PointCloud cloud;
    std::cout << "Reading PointCloud... " << std::flush;
//    auto search_param = open3d::geometry::KDTreeSearchParamKNN(30);
    open3d::io::ReadPointCloud(path, cloud);
    std::cout << "- done." << std::endl;

    std::cout << "Cloud Size: " << cloud.points_.size() << std::endl;

    return cloud;
}

//void estimatingNormals(geometry::PointCloud cloud&) {
//    // Estimating Normals
//    std::cout << "Estimating Normals... ";
//    cloud.EstimateNormals(search_param);
//    std::cout << "- done." << std::endl;
//
//    return;
//}

open3d::geometry::PointCloud computeISSPointClouds(open3d::geometry::PointCloud cloud)
{
    // ISS
    std::cout << "Computing ISS keypoints... " << std::endl;
    double SALIENT_RADIUS = 0;
    double NON_MAX_RADIUS = 0;
    double GAMMA_21 = 0.975;
    double GAMMA_32 = 0.975;
    int MIN_NEIGHBORS = 5;
    open3d::geometry::PointCloud iss_cloud = *open3d::geometry::keypoint::ComputeISSKeypoints(cloud,

                                                                                              SALIENT_RADIUS, NON_MAX_RADIUS, GAMMA_21, GAMMA_32, MIN_NEIGHBORS);
    std::cout << "- done." << std::endl;
    std::cout << "ISS Keypoints size: " << iss_cloud.points_.size() << std::endl;
    return iss_cloud;
}

pipelines::registration::Feature computeFPFHs(open3d::geometry::PointCloud iss_cloud) {
    // Compute Features
        std::cout << "Computing FPFHs... " << std::flush;
        auto feature =  *pipelines::registration::ComputeFPFHFeature(iss_cloud, search_param);
        std::cout << "- done." << std::endl;

    // print feature information
//        std::cout <<  "Dimensions: " << feature.Dimension() << std::endl;
        std::cout <<  "Number of features: " << feature.Num() << std::endl;

//        std::cout << "feature.rows(): " << feature.data_.rows() << std::endl;
//        std::cout << "feature.cols(): " << feature.data_.cols() << std::endl;

        return feature;
}

teaser::PointCloud convertToTeaserCloud(geometry::PointCloud cloud) {
    // convert Open3D cloud i
    teaser::PointCloud teaser_cloud;
    for (size_t i = 0; i < cloud.points_.size(); ++i) {
        teaser_cloud.push_back(
                {static_cast<float>(cloud.points_[i].x()), static_cast<float>(cloud.points_[i].y()),
                 static_cast<float>(cloud.points_[i].z())});
    }
    return teaser_cloud;
}

Eigen::Matrix<double, 3, Eigen::Dynamic> convertToEigen(teaser::PointCloud cloud) {
    std::cout << "converting to eigen3 Matrix... " << std::flush;
    int N = cloud.size();
    Eigen::Matrix<double, 3, Eigen::Dynamic> eigen(3, N);
    for (size_t i = 0; i < cloud.size(); ++i) {
        eigen.col(i) << cloud[i].x, cloud[i].y, cloud[i].z;
    }
    std::cout << "- done." << std::endl;
    return eigen;
}

void solveTeaserWithoutCorrespondences(Eigen::Matrix<double, 3, Eigen::Dynamic> src_cloud, Eigen::Matrix<double, 3, Eigen::Dynamic> target_cloud) {
    // Prepare solver parameters
    std::cout << "Teaser solving... " << std::flush;
    teaser::RobustRegistrationSolver::Params params;
    params.noise_bound = NOISE_BOUND;
    params.cbar2 = 1;
    params.estimate_scaling = false;
    params.rotation_max_iterations = 100;
    params.rotation_gnc_factor = 1.4;
    params.rotation_estimation_algorithm =
            teaser::RobustRegistrationSolver::ROTATION_ESTIMATION_ALGORITHM::GNC_TLS;
    params.rotation_cost_threshold = 0.005;

    // Solve with TEASER++
    teaser::RobustRegistrationSolver solver(params);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    solver.solve(src_cloud, target_cloud);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    auto solution = solver.getSolution();


    // Compare results
    std::cout << "=====================================" << std::endl;
    std::cout << "          TEASER++ Results           " << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << "Estimated rotation: " << std::endl;
    std::cout << solution.rotation << std::endl;
    std::cout << "Estimated translation: " << std::endl;
    std::cout << solution.translation << std::endl;
    std::cout << "Time taken (s): "
           << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() /
                 1000000.0
                 << std::endl;

    Eigen::Matrix<float_t ,4,4> transformation;
    for (int i=0;i<=4;i++){
        for (int j=0;j<=4;j++){
            if(i<3){
                if(j<3){
                    transformation(i,j)=solution.rotation(i,j);
                }
                else{
                    transformation(i,j)=solution.translation(i);
                }
            }
            else{
                transformation(i,j)=0;
            }


        }
    }

    std::cout << "transformation "<< std::endl;

    std::cout << transformation << std::endl;



}

void solveTeaserWithCorrespondences(teaser::PointCloud src_cloud, teaser::PointCloud target_cloud, std::vector<std::pair<int, int>> correspondences) {
    // Prepare solver parameters
    std::cout << "Teaser solving... ";
    teaser::RobustRegistrationSolver::Params params;
    params.noise_bound = NOISE_BOUND;
    params.cbar2 = 1;
    params.estimate_scaling = false;
    params.rotation_max_iterations = 100;
    params.rotation_gnc_factor = 1.4;
    params.rotation_estimation_algorithm =
            teaser::RobustRegistrationSolver::ROTATION_ESTIMATION_ALGORITHM::GNC_TLS;
    params.rotation_cost_threshold = 0.005;

    // Solve with TEASER++
    teaser::RobustRegistrationSolver solver(params);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    solver.solve(target_cloud, src_cloud, correspondences);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    auto solution = solver.getSolution();


    // Compare results
    std::cout << "=====================================" << std::endl;
    std::cout << "          TEASER++ Results           " << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << "Estimated rotation: " << std::endl;
    std::cout << solution.rotation << std::endl;
    std::cout << "Estimated translation: " << std::endl;
    std::cout << solution.translation << std::endl;
    std::cout << "Time taken (s): "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() /
                 1000000.0
              << std::endl;
    Eigen::Matrix<float_t ,4,4> transformation;

    for (int i=0;i<=3;i++){
        for (int j=0;j<=3;j++){
            if(i<3){
                if(j<3){
                    transformation(i,j)=solution.rotation(i,j);
                }
                else{
                    transformation(i,j)=solution.translation(i);
                }
            }
            else{
                transformation(i,j)=0;
            }

        }
    }

    std::cout << "transformation "<< std::endl;
    std::cout << transformation << std::endl;
    std::cout << lvr2::transformRegistration(registration,transformation)<<std::endl;
    registration = transformation;
    std::ofstream file;
    stringstream s;
    s <<"/home/praktikum/Desktop/ply/" <<count<< ".meta";
    file.open(s.str());
    file <<registration;
    count++;

}

void workflowCorrespondences(std::string src_path, std::string target_path) {

    geometry::PointCloud src_cloud = readAndPreprocessPointCloud(src_path);
    geometry::PointCloud target_cloud = readAndPreprocessPointCloud(target_path);

    std::cout << "src cloud Has Normals before estimating normals: " << src_cloud.HasNormals() << std::endl;
    std::cout << "target cl Has Normals before estimating normals: " << target_cloud.HasNormals() << std::endl;

    src_cloud.EstimateNormals();
    target_cloud.EstimateNormals();

    std::cout << "src cloud Has Normals after estimating normals: " << src_cloud.HasNormals() << std::endl;
    std::cout << "target cl Has Normals after estimating normals: " << target_cloud.HasNormals() << std::endl;


    geometry::PointCloud src_iss_cloud = computeISSPointClouds(src_cloud);
    geometry::PointCloud target_iss_cloud = computeISSPointClouds(target_cloud);

    std::cout << "src cloud Has Normals after ISS: " << src_cloud.HasNormals() << std::endl;
    std::cout << "target cl Has Normals after ISS: " << target_cloud.HasNormals() << std::endl;

    auto src_feature = computeFPFHs(src_iss_cloud);
    auto target_feature = computeFPFHs(target_iss_cloud);

//    std::cout << "src_feature number of rows"

//    std::cout << "Computing correspondo... ";
//    std::vector<std::pair<int, int>> correspondences = MyMatching(target_feature, src_feature);
//    std::cout << "- done." << std::endl;
//
//
//    // convert Open3D cloud i
//    teaser::PointCloud src_teaser_cloud = convertToTeaserCloud(src_iss_cloud);
//    teaser::PointCloud target_teaser_cloud = convertToTeaserCloud(target_iss_cloud);
//
//    // convert pointcloud to Eigen3 Matrix to use teaser::solver without correspondences
//
////    Eigen::Matrix<double, 3, Eigen::Dynamic> src_eigen = convertToEigen(src_teaser_cloud);
////    Eigen::Matrix<double, 3, Eigen::Dynamic> target_eigen = convertToEigen(target_teaser_cloud);
//
//    solveTeaserWithCorrespondences(src_teaser_cloud, target_teaser_cloud, correspondences);
}

void workflowDirectlyWithISSAndDownSampling(std::string src_path, std::string target_path) {

    geometry::PointCloud src_cloud = readAndPreprocessPointCloud(src_path);
    geometry::PointCloud target_cloud = readAndPreprocessPointCloud(target_path);

    double voxel_size = 1.5;
    geometry::PointCloud sampled_src_cloud = *src_cloud.VoxelDownSample(voxel_size);
    geometry::PointCloud sampled_target_cloud = *target_cloud.VoxelDownSample(voxel_size);
    std::cout << "Voxel source size: " << sampled_src_cloud.points_.size() << std::endl;
    std::cout << "Voxel target size: " << sampled_target_cloud.points_.size() << std::endl;

//    sampled_src_cloud.EstimateNormals();
//    sampled_target_cloud.EstimateNormals();

//    geometry::PointCloud src_iss_cloud = computeISSPointClouds(sampled_src_cloud);
//    geometry::PointCloud target_iss_cloud = computeISSPointClouds(sampled_target_cloud);

//    auto src_feature = computeFPFHs(src_iss_cloud);
//    auto target_feature = computeFPFHs(target_iss_cloud);

//    std::cout << "Computing correspondo... ";
//    std::vector<std::pair<int, int>> correspondences = MyMatching(target_feature, src_feature);
//    std::cout << "- done." << std::endl;


    // convert Open3D cloud i
    teaser::PointCloud src_teaser_cloud = convertToTeaserCloud(sampled_src_cloud);
    teaser::PointCloud target_teaser_cloud = convertToTeaserCloud(sampled_target_cloud);

    // convert pointcloud to Eigen3 Matrix to use teaser::solver without correspondences
    Eigen::Matrix<double, 3, Eigen::Dynamic> src_eigen = convertToEigen(src_teaser_cloud);
    Eigen::Matrix<double, 3, Eigen::Dynamic> target_eigen = convertToEigen(target_teaser_cloud);

//    for (int i = 0; i < 10; ++i) {
//        src_eigen.[i];
//
//    }

    solveTeaserWithoutCorrespondences(src_eigen, target_eigen);
}

void workflowCorrespondencesAndDownSampling(std::string src_path, std::string target_path) {

    geometry::PointCloud src_cloud = readAndPreprocessPointCloud(src_path);
    geometry::PointCloud target_cloud = readAndPreprocessPointCloud(target_path);

    geometry::PointCloud sampled_src_cloud = *src_cloud.VoxelDownSample(0.5);
    geometry::PointCloud sampled_target_cloud = *target_cloud.VoxelDownSample(0.5);
    std::cout << "Voxel source size: " << sampled_src_cloud.points_.size() << std::endl;
    std::cout << "Voxel target size: " << sampled_target_cloud.points_.size() << std::endl;

    sampled_src_cloud.EstimateNormals();
    sampled_target_cloud.EstimateNormals();

    geometry::PointCloud src_iss_cloud = computeISSPointClouds(sampled_src_cloud);
    geometry::PointCloud target_iss_cloud = computeISSPointClouds(sampled_target_cloud);

    auto src_feature = computeFPFHs(src_iss_cloud);
    auto target_feature = computeFPFHs(target_iss_cloud);

    std::cout << "Computing correspondo... ";
    std::vector<std::pair<int, int>> correspondences = MyMatching(target_feature, src_feature);
    std::cout << "- done." << std::endl;

    // convert Open3D cloud i
    teaser::PointCloud src_teaser_cloud = convertToTeaserCloud(src_iss_cloud);
    teaser::PointCloud target_teaser_cloud = convertToTeaserCloud(target_iss_cloud);

    // convert pointcloud to Eigen3 Matrix to use teaser::solver without correspondences

//    Eigen::Matrix<double, 3, Eigen::Dynamic> src_eigen = convertToEigen(src_teaser_cloud);
//    Eigen::Matrix<double, 3, Eigen::Dynamic> target_eigen = convertToEigen(target_teaser_cloud);

    solveTeaserWithCorrespondences(src_teaser_cloud, target_teaser_cloud, correspondences);
}

void workflowCorrespondencesAndDownSamplingWithoutISS(std::string src_path, std::string target_path) {

    geometry::PointCloud src_cloud = readAndPreprocessPointCloud(src_path);
    geometry::PointCloud target_cloud = readAndPreprocessPointCloud(target_path);

    double voxel_size =  0.5;
    geometry::PointCloud sampled_src_cloud = *src_cloud.VoxelDownSample(voxel_size);
    geometry::PointCloud sampled_target_cloud = *target_cloud.VoxelDownSample(voxel_size);
    std::cout << "Voxel source size: " << sampled_src_cloud.points_.size() << std::endl;
    std::cout << "Voxel target size: " << sampled_target_cloud.points_.size() << std::endl;

    sampled_src_cloud.EstimateNormals();
    sampled_target_cloud.EstimateNormals();

    auto src_feature = computeFPFHs(sampled_src_cloud);
    auto target_feature = computeFPFHs(sampled_target_cloud);

    std::vector<std::pair<int, int>> correspondences = MyMatching(target_feature, src_feature);

    // convert Open3D cloud i
    teaser::PointCloud src_teaser_cloud = convertToTeaserCloud(sampled_src_cloud);
    teaser::PointCloud target_teaser_cloud = convertToTeaserCloud(sampled_target_cloud);

    solveTeaserWithCorrespondences(src_teaser_cloud, target_teaser_cloud, correspondences);
}

int main() {
    // file paths
    std::string robo_dir = "/home/praktikum/Desktop/ScanProject/raw";
    std::string matrix_dir = "/home/praktikum/Desktop/ply";

    std::string end = ".ply";
    std::string vertex = "_vertex";

//    std::string source_path_vertex = robo_dir + "/00000001/lidar_00000000/00000000/points.ply";
//    std::string target_path_vertex = robo_dir + "/00000002/lidar_00000000/00000000/points.ply";
//    pointtovertex(source_path_vertex, "/home/praktikum/Desktop/teaserOpen3d/ply/001.ply");
//    pointtovertex(target_path_vertex, "/home/praktikum/Desktop/teaserOpen3d/ply/002.ply");

    int numberOfScans = 5;

    for (int i = 1; i <= numberOfScans; ++i) {
        std::string source_path_vertex = robo_dir + "/0000000" + std::to_string(i) + "/lidar_00000000/00000000/points.ply";
        std::cout << source_path_vertex << '\n';
        pointtovertex(source_path_vertex, "/home/praktikum/Desktop/ply/00" + std::to_string(i) + ".ply");
    }
    registration << 1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1;
    for (int i = 1; i <= numberOfScans; ++i) {
        if(i < numberOfScans)
        {
            workflowCorrespondencesAndDownSamplingWithoutISS(matrix_dir + "/00" + std::to_string(i) + ".ply", "/home/praktikum/Desktop/ply/00" + std::to_string(i + 1) + ".ply");
        }
        else if(i == numberOfScans)
        {
            //workflowCorrespondencesAndDownSamplingWithoutISS(matrix_dir + "/00" + std::to_string(i) + ".ply", "/home/praktikum/Desktop/teaserOpen3d/ply/00" + std::to_string(1) + ".ply");
            break;
        }
    }

}
