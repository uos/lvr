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
 * SlamAlign.hpp
 *
 *  @date May 6, 2019
 *  @author Malte Hillmann
 */
#ifndef SLAMALIGN_HPP_
#define SLAMALIGN_HPP_

#include "Scan.hpp"
#include "SlamOptions.hpp"
#include "GraphSlam.hpp"

namespace lvr2
{

/**
 * @brief A class to run Slam on Scans
 */
class SlamAlign
{

public:
    /**
     * @brief Creates a new SlamAlign instance with the given Options and Scans
     *
     * This does not yet register the Scans, it only applies reduction options if specified
     *
     * @param options The Options to use
     * @param scans The Scans to start with
     */
    SlamAlign(const SlamOptions& options, vector<ScanPtr>& scans);

    /**
     * @brief Creates a new SlamAlign instance with the given Options
     *
     * @param options The Options to use
     */
    SlamAlign(const SlamOptions& options = SlamOptions());

    virtual ~SlamAlign() = default;

    /**
     * @brief Adds a new Scan to the Slam instance
     *
     * This method will apply any reduction options that are specified
     *
     * @param scan The new Scan
     * @param match true: Immediately call match() with the new Scan added
     */
    void addScan(const ScanPtr& scan, bool match = false);

    /**
     * @brief Returns a shared_ptr to a Scan
     *
     * @param index The index of the Scan
     */
    ScanPtr getScan(size_t index) const;

    /**
     * @brief Executes SLAM on all current Scans
     *
     * This methods registers any new Scans added since the last call to match()
     * (or the creation of this instance) using Scanmatching and Loopclosing, as specified by
     * the SlamOptions.
     *
     * Calling this method several times without adding any new Scans has no additional effect
     * after the first call.
     */
    void match();

    /**
     * @brief Indicates that no new Scans will be added
     *
     * This method ensures that all Scans are properly registered, including any Loopclosing
     */
    void finish();

    /**
     * @brief Sets the SlamOptions struct to the parameter
     *
     * Note that changing options on an active SlamAlign instance with previously added / matched
     * Scans can cause Undefined Behaviour.
     *
     * @param options The new options
     */
    void setOptions(const SlamOptions& options);

    /**
     * @brief Returns a reference to the internal SlamOptions struct
     *
     * This can be used to make changes to specific values within the SlamOptions without replacing
     * the entire struct.
     *
     * Note that changing options on an active SlamAlign instance with previously added / matched
     * Scans can cause Undefined Behaviour.
     */
    SlamOptions& options();

    /**
     * @brief Returns a reference to the internal SlamOptions struct
     */
    const SlamOptions& options() const;

protected:

    void reduceScan(const ScanPtr& scan);

    void applyTransform(ScanPtr scan, const Matrix4d& transform);

    void checkLoopClose(size_t last);
    void loopClose(size_t first, size_t last);
    void graphSlam(size_t last);

    SlamOptions             m_options;

    vector<ScanPtr>         m_scans;

    ScanPtr                 m_metascan;

    GraphSlam               m_graph;
    bool                    m_foundLoop;

    size_t                  m_alreadyMatched;
};

} /* namespace lvr2 */

#endif /* SLAMALIGN_HPP_ */
