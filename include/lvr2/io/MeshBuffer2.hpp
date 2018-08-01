#ifndef MESHBUFFER2_HPP
#define MESHBUFFER2_HPP

#include <lvr2/io/BaseBuffer.hpp>
#include <lvr2/texture/Material.hpp>
#include <lvr2/texture/Texture.hpp>

////
/// \brief The MeshBuffer2 Mesh representation for I/O modules.
///
class MeshBuffer2 : public BaseBuffer
{
public:

    ///
    /// \brief MeshBuffer2      Contructor. Builds an empty buffer. Fill elements
    ///                         with add-Methods.
    ///
    MeshBuffer2();

    ///
    /// \brief addVertices      Adds the vertex array. Three floats per vertex
    /// \param vertices         The vertex array
    /// \param n                Number of vertices
    ///
    void addVertices(floatArr vertices, size_t n);

    ///
    /// \brief addVertexNormals Adds vertex normals.
    /// \param normals          Normal defintion. Three floats per vertex.
    ///
    void addVertexNormals(floatArr normals);

    ///
    /// \brief addVertexColors  Adds vertex color information.
    /// \param colors           Vertex color array
    /// \param w                Number of bytes per color. (3 for RGB, 4 for RGBA)
    ///
    void addVertexColors(ucharArr colors, unsigned w = 3);

    ///
    /// \brief addTextureCoordinates    Adds texture coordinates for vertices
    /// \param coordinates      Texture coordinate definitions (2 floats per vertex)
    ///
    void addTextureCoordinates(floatArr coordinates);

    ///
    /// \brief addFaceIndices   Adds the face index array that references to the
    ///                         vertex array
    /// \param indices          The index array (3 indices per face)
    /// \param n                Number of faces
    ///
    void addFaceIndices(indexArray indices, size_t n);

    ///
    /// \brief addFaceMaterialIndices   Adds face material indices. The array references
    ///                         to material definitions in \ref m_materials.
    ///
    /// \param indices          One material index per face
    ///
    void addFaceMaterialIndices(indexArray indices);

    ///
    /// \brief addFaceNormals   Adds face normal information. The number of normals
    ///                         in the array are exspected to match the number of
    ///                         faces in the mesh
    /// \param                  Normal definitions for all faces
    ///
    void addFaceNormals(floatArr normals);

    ///
    /// \brief addFaceColors    Adds face colors the the buffer
    /// \param colors           An array containing color information
    /// \param w                Bytes per color attribute (3 for RGB, 4 for RGBA)
    ///
    void addFaceColors(ucharArr colors, unsigned w = 3);

    ///
    /// \brief numVertices      Number of vertices in the mesh
    ///
    size_t numVertices();

    ///
    /// \brief numFaces         Number of faces in the mesh
    ///
    size_t numFaces();


    ///
    /// \brief getVertices      Return the vertex array.
    ///
    floatArr getVertices();

    ///
    /// \brief getVertexColors  Returns vertex color information or an empty array if
    ///                         vertex colors are not available
    /// \param width            Number of bytes per color (3 for RGB, 4 for RGBA)
    /// \return
    ///
    ucharArr getVertexColors(unsigned& width);

    ///
    /// \brief getVertexNormals Returns an array with vertex normals or an empty array
    ///                         if no normals are present.
    ///
    floatArr getVertexNormals();

    ///
    /// \brief getTextureCoordinates Returns an array with texture coordinates. Two
    ///                         normalized floats per vertex. Returns an empty array
    ///                         if no texture coordinates were loaded.
    ///
    floatArr getTextureCoordinates();

    ///
    /// \brief getFaceIndices   Returns an array with face definitions, i.e., three
    ///                         vertex indices per face.
    indexArray getFaceIndices();

    ///
    /// \brief getFaceColors    Returns an array with wrgb colors
    /// \param width            Number of bytes per color (3 for RGB and 4 for RGBA)
    /// \return                 An array containing point data or an nullptr if
    ///                         no colors are present.
    ///
    ucharArr getFaceColors(unsigned& width);

private:

    /// Channel with attributes for vertices
    AttributeChannel    m_vertexAttributes;

    /// Channel with attributes for faces
    AttributeChannel    m_faceAttributes;

    /// Vector containing all material definitions
    vector<Material>    m_materials;

    /// Vector containing all textures
    vector<Texture>     m_textures;

    /// Number of faces in the mesh
    size_t              m_numFaces;

    /// Number of vertices in the mesh
    size_t              m_numVertices;


};

#endif // MESHBUFFER2_HPP
