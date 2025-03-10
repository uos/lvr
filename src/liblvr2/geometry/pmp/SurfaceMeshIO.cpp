// Copyright 2011-2021 the Polygon Mesh Processing Library developers.
// Copyright 2001-2005 by Computer Graphics Group, RWTH Aachen
// Distributed under a MIT-style license, see PMP_LICENSE.txt for details.

#include "lvr2/geometry/pmp/SurfaceMeshIO.h"

#include <clocale>
#include <cstring>
#include <cctype>

#include <fstream>
#include <limits>

#include <rply.h>

#include "lvr2/util/Hdf5Util.hpp"

// helper function
template <typename T>
void tfread(FILE* in, const T& t)
{
    size_t n_items = fread((char*)&t, 1, sizeof(t), in);
    PMP_ASSERT(n_items > 0);
}

// helper function
template <typename T>
void tfwrite(FILE* out, const T& t)
{
    size_t n_items = fwrite((char*)&t, 1, sizeof(t), out);
    PMP_ASSERT(n_items > 0);
}

namespace pmp {

std::unordered_set<std::string>& SurfaceMeshIO::supported_extensions()
{
    static std::unordered_set<std::string> extensions = {
        "off", "obj", "stl", "ply", "pmp", "xyz", "agi"
    };
    return extensions;
}


void SurfaceMeshIO::read(SurfaceMesh& mesh)
{
    std::setlocale(LC_NUMERIC, "C");

    // clear mesh before reading from file
    mesh.clear();

    // extract file extension
    std::string::size_type dot(filename_.rfind("."));
    if (dot == std::string::npos)
        throw IOException("Could not determine file extension!");
    std::string ext = filename_.substr(dot + 1, filename_.length() - dot - 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](char c) { return std::tolower(c); });

    // extension determines reader
    if (ext == "off")
        read_off(mesh);
    else if (ext == "obj")
        read_obj(mesh);
    else if (ext == "stl")
        read_stl(mesh);
    else if (ext == "ply")
        read_ply(mesh);
    else if (ext == "pmp")
        read_pmp(mesh);
    else if (ext == "xyz")
        read_xyz(mesh);
    else if (ext == "agi")
        read_agi(mesh);
    else
        throw IOException("Could not find reader for " + filename_);

    add_failed_faces(mesh);
}

void SurfaceMeshIO::write_const(const SurfaceMesh& mesh)
{
    if (mesh.has_garbage())
        throw IOException("Cannot write mesh with garbage!");

    // extract file extension
    std::string::size_type dot(filename_.rfind("."));
    if (dot == std::string::npos)
        throw IOException("Could not determine file extension!");
    std::string ext = filename_.substr(dot + 1, filename_.length() - dot - 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](char c) { return std::tolower(c); });

    // extension determines reader
    if (ext == "off")
        write_off(mesh);
    else if (ext == "obj")
        write_obj(mesh);
    else if (ext == "stl")
        write_stl(mesh);
    else if (ext == "ply")
        write_ply(mesh);
    else if (ext == "pmp")
        write_pmp(mesh);
    else if (ext == "xyz")
        write_xyz(mesh);
    else
        throw IOException("Could not find writer for " + filename_);
}

void SurfaceMeshIO::read_obj(SurfaceMesh& mesh)
{
    char s[200];
    float x, y, z;
    std::vector<Vertex> vertices;
    std::vector<TexCoord> all_tex_coords; //individual texture coordinates
    std::vector<int>
        halfedge_tex_idx; //texture coordinates sorted for halfedges
    auto tex_coords = mesh.halfedge_property<TexCoord>("h:tex");
    bool with_tex_coord = false;

    // open file (in ASCII mode)
    FILE* in = fopen(filename_.c_str(), "r");
    if (!in)
        throw IOException("Failed to open file: " + filename_);

    // clear line once
    memset(&s, 0, 200);

    // parse line by line (currently only supports vertex positions & faces
    while (in && !feof(in) && fgets(s, 200, in))
    {
        // comment
        if (s[0] == '#' || isspace(s[0]))
            continue;

        // vertex
        else if (strncmp(s, "v ", 2) == 0)
        {
            if (sscanf(s, "v %f %f %f", &x, &y, &z))
            {
                mesh.add_vertex(Point(x, y, z));
            }
        }

        // normal
        else if (strncmp(s, "vn ", 3) == 0)
        {
            if (sscanf(s, "vn %f %f %f", &x, &y, &z))
            {
                // problematic as it can be either a vertex property when interpolated
                // or a halfedge property for hard edges
            }
        }

        // texture coordinate
        else if (strncmp(s, "vt ", 3) == 0)
        {
            if (sscanf(s, "vt %f %f", &x, &y))
            {
                all_tex_coords.emplace_back(x, y);
            }
        }

        // face
        else if (strncmp(s, "f ", 2) == 0)
        {
            int component(0), nv(0);
            bool end_of_vertex(false);
            char *p0, *p1(s + 1);

            vertices.clear();
            halfedge_tex_idx.clear();

            // skip white-spaces
            while (*p1 == ' ')
                ++p1;

            while (p1)
            {
                p0 = p1;

                // overwrite next separator

                // skip '/', '\n', ' ', '\0', '\r' <-- don't forget Windows
                while (*p1 != '/' && *p1 != '\r' && *p1 != '\n' && *p1 != ' ' &&
                       *p1 != '\0')
                    ++p1;

                // detect end of vertex
                if (*p1 != '/')
                {
                    end_of_vertex = true;
                }

                // replace separator by '\0'
                if (*p1 != '\0')
                {
                    *p1 = '\0';
                    p1++; // point to next token
                }

                // detect end of line and break
                if (*p1 == '\0' || *p1 == '\n')
                {
                    p1 = nullptr;
                }

                // read next vertex component
                if (*p0 != '\0')
                {
                    switch (component)
                    {
                        case 0: // vertex
                        {
                            int idx = atoi(p0);
                            if (idx < 0)
                                idx = mesh.n_vertices() + idx + 1;
                            vertices.emplace_back(idx - 1);
                            break;
                        }
                        case 1: // texture coord
                        {
                            int idx = atoi(p0) - 1;
                            halfedge_tex_idx.push_back(idx);
                            with_tex_coord = true;
                            break;
                        }
                        case 2: // normal
                            break;
                    }
                }

                ++component;

                if (end_of_vertex)
                {
                    component = 0;
                    nv++;
                    end_of_vertex = false;
                }
            }

            Face f = add_face(mesh, vertices);

            // add texture coordinates
            if (with_tex_coord && f.is_valid())
            {
                SurfaceMesh::HalfedgeAroundFaceCirculator h_fit =
                    mesh.halfedges(f);
                SurfaceMesh::HalfedgeAroundFaceCirculator h_end = h_fit;
                unsigned v_idx = 0;
                do
                {
                    tex_coords[*h_fit] =
                        all_tex_coords.at(halfedge_tex_idx.at(v_idx));
                    ++v_idx;
                    ++h_fit;
                } while (h_fit != h_end);
            }
        }
        // clear line
        memset(&s, 0, 200);
    }

    // if there are no textures, delete texture property!
    if (!with_tex_coord)
    {
        mesh.remove_halfedge_property(tex_coords);
    }

    fclose(in);
}

void SurfaceMeshIO::write_obj(const SurfaceMesh& mesh)
{
    FILE* out = fopen(filename_.c_str(), "w");
    if (!out)
        throw IOException("Failed to open file: " + filename_);

    // comment
    fprintf(out, "# OBJ export from PMP\n");

    // write vertices
    auto points = mesh.get_vertex_property<Point>("v:point");
    for (auto v : mesh.vertices())
    {
        const Point& p = points[v];
        fprintf(out, "v %.10f %.10f %.10f\n", p[0], p[1], p[2]);
    }

    // write normals
    auto normals = mesh.get_vertex_property<Normal>("v:normal");
    if (normals)
    {
        for (auto v : mesh.vertices())
        {
            const Normal& n = normals[v];
            fprintf(out, "vn %.10f %.10f %.10f\n", n[0], n[1], n[2]);
        }
    }

    // write texture coordinates
    auto tex_coords = mesh.get_halfedge_property<TexCoord>("h:tex");
    if (tex_coords)
    {
        for (auto h : mesh.halfedges())
        {
            const TexCoord& pt = tex_coords[h];
            fprintf(out, "vt %.10f %.10f\n", pt[0], pt[1]);
        }
    }

    // write faces
    for (auto f : mesh.faces())
    {
        fprintf(out, "f");

        auto h = mesh.halfedges(f);
        for (auto v : mesh.vertices(f))
        {
            auto idx = v.idx() + 1;
            if (tex_coords)
            {
                // write vertex index, texCoord index and normal index
                fprintf(out, " %d/%d/%d", idx, (*h).idx() + 1, idx);
                ++h;
            }
            else
            {
                // write vertex index and normal index
                fprintf(out, " %d//%d", idx, idx);
            }
        }
        fprintf(out, "\n");
    }

    fclose(out);
}

void SurfaceMeshIO::read_off_ascii(SurfaceMesh& mesh, FILE* in,
                                   const bool has_normals,
                                   const bool has_texcoords,
                                   const bool has_colors)
{
    char line[1000], *lp;
    int nc;
    unsigned int i, j, items, idx;
    unsigned int nv, nf, ne;
    float x, y, z, r, g, b;
    Vertex v;

    // properties
    VertexProperty<Normal> normals;
    VertexProperty<TexCoord> texcoords;
    VertexProperty<Color> colors;
    if (has_normals)
        normals = mesh.vertex_property<Normal>("v:normal");
    if (has_texcoords)
        texcoords = mesh.vertex_property<TexCoord>("v:tex");
    if (has_colors)
        colors = mesh.vertex_property<Color>("v:color");

    // #Vertice, #Faces, #Edges
    items = fscanf(in, "%d %d %d\n", (int*)&nv, (int*)&nf, (int*)&ne);
    PMP_ASSERT(items);

    mesh.reserve(nv, std::max(3 * nv, ne), nf);

    // read vertices: pos [normal] [color] [texcoord]
    for (i = 0; i < nv && !feof(in); ++i)
    {
        // read line
        lp = fgets(line, 1000, in);
        lp = line;

        // position
        items = sscanf(lp, "%f %f %f%n", &x, &y, &z, &nc);
        assert(items == 3);
        v = mesh.add_vertex(Point(x, y, z));
        lp += nc;

        // normal
        if (has_normals)
        {
            if (sscanf(lp, "%f %f %f%n", &x, &y, &z, &nc) == 3)
            {
                normals[v] = Normal(x, y, z);
            }
            lp += nc;
        }

        // color
        if (has_colors)
        {
            if (sscanf(lp, "%f %f %f%n", &r, &g, &b, &nc) == 3)
            {
                if (r > 1.0f || g > 1.0f || b > 1.0f)
                {
                    r /= 255.0f;
                    g /= 255.0f;
                    b /= 255.0f;
                }
                colors[v] = Color(r, g, b);
            }
            lp += nc;
        }

        // tex coord
        if (has_texcoords)
        {
            items = sscanf(lp, "%f %f%n", &x, &y, &nc);
            assert(items == 2);
            texcoords[v][0] = x;
            texcoords[v][1] = y;
            lp += nc;
        }
    }

    // read faces: #N v[1] v[2] ... v[n-1]
    std::vector<Vertex> vertices;
    for (i = 0; i < nf; ++i)
    {
        // read line
        lp = fgets(line, 1000, in);
        lp = line;

        // #vertices
        items = sscanf(lp, "%d%n", (int*)&nv, &nc);
        assert(items == 1);
        vertices.resize(nv);
        lp += nc;

        // indices
        for (j = 0; j < nv; ++j)
        {
            items = sscanf(lp, "%d%n", (int*)&idx, &nc);
            assert(items == 1);
            vertices[j] = Vertex(idx);
            lp += nc;
        }
        add_face(mesh, vertices);
    }
}

void SurfaceMeshIO::read_off_binary(SurfaceMesh& mesh, FILE* in,
                                    const bool has_normals,
                                    const bool has_texcoords,
                                    const bool has_colors)
{
    IndexType i, j, idx(0);
    IndexType nv(0), nf(0), ne(0);
    Point p, n, c;
    vec2 t;
    Vertex v;

    // binary cannot (yet) read colors
    if (has_colors)
        throw IOException("Colors not supported for binary OFF file.");

    // properties
    VertexProperty<Normal> normals;
    VertexProperty<TexCoord> texcoords;
    if (has_normals)
        normals = mesh.vertex_property<Normal>("v:normal");
    if (has_texcoords)
        texcoords = mesh.vertex_property<TexCoord>("v:tex");

    // #Vertice, #Faces, #Edges
    tfread(in, nv);
    tfread(in, nf);
    tfread(in, ne);
    mesh.reserve(nv, std::max(3 * nv, ne), nf);

    // read vertices: pos [normal] [color] [texcoord]
    for (i = 0; i < nv && !feof(in); ++i)
    {
        // position
        tfread(in, p);
        v = mesh.add_vertex((Point)p);

        // normal
        if (has_normals)
        {
            tfread(in, n);
            normals[v] = (Normal)n;
        }

        // tex coord
        if (has_texcoords)
        {
            tfread(in, t);
            texcoords[v][0] = t[0];
            texcoords[v][1] = t[1];
        }
    }

    // read faces: #N v[1] v[2] ... v[n-1]
    std::vector<Vertex> vertices;
    for (i = 0; i < nf; ++i)
    {
        tfread(in, nv);
        vertices.resize(nv);
        for (j = 0; j < nv; ++j)
        {
            tfread(in, idx);
            vertices[j] = Vertex(idx);
        }
        add_face(mesh, vertices);
    }
}

void SurfaceMeshIO::write_off_binary(const SurfaceMesh& mesh)
{
    FILE* out = fopen(filename_.c_str(), "w");
    if (!out)
        throw IOException("Failed to open file: " + filename_);

    fprintf(out, "OFF BINARY\n");
    fclose(out);
    IndexType nv = (IndexType)mesh.n_vertices();
    IndexType nf = (IndexType)mesh.n_faces();
    IndexType ne = 0;

    out = fopen(filename_.c_str(), "ab");
    tfwrite(out, nv);
    tfwrite(out, nf);
    tfwrite(out, ne);
    auto points = mesh.get_vertex_property<Point>("v:point");
    for (auto v : mesh.vertices())
    {
        const Point& p = points[v];
        tfwrite(out, p);
    }

    for (auto f : mesh.faces())
    {
        IndexType nv = mesh.valence(f);
        tfwrite(out, nv);
        for (auto fv : mesh.vertices(f))
            tfwrite(out, (IndexType)fv.idx());
    }
    fclose(out);
}

void SurfaceMeshIO::read_off(SurfaceMesh& mesh)
{
    char line[200];
    bool has_texcoords = false;
    bool has_normals = false;
    bool has_colors = false;
    bool has_hcoords = false;
    bool has_dim = false;
    bool is_binary = false;

    // open file (in ASCII mode)
    FILE* in = fopen(filename_.c_str(), "r");
    if (!in)
        throw IOException("Failed to open file: " + filename_);

    // read header: [ST][C][N][4][n]OFF BINARY
    char* c = fgets(line, 200, in);
    assert(c != nullptr);
    c = line;
    if (c[0] == 'S' && c[1] == 'T')
    {
        has_texcoords = true;
        c += 2;
    }
    if (c[0] == 'C')
    {
        has_colors = true;
        ++c;
    }
    if (c[0] == 'N')
    {
        has_normals = true;
        ++c;
    }
    if (c[0] == '4')
    {
        has_hcoords = true;
        ++c;
    }
    if (c[0] == 'n')
    {
        has_dim = true;
        ++c;
    }
    if (strncmp(c, "OFF", 3) != 0)
    {
        fclose(in);
        throw IOException("Failed to parse OFF header");
    }
    if (strncmp(c + 4, "BINARY", 6) == 0)
        is_binary = true;

    if (has_hcoords)
    {
        fclose(in);
        throw IOException("Error: Homogeneous coordinates not supported.");
    }
    if (has_dim)
    {
        fclose(in);
        throw IOException("Error: vertex dimension != 3 not supported");
    }

    // if binary: reopen file in binary mode
    if (is_binary)
    {
        fclose(in);
        in = fopen(filename_.c_str(), "rb");
        c = fgets(line, 200, in);
        assert(c != nullptr);
    }

    // read as ASCII or binary
    if (is_binary)
        read_off_binary(mesh, in, has_normals, has_texcoords, has_colors);
    else
        read_off_ascii(mesh, in, has_normals, has_texcoords, has_colors);

    fclose(in);
}

void SurfaceMeshIO::write_off(const SurfaceMesh& mesh)
{
    if (flags_.use_binary)
    {
        write_off_binary(mesh);
        return;
    }

    FILE* out = fopen(filename_.c_str(), "w");
    if (!out)
        throw IOException("Failed to open file: " + filename_);

    bool has_normals = false;
    bool has_texcoords = false;
    bool has_colors = false;

    auto normals = mesh.get_vertex_property<Normal>("v:normal");
    auto texcoords = mesh.get_vertex_property<TexCoord>("v:tex");
    auto colors = mesh.get_vertex_property<Color>("v:color");

    if (normals && flags_.use_vertex_normals)
        has_normals = true;
    if (texcoords && flags_.use_vertex_texcoords)
        has_texcoords = true;
    if (colors && flags_.use_vertex_colors)
        has_colors = true;

    // header
    if (has_texcoords)
        fprintf(out, "ST");
    if (has_colors)
        fprintf(out, "C");
    if (has_normals)
        fprintf(out, "N");
    fprintf(out, "OFF\n%zu %zu 0\n", mesh.n_vertices(), mesh.n_faces());

    // vertices, and optionally normals and texture coordinates
    auto points = mesh.get_vertex_property<Point>("v:point");
    for (SurfaceMesh::VertexIterator vit = mesh.vertices_begin();
         vit != mesh.vertices_end(); ++vit)
    {
        const Point& p = points[*vit];
        fprintf(out, "%.10f %.10f %.10f", p[0], p[1], p[2]);

        if (has_normals)
        {
            const Normal& n = normals[*vit];
            fprintf(out, " %.10f %.10f %.10f", n[0], n[1], n[2]);
        }

        if (has_colors)
        {
            const Color& c = colors[*vit];
            fprintf(out, " %.10f %.10f %.10f", c[0], c[1], c[2]);
        }

        if (has_texcoords)
        {
            const TexCoord& t = texcoords[*vit];
            fprintf(out, " %.10f %.10f", t[0], t[1]);
        }

        fprintf(out, "\n");
    }

    // faces
    for (SurfaceMesh::FaceIterator fit = mesh.faces_begin();
         fit != mesh.faces_end(); ++fit)
    {
        int nv = mesh.valence(*fit);
        fprintf(out, "%d", nv);
        SurfaceMesh::VertexAroundFaceCirculator fvit = mesh.vertices(*fit),
                                                fvend = fvit;
        do
        {
            fprintf(out, " %d", (*fvit).idx());
        } while (++fvit != fvend);
        fprintf(out, "\n");
    }

    fclose(out);
}

template<int N>
void read_pmp_property(FILE* in, SurfaceMesh& mesh, const std::string& name)
{
    auto prefix = name.substr(0, 2);
    int n;
    char* data = nullptr;
    if (prefix == "v:")
    {
        data = (char*)mesh.add_vertex_property<Eigen::Matrix<Scalar, N, 1>>(name).data();
        n = mesh.n_vertices();
    }
    else if (prefix == "f:")
    {
        data = (char*)mesh.add_face_property<Eigen::Matrix<Scalar, N, 1>>(name).data();
        n = mesh.n_faces();
    }
    else if (prefix == "e:")
    {
        data = (char*)mesh.add_edge_property<Eigen::Matrix<Scalar, N, 1>>(name).data();
        n = mesh.n_edges();
    }
    else if (prefix == "h:")
    {
        data = (char*)mesh.add_halfedge_property<Eigen::Matrix<Scalar, N, 1>>(name).data();
        n = mesh.n_halfedges();
    }
    else
    {
        throw IOException("Error: unknown property type: " + name);
    }
    size_t nread = fread(data, sizeof(Scalar) * N, n, in);
    PMP_ASSERT(nread == n);
}

void SurfaceMeshIO::read_pmp(SurfaceMesh& mesh)
{
    // open file (in binary mode)
    FILE* in = fopen(filename_.c_str(), "rb");
    if (!in)
        throw IOException("Failed to open file: " + filename_);

    // how many elements?
    uint32_t nv(0), ne(0), nh(0), nf(0);
    tfread(in, nv);
    tfread(in, ne);
    tfread(in, nf);
    nh = 2 * ne;

    // this used to be a bool called htex, indicating if the h:tex property exists.
    // => version values of 0 or 1 are false or true, 2 and above are actual version numbers
    uint8_t version;
    tfread(in, version);

    uint32_t nprops = 0;
    std::vector<std::pair<std::string, uint32_t>> props;
    if (version == 0)
    {
        // bool htex was false, no properties
    }
    else if (version == 1)
    {
        // bool htex is true, add h:tex property
        props.emplace_back("h:tex", 2);
    }
    else if (version == 2)
    {
        tfread(in, nprops);
        for (uint32_t i = 0; i < nprops; ++i)
        {
            uint32_t len;
            tfread(in, len);
            auto& prop = props.emplace_back();
            tfread(in, prop.second);
            prop.first.resize(len);
            size_t nread = fread((char*)prop.first.data(), sizeof(char), len, in);
            PMP_ASSERT(nread == len);
        }
    }

    // resize containers
    mesh.vprops_.resize(nv);
    mesh.hprops_.resize(nh);
    mesh.eprops_.resize(ne);
    mesh.fprops_.resize(nf);

    // get properties
    auto vconn = mesh.vertex_property<SurfaceMesh::VertexConnectivity>("v:connectivity");
    auto hconn = mesh.halfedge_property<SurfaceMesh::HalfedgeConnectivity>("h:connectivity");
    auto fconn = mesh.face_property<SurfaceMesh::FaceConnectivity>("f:connectivity");
    auto point = mesh.vertex_property<Point>("v:point");

    // read properties from file
    size_t nvc = fread((char*)vconn.data(),
                       sizeof(SurfaceMesh::VertexConnectivity), nv, in);
    size_t nhc = fread((char*)hconn.data(),
                       sizeof(SurfaceMesh::HalfedgeConnectivity), nh, in);
    size_t nfc = fread((char*)fconn.data(),
                       sizeof(SurfaceMesh::FaceConnectivity), nf, in);
    size_t np = fread((char*)point.data(), sizeof(Point), nv, in);
    PMP_ASSERT(nvc == nv);
    PMP_ASSERT(nhc == nh);
    PMP_ASSERT(nfc == nf);
    PMP_ASSERT(np == nv);

    // read other properties
    for (auto [ name, num_elements ] : props)
    {
        switch (num_elements)
        {
        case 1: read_pmp_property<1>(in, mesh, name); break;
        case 2: read_pmp_property<2>(in, mesh, name); break;
        case 3: read_pmp_property<3>(in, mesh, name); break;
        case 4: read_pmp_property<4>(in, mesh, name); break;
        case 5: read_pmp_property<5>(in, mesh, name); break;
        case 6: read_pmp_property<6>(in, mesh, name); break;
        case 7: read_pmp_property<7>(in, mesh, name); break;
        case 8: read_pmp_property<8>(in, mesh, name); break;
        case 9: read_pmp_property<9>(in, mesh, name); break;
        }
    }

    if (version == 1)
    {
        auto htex = mesh.halfedge_property<TexCoord>("h:tex");
        size_t nhtc = fread((char*)htex.data(), sizeof(TexCoord), nh, in);
        PMP_ASSERT(nhtc == nh);
    }

    fclose(in);
}

template<typename T>
void read_vprop(const HighFive::Group& group, SurfaceMesh& mesh, const std::string& name)
{
    if (group.exist(name)) group.getDataSet(name).read((char*)mesh.vertex_property<T>(name).data());
}
template<typename T>
void read_eprop(const HighFive::Group& group, SurfaceMesh& mesh, const std::string& name)
{
    if (group.exist(name)) group.getDataSet(name).read((char*)mesh.edge_property<T>(name).data());
}
template<typename T>
void read_hprop(const HighFive::Group& group, SurfaceMesh& mesh, const std::string& name)
{
    if (group.exist(name)) group.getDataSet(name).read((char*)mesh.halfedge_property<T>(name).data());
}
template<typename T>
void read_fprop(const HighFive::Group& group, SurfaceMesh& mesh, const std::string& name)
{
    if (group.exist(name)) group.getDataSet(name).read((char*)mesh.face_property<T>(name).data());
}

void SurfaceMeshIO::read_hdf5(const HighFive::Group& group, SurfaceMesh& mesh)
{
    auto vn = lvr2::hdf5util::getAttribute<uint64_t>(group, "n_vertices").get();
    auto en = lvr2::hdf5util::getAttribute<uint64_t>(group, "n_edges").get();
    auto fn = lvr2::hdf5util::getAttribute<uint64_t>(group, "n_faces").get();

    if (fn == 0)
        return;

    mesh.vprops_.resize(vn);
    mesh.eprops_.resize(en);
    mesh.hprops_.resize(en * 2);
    mesh.fprops_.resize(fn);

    read_vprop<SurfaceMesh::VertexConnectivity>(group, mesh, "v:connectivity");
    read_hprop<SurfaceMesh::HalfedgeConnectivity>(group, mesh, "h:connectivity");
    read_fprop<SurfaceMesh::FaceConnectivity>(group, mesh, "f:connectivity");

    read_vprop<Point>(group, mesh, "v:point");

    read_vprop<Normal>(group, mesh, "v:normal");
    read_fprop<Normal>(group, mesh, "f:normal");

    read_vprop<Color>(group, mesh, "v:color");
    read_fprop<Color>(group, mesh, "f:color");

    read_vprop<TexCoord>(group, mesh, "v:tex");
    read_hprop<TexCoord>(group, mesh, "h:tex");
}

void SurfaceMeshIO::read_xyz(SurfaceMesh& mesh)
{
    // open file (in ASCII mode)
    FILE* in = fopen(filename_.c_str(), "r");
    if (!in)
        throw IOException("Failed to open file: " + filename_);

    // add normal property
    // \todo this adds property even if no normals present. change it.
    auto vnormal = mesh.vertex_property<Normal>("v:normal");

    char line[200];
    float x, y, z;
    float nx, ny, nz;
    int n;
    Vertex v;

    // read data
    while (in && !feof(in) && fgets(line, 200, in))
    {
        n = sscanf(line, "%f %f %f %f %f %f", &x, &y, &z, &nx, &ny, &nz);
        if (n >= 3)
        {
            v = mesh.add_vertex(Point(x, y, z));
            if (n >= 6)
            {
                vnormal[v] = Normal(nx, ny, nz);
            }
        }
    }

    fclose(in);
}

// \todo remove duplication with read_xyz
void SurfaceMeshIO::read_agi(SurfaceMesh& mesh)
{
    // open file (in ASCII mode)
    FILE* in = fopen(filename_.c_str(), "r");
    if (!in)
        throw IOException("Failed to open file: " + filename_);

    // add normal property
    auto normal = mesh.vertex_property<Normal>("v:normal");
    auto color = mesh.vertex_property<Color>("v:color");

    char line[200];
    float x, y, z;
    float nx, ny, nz;
    float r, g, b;
    int n;
    Vertex v;

    // read data
    while (in && !feof(in) && fgets(line, 200, in))
    {
        n = sscanf(line, "%f %f %f %f %f %f %f %f %f", &x, &y, &z, &r, &g, &b,
                   &nx, &ny, &nz);
        if (n == 9)
        {
            v = mesh.add_vertex(Point(x, y, z));
            normal[v] = Normal(nx, ny, nz);
            color[v] = Color(r / 255.0, g / 255.0, b / 255.0);
        }
    }

    fclose(in);
}

void SurfaceMeshIO::write_pmp(const SurfaceMesh& mesh)
{
    // open file (in binary mode)
    FILE* out = fopen(filename_.c_str(), "wb");
    if (!out)
        throw IOException("Failed to open file: " + filename_);

    // get properties
    auto vconn = mesh.get_vertex_property<SurfaceMesh::VertexConnectivity>("v:connectivity");
    auto hconn = mesh.get_halfedge_property<SurfaceMesh::HalfedgeConnectivity>("h:connectivity");
    auto fconn =mesh.get_face_property<SurfaceMesh::FaceConnectivity>("f:connectivity");
    auto point = mesh.get_vertex_property<Point>("v:point");

    // how many elements?
    uint32_t nv, ne, nh, nf;
    nv = mesh.n_vertices();
    ne = mesh.n_edges();
    nh = mesh.n_halfedges();
    nf = mesh.n_faces();

    std::vector<std::tuple<std::string, uint32_t, char*, uint32_t>> props;
    uint32_t num_props = 0;
    #define ADD_PROP(name, target, N, n) { \
            auto prop = mesh.get_##target##_property<Eigen::Matrix<Scalar, N, 1>>(name); \
            if (prop) props.emplace_back(name, n, (char*)prop.data(), N); \
        }
    ADD_PROP("v:normal", vertex, 3, nv);
    ADD_PROP("v:color", vertex, 3, nv);
    ADD_PROP("v:tex", vertex, 2, nv);
    ADD_PROP("f:normal", face, 3, nf);
    ADD_PROP("f:color", face, 3, nf);
    ADD_PROP("h:tex", halfedge, 2, nh);

    // write header
    tfwrite(out, nv);
    tfwrite(out, ne);
    tfwrite(out, nf);

    uint8_t version = 2;
    tfwrite(out, version);

    // write property names and sizes
    tfwrite(out, (uint32_t)props.size());
    for (auto [ name, n, data, num_elements ] : props)
    {
        uint32_t len = name.size();
        tfwrite(out, len);
        tfwrite(out, num_elements);
        fwrite(name.c_str(), sizeof(char), len, out);
    }

    // write properties to file
    fwrite((char*)vconn.data(), sizeof(SurfaceMesh::VertexConnectivity), nv,
           out);
    fwrite((char*)hconn.data(), sizeof(SurfaceMesh::HalfedgeConnectivity), nh,
           out);
    fwrite((char*)fconn.data(), sizeof(SurfaceMesh::FaceConnectivity), nf, out);
    fwrite((char*)point.data(), sizeof(Point), nv, out);

    // write other properties
    for (auto [ name, n, data, num_elements ] : props)
    {
        fwrite(data, sizeof(Scalar) * num_elements, n, out);
    }

    fclose(out);
}

template<typename T>
void write_prop(HighFive::Group& group, const ConstProperty<T>& prop, size_t n, const std::string& name)
{
    if (prop)
    {
        std::vector<size_t> dims = { n * sizeof(T) };
        char* data = (char*)prop.data();
        if (group.exist(name))
        {
            auto set = group.getDataSet(name);
            if (set.getDimensions() != dims)
            {
                set.resize(dims);
            }
            set.write_raw(data);
        }
        else
        {
            HighFive::DataSpace dataSpace(dims);
            HighFive::DataSetCreateProps properties;
            properties.add(HighFive::Chunking({ dims[0] }));
            group.createDataSet<char>(name, dataSpace, properties).write_raw(data);
        }
    }
}
template<typename T>
void write_vprop(HighFive::Group& group, const SurfaceMesh& mesh, const std::string& name)
{ write_prop(group, mesh.get_vertex_property<T>(name), mesh.n_vertices(), name); }
template<typename T>
void write_eprop(HighFive::Group& group, const SurfaceMesh& mesh, const std::string& name)
{ write_prop(group, mesh.get_edge_property<T>(name), mesh.n_edges(), name); }
template<typename T>
void write_hprop(HighFive::Group& group, const SurfaceMesh& mesh, const std::string& name)
{ write_prop(group, mesh.get_halfedge_property<T>(name), mesh.n_halfedges(), name); }
template<typename T>
void write_fprop(HighFive::Group& group, const SurfaceMesh& mesh, const std::string& name)
{ write_prop(group, mesh.get_face_property<T>(name), mesh.n_faces(), name); }

void SurfaceMeshIO::write_hdf5_const(HighFive::Group& group, const SurfaceMesh& mesh)
{
    if (mesh.has_garbage())
        throw IOException("Cannot write mesh with garbage");

    lvr2::hdf5util::setAttribute(group, "n_vertices", (uint64_t)mesh.n_vertices());
    lvr2::hdf5util::setAttribute(group, "n_edges", (uint64_t)mesh.n_edges());
    lvr2::hdf5util::setAttribute(group, "n_faces", (uint64_t)mesh.n_faces());

    if (mesh.n_faces() == 0)
        return;

    uint8_t version = 2;
    lvr2::hdf5util::setAttribute(group, "version", version);

    write_vprop<SurfaceMesh::VertexConnectivity>(group, mesh, "v:connectivity");
    write_hprop<SurfaceMesh::HalfedgeConnectivity>(group, mesh, "h:connectivity");
    write_fprop<SurfaceMesh::FaceConnectivity>(group, mesh, "f:connectivity");

    write_vprop<Point>(group, mesh, "v:point");

    write_vprop<Normal>(group, mesh, "v:normal");
    write_fprop<Normal>(group, mesh, "f:normal");

    write_vprop<Color>(group, mesh, "v:color");
    write_fprop<Color>(group, mesh, "f:color");

    write_vprop<TexCoord>(group, mesh, "v:tex");
    write_hprop<TexCoord>(group, mesh, "h:tex");
}


// helper to assemble vertex data
static int vertexCallback(p_ply_argument argument)
{
    long idx;
    void* pdata;
    ply_get_argument_user_data(argument, &pdata, &idx);

    auto* mesh = (pmp::SurfaceMesh*)pdata;
    static thread_local Point point;
    point[idx] = ply_get_argument_value(argument);

    if (idx == 2)
        mesh->add_vertex(point);

    return 1;
}

// helper to assemble face data
static int faceCallback(p_ply_argument argument)
{
    long length, value_index;
    void* pdata;
    long idata;
    ply_get_argument_user_data(argument, &pdata, &idata);
    ply_get_argument_property(argument, nullptr, &length, &value_index);

    auto* mesh = (pmp::SurfaceMesh*)pdata;
    static thread_local std::vector<Vertex> vertices;

    if (value_index == 0)
        vertices.clear();

    pmp::IndexType idx = (pmp::IndexType)ply_get_argument_value(argument);
    vertices.push_back(pmp::Vertex(idx));

    if (value_index == length - 1)
    {
        try
        {
            mesh->add_face(vertices);
        }
        catch (const pmp::TopologyException&)
        { }
    }

    return 1;
}

void SurfaceMeshIO::read_ply(SurfaceMesh& mesh)
{
    // open file, read header
    p_ply ply = ply_open(filename_.c_str(), nullptr, 0, nullptr);

    if (!ply)
        throw IOException("Failed to open file: " + filename_);

    if (!ply_read_header(ply))
        throw IOException("Failed to read PLY header!");

    // setup callbacks for basic properties
    ply_set_read_cb(ply, "vertex", "x", vertexCallback, &mesh, 0);
    ply_set_read_cb(ply, "vertex", "y", vertexCallback, &mesh, 1);
    ply_set_read_cb(ply, "vertex", "z", vertexCallback, &mesh, 2);

    ply_set_read_cb(ply, "face", "vertex_indices", faceCallback, &mesh, 0);

    // read the data
    if (!ply_read(ply))
        throw IOException("Failed to read PLY data!");

    ply_close(ply);
}

void SurfaceMeshIO::write_ply(const SurfaceMesh& mesh)
{
    e_ply_storage_mode mode = flags_.use_binary ? PLY_LITTLE_ENDIAN : PLY_ASCII;
    p_ply ply = ply_create(filename_.c_str(), mode, nullptr, 0, nullptr);

    ply_add_comment(ply, "File written with pmp-library");
    ply_add_element(ply, "vertex", mesh.n_vertices());
    ply_add_scalar_property(ply, "x", PLY_FLOAT);
    ply_add_scalar_property(ply, "y", PLY_FLOAT);
    ply_add_scalar_property(ply, "z", PLY_FLOAT);
    ply_add_element(ply, "face", mesh.n_faces());
    ply_add_property(ply, "vertex_indices", PLY_LIST, PLY_UCHAR, PLY_INT);
    ply_write_header(ply);

    // write vertices
    auto points = mesh.get_vertex_property<Point>("v:point");
    for (auto v : mesh.vertices())
    {
        ply_write(ply, points[v][0]);
        ply_write(ply, points[v][1]);
        ply_write(ply, points[v][2]);
    }

    // write faces
    for (auto f : mesh.faces())
    {
        ply_write(ply, mesh.valence(f));
        for (auto fv : mesh.vertices(f))
            ply_write(ply, fv.idx());
    }

    ply_close(ply);
}

// helper class for STL reader
class CmpVec
{
public:
    CmpVec(Scalar eps = std::numeric_limits<Scalar>::min()) : eps_(eps) {}

    bool operator()(const vec3& v0, const vec3& v1) const
    {
        if (fabs(v0[0] - v1[0]) <= eps_)
        {
            if (fabs(v0[1] - v1[1]) <= eps_)
            {
                return (v0[2] < v1[2] - eps_);
            }
            else
                return (v0[1] < v1[1] - eps_);
        }
        else
            return (v0[0] < v1[0] - eps_);
    }

private:
    Scalar eps_;
};

void SurfaceMeshIO::read_stl(SurfaceMesh& mesh)
{
    char line[100], *c;
    unsigned int i, nT(0);
    vec3 p;
    Vertex v;
    std::vector<Vertex> vertices(3);
    size_t n_items(0);

    CmpVec comp(std::numeric_limits<Scalar>::min());
    std::map<vec3, Vertex, CmpVec> vMap(comp);
    std::map<vec3, Vertex, CmpVec>::iterator vMapIt;

    // open file (in ASCII mode)
    FILE* in = fopen(filename_.c_str(), "r");
    if (!in)
        throw IOException("Failed to open file: " + filename_);

    // ASCII or binary STL?
    c = fgets(line, 6, in);
    PMP_ASSERT(c != nullptr);
    const bool binary =
        ((strncmp(line, "SOLID", 5) != 0) && (strncmp(line, "solid", 5) != 0));

    // parse binary STL
    if (binary)
    {
        // re-open file in binary mode
        fclose(in);
        in = fopen(filename_.c_str(), "rb");
        if (!in)
            throw IOException("Failed to open file: " + filename_);

        // skip dummy header
        n_items = fread(line, 1, 80, in);
        PMP_ASSERT(n_items > 0);

        // read number of triangles
        tfread(in, nT);

        // read triangles
        while (nT)
        {
            // skip triangle normal
            n_items = fread(line, 1, 12, in);
            PMP_ASSERT(n_items > 0);
            // triangle's vertices
            for (i = 0; i < 3; ++i)
            {
                tfread(in, p);

                // has vector been referenced before?
                if ((vMapIt = vMap.find(p)) == vMap.end())
                {
                    // No : add vertex and remember idx/vector mapping
                    v = mesh.add_vertex((Point)p);
                    vertices[i] = v;
                    vMap[p] = v;
                }
                else
                {
                    // Yes : get index from map
                    vertices[i] = vMapIt->second;
                }
            }

            // Add face only if it is not degenerated
            if ((vertices[0] != vertices[1]) && (vertices[0] != vertices[2]) &&
                (vertices[1] != vertices[2]))
                add_face(mesh, vertices);

            n_items = fread(line, 1, 2, in);
            PMP_ASSERT(n_items > 0);
            --nT;
        }
    }

    // parse ASCII STL
    else
    {
        // parse line by line
        while (in && !feof(in) && fgets(line, 100, in))
        {
            // skip white-space
            for (c = line; isspace(*c) && *c != '\0'; ++c)
            {
            };

            // face begins
            if ((strncmp(c, "outer", 5) == 0) || (strncmp(c, "OUTER", 5) == 0))
            {
                // read three vertices
                for (i = 0; i < 3; ++i)
                {
                    // read line
                    c = fgets(line, 100, in);
                    PMP_ASSERT(c != nullptr);

                    // skip white-space
                    for (c = line; isspace(*c) && *c != '\0'; ++c)
                    {
                    };

                    // read x, y, z
                    sscanf(c + 6, "%f %f %f", &p[0], &p[1], &p[2]);

                    // has vector been referenced before?
                    if ((vMapIt = vMap.find(p)) == vMap.end())
                    {
                        // No : add vertex and remember idx/vector mapping
                        v = mesh.add_vertex((Point)p);
                        vertices[i] = v;
                        vMap[p] = v;
                    }
                    else
                    {
                        // Yes : get index from map
                        vertices[i] = vMapIt->second;
                    }
                }

                // Add face only if it is not degenerated
                if ((vertices[0] != vertices[1]) &&
                    (vertices[0] != vertices[2]) &&
                    (vertices[1] != vertices[2]))
                    add_face(mesh, vertices);
            }
        }
    }

    fclose(in);
}

void SurfaceMeshIO::write_stl(const SurfaceMesh& mesh)
{
    if (!mesh.is_triangle_mesh())
    {
        auto what = "SurfaceMeshIO::write_stl: Not a triangle mesh.";
        throw InvalidInputException(what);
    }

    auto fnormals = mesh.get_face_property<Normal>("f:normal");
    if (!fnormals)
    {
        auto what = "SurfaceMeshIO::write_stl: No face normals present.";
        throw InvalidInputException(what);
    }

    std::ofstream ofs(filename_.c_str());
    auto points = mesh.get_vertex_property<Point>("v:point");

    ofs << "solid stl" << std::endl;
    Normal n;
    Point p;

    for (auto f : mesh.faces())
    {
        n = fnormals[f];
        ofs << "  facet normal ";
        ofs << n[0] << " " << n[1] << " " << n[2] << std::endl;
        ofs << "    outer loop" << std::endl;
        for (auto v : mesh.vertices(f))
        {
            p = points[v];
            ofs << "      vertex ";
            ofs << p[0] << " " << p[1] << " " << p[2] << std::endl;
        }
        ofs << "    endloop" << std::endl;
        ofs << "  endfacet" << std::endl;
    }
    ofs << "endsolid" << std::endl;
    ofs.close();
}

void SurfaceMeshIO::write_xyz(const SurfaceMesh& mesh)
{
    std::ofstream ofs(filename_);
    if (!ofs)
        throw IOException("Failed to open file: " + filename_);

    auto vnormal = mesh.get_vertex_property<Normal>("v:normal");
    for (auto v : mesh.vertices())
    {
        ofs << mesh.position(v);
        ofs << " ";
        if (vnormal)
        {
            ofs << vnormal[v];
        }
        ofs << std::endl;
    }

    ofs.close();
}

Face SurfaceMeshIO::add_face(SurfaceMesh& mesh,
                             const std::vector<Vertex>& vertices)
{
    Face f;
    try
    {
        f = mesh.add_face(vertices);
    }
    catch (const TopologyException&)
    {
        failed_faces_.push_back(vertices);
    }
    return f;
}

void SurfaceMeshIO::add_failed_faces(SurfaceMesh& mesh)
{
    for (auto vertices : failed_faces_)
    {
        auto duplicates = duplicate_vertices(mesh, vertices);
        mesh.add_face(duplicates);
    }
    failed_faces_.clear();
}

std::vector<Vertex> SurfaceMeshIO::duplicate_vertices(
    SurfaceMesh& mesh, const std::vector<Vertex>& vertices) const
{
    std::vector<Vertex> duplicates;
    for (auto v : vertices)
    {
        auto dv = mesh.add_vertex(mesh.position(v));
        duplicates.push_back(dv);
    }
    return duplicates;
}

} // namespace pmp
