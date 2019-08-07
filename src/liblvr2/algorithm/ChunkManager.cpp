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
 * ChunkManager.cpp
 *
 * @date 21.07.2019
 * @author Malte kl. Piening
 * @author Marcel Wiegand
 * @author Raphael Marx
 */

#include <cmath>

#include "lvr2/algorithm/ChunkManager.hpp"
#include "lvr2/io/ModelFactory.hpp"

namespace lvr2
{

ChunkManager::ChunkManager(MeshBufferPtr mesh, float chunksize, std::string savePath) : m_chunkSize(chunksize)
{
    initBoundingBox(mesh);

    // compute number of chunks for each dimension
    m_amount.x = (std::size_t) std::ceil(m_boundingBox.getXSize() / m_chunkSize);
    m_amount.y = (std::size_t) std::ceil(m_boundingBox.getYSize() / m_chunkSize);
    m_amount.z = (std::size_t) std::ceil(m_boundingBox.getZSize() / m_chunkSize);

    buildChunks(mesh, savePath);
}

MeshBufferPtr ChunkManager::extractArea(const BoundingBox<BaseVector<float>>& area)
{
    std::vector<MeshBufferPtr> chunks;

    // find all required chunks
    // TODO: check if we need + 1
    BaseVector<float> maxSteps = (area.getMax() - area.getMin()) / m_chunkSize;
    for (std::size_t i = 0; i < maxSteps.x; ++i)
    {
        for (std::size_t j = 0; j < maxSteps.y; ++j)
        {
            for (std::size_t k = 0; k < maxSteps.z; ++k)
            {
                std::size_t cellIndex = getCellIndex(
                        area.getMin() + BaseVector<float>(i * m_chunkSize, j * m_chunkSize, k * m_chunkSize));

                auto it = m_hashGrid.find(cellIndex);
                if(it == m_hashGrid.end())
                {
                  continue;
                }
                // TODO: remove saving tmp chunks later
                ModelFactory::saveModel(lvr2::ModelPtr(new lvr2::Model(it->second)), "area/" + std::to_string(cellIndex) + ".ply");
                chunks.push_back(it->second);
            }
        }
    }

    // TODO: concat chunks
    MeshBufferPtr areaMeshPtr = nullptr;

    return areaMeshPtr;
}

void ChunkManager::initBoundingBox(MeshBufferPtr mesh)
{
    BaseVector<float> currVertex;

    for (std::size_t i = 0; i < mesh->numVertices(); i++)
    {
        currVertex.x = mesh->getVertices()[3 * i];
        currVertex.y = mesh->getVertices()[3 * i + 1];
        currVertex.z = mesh->getVertices()[3 * i + 2];

        m_boundingBox.expand(currVertex);
    }
}

bool ChunkManager::cutFace(BaseVector<float> v1, BaseVector<float> v2, BaseVector<float> v3, float alpha, std::vector<ChunkBuilderPtr>& chunkBuilders)
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (i != j)
            {
                BaseVector<float> v;
                switch(i)
                {
                    case 0:
                        v = v1;
                        break;
                    case 1:
                        v = v2;
                        break;
                    case 2:
                        v = v3;
                        break;
                }

                BaseVector<float> vw;
                switch(j)
                {
                    case 0:
                        vw = v1;
                        break;
                    case 1:
                        vw = v2;
                        break;
                    case 2:
                        vw = v3;
                        break;
                }

                for (int k = 0; k < 3; k++)
                {
                    float vKey = v[k];
                    float vwKey = vw[k];

                    float plane = m_chunkSize * ((int) (vKey / m_chunkSize));
                    if (vKey < vwKey)
                    {
                        plane += m_chunkSize;
                    }

                    bool large = false;
                    if (vKey - plane < 0 && vwKey - plane >= 0)
                    {
                        if (vKey - plane > -alpha * m_chunkSize
                                && vwKey - plane > alpha * m_chunkSize)
                        {
                            large = true;
                        }
                    }
                    else if (vKey - plane >= 0 && vwKey - plane < 0)
                    {
                        if (vKey - plane > alpha * m_chunkSize
                                && vwKey - plane > -alpha * m_chunkSize)
                        {
                            large = true;
                        }
                    }

                    if (large)
                    {
                        BaseVector<float> vec11, vec12, vec13;
                        BaseVector<float> vec21, vec22, vec23;
                        switch(i)
                        {
                            case 0:
                                switch(j)
                                {
                                    case 1:
                                        vec11 = v1;
                                        vec12 = (v + vw) / 2;
                                        vec13 = v3;

                                        vec21 = (v + vw) / 2;
                                        vec22 = v2;
                                        vec23 = v3;
                                        break;
                                    case 2:
                                        vec11 = v1;
                                        vec12 = v2;
                                        vec13 = (v + vw) / 2;

                                        vec21 = v2;
                                        vec22 = v3;
                                        vec23 = (v + vw) / 2;
                                        break;
                                }
                                break;
                            case 1:
                                switch(j)
                                {
                                    case 0:
                                        vec11 = v1;
                                        vec12 = (v + vw) / 2;
                                        vec13 = v3;

                                        vec21 = (v + vw) / 2;
                                        vec22 = v2;
                                        vec23 = v3;
                                        break;
                                    case 2:
                                        vec11 = v1;
                                        vec12 = v2;
                                        vec13 = (v + vw) / 2;

                                        vec21 = v1;
                                        vec22 = (v + vw) / 2;
                                        vec23 = v3;
                                        break;
                                }
                                break;
                            case 2:
                                switch(j)
                                {
                                    case 0:
                                        vec11 = v1;
                                        vec12 = v2;
                                        vec13 = (v + vw) / 2;

                                        vec21 = v2;
                                        vec22 = v3;
                                        vec23 = (v + vw) / 2;
                                        break;
                                    case 1:
                                        vec11 = v1;
                                        vec12 = v2;
                                        vec13 = (v + vw) / 2;

                                        vec21 = v1;
                                        vec22 = (v + vw) / 2;
                                        vec23 = v3;
                                        break;
                                }
                                break;
                        }

                        if (!cutFace(vec11, vec12, vec13, alpha, chunkBuilders))
                        {
                            int a = chunkBuilders[getCellIndex((vec11 + vec12 + vec13) / 3)]->addAdditionalVertex(vec11.x, vec11.y, vec11.z);
                            int b = chunkBuilders[getCellIndex((vec11 + vec12 + vec13) / 3)]->addAdditionalVertex(vec12.x, vec12.y, vec12.z);
                            int c = chunkBuilders[getCellIndex((vec11 + vec12 + vec13) / 3)]->addAdditionalVertex(vec13.x, vec13.y, vec13.z);

                            chunkBuilders[getCellIndex((vec11 + vec12 + vec13) / 3)]->addAdditionalFace(-a, -b, -c);
                        }
                        if (!cutFace(vec21, vec22, vec23, alpha, chunkBuilders))
                        {
                            int a = chunkBuilders[getCellIndex((vec21 + vec22 + vec23) / 3)]->addAdditionalVertex(vec21.x, vec21.y, vec21.z);
                            int b = chunkBuilders[getCellIndex((vec21 + vec22 + vec23) / 3)]->addAdditionalVertex(vec22.x, vec22.y, vec22.z);
                            int c = chunkBuilders[getCellIndex((vec21 + vec22 + vec23) / 3)]->addAdditionalVertex(vec23.x, vec23.y, vec23.z);

                            chunkBuilders[getCellIndex((vec21 + vec22 + vec23) / 3)]->addAdditionalFace(-a, -b, -c);
                        }

                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void ChunkManager::buildChunks(MeshBufferPtr mesh, std::string savePath)
{
    // one vector of variable size for each vertex - this is used for duplicate detection
    std::shared_ptr<std::vector<std::vector<ChunkBuilderPtr>>> vertexUse(new std::vector<std::vector<ChunkBuilderPtr>>(mesh->numVertices(), std::vector<std::shared_ptr<ChunkBuilder>>()));

    std::vector<ChunkBuilderPtr> chunkBuilders(m_amount.x * m_amount.y * m_amount.z);

    for (std::size_t i = 0; i < m_amount.x; i++)
    {
        for (std::size_t j = 0; j < m_amount.y; j++)
        {
            for (std::size_t k = 0; k < m_amount.z; k++)
            {
                chunkBuilders[hashValue(i, j, k)] = ChunkBuilderPtr(new ChunkBuilder(mesh, vertexUse));
            }
        }
    }

    // assign the faces to the chunks
    FloatChannel verticesChannel = *mesh->getFloatChannel("vertices");
    IndexChannel facesChannel = *mesh->getIndexChannel("face_indices");
    BaseVector<float> currentCenterPoint;
    for (std::size_t i = 0; i < mesh->numFaces(); i++)
    {
        currentCenterPoint = getFaceCenter(verticesChannel, facesChannel, i);
        unsigned int cellIndex = getCellIndex(currentCenterPoint);
        bool added = false;
        for (int j = 0; j < 3; j++)
        {
            BaseVector<float> vertex(verticesChannel[facesChannel[i][j]]);

            if (getCellIndex(vertex) != cellIndex)
            {
                added = cutFace(BaseVector<float>(verticesChannel[facesChannel[i][0]]),
                            BaseVector<float>(verticesChannel[facesChannel[i][1]]),
                            BaseVector<float>(verticesChannel[facesChannel[i][2]]),
                            0.001,
                            chunkBuilders);
                break;
            }
        }

        if (!added)
        {
            chunkBuilders[cellIndex]->addFace(i);
        }
        else
        {
            std::cout << "large" << std::endl;
        }
    }

    // save the chunks as .ply
    for (std::size_t i = 0; i < m_amount.x; i++)
    {
        for (std::size_t j = 0; j < m_amount.y; j++)
        {
            for (std::size_t k = 0; k < m_amount.z; k++)
            {
                std::size_t hash = hashValue(i, j, k);

                if (chunkBuilders[hash]->numFaces() > 0)
                {
                    std::cout << "writing " << i << " " << j << " " << k << std::endl;

                    // get mesh of chunk from chunk builder
                    MeshBufferPtr chunkMeshPtr = chunkBuilders[hash]->buildMesh();

                    // insert chunked mesh into hash grid
                    m_hashGrid.insert({hash, chunkMeshPtr});

                    // export chunked meshes for debugging
                    ModelFactory::saveModel(
                        ModelPtr(new Model(chunkMeshPtr)),
                        savePath + "/" + std::to_string(i) + "-" + std::to_string(j) + "-" + std::to_string(k) + ".ply");
                }
            }
        }
    }
}

BaseVector<float> ChunkManager::getFaceCenter(FloatChannel verticesChannel, IndexChannel facesChannel, unsigned int faceIndex)
{
    BaseVector<float> vertex1(verticesChannel[facesChannel[faceIndex][0]]);
    BaseVector<float> vertex2(verticesChannel[facesChannel[faceIndex][1]]);
    BaseVector<float> vertex3(verticesChannel[facesChannel[faceIndex][2]]);

    return (vertex1 + vertex2 + vertex3) / 3;
}

std::size_t ChunkManager::getCellIndex(const BaseVector<float>& vec)
{
    BaseVector<float> tmpVec = (vec - m_boundingBox.getMin()) / m_chunkSize;
    return (std::size_t) tmpVec.x * m_amount.y * m_amount.z + (std::size_t) tmpVec.y * m_amount.z + (std::size_t) tmpVec.z;
}

} /* namespace lvr2 */
