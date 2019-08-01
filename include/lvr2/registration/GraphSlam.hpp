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
 * GraphSlam.hpp
 *
 *  @date July 22, 2019
 *  @author Malte Hillmann
 */
#ifndef GRAPHSLAM_HPP_
#define GRAPHSLAM_HPP_

#include "Scan.hpp"
#include "SlamOptions.hpp"
#include "KDTree.hpp"

#include <Eigen/Sparse>

using Matrix6d = Eigen::Matrix<double, 6, 6>;
using Vector6d = Eigen::Matrix<double, 6, 1>;
using GraphMatrix = Eigen::SparseMatrix<double>;
using GraphVector = Eigen::VectorXd;
using Graph = vector<pair<int, int>>;

namespace lvr2
{

bool findCloseScans(const vector<ScanPtr>& scans, size_t scan, const SlamOptions& options, vector<size_t>& output);

class GraphSlam
{

public:
    GraphSlam(const SlamOptions* options);

    virtual ~GraphSlam() = default;

    void doGraphSlam(const vector<ScanPtr>& scans, size_t last);

protected:

    void createGraph(const vector<ScanPtr>& scans, size_t last, Graph& graph);
    void fillEquation(const vector<ScanPtr>& scans, const Graph& graph, GraphMatrix& mat, GraphVector& vec);
    void eulerCovariance(KDTreePtr tree, ScanPtr scan, Matrix6d& outMat, Vector6d& outVec) const;

    const SlamOptions*     m_options;
};

} /* namespace lvr2 */

#endif /* GRAPHSLAM_HPP_ */
