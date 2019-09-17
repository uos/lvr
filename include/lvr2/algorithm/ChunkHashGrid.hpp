/**
 * Copyright (c) 2019, University Osnabrück
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
* ChunkHashGrid.hpp
*
* @date 17.09.2019
* @author Raphael Marx
*/

#ifndef CHUNK_HASH_GRID_HPP
#define CHUNK_HASH_GRID_HPP

#include "lvr2/io/ChunkIO.hpp"

namespace lvr2
{
class ChunkHashGrid
{
public:
    ChunkHashGrid() = default;
    /**
     * @brief class to load (and in the future cache) chunks from an HDF5 file
     *
     * @param hdf5Path path to the HDF5 file
     */
    explicit ChunkHashGrid(std::string hdf5Path);
    /**
     * @brief loads a chunk from the HDF5 file into the hash grid
     *
     * @param cellIndex hash-value of the chunk
     * @return true if loading was possible, false if loading was not possible
     *         (for example if the hash-value does not belong to a mesh with vertices)
     */
    bool loadChunk(size_t cellIndex);
    /**
     * @brief returns a mesh of a given cellIndex
     *
     * @param cellIndex hash-value of the chunk
     * @return the MeshBufferPtr containing the chunk
     */
    MeshBufferPtr findChunk(size_t cellIndex);
private:
    // hash grid containing chunked meshes
    std::unordered_map<std::size_t, MeshBufferPtr> m_hashGrid;

    // chunkIO for the HDF5 file-IO
    std::shared_ptr<lvr2::ChunkIO> m_chunkIO;

    // number of chunks that will be cached before deleting old chunks
    // size_t m_cacheSize = 100;
};

} /* namespace lvr2 */
#endif //CHUNK_HASH_GRID_HPP
