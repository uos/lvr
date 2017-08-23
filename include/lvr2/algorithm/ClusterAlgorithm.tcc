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
 * ClusterAlgorithm.tcc
 *
 *  @date 20.07.2017
 *  @author Jan Philipp Vogtherr <jvogtherr@uni-osnabrueck.de>
 *  @author Kristin Schmidt <krschmidt@uni-osnabrueck.de>
 */

#include <lvr2/geometry/HalfEdgeMesh.hpp>

#include <boost/math/constants/constants.hpp>
#include <cmath>
#include <limits>
#include <unordered_set>

namespace lvr2
{

template<typename BaseVecT>
std::vector<VertexHandle> calculateClusterContourVertices(
    ClusterHandle clusterH,
    const BaseMesh<BaseVecT>& mesh,
    const ClusterBiMap<FaceHandle>& clusterBiMap
)
{
    std::unordered_set<VertexHandle> contourVertices;

    auto cluster = clusterBiMap.getCluster(clusterH);

    // iterate over all faces in cluster
    for (auto faceH : cluster.handles)
    {
        // get edges of each face
        const auto edgesOfFace = mesh.getEdgesOfFace(faceH);


        // check for each edge if it is a contour edge
        for (auto edgeH : edgesOfFace)
        {
            const auto faces = mesh.getFacesOfEdge(edgeH);
            int numFaces = 0;

            // count how many faces the edge has that are in the same cluster
            numFaces += faces[0] && clusterH == clusterBiMap.getClusterH(faces[0].unwrap()) ? 1 : 0;
            numFaces += faces[1] && clusterH == clusterBiMap.getClusterH(faces[1].unwrap()) ? 1 : 0;

            // if there is exactly one face, the edge is a contour edge
            if (numFaces == 1)
            {
                // add the vertices of contour edges to an unordered set, which
                // automatically doesn't add duplicates
                for (auto vertexH : mesh.getVerticesOfEdge(edgeH))
                {
                    contourVertices.insert(vertexH);
                }
            }
        }
    }

    return std::vector<VertexHandle>(contourVertices.begin(), contourVertices.end());
}


template<typename BaseVecT>
BoundingRectangle<BaseVecT> calculateBoundingRectangle(
    const std::vector<VertexHandle>& contour,
    const BaseMesh<BaseVecT>& mesh,
    const Cluster<FaceHandle>& cluster,
    const FaceMap<Normal<BaseVecT>>& normals,
    float texelSize,
    const ClusterBiMap<FaceHandle>& clusterBiMap,
    ClusterHandle clusterH
)
{

    // TODO error handling for texelSize = 0
    // TODO reasonable error handling necessary for empty contour vector
    if (contour.size() == 0)
    {
        cout << "Empty contour array." << endl;
    }
    int minArea = std::numeric_limits<int>::max();

    float bestMinA, bestMaxA, bestMinB, bestMaxB;
    Vector<BaseVecT> bestVec1, bestVec2;

    // // calculate regression plane for the cluster
    Plane<BaseVecT> regressionPlane = calcRegressionPlane(mesh, cluster, normals);
    // Plane<BaseVecT> regressionPlane = calcRegressionPlane2(mesh, cluster, normals);

    SparseClusterMap<Plane<BaseVecT>> planes;
    planes.insert(clusterH, regressionPlane);

    std::stringstream ss;
    ss << "debugplane" << clusterH << ".ply";
    debugPlanes(mesh, clusterBiMap, planes, ss.str() , 1000);

    // support vector for the plane
    Vector<BaseVecT> supportVector = regressionPlane.pos.asVector();

    // calculate two orthogonal vectors in the plane
    auto normal = regressionPlane.normal;
    // auto vec1 = normal.cross(Vector<BaseVecT>(-normal.getY(), normal.getX(), 0) + normal.asVector());
    // Vector<BaseVecT> vec2 = normal.cross(vec1);
    auto vec1 = normal.cross(mesh.getVertexPosition(contour[0]) - mesh.getVertexPosition(contour[1]));
    vec1.normalize();
    Vector<BaseVecT> vec2 = vec1.cross(normal.asVector());
    vec2.normalize();



    // Vector<BaseVecT> contour0 = mesh.getVertexPosition(contour[0]).asVector();
    // Vector<BaseVecT> contour1 = mesh.getVertexPosition(contour[1]).asVector();
    // Vector<BaseVecT> contour2 = mesh.getVertexPosition(contour[2]).asVector();

    // Vector<BaseVecT> n = (contour1-contour0).cross(contour2-contour0);
    // if (n.x < 0)
    // {
    //     n *= -1;
    // }
    // Normal<BaseVecT> normal(n);


    // Vector<BaseVecT> supportVector, vec1, vec2;
    // supportVector = contour0;
    // vec1 = contour1 - contour0;
    // vec2.x = vec2.y = vec2.z = 0;


    const float pi = boost::math::constants::pi<float>();

    // resolution of iterative improvement steps for a fourth rotation
    float delta = (pi / 2) / 90;

    for(float theta = 0; theta < M_PI / 2; theta += delta)
    {
        // rotate the bounding box
        vec1 = vec1 * cos(theta) + vec2 * sin(theta);
        vec2 = vec1.cross(normal.asVector());

        // calculate hessian normal forms for both planes to which the distances will be calculated
        Normal<BaseVecT> planeNormal1 = (supportVector.dot(vec1) >= 0)? vec1.normalized() : -vec1.normalized();
        float planeDist1 = planeNormal1.dot(supportVector);
        Normal<BaseVecT> planeNormal2 = (supportVector.dot(vec2) >= 0)? vec2.normalized() : -vec2.normalized();
        float planeDist2 = planeNormal2.dot(supportVector);

        float minA = std::numeric_limits<float>::max();
        float maxA = std::numeric_limits<float>::lowest();
        float minB = std::numeric_limits<float>::max();
        float maxB = std::numeric_limits<float>::lowest();


        // calculate the bounding box

        for(auto contourVertexH: contour)
        {
            // TODO: Besser vorberechnen?
            auto contourPoint = mesh.getVertexPosition(contourVertexH);

            // calculate distance to plane1
            float dist1 = planeNormal1.dot(contourPoint) - planeDist1;
            // calculate distance to plane2
            float dist2 = planeNormal2.dot(contourPoint) - planeDist2;

            float a = dist1;
            float b = dist2;

            // memorize largest positive and negative distance to both planes
            if (a > maxA)
            {
                maxA = a;
            }
            if (a < minA)
            {
                minA = a;
            }
            if (b > maxB)
            {
                maxB = b;
            }
            if (b < minB)
            {
                minB = b;
            }
        }

        // calculate predicted number of texels for both dimesions
        int texelsX = std::ceil((maxA - minA) / texelSize);
        int texelsY = std::ceil((maxB - minB) / texelSize);

        //iterative improvement of the area
        if(texelsX * texelsY < minArea)
        {
            minArea = texelsX * texelsY;
            bestMinA = minA;
            bestMaxA = maxA;
            bestMinB = minB;
            bestMaxB = maxB;
            bestVec1 = vec1;
            bestVec2 = vec2;
        }
    }

    // cout << "min area: " << minArea << endl;

    return BoundingRectangle<BaseVecT>(
        supportVector,
        bestVec1,
        bestVec2,
        normal,
        bestMinA,
        bestMaxA,
        bestMinB,
        bestMaxB
    );

}


} // namespace lvr2
