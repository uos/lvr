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

/*
 * BigGridKdTree.hpp
 *
 *  Created on: Aug 30, 2017
 *      Author: Isaak Mitschke
 */

#ifndef LAS_VEGAS_BIGGRIDKDTREE_H
#define LAS_VEGAS_BIGGRIDKDTREE_H
#include "BigGrid.hpp"

#include <lvr2/geometry/BoundingBox.hpp>
#include <memory>
#include <vector>

namespace lvr2
{

template <typename BaseVecT>
class BigGridKdTree
{
  public:
    BigGridKdTree(BoundingBox<BaseVecT>& bb,
                  size_t maxNodePoints,
                  BigGrid<BaseVecT>* grid,
                  float voxelsize,
                  size_t numPoints = 0);
    virtual ~BigGridKdTree();
    void insert(size_t numPoints, BaseVecT pos);
    static std::vector<BigGridKdTree*> getLeafs();
    static std::vector<BigGridKdTree*> getNodes() { return s_nodes; }
    inline size_t getNumPoints() { return m_numPoints; }
    inline BoundingBox<BaseVecT>& getBB() { return m_bb; }

  private:
    BoundingBox<BaseVecT> m_bb;
    size_t m_numPoints;
    std::vector<BigGridKdTree*> m_children;
    BigGridKdTree(BoundingBox<BaseVecT>& bb, size_t numPoints = 0);

    static std::vector<BigGridKdTree*> s_nodes;
    static float s_voxelsize;
    static size_t s_maxNodePoints;
    static BigGrid<BaseVecT>* m_grid;
    inline bool fitsInBox(BaseVecT& pos)
    {
        return pos.x > m_bb.getMin().x && pos.y > m_bb.getMin().y && pos.z > m_bb.getMin().z &&
               pos.x < m_bb.getMax().x && pos.y < m_bb.getMax().y && pos.z < m_bb.getMax().z;
    }
};

} // namespace lvr2

#include "lvr2/reconstruction/BigGridKdTree.tcc"

#endif // LAS_VEGAS_BIGGRIDKDTREE_H
