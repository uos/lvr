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

#ifndef LVR2_REDUCTION_ALGORITHM_HPP
#define LVR2_REDUCTION_ALGORITHM_HPP

#include "lvr2/io/PointBuffer.hpp"

#include <memory>

namespace lvr2
{

/**
 * @brief Interface defintion for algorithms that reduce point 
 *        buffer objects
 * 
 */
class ReductionAlgorithm
{
public:

     /**
      * @brief Returns a reduced version of a point buffer object
      *        The structure of the reduced point buffer depends 
      *        on the implementation that provides this interface
      *        method.
      */
     virtual PointBufferPtr getReducedPoints() = 0;

     /**
      * @brief Sets the Point Buffer object. Overriding classes 
      *        may replace the default behavior, which just stores
      *        the internal pointer, to provided internal structures
      *        that are needed to reduce the data, e.g., Octrees
      * 
      * @param ptr Point buffer containing the original point cloud data
      */
     virtual void setPointBuffer(PointBufferPtr ptr) { m_pointBuffer = ptr; }
protected:

     PointBufferPtr      m_pointBuffer;
};

using ReductionAlgorithmPtr = std::shared_ptr<ReductionAlgorithm>;




// EXAMPLE: Reduce All: return empty
class AllReductionAlgorithm : public ReductionAlgorithm 
{
public:
     virtual PointBufferPtr getReducedPoints() 
     {
          PointBufferPtr empty;
          return empty;
     }
};


class NoReductionAlgorithm : public ReductionAlgorithm 
{
public:
     virtual PointBufferPtr getReducedPoints() 
     {
          return m_pointBuffer;
     }
};


} // namespace lvr2

#endif // LVR2_REDUCTION_ALGORITHM_HPP