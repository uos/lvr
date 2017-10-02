/*
 * MPITree.hpp
 *
 *  Created on: 15.01.2013
 *      Author: Dominik Feldschnieders
 */

#ifndef MPITREE_HPP_
#define MPITREE_HPP_

#include <lvr/geometry/BoundingBox.hpp>
#include <lvr/io/ModelFactory.hpp>
#include <lvr/io/PointBuffer.hpp>
#include <lvr/geometry/Vertex.hpp>
#include <lvr/io/Model.hpp>

#include <lvr/io/PLYIO.hpp>
#include <lvr/io/AsciiIO.hpp>
#include <lvr/io/UosIO.hpp>

// vorerst
#include <cstring>
#include <iostream>
#include <list>
#include <cctype>
#include "MPINode.hpp"
#include "boost/shared_array.hpp"

namespace lvr{

/**
 * @brief A class for execution of a distribution using a very simple Kd-tree.
 *     The classification criterion is to halve the longest axis.
 */
template<typename VertexT>
class MPITree {
public:

    virtual ~MPITree();

    /**
     * @brief Constructor.
     *
     * @param pointcloud The point cloud, which is to be divided
     * @param max        Max number of Points
     * @param min        Min number of Points
     * @param median     is True, if the median is used for segmenting the pointcloud
     */
    MPITree(PointBufferPtr pointcloud, long int max, long int min, bool median);

    /**
     * @brief starts the recursion and saves the packages in files (scan*.3d)
     *
     * @param first The first Node with the whole pointcloud in it
     */
    void Rekursion(MPINode<VertexT> * first);


    /**
     * @brief This function devide the pointcloud in two packages.
     *
     *     The classification criterion is to halve the longest axis.
     *    When a partial pointcloud has less than the maximum number of points is thus, it will be stored in a 3D file.
     *
     *@param child Instance MPINode 
     *@param again count if this is the second call with the same MPINode, if it is take another axis
     */
    void splitPointcloud(MPINode<VertexT> * child, int again);

    /**
     * @brief returns the list of MPINodes
     */
    list<MPINode<VertexT>*> GetList();
    

    /**
     *@brief function to sort list
     */
    bool static compare_nocase (MPINode<VertexT> * first, MPINode<VertexT> * second);    

    /**
     * @brief return the BoundingBox
     */
    BoundingBox<VertexT> GetBoundingBox();


    // The pointcloud
    PointBufferPtr         m_loader;

    /// The currently stored points
    coord3fArr               m_points;

    /// The bounding box of the point cloud
    BoundingBox<VertexT>   m_boundingBox;

    // A list for all Nodes with less than MAX_POINTS
    std::list<MPINode<VertexT>*> nodelist;

    // number of max Points in one packete
    long int max_points;
  
    // number of min Points in one packete
    long int min_points;


    // Number of points in the point cloud
    size_t m_numpoint;

    //use median
    bool m_median;

    // Number of points per packet
    double m_bucketsize;

    // A shared-Pointer for the model, with the pointcloud in it  */
    ModelPtr m_model;
};
}

#include "MPITree.tcc"
#endif /* MPITREE_HPP_ */
