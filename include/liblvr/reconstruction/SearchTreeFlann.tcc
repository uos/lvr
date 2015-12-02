/*
 * SearchTreeFlann.tcc
 *
 *  Created on: Sep 22, 2015
 *      Author: twiemann
 */

#include "SearchTreeFlann.hpp"

namespace lvr
{

template<typename VertexT>
SearchTreeFlann< VertexT >::SearchTreeFlann( PointBufferPtr buffer, size_t &n_points, const int &kn, const int &ki, const int &kd )
{
	flann::Matrix<float> points(new float[3 * n_points], n_points, 3);
	m_points = buffer->getPointArray(m_numPoints);
	for(size_t i = 0; i < n_points; i++)
	{
		points[i][0] = m_points[3 * i];
		points[i][1] = m_points[3 * i + 1];
		points[i][2] = m_points[3 * i + 2];
	}


	m_tree = boost::shared_ptr<flann::Index<flann::L2_Simple<float> > >(new flann::Index<flann::L2_Simple<float>>(points, ::flann::KDTreeSingleIndexParams (10, false)));
	m_tree->buildIndex();

}


template<typename VertexT>
SearchTreeFlann< VertexT >::~SearchTreeFlann() {
}


template<typename VertexT>
void SearchTreeFlann< VertexT >::kSearch( coord< float > &qp, int k, vector< ulong > &indices, vector< float > &distances )
{
	flann::Matrix<float> query_point(new float[3], 1, 3);
	query_point[0][0] = qp.x;
	query_point[0][1] = qp.y;
	query_point[0][2] = qp.z;

	indices.resize(k);
	distances.resize(k);

	flann::Matrix<ulong> ind (&indices[0], 1, k);
	flann::Matrix<float> dist (&distances[0], 1, k);


	m_tree->knnSearch(query_point, ind, dist, k, flann::SearchParams());
}

template<typename VertexT>
void SearchTreeFlann< VertexT >::kSearch(VertexT qp, int k, vector< VertexT > &nb)
{
	/*flann::Matrix<float> query_point(new float[3], 1, 3);
	query_point[0][0] = qp[0];
	query_point[0][1] = qp[1];
	query_point[0][2] = qp[2];

	cout << "OP: " << qp[0] << " " << qp[1] << " " << qp[2] << endl;
	cout << "QP: " << query_point[0][0] << " " << query_point[0][1] << " " << query_point[0][2] << endl;*/

	flann::Matrix<float> query_point(new float[3], 1, 3);
	query_point[0][0] = qp.x;
	query_point[0][1] = qp.y;
	query_point[0][2] = qp.z;

	vector<size_t> indices(k);
	vector<float> distances(k);

	flann::Matrix<ulong> ind (&indices[0], 1, k);
	flann::Matrix<float> dist (&distances[0], 1, k);

	m_tree->knnSearch(query_point, ind, dist, k, flann::SearchParams(-1, 0.0f));

	for(size_t i = 0; i < k; i++)
	{
		size_t index = indices[k];

			cout << index << " " << dist[0][k] << endl;
			VertexT v(m_points[3 * index], m_points[3 * index + 1], m_points[3 * index + 2]);
			cout << index << v;
			nb.push_back(v);

	}
	cout << endl;
}

/*
   Begin of radiusSearch implementations
 */
template<typename VertexT>
void SearchTreeFlann< VertexT >::radiusSearch( float qp[3], float r, vector< ulong > &indices )
{
    // TODO: Implement me!
}


template<typename VertexT>
void SearchTreeFlann< VertexT >::radiusSearch( VertexT& qp, float r, vector< ulong > &indices )
{
    float qp_arr[3];
    qp_arr[0] = qp[0];
    qp_arr[1] = qp[1];
    qp_arr[2] = qp[2];
    this->radiusSearch( qp_arr, r, indices );
}


template<typename VertexT>
void SearchTreeFlann< VertexT >::radiusSearch( const VertexT& qp, float r, vector< ulong > &indices )
{
    float qp_arr[3];
    qp_arr[0] = qp[0];
    qp_arr[1] = qp[1];
    qp_arr[2] = qp[2];
    this->radiusSearch( qp_arr, r, indices );
}


template<typename VertexT>
void SearchTreeFlann< VertexT >::radiusSearch( coord< float >& qp, float r, vector< ulong > &indices )
{
    float qp_arr[3];
    qp_arr[0] = qp[0];
    qp_arr[1] = qp[1];
    qp_arr[2] = qp[2];
    this->radiusSearch( qp_arr, r, indices );
}


template<typename VertexT>
void SearchTreeFlann< VertexT >::radiusSearch( const coord< float >& qp, float r, vector< ulong > &indices )
{
    float qp_arr[3];
    coord< float > qpcpy = qp;
    qp_arr[0] = qpcpy[0];
    qp_arr[1] = qpcpy[1];
    qp_arr[2] = qpcpy[2];
    this->radiusSearch( qp_arr, r, indices );
}

} /* namespace lvr */
