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
 * ICPPointAlign.cpp
 *
 *  @date Mar 18, 2014
 *  @author Thomas Wiemann
 */
#include <lvr2/registration/ICPPointAlign.hpp>
#include <lvr2/registration/EigenSVDPointAlign.hpp>
#include <lvr2/io/Timestamp.hpp>
#include <lvr2/io/IOUtils.hpp>
#include <lvr2/geometry/Matrix4.hpp>

// TODO: remove
#include <chrono>
using namespace std::chrono;

#include <omp.h>
#include <fstream>
using std::ofstream;

using namespace std;

namespace lvr2
{

ICPPointAlign::ICPPointAlign(ScanPtr model, ScanPtr data) :
    m_dataCloud(data), m_modelCloud(model)
{
    // Init default values
    m_maxDistanceMatch  = 25;
    m_maxIterations     = 50;
    m_epsilon           = 0.00001;
    m_verbose           = false;

    m_searchTree = KDTree::create(model->points(), model->count());
}

Matrix4f ICPPointAlign::match()
{
    if (m_maxIterations == 0)
    {
        return Matrix4f();
    }

    auto start_time = steady_clock::now();

    double ret = 0.0, prev_ret = 0.0, prev_prev_ret = 0.0;
    EigenSVDPointAlign align;
    int iteration;

    Vector3f centroid_m;
    Vector3f centroid_d;
    Matrix4f transform;

    PointPairVector pairs;
    pairs.reserve(m_dataCloud->count());

    for (iteration = 0; iteration < m_maxIterations; iteration++)
    {
        // Update break variables
        prev_prev_ret = prev_ret;
        prev_ret = ret;

        // Get point pairs
        getPointPairs(pairs, centroid_m, centroid_d);

        // Get transformation
        transform = Matrix4f::Identity();
        ret = align.alignPoints(pairs, centroid_m, centroid_d, transform);

        // Apply transformation
        m_dataCloud->transform(transform, false);

        if (m_verbose)
        {
            cout << timestamp << "ICP Error is " << ret << " in iteration " << iteration << " / " << m_maxIterations << " using " << pairs.size() << " points." << endl;
        }

        // Check minimum distance
        if ((fabs(ret - prev_ret) < m_epsilon) && (fabs(ret - prev_prev_ret) < m_epsilon))
        {
            break;
        }
    }
    auto duration = steady_clock::now() - start_time;
    cout << setw(6) << (int)(duration.count() / 1e6) << " ms, ";
    cout << "Error: " << fixed << setprecision(3) << setw(7) << ret;
    if (iteration < m_maxIterations)
    {
        cout << " after " << iteration << " Iterations";
    }
    cout << endl;
    if (m_verbose)
    {
        cout << "Result: " << endl << m_dataCloud->getDeltaPose() << endl;
    }
    return m_dataCloud->getPose();
}

void ICPPointAlign::getPointPairs(PointPairVector& pairs, Vector3f& centroid_m, Vector3f& centroid_d) const
{
    size_t n = m_dataCloud->count();
    Vector3f* dataPoints = m_dataCloud->points();

    centroid_m = Vector3f::Zero();
    centroid_d = Vector3f::Zero();
    pairs.clear();

    #pragma omp parallel
    {
        Vector3f my_centroid_m = Vector3f::Zero();
        Vector3f my_centroid_d = Vector3f::Zero();
        PointPairVector my_pairs;
        my_pairs.reserve(n / omp_get_num_threads());

        Vector3f* point;
        Vector3f* neighbor;
        float distance;

        #pragma omp for nowait
        for (size_t i = 0; i < n; i++)
        {
            point = dataPoints + i;

            m_searchTree->nearestNeighbor(*point, neighbor, distance, m_maxDistanceMatch);

            if (neighbor != nullptr)
            {
                my_centroid_m += *point;
                my_centroid_d += *neighbor;
                my_pairs.push_back(make_pair(point, neighbor));
            }
        }

        #pragma omp critical
        {
            centroid_m += my_centroid_m;
            centroid_d += my_centroid_d;
            pairs.insert(pairs.end(), my_pairs.begin(), my_pairs.end());
        }
    }

    centroid_m /= pairs.size();
    centroid_d /= pairs.size();
}

void ICPPointAlign::setMaxMatchDistance(float d)
{
    m_maxDistanceMatch = d;
}

void ICPPointAlign::setMaxIterations(int i)
{
    m_maxIterations = i;
}

void ICPPointAlign::setEpsilon(double e)
{
    m_epsilon = e;
}
void ICPPointAlign::setVerbose(bool verbose)
{
    m_verbose = verbose;
}

float ICPPointAlign::getMaxMatchDistance() const
{
    return m_maxDistanceMatch;
}

int ICPPointAlign::getMaxIterations() const
{
    return m_maxIterations;
}

double ICPPointAlign::getEpsilon() const
{
    return m_epsilon;
}

bool ICPPointAlign::getVerbose() const
{
    return m_verbose;
}

} /* namespace lvr2 */
