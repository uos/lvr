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

#pragma once

// std includes
#include <memory>

// lvr2 includes
#include "lvr2/algorithm/Texturizer.hpp"


namespace lvr2
{

template <typename BaseVecT>
class RayCastingTexturizer: public Texturizer<BaseVecT>
{
public:
    using Ptr = std::shared_ptr<RayCastingTexturizer<BaseVecT>>;
    
    /**
     * @brief Construct a new Ray Casting Texturizer object
     * 
     * @param texelSize The size of one texture pixel, relative to the coordinate system of the point cloud
     * @param texMinClusterSize The minimum number of faces a cluster needs to be texturized
     * @param texMaxClusterSize The maximum number of faces a cluster needs to be texturized
     */
    RayCastingTexturizer(
        float texelMinSize,
        int texMinClusterSize,
        int texMaxClusterSize
    );

    /**
     * @brief Generates a texture for a given bounding rectangle
     *
     * Create a grid, based on given information (texel size, bounding rectangle).
     * For each cell in the grid (which represents a texel), cast a ray from the camera
     * to the texel in 3D space to check for visibility. If the texel is visible calculate
     * the texel color based on the RGB image data.
     *
     * @param index The index the texture will get
     * @param surface The point cloud
     * @param boundingRect The bounding rectangle of the cluster
     *
     * @return Texture handle of the generated texture
     */
    virtual TextureHandle generateTexture(
        int index,
        const PointsetSurface<BaseVecT>& surface,
        const BoundingRectangle<typename BaseVecT::CoordType>& boundingRect
    );

};

} // namespace lvr2

#include "RayCastingTexturizer.tcc"