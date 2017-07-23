/* Copyright (C) 2011 Uni Osnabrück
* This file is part of the LAS VEGAS Reconstruction Toolkit,
*
* LAS VEGAS is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* LAS VEGAS is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
*/

/*
* Texturizer.tcc
*
*  @date 17.06.2017
*  @author Jan Philipp Vogtherr <jvogtherr@uni-osnabrueck.de>
*  @author Kristin Schmidt <krschmidt@uni-osnabrueck.de>
*/

#include "ClusterAlgorithm.hpp"


namespace lvr2
{

template<typename BaseVecT>
TexturizerResult generateTextures(
    float texelSize,
    int textureThreshold,
    HalfEdgeMesh<BaseVecT>& mesh,
    ClusterSet<FaceHandle>& faceHandleClusterSet,
    PointsetSurfacePtr<BaseVecT> surface,
    const FaceMap<Normal<BaseVecT>>& normals
)
{
    int numFacesThreshold = 20000; // TODO: read from config
    int textureIndex = 1;

    for (auto clusterH: faceHandleClusterSet)
    {
        const Cluster<FaceHandle> cluster = faceHandleClusterSet.getCluster(clusterH);
        int numFacesInCluster = cluster.handles.size();

        // only create textures for clusters that are large enough
        if (numFacesInCluster >= textureThreshold)
        // if (numFacesInCluster >= numFacesThreshold && numFacesInCluster < 200000)
        {
            // contour
            // TODO: use better contour function
            std::vector<Point<BaseVecT>> contour = calculateAllContourVertices(clusterH, mesh, faceHandleClusterSet);

            // bounding rectangle
            BoundingRectangle<BaseVecT> br = calculateBoundingRectangle(contour, mesh, cluster, normals, texelSize);

            // cout << endl;
            cout << "bounding box: " << br.minDistA << "  " << br.maxDistA
            << ",  b: " << br.minDistB << "  " << br.maxDistB
            << " (contourSize: " << contour.size() << ")"
            << " (numFaces: " << numFacesInCluster << ")" << endl;
            cout << "vec1: " << br.vec1 << "  vec2: " << br.vec2 << endl;

            // initial texture
            TextureToken<BaseVecT>* t = generateTexture(br, surface, texelSize, textureIndex++);
            t->m_texture->save(t->m_textureIndex);

            // TODO:
            // zuordnen & speichern
        }
    }

    return TexturizerResult();
}

template <typename BaseVecT>
TextureToken<BaseVecT>* generateTexture(
    BoundingRectangle<BaseVecT>& boundingRect,
    PointsetSurfacePtr<BaseVecT> surface,
    float texelSize,
    int textureIndex
)
{
    // calculate the texture size
    unsigned short int sizeX = ceil((boundingRect.maxDistA - boundingRect.minDistA) / texelSize);
    unsigned short int sizeY = ceil((boundingRect.maxDistB - boundingRect.minDistB) / texelSize);

    // create texture
    Texture* texture = new Texture(sizeX, sizeY, 3, 1, 0);//, 0, 0, 0, 0, false, 0, 0);

    // create TextureToken
    // TODO TextureToken<BaseVecT>* result = new TextureToken<BaseVecT>(boundingRect, texture, textureIndex);
    TextureToken<BaseVecT>* result = new TextureToken<BaseVecT>(
        boundingRect.vec1,
        boundingRect.vec2,
        boundingRect.supportVector,
        boundingRect.minDistA,
        boundingRect.minDistB,
        texture,
        textureIndex
    );

    //cout << "PIXELS IN TEXTURE: " << sizeX * sizeY << endl;
    string msg = lvr::timestamp.getElapsedTime() + "Calculating Texture Pixels... ";
    lvr::ProgressBar progress(sizeX * sizeY, msg);

    int dataCounter = 0;

    for (int y = 0; y < sizeY; y++)
    {
        for (int x = 0; x < sizeX; x++)
        {
            std::vector<char> v;

            int k = 1; // k-nearest-neighbors

            vector<size_t> cv;

            Point<BaseVecT> currentPos =
                boundingRect.supportVector +
                boundingRect.vec1 * ((boundingRect.minDistA + x * texelSize) + texelSize / 2) +
                boundingRect.vec2 * ((boundingRect.minDistB + y * texelSize) + texelSize / 2);

            // Point<BaseVecT> currentPosition = boundingRect.supportVector + boundingRect.vec1
            //     * (x * texelSize + boundingRect.minDistA - texelSize / 2.0)
            //     + boundingRect.vec2
            //     * (y * texelSize + boundingRect.minDistB - texelSize / 2.0);

            surface->searchTree().kSearch(currentPos, k, cv);

            uint8_t r = 0, g = 0, b = 0;

            for (size_t pointIdx : cv)
            {
                array<uint8_t,3> colors = *(surface->pointBuffer()->getRgbColor(pointIdx));
                r += colors[0];
                g += colors[1];
                b += colors[2];
            }

            r /= k;
            g /= k;
            b /= k;

            // texture->m_data[(sizeY - y - 1) * (sizeX * 3) + 3 * x + 0] = r;
            // texture->m_data[(sizeY - y - 1) * (sizeX * 3) + 3 * x + 1] = g;
            // texture->m_data[(sizeY - y - 1) * (sizeX * 3) + 3 * x + 2] = b;
            texture->m_data[(y) * (sizeX * 3) + 3 * x + 0] = r;
            texture->m_data[(y) * (sizeX * 3) + 3 * x + 1] = g;
            texture->m_data[(y) * (sizeX * 3) + 3 * x + 2] = b;
            // texture->m_data[(sizeX - x - 1) * (sizeY * 3) + 3 * y] = r;
            // texture->m_data[(sizeX - x - 1) * (sizeY * 3) + 3 * y + 1] = g;
            // texture->m_data[(sizeX - x - 1) * (sizeY * 3) + 3 * y + 2] = b;
            // texture->m_data[(dataCounter * 3) + 0] = r;
            // texture->m_data[(dataCounter * 3) + 1] = g;
            // texture->m_data[(dataCounter * 3) + 2] = b;
            // dataCounter++;

        }
        ++progress;
    }

    return result;

}

} // namespace lvr2
