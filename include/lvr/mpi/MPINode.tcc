/*
 * MPINode.tcc
 *
 *  Created on: 17.01.2013
 *      Author: Dominik Feldschnieders
 */


namespace lvr
{

template<typename VertexT>
MPINode<VertexT>::MPINode(coord3fArr points, VertexT min, VertexT max)
{
    m_minvertex = min;
    m_maxvertex = max;
    node_points = points;
    m_numpoints = 0;
    again = 0;


}

template<typename VertexT>
double MPINode<VertexT>::getnumpoints(){
    return m_numpoints;
}

template<typename VertexT>
void MPINode<VertexT>::setnumpoints(double num){
    m_numpoints = num;
}

template<typename VertexT>
coord3fArr MPINode<VertexT>::getPoints(){
    return node_points;
}


template<typename VertexT>
void MPINode<VertexT>::setIndizes(boost::shared_array<size_t> indi){
    indizes = indi;
}


template<typename VertexT>
boost::shared_array<size_t>  MPINode<VertexT>::getIndizes(){
    return indizes;
}

template<typename VertexT>
MPINode<VertexT>::~MPINode() {
    
    node_points.reset();
    indizes.reset();
    //delete [] &indizes;
    //delete [] &node_points;
    // TODO Auto-generated destructor stub
}

}
