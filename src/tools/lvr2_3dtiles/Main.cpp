/**
 * Copyright (c) 2022, University Osnabrück
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


#include <iostream>
#include <memory>
#include <tuple>
#include <stdlib.h>

#include <boost/optional.hpp>

#include "lvr2/geometry/BaseVector.hpp"
#include "lvr2/geometry/LazyMesh.hpp"
#include "lvr2/geometry/pmp/SurfaceMeshIO.h"
#include "lvr2/algorithm/pmp/SurfaceNormals.h"
#include "lvr2/io/ModelFactory.hpp"
#include "lvr2/io/meshio/HDF5IO.hpp"
#include "lvr2/util/Timestamp.hpp"
#include "lvr2/util/Progress.hpp"
#include "lvr2/config/lvropenmp.hpp"
#include "B3dmWriter.hpp"
#include "Segmenter.hpp"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <Cesium3DTiles/Tileset.h>
#include <Cesium3DTiles/Tile.h>
#include <Cesium3DTilesWriter/TilesetWriter.h>

using namespace lvr2;
using namespace Cesium3DTiles;

using Mesh = PMPMesh<BaseVector<float>>;

#include "viewer.html"

void paint_mesh(pmp::SurfaceMesh& mesh, const pmp::BoundingBox& bb, size_t index, float chunk_size);

Eigen::Vector3i read_chunks(std::vector<std::pair<pmp::Point, MeshSegment>>& chunks,
                            const std::vector<std::string>& chunk_files,
                            float chunk_size,
                            float voxel_size,
                            bool& assign_colors,
                            std::shared_ptr<HighFive::File> mesh_file,
                            std::shared_ptr<HighFive::Group> chunk_group = nullptr,
                            const std::string& folder = "");


int main(int argc, char** argv)
{
    fs::path input_file;
    std::string input_file_extension;
    fs::path output_dir = "chunk.3dtiles";
    bool calc_normals;
    float chunk_size = -1;
    std::vector<fs::path> mesh_out_files;
    bool fix_mesh;
    bool assign_colors;
    bool big;

    try
    {
        using namespace boost::program_options;

        bool help = false;

        options_description options("General Options");
        options.add_options()
        ("inputFile", value<fs::path>(&input_file),
         "The input. See below for accepted formats.")

        ("outputDir", value<fs::path>(&output_dir)->default_value(output_dir),
         "A Directory for the output.")

        ("big,b", bool_switch(&big),
         "Indicates that the input is big and might not fit into memory.\n"
         "This will make the entire process slower, but a lot less memory intensive.\n"
         "Requires the input to be already chunked.")

        ("calcNormals,N", bool_switch(&calc_normals),
         "Calculate normals")

        ("chunkSize,c", value<float>(&chunk_size)->default_value(chunk_size),
         "When loading a Mesh: Split the Mesh into parts with this size.")

        ("fix,f", bool_switch(&fix_mesh),
         "Fixes some common errors in meshes that might break this algorithm.\n"
         "Only works when loading a single mesh.")

        ("writeMesh,w", value<std::vector<fs::path>>(&mesh_out_files)->multitoken(),
         "Save the mesh after --fix has been applied.")

        ("assignColors,C", bool_switch(&assign_colors),
         "Use existing colors on to vertices. If there are no colors in the input,\n"
         "assigned one color per chunk for visualization purposes.")

        ("help,h", bool_switch(&help),
         "Print this message here.")
        ;

        positional_options_description pos;
        pos.add("inputFile", 1);
        pos.add("outputDir", 1);

        variables_map variables;
        store(command_line_parser(argc, argv).options(options).positional(pos).run(), variables);
        notify(variables);

        if (help)
        {
            std::cout << "The Mesh to 3D Tiles conversion tool" << std::endl;
            std::cout << "Usage: " << std::endl;
            std::cout << "\tlvr2_3dtiles [OPTIONS] <inputFile> [<outputDir>]" << std::endl;
            std::cout << std::endl;
            options.print(std::cout);
            std::cout << std::endl;
            std::cout << "<inputFile> is the file where the input mesh is stored" << std::endl;
            std::cout << "    Possible inputs:" << std::endl;
            std::cout << "        - most Mesh formats (see meshio for a full list)" << std::endl;
            std::cout << "        - a HDF5 file with a single mesh" << std::endl;
            std::cout << "        - a HDF5 file with chunks in /chunks/x_y_z" << std::endl;
            std::cout << "        - a directory containing chunks named x_y_z.*" << std::endl;
            std::cout << "          This option requires a chunk_metadata.yaml in the folder containing" << std::endl;
            std::cout << "          at least chunk_size and voxel_size." << std::endl;
            std::cout << std::endl;
            std::cout << "<outputDir> is the directory to create the output in." << std::endl;
            std::cout << "    THE CONTENT OF THIS DIRECTORY WILL BE DELETED!" << std::endl;
            return EXIT_SUCCESS;
        }

        if (variables.count("inputFile") == 0)
        {
            throw error("Missing <inputFile> Parameter");
        }
        if (!fs::exists(input_file))
        {
            throw error("Input file does not exist");
        }
        if (!fs::is_directory(input_file))
        {
            input_file_extension = input_file.extension().string();
            if (input_file_extension.empty())
            {
                throw error("Input file has no extension");
            }
            input_file_extension = input_file_extension.substr(1);
        }
    }
    catch (const boost::program_options::error& ex)
    {
        std::cerr << ex.what() << std::endl;
        std::cerr << std::endl;
        std::cerr << "Use '--help' to see the list of possible options" << std::endl;
        return EXIT_FAILURE;
    }

    fs::remove_all(output_dir);
    fs::create_directories(output_dir);

    Mesh mesh;
    std::vector<std::pair<pmp::Point, MeshSegment>> chunks;
    std::vector<MeshSegment> segments;
    pmp::BoundingBox bb;
    Eigen::Vector3i num_chunks = Eigen::Vector3i::Zero();
    std::vector<std::string> texture_files;

    std::cout << timestamp << "Reading mesh " << input_file;

    std::shared_ptr<HighFive::File> mesh_file = nullptr;
    if (big)
    {
        mesh_file = hdf5util::open("temp_meshes.h5", HighFive::File::Truncate);
    }

    if (fs::is_directory(input_file))
    {
        std::string path = input_file.string();
        if (path.back() != '/')
        {
            path += '/';
        }
        YAML::Node metadata = YAML::LoadFile(path + "chunk_metadata.yaml");
        chunk_size = metadata["chunk_size"].as<float>();
        float voxel_size = metadata["voxel_size"].as<float>();

        std::vector<std::string> chunk_files;
        for (auto& entry : fs::directory_iterator(input_file))
        {
            chunk_files.push_back(entry.path().string());
        }

        num_chunks = read_chunks(chunks, chunk_files, chunk_size, voxel_size, assign_colors, mesh_file, nullptr, path);

        for (auto& [ _, chunk ] : chunks)
        {
            bb += chunk.bb;
        }
    }
    else if (input_file_extension == "h5")
    {
        const auto kernel = std::make_shared<HDF5Kernel>(input_file.string());
        const auto root = kernel->m_hdf5File->getGroup("/");
        if (root.hasAttribute("chunk_size"))
        {
            std::cout << " from chunks" << std::endl;

            chunk_size = hdf5util::getAttribute<float>(root, "chunk_size").get();
            float voxel_size = hdf5util::getAttribute<float>(root, "voxel_size").get();

            auto chunk_group = std::make_shared<HighFive::Group>(root.getGroup("/chunks"));
            std::vector<std::string> chunk_files = chunk_group->listObjectNames();

            num_chunks = read_chunks(chunks, chunk_files, chunk_size, voxel_size, assign_colors, mesh_file, chunk_group, "");

            for (auto& [ _, chunk ] : chunks)
            {
                bb += chunk.bb;
            }
        }
        else
        {
            std::cout << " using meshio::HDF5IO" << std::endl;

            std::vector<std::string> mesh_names;
            kernel->subGroupNames("/meshes/", mesh_names);
            auto mesh_name = mesh_names.front();

            auto schema = std::make_shared<MeshSchemaHDF5>();
            meshio::HDF5IO io(kernel, schema);
            MeshBufferPtr buffer;

            buffer = io.loadMesh(mesh_name);

            std::cout << timestamp << "Converting to PMPMesh" << std::endl;
            mesh = Mesh(buffer);

            if (!buffer->getMaterials().empty() && !assign_colors)
            {
                auto materials = buffer->getMaterials();
                auto textures = buffer->getTextures();

                texture_files.resize(textures.size());
                for (size_t i = 0; i < textures.size(); ++i)
                {
                    textures[i].save();
                    auto in_filename = "texture_" + std::to_string(textures[i].m_index) + ".ppm";
                    auto out_filename = "texture.png";
                    texture_files[textures[i].m_index] = out_filename;
                    // move texture file to output directory
                    auto out_path = output_dir / "segments" / ("s" + std::to_string(textures[i].m_index)) / out_filename;
                    fs::create_directories(out_path.parent_path());
                    auto command = "convert " + in_filename + " " + out_path.string();
                    int ret = system(command.c_str());
                    if (ret != 0)
                    {
                        throw std::runtime_error("Failed to convert texture");
                    }
                    fs::remove(in_filename);
                    // fs::rename(filename, output_dir / path / filename);
                }

                std::vector<pmp::IndexType> mat_to_texture(materials.size(), pmp::PMP_MAX_INDEX);
                for (size_t i = 0; i < materials.size(); i++)
                {
                    auto& material = materials[i];
                    if (!material.m_texture)
                    {
                        continue;
                    }
                    auto& texture = mat_to_texture[i];
                    texture = material.m_texture->idx();
                    for (auto& [ name, tex ] : material.m_layers)
                    {
                        if (name.find("rgb") != std::string::npos || name.find("RGB") != std::string::npos)
                        {
                            texture = tex.idx();
                            break;
                        }
                    }
                }

                auto& surface_mesh = mesh.getSurfaceMesh();
                auto f_material = surface_mesh.face_property<pmp::IndexType>("f:material");
                #pragma omp parallel for schedule(static)
                for (size_t i = 0; i < surface_mesh.faces_size(); i++)
                {
                    pmp::Face fH(i);
                    if (!surface_mesh.is_deleted(fH))
                    {
                        f_material[fH] = mat_to_texture[f_material[fH]];
                    }
                }
            }

        }
    }
    else if (pmp::SurfaceMeshIO::supported_extensions().count(input_file_extension))
    {
        std::cout << " using pmp::SurfaceMeshIO" << std::endl;
        pmp::SurfaceMeshIO io(input_file.string(), pmp::IOFlags());
        io.read(mesh.getSurfaceMesh());
    }
    else
    {
        std::cout << " using ModelFactory" << std::endl;
        ModelPtr model = ModelFactory::readModel(input_file.string());
        std::cout << timestamp << "Converting to PMPMesh" << std::endl;
        mesh = Mesh(model->m_mesh);
    }

    auto& surface_mesh = mesh.getSurfaceMesh();

    if (mesh.numFaces() > 0)
    {
        if (fix_mesh)
        {
            std::cout << timestamp << "Fixing mesh" << std::endl;
            surface_mesh.duplicate_non_manifold_vertices();
            surface_mesh.remove_degenerate_faces();
        }
        surface_mesh.garbage_collection();

        for (auto file : mesh_out_files)
        {
            std::cout << timestamp << "Writing mesh to " << file << std::endl;
            pmp::IOFlags flags;
            flags.use_binary = true;
            surface_mesh.write(file.string(), flags);
        }

        bb = surface_mesh.bounds();
    }

    if (!chunks.empty())
    {
        // chunks were loaded, no further splitting required
    }
    else if (!texture_files.empty())
    {
        auto face_dist = surface_mesh.get_face_property<pmp::IndexType>("f:material");
        std::vector<pmp::SurfaceMesh> meshes(texture_files.size());
        surface_mesh.split_mesh(meshes, face_dist);

        segments.resize(meshes.size());
        for (size_t i = 0; i < segments.size(); ++i)
        {
            Mesh mesh;
            mesh.getSurfaceMesh() = std::move(meshes[i]);
            segments[i].bb = mesh.getSurfaceMesh().bounds();
            segments[i].mesh.reset(new LazyMesh(mesh, mesh_file));
            segments[i].texture_file.reset(new std::string(texture_files[i]));
        }
    }
    else if (chunk_size > 0)
    {
        std::cout << timestamp << "Segmenting mesh" << std::endl;
        segment_mesh(surface_mesh, bb, chunk_size * 2, chunks, segments, mesh_file);
        // chunk_size * 2 to merge the small segments into larger chunks, otherwise we get a lot of small chunks
    }
    else
    {
        auto& out = segments.emplace_back();
        out.mesh.reset(new LazyMesh(mesh, mesh_file));
        out.bb = bb;
        out.filename = "mesh.b3dm";
    }
    mesh = {};

    if (assign_colors)
    {
        std::cout << timestamp << "Assigning colors" << std::endl;
        for (size_t i = 0; i < chunks.size(); i++)
        {
            auto& [ index, segment ] = chunks[i];
            auto pmp_mesh = segment.mesh->get();
            paint_mesh(pmp_mesh->getSurfaceMesh(), segment.bb, i, chunk_size);
            pmp_mesh->changed();
        }
        for (size_t i = 0; i < segments.size(); i++)
        {
            auto& segment = segments[i];
            auto pmp_mesh = segment.mesh->get();
            paint_mesh(pmp_mesh->getSurfaceMesh(), segment.bb, i + chunks.size(), chunk_size);
            pmp_mesh->changed();
        }
    }

    std::cout << timestamp << "Creating 3D Tiles" << std::endl;

    Cesium3DTiles::Tileset tileset;
    tileset.asset.version = "1.0";
    tileset.geometricError = 1e6; // tileset should always be rendered -> set error very high
    auto& root = tileset.root;
    root.refine = Cesium3DTiles::Tile::Refine::ADD;
    root.transform =
    {
        // 4x4 matrix to place the object somewhere on the globe
        96.86356343768793, 24.848542777253734, 0, 0,
        -15.986465724980844, 62.317780594908875, 76.5566922962899, 0,
        19.02322243409411, -74.15554020821229, 64.3356267137516, 0,
        1215107.7612304366, -4736682.902037748, 4081926.095098698, 1
    };

    convert_bounding_box(bb, root.boundingVolume);
    SegmentTreeNode root_segment;
    root_segment.m_skipped = true;

    if (!chunks.empty())
    {
        std::string path = "chunks/";
        fs::create_directories(output_dir / path);

        std::cout << timestamp << "Partitioning chunks                " << std::endl;
        auto& tile = root.children.emplace_back();
        auto chunk_root = SegmentTree::octree_partition(chunks, num_chunks, 2);
        chunk_root->segment().filename = path + "c";
        root_segment.add_child(std::move(chunk_root));

        chunks.clear();
    }

    if (!segments.empty())
    {
        ProgressBar* progress = nullptr;
        if (chunk_size > 0)
        {
            std::cout << timestamp << "Splitting " << segments.size() << " large segments" << std::endl;
            size_t total_faces = 0;
            for (auto& segment : segments)
            {
                total_faces += segment.mesh->n_faces();
            }
            progress = new ProgressBar(total_faces, "Splitting segments");
        }

        std::vector<SegmentTree::Ptr> segment_trees;

        for (size_t i = 0; i < segments.size(); i++)
        {
            auto& segment = segments[i];
            std::string path = "segments/s" + std::to_string(i) + "/";

            fs::create_directories(output_dir / path);

            auto& tree = segment_trees.emplace_back();
            if (chunk_size > 0)
            {
                tree = split_mesh(segment, chunk_size, mesh_file);
                *progress += segment.mesh->n_faces();
            }
            else
            {
                tree.reset(new SegmentTreeLeaf(segment));
            }
            tree->segment().filename = path + "s";
        }
        if (progress != nullptr)
        {
            std::cout << "\r";
        }

        if (segment_trees.size() > 50)
        {
            auto segment_root = SegmentTree::octree_partition(segment_trees);
            root_segment.add_child(std::move(segment_root));
        }
        else
        {
            for (auto& tree : segment_trees)
            {
                root_segment.add_child(std::move(tree));
            }
        }

        segments.clear();
    }

    std::cout << timestamp << "Constructed tree with depth " << root_segment.m_depth << ". Creating meta-segments" << std::endl;
    root_segment.simplify(mesh_file);

    root_segment.fill_tile(root, "");

    std::vector<MeshSegment> all_segments;
    root_segment.collect_segments(all_segments);
    root_segment = SegmentTreeNode();

    if (calc_normals)
    {
        std::cout << timestamp << "Calculating normals" << std::endl;
        ProgressBar progress_normals(all_segments.size(), "Calculating normals");
        for (size_t i = 0; i < all_segments.size(); i++)
        {
            auto pmp_mesh = all_segments[i].mesh->get();
            pmp_mesh->changed();
            auto& mesh = pmp_mesh->getSurfaceMesh();
            auto v_normal = mesh.vertex_property<pmp::Normal>("v:normal");
            auto v_feature = mesh.get_vertex_property<bool>("v:feature");
            auto v_color = mesh.get_vertex_property<pmp::Color>("v:color");
            bool color_features = false; // v_feature && v_color;
            #pragma omp parallel for schedule(dynamic,64)
            for (size_t i = 0; i < mesh.vertices_size(); i++)
            {
                pmp::Vertex vH(i);
                if (mesh.is_deleted(vH))
                {
                    continue;
                }
                if (color_features && v_feature[vH])
                {
                    v_color[vH] = pmp::Color(1, 0, 0);
                }
                v_normal[vH] = pmp::SurfaceNormals::compute_vertex_normal(mesh, vH);
            }
            ++progress_normals;
        }
    }
    std::cout << "\r";

    std::cout << timestamp << "Writing " << all_segments.size() << " segments        " << std::endl;
    ProgressBar progress_write(all_segments.size(), "Writing segments");
    size_t num_threads = big ? 1 : OpenMPConfig::getNumThreads();
    #pragma omp parallel for num_threads(num_threads)
    for (size_t i = 0; i < all_segments.size(); i++)
    {
        write_b3dm(output_dir, all_segments[i], false);
        ++progress_write;
    }
    std::cout << "\r";

    all_segments.clear();

    if (mesh_file)
    {
        auto name = mesh_file->getName();
        mesh_file.reset();
        fs::remove(name);
    }

    Cesium3DTilesWriter::TilesetWriter writer;
    auto result = writer.writeTileset(tileset);

    if (!result.warnings.empty())
    {
        std::cerr << "Warnings writing tileset: " << std::endl;
        for (auto& e : result.warnings)
        {
            std::cerr << e << std::endl;
        }
    }
    if (!result.errors.empty())
    {
        std::cerr << "Errors writing tileset: " << std::endl;
        for (auto& e : result.errors)
        {
            std::cerr << e << std::endl;
        }
        return EXIT_FAILURE;
    }

    fs::path tileset_file = output_dir / "tileset.json";
    std::cout << timestamp << "Writing " << tileset_file << std::endl;

    std::ofstream tileset_out(tileset_file.string(), std::ios::binary);
    tileset_out.write((char*)result.tilesetBytes.data(), result.tilesetBytes.size());
    tileset_out.close();

    fs::path viewer_file = output_dir / "index.html";
    std::cout << timestamp << "Writing " << viewer_file << std::endl;

    std::ofstream viewer_out(viewer_file.string());
    viewer_out << VIEWER_HTML;
    viewer_out.close();

    std::cout << timestamp << "Finished" << std::endl;

    return 0;
}


Eigen::Vector3i read_chunks(std::vector<std::pair<pmp::Point, MeshSegment>>& chunks,
                            const std::vector<std::string>& chunk_files,
                            float chunk_size,
                            float voxel_size,
                            bool& assign_colors,
                            std::shared_ptr<HighFive::File> mesh_file,
                            std::shared_ptr<HighFive::Group> chunk_group,
                            const std::string& folder)
{
    chunks.resize(chunk_files.size());
    size_t empty = 0;

    ProgressBar progress(chunk_files.size(), "Reading chunks");
    for (size_t i = 0; i < chunk_files.size(); i++)
    {
        auto name = chunk_files[i];
        auto& [ chunk_pos, segment ] = chunks[i];
        int x, y, z;
        int read = std::sscanf(name.c_str(), "%d_%d_%d", &x, &y, &z);
        if (read != 3)
        {
            std::cout << "Skipping " << name << std::endl;
            ++progress;
            continue;
        }
        chunk_pos = pmp::Point(x, y, z);

        Mesh pmp_mesh;
        auto& mesh = pmp_mesh.getSurfaceMesh();
        if (chunk_group)
        {
            mesh.read(chunk_group->getGroup(name));
        }
        else
        {
            mesh.read(folder + name);
        }

        // remove overlap
        pmp::BoundingBox expected_bb;
        expected_bb += chunk_pos * chunk_size;
        expected_bb += (chunk_pos + pmp::Point::Constant(1)) * chunk_size + pmp::Point::Constant(voxel_size * 1.1);
        auto f_delete = mesh.add_face_property<bool>("f:mark_delete");
        auto v_feature = mesh.add_vertex_property<bool>("v:feature", false);
        bool has_features = false;
        #pragma omp parallel for schedule(dynamic,64)
        for (size_t i = 0; i < mesh.n_faces(); i++)
        {
            pmp::Face fH(i);
            for (auto vH : mesh.vertices(fH))
            {
                if (!expected_bb.contains(mesh.position(vH)))
                {
                    #pragma omp critical
                    f_delete[fH] = true;
                    break;
                }
            }
            if (f_delete[fH])
            {
                has_features = true;
                #pragma omp critical
                for (auto vH : mesh.vertices(fH))
                {
                    v_feature[vH] = true;
                }
            }
        }
        mesh.delete_many_faces(f_delete);
        mesh.remove_face_property(f_delete);
        mesh.garbage_collection();
        mesh.add_edge_property<bool>("e:feature", false);

        if (mesh.n_faces() == 0)
        {
            empty++;
            ++progress;
            continue;
        }

        segment.bb = mesh.bounds();

        if (assign_colors)
        {
            paint_mesh(mesh, segment.bb, i, chunk_size);
        }

        segment.mesh.reset(new LazyMesh(pmp_mesh, mesh_file));
        ++progress;
    }
    std::cout << std::endl;

    assign_colors = false; // already assigned

    if (empty > 0)
    {
        std::cout << timestamp << empty << " chunks contained only overlap with other chunks" << std::endl;
    }

    Eigen::Vector3i min_chunk = Eigen::Vector3i::Constant(std::numeric_limits<int>::max());
    Eigen::Vector3i max_chunk = Eigen::Vector3i::Constant(std::numeric_limits<int>::min());
    for (size_t i = 0; i < chunks.size(); i++)
    {
        if (chunks[i].second.mesh == nullptr)
        {
            std::swap(chunks[i], chunks.back());
            chunks.pop_back();
            i--;
        }
        else
        {
            min_chunk = min_chunk.cwiseMin(chunks[i].first.cast<int>());
            max_chunk = max_chunk.cwiseMax(chunks[i].first.cast<int>());
        }
    }
    Eigen::Vector3i num_chunks = max_chunk - min_chunk + Eigen::Vector3i::Ones();

    std::cout << timestamp << "Found " << num_chunks.x() << "x" << num_chunks.y() << "x" << num_chunks.z()
              << " Chunks from " << min_chunk.transpose() << " to " << max_chunk.transpose() << std::endl;

    for (auto& [ chunk_pos, segment ] : chunks)
    {
        chunk_pos -= min_chunk.cast<float>();
    }
    return num_chunks;
}

void paint_mesh(pmp::SurfaceMesh& mesh, const pmp::BoundingBox& bb, size_t index, float chunk_size)
{
    float step_size = chunk_size > 0 ? chunk_size : 10;
    float variation = 0.1;
    if (mesh.has_vertex_property("v:color"))
    {
        return;
    }
    auto v_color = mesh.add_vertex_property<pmp::Color>("v:color");
    float r = std::abs(std::sin(index * 2));
    float g = std::abs(std::cos(index));
    float b = std::abs(std::sin(index * 30));
    float min_z = bb.min().z();
    float max_z = bb.max().z();
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < mesh.vertices_size(); i++)
    {
        pmp::Vertex vH(i);
        if (mesh.is_deleted(vH))
        {
            continue;
        }
        auto pos = mesh.position(vH);
        float dr = std::sin(pos.x() / step_size) * variation;
        float dg = std::sin(pos.y() / step_size) * variation;
        float db = ((pos.z() - min_z) / (max_z - min_z) - 0.5f) * variation;
        v_color[vH] = pmp::Color(std::clamp(r + dr, 0.0f, 1.0f),
                                 std::clamp(g + dg, 0.0f, 1.0f),
                                 std::clamp(b + db, 0.0f, 1.0f));
    }
}
