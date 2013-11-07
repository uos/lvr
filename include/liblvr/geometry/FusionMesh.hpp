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
 * FusionMesh.hpp
 *
 *  @date   11.07.2013
 *  @author Ann-Katrin Häuser (ahaeuser@uos.de)
 *  @author Henning Deeken (hdeeken@uos.de)
 *  @author Thomas Wiemann (twiemann@uos.de)
 */

#ifndef FUSIONMESH_H_
#define FUSIONMESH_H_

#include <boost/unordered_map.hpp>

#include <vector>
#include <stack>
#include <set>
#include <list>
#include <map>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <float.h>
#include <math.h>
#include <algorithm>
#include <queue>

#include <glu.h>
#include <glut.h>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Projection_traits_xy_3.h>
#include <CGAL/Projection_traits_yz_3.h>
#include <CGAL/Projection_traits_xz_3.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/squared_distance_3.h>

#include "Vertex.hpp"
#include "VertexTraits.hpp"
#include "Normal.hpp"
#include "BaseMesh.hpp"

#include "FusionVertex.hpp"
#include "FusionFace.hpp"
//#include "FusionEdge.hpp"

#include "io/Timestamp.hpp"
#include "io/Progress.hpp"
#include "io/Model.hpp"

using namespace std;

typedef CGAL::Simple_cartesian<double> K;
typedef K::FT FT;
typedef K::Segment_3 Segment;
typedef K::Point_3 Point;
typedef K::Triangle_3 Triangle;
typedef K::Plane_3 Plane;

typedef CGAL::Exact_predicates_inexact_constructions_kernel KD;
typedef CGAL::Projection_traits_xz_3<KD>  Gt;
typedef CGAL::Delaunay_triangulation_2<Gt> Delaunay;
typedef KD::Point_3 PointD;
typedef KD::Triangle_3 TriangleD;

// CGAL Extensions

class ExtendedTriangle: public Triangle
{
public:
	
	ExtendedTriangle(Point& p, Point& q, Point& r) : Triangle(p, q, r)
		{ m_self_index = 1773; }
	
	ExtendedTriangle(Point& p, Point& q, Point& r, int i) : Triangle(p, q, r)
		{ m_self_index = i; }

	/// The corresponding face's index in the mesh
	int m_self_index;
};

typedef ExtendedTriangle ETriangle;

typedef std::vector<ETriangle>::iterator Iterator;
typedef CGAL::AABB_triangle_primitive<K,Iterator> Primitive;
typedef CGAL::AABB_traits<K, Primitive> AABB_triangle_traits;
typedef CGAL::AABB_tree<AABB_triangle_traits> Tree;
typedef Tree::Object_and_primitive_id Object_and_primitive_id;
typedef Tree::Primitive_id Primitive_id;


namespace lvr
{
cd b
template<typename VertexT, typename NormalT> class FusionVertex;
template<typename VertexT, typename NormalT> class FusionFace;

/**
 * @brief Implementation of a mesh structure that can be used to incrementally fuse different meshes into one.
 */
 
template<typename VertexT, typename NormalT> class FusionMesh : public BaseMesh<VertexT, NormalT>
{
	
public:
	
	typedef FusionFace<VertexT, NormalT> FFace;
	typedef FusionVertex<VertexT, NormalT> FVertex;
	
	typedef map<VertexT, size_t> Map;
	typedef typename map<VertexT, size_t>::iterator MapIterator;
	
	typedef set<Point> PointSet;
	typedef typename set<Point>::iterator PointSetIterator;


// Constructors

	/**
	 * @brief   Creates an empty FusionMesh 
	 */
	FusionMesh();

	/**
	 * @brief   Creates a FusionMesh from the given mesh buffer
	 */
	FusionMesh(MeshBufferPtr model);

	/**
	 * @brief   Destructor.
	 */
	virtual ~FusionMesh() {};

	
// Methods of BaseMesh

	/**
	 * @brief 	This method should be called every time
	 * 			a new vertex is created.
	 *
	 * @param	v 		A supported vertex type. All used vertex types
	 * 					must support []-access.
	 */
	virtual void addVertex(VertexT v);

	/**
	 * @brief 	This method should be called every time
	 * 			a new vertex is created to ensure that vertex
	 * 			and normal buffer always have the same size
	 *
	 * @param	n 		A supported vertex type. All used vertex types
	 * 					must support []-access.
	 */
	virtual void addNormal(NormalT n);

	/**
	 * @brief 	Insert a new triangle into the mesh
	 *
	 * @param	a 		The first vertex of the triangle
	 * @param 	b		The second vertex of the triangle
	 * @param	c		The third vertex of the triangle
	 */
	virtual void addTriangle(uint a, uint b, uint c);

	/**
	 * @brief 	Finalizes a mesh, i.e. converts the template based buffers
	 * 			to OpenGL compatible buffers
	 */
	virtual void finalize();
	
	//unused
	/**
	 * @brief	Flip the edge between vertex index v1 and v2
	 *
	 * @param	v1	The index of the first vertex
	 * @param	v2	The index of the second vertex
	 */
	virtual void flipEdge(uint v1, uint v2);

	
// Fusion Specific Methods
	
	/**
     * @brief   Insert an entire mesh into the local fusion buffer. It is advised to call integrate() afterwards.
     *
     * @param   mesh      A pointer to the mesh to be inserted
     */
	virtual void addMesh(MeshBufferPtr model);
	
	/**
     * @brief   Integrate the local buffer into the global fused mesh
	 *
     */
	virtual void integrate();
	
	/**
     * @brief   Integrate the local buffer into the global mesh, by simply adding all vertices and shifting the face indices
	 *
     */
	virtual void lazyIntegrate();
	
	/**
     * @brief   Insert an entire mesh into the local fusion buffer and integrate it imediately.
     *
     * @param   mesh      A pointer to the mesh to be inserted
     */
	virtual void addMeshAndIntegrate(MeshBufferPtr model);
	
	/**
     * @brief   Insert an entire mesh into the local fusion buffer and lazyintegrate it imediately.
     *
     * @param   mesh      A pointer to the mesh to be inserted
     */
	virtual void addMeshAndLazyIntegrate(MeshBufferPtr model);


// Parameter Methods

	/**
	 * Sets the distance treshold used for AABB search
	 *
	 * @param t 	distance treshold
	 */ 
	void setDistanceThreshold(double_t t) {threshold = t*t;};


private:

	/// The faces in the fusion buffer 
	vector<FusionFace<VertexT, NormalT>*>   m_local_faces;

	/// The vertices of the fusion buffer
	vector<FusionVertex<VertexT, NormalT>*>   m_local_vertices;

	/// The length of the local vertex buffer
	size_t                                      m_local_index;

	/// The faces in the fused mesh
	vector<FusionFace<VertexT, NormalT>*>     m_global_faces;

	/// The vertices of the fused mesh
	vector<FusionVertex<VertexT, NormalT>*>   m_global_vertices;
	///  The length of the global vertex buffer
	size_t                                      m_global_index;

	/// Squared maximal distance for fusion
	double	threshold;
	
	// Nochmal überarbeiten
	/// FaceBuffer used during integration process
	vector<FFace*> remote_faces; 
    vector<FFace*> intersection_faces;
    vector<FFace*> closeby_faces;
    int redundant_faces;
	int special_case_faces;	

	/// The CGAL AABB Tree
	Tree		tree;
	/// The Vector the CGAL Tree is based on
	vector<ETriangle> tree_triangles;
	
	Tree		local_tree;
	vector<ETriangle> local_tree_triangles;
	
	
	/// The Map with all global vertices
	Map			global_vertices_map;
	
	

	/**
     * @brief   Reset the the local buffer e.g. after integration or at initialization.
     */
	virtual void clearLocalBuffer();

	/**
     * @brief   Reset the the global buffer e.g. at initialization.
     */
	//virtual void clearGlobalBuffer();
	
// Printing Methods
		
	/**
     * @brief   Prints the current status of the local buffer on the console.
     */
	virtual void printLocalBufferStatus();
	
	/**
     * @brief   Prints the current status of the local buffer on the console.
     */
	virtual void printGlobalBufferStatus();
	
	/**
     * @brief   Prints the current status of the face sorting process on the console.
     */
	virtual void printFaceSortingStatus();


	
	/**
	 * @brief 	This method should be called every time
	 * 			a vertex is transferred into the global buffer
	 *
	 * @param	v 		A FusionVertex from the local buffer.
	 */
	virtual void addGlobalVertex(FVertex *v);
	
	/**
	 * @brief 	creates CGAL ETriangle from lvr Face
	 */
	virtual ETriangle faceToETriangle(FFace *face);
	
	/**
	 * @brief 	creates CGAL Plane from lvr Face
	 */
	virtual Plane faceToPlane(FFace *face);
	
	/**
     * @brief   Insert a new triangle into global mesh
     *
     * @param   f       A face from the local buffer
     */
	virtual void addGlobalFace(FFace *f);	
    
    /**
     * @brief   build CGAL-AABB-Tree from global mesh
	 *
     */ 
	virtual void buildTree();
	
	/**
     * @brief   build map of global vertices with global buffer index
	 *
     */
	virtual void buildVertexMap();
	
	/**
     * @brief   Sort faces based on how to integrate them
     */
	virtual void sortFaces();
	
	/**
     * @brief   Adds faces to global buffer taking care of redundant vertices
     * 
	 * @param	faces	Faces to be added to global buffer
     */
	virtual void addGlobal(vector<FFace*>& faces);
		
	/**
     * @brief   Form new Triangles via Delauny Triangulation and add them to globale buffer
	 *
	 * @param	vertices	Vertices to triangulate
     */
	virtual void triangulateAndAdd(vector<Point>& vertices, Tree& tree);
	
	/**
     * @brief   assigns new vertices for triangulation to their border region
     * 
     * @param	vertexRegions	Vector of so far formed borderRegions
	 * @param	new_vertices	Vertices to be added to a single border Region
     */
	virtual void assignToBorderRegion(vector<PointSet>& vertexRegions, vector<Point>new_vertices);
	
	/**
     * @brief  Integrate intersection faces
     * 
     * @param	faces	Faces to be integrated
     */
	virtual void intersectIntegrate(vector<FFace*>& faces);
	
	/**
	 * @brief	Finds all intersecting Triangles in Tree
	 * 
	 * @param	face	Face to find intersections with
	 * 
	 * @return	Adresses of Triangles in Global Buffer
	 */
	virtual vector<int> getIntersectingTriangles(FFace* face);
	
	/**
	 * @brief	Finds all intersection points with tree
	 * 
	 * @param	face	Face to find intersections with
	 * 			points		vector to add found intersections points to
	 */
	virtual void getIntersectionPoints(FFace* face, vector<Point>& local_points, vector<Point>& global_points);
};

} // namespace lvr


#include "FusionMesh.cpp"

#endif /* FUSIONMESH_H_ */
