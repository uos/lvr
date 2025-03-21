#pragma once

#include "FaceIO.hpp"
#include "lvr2/io/schema/MeshSchema.hpp"
#include "lvr2/util/Timestamp.hpp"

namespace lvr2
{
namespace meshio
{
template <typename BaseIO>
void FaceIO<BaseIO>::saveFaces(
    const std::string& mesh_name,
    const MeshBufferPtr mesh
) const
{
    bool hasIndices = saveFaceIndices(mesh_name, mesh);

    std::cout << timestamp << "[FaceIO] Has Face Indices  : " << (hasIndices ? "yes" : "no") << std::endl;

    bool hasColors = saveFaceColors(mesh_name, mesh);

    std::cout << timestamp << "[FaceIO] Has Face Colors   : " << (hasColors ? "yes" : "no") << std::endl;

    bool hasNormals = saveFaceNormals(mesh_name, mesh);

    std::cout << timestamp << "[FaceIO] Has Face Normals  : " << (hasNormals ? "yes" : "no") << std::endl;

    bool hasMaterials = saveFaceMaterialIndices(mesh_name, mesh);

    std::cout << timestamp << "[FaceIO] Has Face Materials: " << (hasMaterials ? "yes" : "no") << std::endl;
}

template <typename BaseIO>
void FaceIO<BaseIO>::loadFaces(
    const std::string& mesh_name,
    const MeshBufferPtr mesh
) const
{
    loadFaceIndices(mesh_name, mesh);

    loadFaceColors(mesh_name, mesh);

    loadFaceNormals(mesh_name, mesh);

    loadFaceMaterialIndices(mesh_name, mesh);
}

template <typename BaseIO>
bool FaceIO<BaseIO>::saveFaceIndices(
    const std::string& mesh_name,
    const MeshBufferPtr mesh) const
{
    if (!mesh->hasFaces())
    {
        return false;
    }

    // Save face indices
    std::vector<size_t> shape = {mesh->numFaces(), 3};
    auto desc = m_baseIO->m_description->faceIndices(mesh_name);
    m_baseIO->m_kernel->saveArray(
        *desc.dataRoot,
        *desc.data,
        shape,
        mesh->getFaceIndices()
        );
    {
        // TODO: More meta, type eg.
        YAML::Node meta;
        meta["shape"] = shape;
        m_baseIO->m_kernel->saveMetaYAML(
            *desc.metaRoot,
            *desc.meta,
            meta
        );
    }

    return true;
}

template <typename BaseIO>
bool FaceIO<BaseIO>::saveFaceColors(
    const std::string& mesh_name,
    const MeshBufferPtr mesh) const
{
    if (!mesh->hasFaceColors())
    {
        return false;
    }

    // Save face colors
    size_t color_width;
    const auto& face_colors = mesh->getFaceColors(color_width);
    std::vector<size_t> shape = {mesh->numFaces(), color_width};
    Description desc = m_baseIO->m_description->faceColors(mesh_name);
    m_baseIO->m_kernel->saveArray(
        *desc.dataRoot,
        *desc.data,
        shape,
        face_colors
        );
    {
        // TODO: More meta, type eg.
        YAML::Node meta;
        meta["shape"] = shape;
        m_baseIO->m_kernel->saveMetaYAML(
            *desc.metaRoot,
            *desc.meta,
            meta
        );
    }

    return true;
}

template <typename BaseIO>
bool FaceIO<BaseIO>::saveFaceNormals(
    const std::string& mesh_name,
    const MeshBufferPtr mesh) const
{
    if (!mesh->hasFaceNormals())
    {
        return false;
    }

    // Save face normals
    std::vector<size_t> shape = {mesh->numFaces(), 3};
    Description desc = m_baseIO->m_description->faceNormals(mesh_name);
    m_baseIO->m_kernel->saveArray(
        *desc.dataRoot,
        *desc.data,
        shape,
        mesh->getFaceNormals()
        );
    {
        // TODO: More meta, type eg.
        YAML::Node meta;
        meta["shape"] = shape;
        m_baseIO->m_kernel->saveMetaYAML(
            *desc.metaRoot,
            *desc.meta,
            meta
        );
    }

    return true;
}

template <typename BaseIO>
bool FaceIO<BaseIO>::saveFaceMaterialIndices(
    const std::string& mesh_name,
    const MeshBufferPtr mesh) const
{
    indexArray face_materials = mesh->getFaceMaterialIndices();

    if (!face_materials)
    {
        return false;
    }

    // Save face materials
    std::vector<size_t> shape = {mesh->numFaces(), 1};
    Description desc = m_baseIO->m_description->faceMaterialIndices(mesh_name);
    m_baseIO->m_kernel->saveArray(
        *desc.dataRoot,
        *desc.data,
        shape,
        mesh->getFaceMaterialIndices()
        );
    {
        // TODO: More meta, type eg.
        YAML::Node meta;
        meta["shape"] = shape;
        m_baseIO->m_kernel->saveMetaYAML(
            *desc.metaRoot,
            *desc.meta,
            meta
        );
    }

    return true;
}

template <typename BaseIO>
bool FaceIO<BaseIO>::loadFaceIndices(
    const std::string& mesh_name,
    MeshBufferPtr mesh) const
{
    Description desc = m_baseIO->m_description->faceIndices(mesh_name);
    // Check if the data exists
    if (!m_baseIO->m_kernel->exists(
        *desc.dataRoot,
        *desc.data
    ))
    {
        return false;
    }
    // Load Meta
    YAML::Node meta;
    m_baseIO->m_kernel->loadMetaYAML(
        *desc.metaRoot,
        *desc.meta,
        meta
    );

    // Get dimensions
    std::vector<size_t> shape;
    // Load the data
    indexArray indices = m_baseIO->m_kernel->template loadArray<indexArray::element_type>(
        *desc.dataRoot,
        *desc.data,
        shape        
    );

    mesh->setFaceIndices(indices, shape[0]);

    return true;
}

template <typename BaseIO>
bool FaceIO<BaseIO>::loadFaceColors(
    const std::string& mesh_name,
    MeshBufferPtr mesh) const
{
    Description desc = m_baseIO->m_description->faceColors(mesh_name);
    // Check if the data exists
    if (!m_baseIO->m_kernel->exists(
        *desc.dataRoot,
        *desc.data
    ))
    {
        return false;
    }
    // Load Meta
    YAML::Node meta;
    m_baseIO->m_kernel->loadMetaYAML(
        *desc.metaRoot,
        *desc.meta,
        meta
    );

    // Get dimensions
    std::vector<size_t> shape;
    // Load the data
    ucharArr indices = m_baseIO->m_kernel->template loadArray<ucharArr::element_type>(
        *desc.dataRoot,
        *desc.data,
        shape        
    );

    mesh->setFaceColors(indices, shape[1]);

    return true;
}

template <typename BaseIO>
bool FaceIO<BaseIO>::loadFaceNormals(
    const std::string& mesh_name,
    MeshBufferPtr mesh) const
{
    Description desc = m_baseIO->m_description->faceNormals(mesh_name);
    // Check if the data exists
    if (!m_baseIO->m_kernel->exists(
        *desc.dataRoot,
        *desc.data
    ))
    {
        return false;
    }
    // Load Meta
    YAML::Node meta;
    m_baseIO->m_kernel->loadMetaYAML(
        *desc.metaRoot,
        *desc.meta,
        meta
    );

    // Get dimensions
    std::vector<size_t> shape;
    // Load the data
    floatArr indices = m_baseIO->m_kernel->template loadArray<floatArr::element_type>(
        *desc.dataRoot,
        *desc.data,
        shape        
    );

    mesh->setFaceNormals(indices);

    return true;
}

template <typename BaseIO>
bool FaceIO<BaseIO>::loadFaceMaterialIndices(
    const std::string& mesh_name,
    MeshBufferPtr mesh) const
{
    Description desc = m_baseIO->m_description->faceMaterialIndices(mesh_name);
    // Check if the data exists
    if (!m_baseIO->m_kernel->exists(
        *desc.dataRoot,
        *desc.data
    ))
    {
        return false;
    }
    // Load Meta
    YAML::Node meta;
    m_baseIO->m_kernel->loadMetaYAML(
        *desc.metaRoot,
        *desc.meta,
        meta
    );

    // Get dimensions
    std::vector<size_t> shape;
    // Load the data
    indexArray indices = m_baseIO->m_kernel->template loadArray<indexArray::element_type>(
        *desc.dataRoot,
        *desc.data,
        shape        
    );

    mesh->setFaceMaterialIndices(indices);

    return true;
}

} // namespace meshio
} // namespace lvr2