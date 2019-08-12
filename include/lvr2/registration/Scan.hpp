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

/**
 * Scan.hpp
 *
 *  @date May 6, 2019
 *  @author Malte Hillmann
 */
#ifndef SCAN_HPP_
#define SCAN_HPP_

#include <lvr2/io/PointBuffer.hpp>

#include <Eigen/Dense>
#include <vector>

using Eigen::Matrix4d;
using Eigen::Vector3d;
using std::vector;
using std::pair;

namespace lvr2
{

using Vector3fArr = boost::shared_array<Eigen::Vector3f>;

/**
 * @brief Annotates the use of a Scan when creating an slam6D .frames file
 */
enum class ScanUse
{
    /// The Scan has not been registered yet
    INVALID = 0,
    /// The Scan changed since the last Frame
    UPDATED = 1,
    /// The Scan did not change since the last Frame
    UNUSED = 2,
    /// The Scan was part of a GraphSLAM Iteration
    GRAPHSLAM = 3,
    /// The Scan was part of a Loopclose Iteration
    LOOPCLOSE = 4,
};

/**
 * @brief Represents a Scan as a Pointcloud and a Pose
 * 
 * TODO: This will be replaced with ScanData from ../io/ScanData.hpp eventually
 */
class Scan
{
public:
    Scan(PointBufferPtr points, const Eigen::Matrix<double, 4, 4, Eigen::RowMajor>& pose);

    virtual ~Scan() = default;

    void transform(const Eigen::Matrix<double, 4, 4, Eigen::RowMajor>& transform, bool writeFrame = true, ScanUse use = ScanUse::UPDATED);
    void addFrame(ScanUse use = ScanUse::UNUSED);

    void reduce(double voxelSize, int maxLeafSize);
    void setMinDistance(double minDistance);
    void setMaxDistance(double maxDistance);
    void trim();

    virtual Vector3d getPoint(size_t index) const;
    size_t numPoints() const;

    const Eigen::Matrix<double, 4, 4, Eigen::RowMajor>& getPose() const;
    const Eigen::Matrix<double, 4, 4, Eigen::RowMajor>& getDeltaPose() const;
    const Eigen::Matrix<double, 4, 4, Eigen::RowMajor>& getInitialPose() const;

    Vector3d getPosition() const;

    void writeFrames(std::string path) const;

    PointBufferPtr toPointBuffer() const;
    Vector3fArr toVector3fArr() const;

protected:
    floatArr m_points;
    size_t   m_numPoints;

    Eigen::Matrix<double, 4, 4, Eigen::RowMajor> m_pose;
    Eigen::Matrix<double, 4, 4, Eigen::RowMajor> m_deltaPose;
    Eigen::Matrix<double, 4, 4, Eigen::RowMajor> m_initialPose;

    vector<pair<Eigen::Matrix<double, 4, 4, Eigen::RowMajor>, ScanUse>> m_frames;
};

using ScanPtr = std::shared_ptr<Scan>;

} /* namespace lvr2 */

#endif /* SCAN_HPP_ */
