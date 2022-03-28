// Copyright 2011-2021 the Polygon Mesh Processing Library developers.
// Copyright 2001-2005 by Computer Graphics Group, RWTH Aachen
// Distributed under a MIT-style license, see PMP_LICENSE.txt for details.

#pragma once

#include <map>
#include <vector>
#include <limits>
#include <numeric>
#include <memory>
#include <unordered_set>

#include "Types.h"
#include "Properties.h"
#include "BoundingBox.h"

namespace pmp
{
class Handle;
}

namespace lvr2
{

/**
 * @brief An iterator for handles in the BaseMesh.
 *
 * Important: This is not a fail fast iterator! If the mesh struct is changed
 * while using an instance of this iterator the behavior is undefined!
 *
 * @tparam HandleT The type of the requested handle
 */
template<typename HandleT>
class MeshHandleIterator
{
    static_assert(std::is_base_of<pmp::Handle, HandleT>::value, "HandleT must inherit from BaseHandle!");
public:
    /// Advances the iterator once. Using the dereference operator afterwards
    /// will yield the next handle.
    virtual MeshHandleIterator& operator++() = 0;
    virtual bool operator==(const MeshHandleIterator& other) const = 0;
    virtual bool operator!=(const MeshHandleIterator& other) const = 0;

    /// Returns the current handle.
    virtual HandleT operator*() const = 0;

    virtual ~MeshHandleIterator() = default;

    using HandleType = HandleT;
};

/// A wrapper for the MeshHandleIterator
template<typename HandleT>
class MeshHandleIteratorPtr
{
public:
    MeshHandleIteratorPtr(std::unique_ptr<MeshHandleIterator<HandleT>> iter) : m_iter(std::move(iter)) {};
    MeshHandleIteratorPtr& operator++();
    bool operator==(const MeshHandleIteratorPtr& other) const;
    bool operator!=(const MeshHandleIteratorPtr& other) const;
    HandleT operator*() const;

    using HandleType = HandleT;
private:
    std::unique_ptr<MeshHandleIterator<HandleT>> m_iter;
};

} // namespace lvr2

namespace pmp
{

class SurfaceMeshIO;

//! \addtogroup core
//!@{

// Handle Types

//! Base class for all entity handles types.
//! \details internally it is basically an index.
class Handle
{
public:
    //! default constructor with invalid index
    explicit Handle(IndexType idx = PMP_MAX_INDEX) : idx_(idx) {}

    //! Get the underlying index of this handle
    IndexType idx() const { return idx_; }

    //! reset handle to be invalid (index=PMP_MAX_INDEX.)
    void reset() { idx_ = PMP_MAX_INDEX; }

    //! return whether the handle is valid, i.e., the index is not equal to PMP_MAX_INDEX.
    bool is_valid() const { return idx_ != PMP_MAX_INDEX; }

    //! are two handles equal?
    bool operator==(const Handle& rhs) const { return idx_ == rhs.idx_; }

    //! are two handles different?
    bool operator!=(const Handle& rhs) const { return idx_ != rhs.idx_; }

    //! compare operator useful for sorting handles
    bool operator<(const Handle& rhs) const { return idx_ < rhs.idx_; }

private:
    friend class SurfaceMesh;
    IndexType idx_;
};

//! this type represents a vertex (internally it is basically an index)
class Vertex : public Handle
{
    using Handle::Handle;
};

//! this type represents a halfedge (internally it is basically an index)
class Halfedge : public Handle
{
    using Handle::Handle;
};

//! this type represents an edge (internally it is basically an index)
class Edge : public Handle
{
    using Handle::Handle;
};

//! this type represents a face (internally it is basically an index)
class Face : public Handle
{
    using Handle::Handle;
};

// Output operators

//! output a Vertex to a stream
inline std::ostream& operator<<(std::ostream& os, Vertex v)
{
    return (os << 'v' << v.idx());
}

//! output a Halfedge to a stream
inline std::ostream& operator<<(std::ostream& os, Halfedge h)
{
    return (os << 'h' << h.idx());
}

//! output an Edge to a stream
inline std::ostream& operator<<(std::ostream& os, Edge e)
{
    return (os << 'e' << e.idx());
}

//! output a Face to a stream
inline std::ostream& operator<<(std::ostream& os, Face f)
{
    return (os << 'f' << f.idx());
}

} // namespace pmp

// hash functions
namespace std
{
#define IMPL_HANDLE_HASH(Type) \
    template<> struct hash<Type> \
    { \
        std::size_t operator()(const Type& h) const \
        { return std::hash<pmp::IndexType>()(h.idx()); } \
    }; \
    template<> struct less<Type> \
    { \
        bool operator()(const Type& l, const Type& r) const \
        { return std::less<pmp::IndexType>()(l.idx(), r.idx()); } \
    };

    IMPL_HANDLE_HASH(pmp::Handle);
    IMPL_HANDLE_HASH(pmp::Vertex);
    IMPL_HANDLE_HASH(pmp::Halfedge);
    IMPL_HANDLE_HASH(pmp::Edge);
    IMPL_HANDLE_HASH(pmp::Face);
} // namespace std

namespace pmp
{

// Property Types

//! Vertex property of type T
template <class T>
class VertexProperty : public Property<T>
{
public:
    //! default constructor
    explicit VertexProperty() {}
    explicit VertexProperty(Property<T> p) : Property<T>(p) {}

    //! access the data stored for vertex \p v
    typename Property<T>::reference operator[](Vertex v)
    {
        return Property<T>::operator[](v.idx());
    }

    //! access the data stored for vertex \p v
    typename Property<T>::const_reference operator[](Vertex v) const
    {
        return Property<T>::operator[](v.idx());
    }
};

//! Halfedge property of type T
template <class T>
class HalfedgeProperty : public Property<T>
{
public:
    //! default constructor
    explicit HalfedgeProperty() {}
    explicit HalfedgeProperty(Property<T> p) : Property<T>(p) {}

    //! access the data stored for halfedge \p h
    typename Property<T>::reference operator[](Halfedge h)
    {
        return Property<T>::operator[](h.idx());
    }

    //! access the data stored for halfedge \p h
    typename Property<T>::const_reference operator[](Halfedge h) const
    {
        return Property<T>::operator[](h.idx());
    }
};

//! Edge property of type T
template <class T>
class EdgeProperty : public Property<T>
{
public:
    //! default constructor
    explicit EdgeProperty() {}
    explicit EdgeProperty(Property<T> p) : Property<T>(p) {}

    //! access the data stored for edge \p e
    typename Property<T>::reference operator[](Edge e)
    {
        return Property<T>::operator[](e.idx());
    }

    //! access the data stored for edge \p e
    typename Property<T>::const_reference operator[](Edge e) const
    {
        return Property<T>::operator[](e.idx());
    }
};

//! Face property of type T
template <class T>
class FaceProperty : public Property<T>
{
public:
    //! default constructor
    explicit FaceProperty() {}
    explicit FaceProperty(Property<T> p) : Property<T>(p) {}

    //! access the data stored for face \p f
    typename Property<T>::reference operator[](Face f)
    {
        return Property<T>::operator[](f.idx());
    }

    //! access the data stored for face \p f
    typename Property<T>::const_reference operator[](Face f) const
    {
        return Property<T>::operator[](f.idx());
    }
};

//! Object property of type T
template <class T>
class ObjectProperty : public Property<T>
{
public:
    //! default constructor
    explicit ObjectProperty() {}
    explicit ObjectProperty(Property<T> p) : Property<T>(p) {}

    //! access the data stored for the object
    typename Property<T>::reference operator[](IndexType idx)
    {
        return Property<T>::operator[](idx);
    }

    //! access the data stored for the object
    typename Property<T>::const_reference operator[](IndexType idx) const
    {
        return Property<T>::operator[](idx);
    }
};

//! A halfedge data structure for polygonal meshes.
class SurfaceMesh
{
public:
    //! \name Iterator Types
    //!@{

    template<class HandleT>
    class HandleIterator : public lvr2::MeshHandleIterator<HandleT>
    {
    public:
        //! Default constructor
        HandleIterator(HandleT handle, const SurfaceMesh* m)
            : handle_(handle), mesh_(m)
        {
            while (mesh_->is_valid(handle_) && mesh_->is_deleted(handle_))
                ++handle_.idx_;
        }

        //! get the handle the iterator refers to
        HandleT operator*() const { return handle_; }

        //! are two iterators equal?
        bool operator==(const HandleIterator<HandleT>& rhs) const
        {
            assert(mesh_ == rhs.mesh_);
            return handle_ == rhs.handle_;
        }
        bool operator==(const lvr2::MeshHandleIterator<HandleT>& other) const override
        {
            auto cast = dynamic_cast<const HandleIterator<HandleT>*>(&other);
            return cast && operator==(*cast);
        }

        //! are two iterators different?
        bool operator!=(const HandleIterator<HandleT>& rhs) const
        {
            return !operator==(rhs);
        }
        bool operator!=(const lvr2::MeshHandleIterator<HandleT>& other) const override
        {
            auto cast = dynamic_cast<const HandleIterator<HandleT>*>(&other);
            return !cast || operator!=(*cast);
        }

        //! pre-increment iterator
        HandleIterator& operator++()
        {
            ++handle_.idx_;
            while (mesh_->is_valid(handle_) && mesh_->is_deleted(handle_))
                ++handle_.idx_;
            return *this;
        }

        //! post-increment iterator
        HandleIterator operator++(int)
        {
            HandleIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        //! pre-decrement iterator
        HandleIterator& operator--()
        {
            --handle_.idx_;
            while (mesh_->is_valid(handle_) && mesh_->is_deleted(handle_))
                --handle_.idx_;
            return *this;
        }

    protected:
        HandleT handle_;
        const SurfaceMesh* mesh_;
    };

    //! this class iterates linearly over all vertices
    //! \sa vertices_begin(), vertices_end()
    //! \sa HalfedgeIterator, EdgeIterator, FaceIterator
    using VertexIterator = HandleIterator<Vertex>;

    //! this class iterates linearly over all halfedges
    //! \sa halfedges_begin(), halfedges_end()
    //! \sa VertexIterator, EdgeIterator, FaceIterator
    using HalfedgeIterator = HandleIterator<Halfedge>;

    //! this class iterates linearly over all edges
    //! \sa edges_begin(), edges_end()
    //! \sa VertexIterator, HalfedgeIterator, FaceIterator
    using EdgeIterator = HandleIterator<Edge>;

    //! this class iterates linearly over all faces
    //! \sa faces_begin(), faces_end()
    //! \sa VertexIterator, HalfedgeIterator, EdgeIterator
    using FaceIterator = HandleIterator<Face>;

    //!@}
    //! \name Container Types
    //!@{

    template<class HandleT>
    class HandleContainer
    {
    public:
        HandleContainer(HandleIterator<HandleT> begin, HandleIterator<HandleT> end)
            : begin_(begin), end_(end)
        {
        }
        HandleIterator<HandleT> begin() const { return begin_; }
        HandleIterator<HandleT> end() const { return end_; }

    private:
        HandleIterator<HandleT> begin_, end_;
    };

    //! helper class for iterating through all vertices using range-based
    //! for-loops. \sa vertices()
    using VertexContainer = HandleContainer<Vertex>;

    //! helper class for iterating through all halfedges using range-based
    //! for-loops. \sa halfedges()
    using HalfedgeContainer = HandleContainer<Halfedge>;

    //! helper class for iterating through all edges using range-based
    //! for-loops. \sa edges()
    using EdgeContainer = HandleContainer<Edge>;

    //! helper class for iterating through all faces using range-based
    //! for-loops. \sa faces()
    using FaceContainer = HandleContainer<Face>;

    //!@}
    //! \name Circulator Types
    //!@{

    class CirculatorLoopDetector
    {
    public:
        CirculatorLoopDetector(Halfedge start)
            : iter_count_(0), start_(start)
        {
        }
        void set_start(Halfedge new_start)
        {
            start_ = new_start;
            cancel();
        }
        void loop_detection(Halfedge current)
        {
            if (current == start_)
            {
                // Circulators are expected to circulate around start_
                cancel();
            }
            else if (++iter_count_ > 100 && !visited_.insert(current).second)
            {
                // any loops that don't contain start_ are an error in the mesh
                throw TopologyException("Loop detected in mesh!");
            }
        }
        void cancel()
        {
            if (iter_count_ > 0)
            {
                iter_count_ = 0;
                visited_.clear();
            }
        }
    private:
        size_t iter_count_;
        Halfedge start_;
        std::unordered_set<Halfedge> visited_;
    };

    //! this class circulates through all one-ring neighbors of a vertex.
    //! it also acts as a container-concept for C++11 range-based for loops.
    //! \sa HalfedgeAroundVertexCirculator, vertices(Vertex)
    class VertexAroundVertexCirculator
    {
    public:
        //! default constructor
        VertexAroundVertexCirculator(const SurfaceMesh* mesh, Vertex v)
            : mesh_(mesh), halfedge_(mesh_->halfedge(v)), is_active_(true), loop_helper_(halfedge_)
        {
        }

        //! are two circulators equal?
        bool operator==(const VertexAroundVertexCirculator& rhs) const
        {
            assert(mesh_ == rhs.mesh_);
            return (is_active_ && (halfedge_ == rhs.halfedge_));
        }

        //! are two circulators different?
        bool operator!=(const VertexAroundVertexCirculator& rhs) const
        {
            return !operator==(rhs);
        }

        //! pre-increment (rotate couter-clockwise)
        VertexAroundVertexCirculator& operator++()
        {
            assert(halfedge_.is_valid());
            halfedge_ = mesh_->ccw_rotated_halfedge(halfedge_);
            is_active_ = true;
            loop_helper_.loop_detection(halfedge_);
            return *this;
        }

        //! pre-decrement (rotate clockwise)
        VertexAroundVertexCirculator& operator--()
        {
            assert(halfedge_.is_valid());
            halfedge_ = mesh_->cw_rotated_halfedge(halfedge_);
            loop_helper_.cancel();
            return *this;
        }

        //! get the vertex the circulator refers to
        Vertex operator*() const
        {
            return mesh_->to_vertex(halfedge_);
        }

        //! cast to bool: true if vertex is not isolated
        operator bool() const { return halfedge_.is_valid(); }

        //! return current halfedge
        Halfedge halfedge() const { return halfedge_; }

        // helper for C++11 range-based for-loops
        VertexAroundVertexCirculator& begin()
        {
            is_active_ = !halfedge_.is_valid();
            return *this;
        }
        // helper for C++11 range-based for-loops
        VertexAroundVertexCirculator& end()
        {
            is_active_ = true;
            return *this;
        }

    private:
        const SurfaceMesh* mesh_;
        Halfedge halfedge_;
        bool is_active_; // helper for C++11 range-based for-loops
        CirculatorLoopDetector loop_helper_;
    };

    //! this class circulates through all outgoing halfedges of a vertex.
    //! it also acts as a container-concept for C++11 range-based for loops.
    //! \sa VertexAroundVertexCirculator, halfedges(Vertex)
    class HalfedgeAroundVertexCirculator
    {
    public:
        //! default constructor
        HalfedgeAroundVertexCirculator(const SurfaceMesh* mesh, Vertex v)
            : mesh_(mesh), halfedge_(mesh_->halfedge(v)), is_active_(true), loop_helper_(halfedge_)
        {
        }

        //! are two circulators equal?
        bool operator==(const HalfedgeAroundVertexCirculator& rhs) const
        {
            assert(mesh_ == rhs.mesh_);
            return (is_active_ && (halfedge_ == rhs.halfedge_));
        }

        //! are two circulators different?
        bool operator!=(const HalfedgeAroundVertexCirculator& rhs) const
        {
            return !operator==(rhs);
        }

        //! pre-increment (rotate couter-clockwise)
        HalfedgeAroundVertexCirculator& operator++()
        {
            assert(halfedge_.is_valid());
            halfedge_ = mesh_->ccw_rotated_halfedge(halfedge_);
            is_active_ = true;
            loop_helper_.loop_detection(halfedge_);
            return *this;
        }

        //! pre-decrement (rotate clockwise)
        HalfedgeAroundVertexCirculator& operator--()
        {
            assert(halfedge_.is_valid());
            halfedge_ = mesh_->cw_rotated_halfedge(halfedge_);
            loop_helper_.cancel();
            return *this;
        }

        //! get the halfedge the circulator refers to
        Halfedge operator*() const { return halfedge_; }

        //! cast to bool: true if vertex is not isolated
        operator bool() const { return halfedge_.is_valid(); }

        // helper for C++11 range-based for-loops
        HalfedgeAroundVertexCirculator& begin()
        {
            is_active_ = !halfedge_.is_valid();
            return *this;
        }
        // helper for C++11 range-based for-loops
        HalfedgeAroundVertexCirculator& end()
        {
            is_active_ = true;
            return *this;
        }

    private:
        const SurfaceMesh* mesh_;
        Halfedge halfedge_;
        bool is_active_; // helper for C++11 range-based for-loops
        CirculatorLoopDetector loop_helper_;
    };

    //! this class circulates through all incident faces of a vertex.
    //! it also acts as a container-concept for C++11 range-based for loops.
    //! \sa VertexAroundVertexCirculator, HalfedgeAroundVertexCirculator, faces(Vertex)
    class FaceAroundVertexCirculator
    {
    public:
        //! construct with mesh and vertex (vertex should not be isolated!)
        FaceAroundVertexCirculator(const SurfaceMesh* m, Vertex v)
            : mesh_(m), halfedge_(mesh_->halfedge(v)), is_active_(true), loop_helper_(halfedge_)
        {
            if (halfedge_.is_valid() && mesh_->is_boundary(halfedge_))
            {
                operator++();
                loop_helper_.set_start(halfedge_);
            }
        }

        //! are two circulators equal?
        bool operator==(const FaceAroundVertexCirculator& rhs) const
        {
            assert(mesh_ == rhs.mesh_);
            return (is_active_ && (halfedge_ == rhs.halfedge_));
        }

        //! are two circulators different?
        bool operator!=(const FaceAroundVertexCirculator& rhs) const
        {
            return !operator==(rhs);
        }

        //! pre-increment (rotates counter-clockwise)
        FaceAroundVertexCirculator& operator++()
        {
            assert(halfedge_.is_valid());
            do
            {
                halfedge_ = mesh_->ccw_rotated_halfedge(halfedge_);
            } while (mesh_->is_boundary(halfedge_));
            is_active_ = true;
            loop_helper_.loop_detection(halfedge_);
            return *this;
        }

        //! pre-decrement (rotate clockwise)
        FaceAroundVertexCirculator& operator--()
        {
            assert(halfedge_.is_valid());
            do
                halfedge_ = mesh_->cw_rotated_halfedge(halfedge_);
            while (mesh_->is_boundary(halfedge_));
            loop_helper_.cancel();
            return *this;
        }

        //! get the face the circulator refers to
        Face operator*() const
        {
            assert(halfedge_.is_valid());
            return mesh_->face(halfedge_);
        }

        //! cast to bool: true if vertex is not isolated
        operator bool() const { return halfedge_.is_valid(); }

        // helper for C++11 range-based for-loops
        FaceAroundVertexCirculator& begin()
        {
            is_active_ = !halfedge_.is_valid();
            return *this;
        }
        // helper for C++11 range-based for-loops
        FaceAroundVertexCirculator& end()
        {
            is_active_ = true;
            return *this;
        }

    private:
        const SurfaceMesh* mesh_;
        Halfedge halfedge_;
        bool is_active_; // helper for C++11 range-based for-loops
        CirculatorLoopDetector loop_helper_;
    };

    //! this class circulates through the vertices of a face.
    //! it also acts as a container-concept for C++11 range-based for loops.
    //! \sa HalfedgeAroundFaceCirculator, vertices(Face)
    class VertexAroundFaceCirculator
    {
    public:
        //! default constructor
        VertexAroundFaceCirculator(const SurfaceMesh* m, Face f)
            : mesh_(m), halfedge_(mesh_->halfedge(f)), is_active_(true), loop_helper_(halfedge_)
        {
        }

        //! are two circulators equal?
        bool operator==(const VertexAroundFaceCirculator& rhs) const
        {
            assert(mesh_ == rhs.mesh_);
            return (is_active_ && (halfedge_ == rhs.halfedge_));
        }

        //! are two circulators different?
        bool operator!=(const VertexAroundFaceCirculator& rhs) const
        {
            return !operator==(rhs);
        }

        //! pre-increment (rotates counter-clockwise)
        VertexAroundFaceCirculator& operator++()
        {
            assert(halfedge_.is_valid());
            halfedge_ = mesh_->next_halfedge(halfedge_);
            is_active_ = true;
            loop_helper_.loop_detection(halfedge_);
            return *this;
        }

        //! pre-decrement (rotates clockwise)
        VertexAroundFaceCirculator& operator--()
        {
            assert(halfedge_.is_valid());
            halfedge_ = mesh_->prev_halfedge(halfedge_);
            loop_helper_.cancel();
            return *this;
        }

        //! get the vertex the circulator refers to
        Vertex operator*() const
        {
            assert(halfedge_.is_valid());
            return mesh_->to_vertex(halfedge_);
        }

        // helper for C++11 range-based for-loops
        VertexAroundFaceCirculator& begin()
        {
            is_active_ = false;
            return *this;
        }
        // helper for C++11 range-based for-loops
        VertexAroundFaceCirculator& end()
        {
            is_active_ = true;
            return *this;
        }

    private:
        const SurfaceMesh* mesh_;
        Halfedge halfedge_;
        bool is_active_; // helper for C++11 range-based for-loops
        CirculatorLoopDetector loop_helper_;
    };

    //! this class circulates through all halfedges of a face.
    //! it also acts as a container-concept for C++11 range-based for loops.
    //! \sa VertexAroundFaceCirculator, halfedges(Face)
    class HalfedgeAroundFaceCirculator
    {
    public:
        //! default constructur
        HalfedgeAroundFaceCirculator(const SurfaceMesh* m, Face f)
            : mesh_(m), halfedge_(mesh_->halfedge(f)), is_active_(true), loop_helper_(halfedge_)
        {
        }

        //! are two circulators equal?
        bool operator==(const HalfedgeAroundFaceCirculator& rhs) const
        {
            assert(mesh_ == rhs.mesh_);
            return (is_active_ && (halfedge_ == rhs.halfedge_));
        }

        //! are two circulators different?
        bool operator!=(const HalfedgeAroundFaceCirculator& rhs) const
        {
            return !operator==(rhs);
        }

        //! pre-increment (rotates counter-clockwise)
        HalfedgeAroundFaceCirculator& operator++()
        {
            assert(halfedge_.is_valid());
            halfedge_ = mesh_->next_halfedge(halfedge_);
            is_active_ = true;
            loop_helper_.loop_detection(halfedge_);
            return *this;
        }

        //! pre-decrement (rotates clockwise)
        HalfedgeAroundFaceCirculator& operator--()
        {
            assert(halfedge_.is_valid());
            halfedge_ = mesh_->prev_halfedge(halfedge_);
            loop_helper_.cancel();
            return *this;
        }

        //! get the halfedge the circulator refers to
        Halfedge operator*() const { return halfedge_; }

        // helper for C++11 range-based for-loops
        HalfedgeAroundFaceCirculator& begin()
        {
            is_active_ = false;
            return *this;
        }
        // helper for C++11 range-based for-loops
        HalfedgeAroundFaceCirculator& end()
        {
            is_active_ = true;
            return *this;
        }

    private:
        const SurfaceMesh* mesh_;
        Halfedge halfedge_;
        bool is_active_; // helper for C++11 range-based for-loops
        CirculatorLoopDetector loop_helper_;
    };

    //!@}
    //! \name Construction, destruction, assignment
    //!@{

    //! default constructor
    SurfaceMesh();

    //! destructor
    virtual ~SurfaceMesh();

    //! copy constructor: copies \p rhs to \p *this. performs a deep copy of all
    //! properties.
    SurfaceMesh(const SurfaceMesh& rhs) { operator=(rhs); }

    //! move constructor: moves \p rhs to \p *this.
    SurfaceMesh(SurfaceMesh&& rhs) = default;

    //! assign \p rhs to \p *this. performs a deep copy of all properties.
    SurfaceMesh& operator=(const SurfaceMesh& rhs);

    //! move assign \p rhs to \p *this.
    SurfaceMesh& operator=(SurfaceMesh&& rhs) = default;

    //! assign \p rhs to \p *this. does not copy custom properties.
    SurfaceMesh& assign(const SurfaceMesh& rhs);

    //!@}
    //! \name File IO
    //!@{

    //! \brief Read mesh from file \p filename controlled by \p flags
    //! \details File extension determines file type. Supported formats and
    //! vertex attributes (a=ASCII, b=binary):
    //!
    //! Format | ASCII | Binary | Normals | Colors | Texcoords
    //! -------|-------|--------|---------|--------|----------
    //! OFF    | yes   | yes    | a / b   | a      | a / b
    //! OBJ    | yes   | no     | a       | no     | no
    //! STL    | yes   | yes    | no      | no     | no
    //! PLY    | yes   | yes    | no      | no     | no
    //! PMP    | no    | yes    | no      | no     | no
    //! XYZ    | yes   | no     | a       | no     | no
    //! AGI    | yes   | no     | a       | a      | no
    //!
    //! In addition, the OBJ and PMP formats support reading per-halfedge
    //! texture coordinates.
    void read(const std::string& filename, const IOFlags& flags = IOFlags());

    //! \brief Write mesh to file \p filename controlled by \p flags
    //! \details File extension determines file type. Supported formats and
    //! vertex attributes (a=ASCII, b=binary):
    //!
    //! Format | ASCII | Binary | Normals | Colors | Texcoords
    //! -------|-------|--------|---------|--------|----------
    //! OFF    | yes   | yes    | a       | a      | a
    //! OBJ    | yes   | no     | a       | no     | no
    //! STL    | yes   | no     | no      | no     | no
    //! PLY    | yes   | yes    | no      | no     | no
    //! PMP    | no    | yes    | no      | no     | no
    //! XYZ    | yes   | no     | a       | no     | no
    //!
    //! In addition, the OBJ and PMP formats support writing per-halfedge
    //! texture coordinates.
    void write(const std::string& filename,
               const IOFlags& flags = IOFlags()) const;

    //!@}
    //! \name Add new elements by hand
    //!@{

    //! add a new vertex with position \p p
    Vertex add_vertex(const Point& p);

    //! \brief Add a new face with vertex list \p vertices
    //! \throw TopologyException in case a topological error occurs.
    //! \sa add_triangle, add_quad
    Face add_face(const std::vector<Vertex>& vertices);

    //! add a new triangle connecting vertices \p v0, \p v1, \p v2
    //! \sa add_face, add_quad
    Face add_triangle(Vertex v0, Vertex v1, Vertex v2);

    //! add a new quad connecting vertices \p v0, \p v1, \p v2, \p v3
    //! \sa add_triangle, add_face
    Face add_quad(Vertex v0, Vertex v1, Vertex v2, Vertex v3);

    //!@}
    //! \name Memory Management
    //!@{

    //! returns number of (deleted and valid) vertices in the mesh
    size_t vertices_size() const { return vprops_.size(); }

    //! returns number of (deleted and valid) halfedges in the mesh
    size_t halfedges_size() const { return hprops_.size(); }

    //! returns number of (deleted and valid) edges in the mesh
    size_t edges_size() const { return eprops_.size(); }

    //! returns number of (deleted and valid) faces in the mesh
    size_t faces_size() const { return fprops_.size(); }

    //! returns number of vertices in the mesh
    size_t n_vertices() const { return vertices_size() - deleted_vertices_; }

    //! returns number of halfedge in the mesh
    size_t n_halfedges() const { return halfedges_size() - 2 * deleted_edges_; }

    //! returns number of edges in the mesh
    size_t n_edges() const { return edges_size() - deleted_edges_; }

    //! returns number of faces in the mesh
    size_t n_faces() const { return faces_size() - deleted_faces_; }

    //! returns true if the mesh is empty, i.e., has no vertices
    bool is_empty() const { return n_vertices() == 0; }

    //! clear mesh: remove all vertices, edges, faces
    virtual void clear();

    //! remove unused memory from vectors
    void free_memory();

    //! \brief reserve memory (mainly used in file readers)
    //! \details Passing in zero for any parameter will attempt to calculate
    //! that value using Euler's formula.
    void reserve(size_t nvertices = 0, size_t nedges = 0, size_t nfaces = 0);

    //! remove deleted elements
    void garbage_collection();

    //! returns whether vertex \p v is deleted
    //! \sa garbage_collection()
    bool is_deleted(Vertex v) const { return vdeleted_[v]; }

    //! returns whether halfedge \p h is deleted
    //! \sa garbage_collection()
    bool is_deleted(Halfedge h) const { return edeleted_[edge(h)]; }

    //! returns whether edge \p e is deleted
    //! \sa garbage_collection()
    bool is_deleted(Edge e) const { return edeleted_[e]; }

    //! returns whether face \p f is deleted
    //! \sa garbage_collection()
    bool is_deleted(Face f) const { return fdeleted_[f]; }

    //! return whether vertex \p v is valid, i.e. the index is stores
    //! it within the array bounds.
    bool is_valid(Vertex v) const { return v.idx() < vertices_size(); }

    //! return whether halfedge \p h is valid, i.e. the index is stores it
    //! within the array bounds.
    bool is_valid(Halfedge h) const { return h.idx() < halfedges_size(); }

    //! return whether edge \p e is valid, i.e. the index is stores it within the array bounds.
    bool is_valid(Edge e) const { return e.idx() < edges_size(); }

    //! returns whether the face \p f is valid.
    bool is_valid(Face f) const { return f.idx() < faces_size(); }

    //!@}
    //! \name Low-level connectivity
    //!@{

    //! returns an outgoing halfedge of vertex \p v.
    //! if \p v is a boundary vertex this will be a boundary halfedge.
    Halfedge halfedge(Vertex v) const { return vconn_[v].halfedge_; }

    //! set the outgoing halfedge of vertex \p v to \p h
    void set_halfedge(Vertex v, Halfedge h) { vconn_[v].halfedge_ = h; }

    //! returns whether \p v is a boundary vertex
    bool is_boundary(Vertex v) const
    {
        Halfedge h(halfedge(v));
        return (!(h.is_valid() && face(h).is_valid()));
    }

    //! returns whether \p v is isolated, i.e., not incident to any edge
    bool is_isolated(Vertex v) const { return !halfedge(v).is_valid(); }

    //! returns whether \p v is a manifold vertex (not incident to several patches)
    bool is_manifold(Vertex v) const
    {
        // The vertex is non-manifold if more than one gap exists, i.e.
        // more than one outgoing boundary halfedge.
        int n(0);
        HalfedgeAroundVertexCirculator hit = halfedges(v), hend = hit;
        if (hit)
            do
            {
                if (is_boundary(*hit))
                    ++n;
            } while (++hit != hend);
        return n < 2;
    }

    //! returns the vertex the halfedge \p h points to
    inline Vertex to_vertex(Halfedge h) const { return hconn_[h].vertex_; }

    //! returns the vertex the halfedge \p h emanates from
    inline Vertex from_vertex(Halfedge h) const
    {
        return to_vertex(opposite_halfedge(h));
    }

    //! sets the vertex the halfedge \p h points to to \p v
    inline void set_vertex(Halfedge h, Vertex v) { hconn_[h].vertex_ = v; }

    //! returns the face incident to halfedge \p h
    Face face(Halfedge h) const { return hconn_[h].face_; }

    //! sets the incident face to halfedge \p h to \p f
    void set_face(Halfedge h, Face f) { hconn_[h].face_ = f; }

    //! returns the next halfedge within the incident face
    inline Halfedge next_halfedge(Halfedge h) const
    {
        return hconn_[h].next_halfedge_;
    }

    //! sets the next halfedge of \p h within the face to \p nh
    inline void set_next_halfedge(Halfedge h, Halfedge nh)
    {
        hconn_[h].next_halfedge_ = nh;
        hconn_[nh].prev_halfedge_ = h;
    }

    //! sets the previous halfedge of \p h and the next halfedge of \p ph to \p nh
    inline void set_prev_halfedge(Halfedge h, Halfedge ph)
    {
        hconn_[h].prev_halfedge_ = ph;
        hconn_[ph].next_halfedge_ = h;
    }

    //! returns the previous halfedge within the incident face
    inline Halfedge prev_halfedge(Halfedge h) const
    {
        return hconn_[h].prev_halfedge_;
    }

    //! returns the opposite halfedge of \p h
    inline Halfedge opposite_halfedge(Halfedge h) const
    {
        return Halfedge(h.idx() ^ 1);
    }

    //! returns the halfedge that is rotated counter-clockwise around the
    //! start vertex of \p h. it is the opposite halfedge of the previous
    //! halfedge of \p h.
    inline Halfedge ccw_rotated_halfedge(Halfedge h) const
    {
        return opposite_halfedge(prev_halfedge(h));
    }

    //! returns the halfedge that is rotated clockwise around the start
    //! vertex of \p h. it is the next halfedge of the opposite halfedge of
    //! \p h.
    inline Halfedge cw_rotated_halfedge(Halfedge h) const
    {
        return next_halfedge(opposite_halfedge(h));
    }

    //! return the edge that contains halfedge \p h as one of its two
    //! halfedges.
    inline Edge edge(Halfedge h) const { return Edge(h.idx() >> 1); }

    //! returns whether h is a boundary halfege, i.e., if its face does not exist.
    inline bool is_boundary(Halfedge h) const { return !face(h).is_valid(); }

    //! returns the \p i'th halfedge of edge \p e. \p i has to be 0 or 1.
    inline Halfedge halfedge(Edge e, unsigned int i) const
    {
        assert(i <= 1);
        return Halfedge((e.idx() << 1) + i);
    }

    //! returns the \p i'th vertex of edge \p e. \p i has to be 0 or 1.
    inline Vertex vertex(Edge e, unsigned int i) const
    {
        assert(i <= 1);
        return to_vertex(halfedge(e, i));
    }

    //! returns the face incident to the \p i'th halfedge of edge \p e. \p i has to be 0 or 1.
    Face face(Edge e, unsigned int i) const
    {
        assert(i <= 1);
        return face(halfedge(e, i));
    }

    //! returns whether \p e is a boundary edge, i.e., if one of its
    //! halfedges is a boundary halfedge.
    bool is_boundary(Edge e) const
    {
        return (is_boundary(halfedge(e, 0)) || is_boundary(halfedge(e, 1)));
    }

    //! returns a halfedge of face \p f
    Halfedge halfedge(Face f) const { return fconn_[f].halfedge_; }

    //! sets the halfedge of face \p f to \p h
    void set_halfedge(Face f, Halfedge h) { fconn_[f].halfedge_ = h; }

    //! returns whether \p f is a boundary face, i.e., it one of its edges is a boundary edge.
    bool is_boundary(Face f) const
    {
        Halfedge h = halfedge(f);
        Halfedge hh = h;
        do
        {
            if (is_boundary(opposite_halfedge(h)))
                return true;
            h = next_halfedge(h);
        } while (h != hh);
        return false;
    }

    //!@}
    //! \name Property handling
    //!@{

    //! add a object property of type \p T with name \p name and default value \p t.
    //! fails if a property named \p name exists already, since the name has to
    //! be unique. in this case it returns an invalid property
    template <class T>
    ObjectProperty<T> add_object_property(const std::string& name,
                                          const T t = T())
    {
        return ObjectProperty<T>(oprops_.add<T>(name, t));
    }

    //! get the object property named \p name of type \p T. returns an invalid
    //! ObjectProperty if the property does not exist or if the type does not
    //! match.
    template <class T>
    ObjectProperty<T> get_object_property(const std::string& name) const
    {
        return ObjectProperty<T>(oprops_.get<T>(name));
    }

    //! if a object property of type \p T with name \p name exists, it is
    //! returned.  otherwise this property is added (with default value \p t)
    template <class T>
    ObjectProperty<T> object_property(const std::string& name, const T t = T())
    {
        return ObjectProperty<T>(oprops_.get_or_add<T>(name, t));
    }

    //! remove the object property \p p
    template <class T>
    void remove_object_property(ObjectProperty<T>& p)
    {
        oprops_.remove(p);
    }
    template <class T>
    void remove_object_property(const std::string& name)
    {
        auto p = get_object_property<T>(name);
        remove_object_property(p);
    }

    //! get the type_info \p T of face property named \p name. returns an
    //! typeid(void) if the property does not exist or if the type does not
    //! match.
    const std::type_info& get_object_propertyType(const std::string& name)
    {
        return oprops_.get_type(name);
    }

    //! returns the names of all face properties
    std::vector<std::string> object_properties() const
    {
        return oprops_.properties();
    }

    //! add a vertex property of type \p T with name \p name and default
    //! value \p t. fails if a property named \p name exists already,
    //! since the name has to be unique. in this case it returns an
    //! invalid property
    template <class T>
    VertexProperty<T> add_vertex_property(const std::string& name,
                                          const T t = T())
    {
        return VertexProperty<T>(vprops_.add<T>(name, t));
    }

    //! get the vertex property named \p name of type \p T. returns an
    //! invalid VertexProperty if the property does not exist or if the
    //! type does not match.
    template <class T>
    VertexProperty<T> get_vertex_property(const std::string& name) const
    {
        return VertexProperty<T>(vprops_.get<T>(name));
    }

    //! if a vertex property of type \p T with name \p name exists, it is
    //! returned. otherwise this property is added (with default value \c
    //! t)
    template <class T>
    VertexProperty<T> vertex_property(const std::string& name, const T t = T())
    {
        return VertexProperty<T>(vprops_.get_or_add<T>(name, t));
    }

    //! remove the vertex property \p p
    template <class T>
    void remove_vertex_property(VertexProperty<T>& p)
    {
        vprops_.remove(p);
    }
    template <class T>
    void remove_vertex_property(const std::string& name)
    {
        auto p = get_vertex_property<T>(name);
        remove_vertex_property(p);
    }

    //! does the mesh have a vertex property with name \p name?
    bool has_vertex_property(const std::string& name) const
    {
        return vprops_.exists(name);
    }

    //! add a halfedge property of type \p T with name \p name and default
    //! value \p t.  fails if a property named \p name exists already,
    //! since the name has to be unique. in this case it returns an
    //! invalid property.
    template <class T>
    HalfedgeProperty<T> add_halfedge_property(const std::string& name,
                                              const T t = T())
    {
        return HalfedgeProperty<T>(hprops_.add<T>(name, t));
    }

    //! add a edge property of type \p T with name \p name and default
    //! value \p t.  fails if a property named \p name exists already,
    //! since the name has to be unique.  in this case it returns an
    //! invalid property.
    template <class T>
    EdgeProperty<T> add_edge_property(const std::string& name, const T t = T())
    {
        return EdgeProperty<T>(eprops_.add<T>(name, t));
    }

    //! get the halfedge property named \p name of type \p T. returns an
    //! invalid VertexProperty if the property does not exist or if the
    //! type does not match.
    template <class T>
    HalfedgeProperty<T> get_halfedge_property(const std::string& name) const
    {
        return HalfedgeProperty<T>(hprops_.get<T>(name));
    }

    //! get the edge property named \p name of type \p T. returns an
    //! invalid VertexProperty if the property does not exist or if the
    //! type does not match.
    template <class T>
    EdgeProperty<T> get_edge_property(const std::string& name) const
    {
        return EdgeProperty<T>(eprops_.get<T>(name));
    }

    //! if a halfedge property of type \p T with name \p name exists, it is
    //! returned.  otherwise this property is added (with default value \c
    //! t)
    template <class T>
    HalfedgeProperty<T> halfedge_property(const std::string& name,
                                          const T t = T())
    {
        return HalfedgeProperty<T>(hprops_.get_or_add<T>(name, t));
    }

    //! if an edge property of type \p T with name \p name exists, it is
    //! returned.  otherwise this property is added (with default value \c
    //! t)
    template <class T>
    EdgeProperty<T> edge_property(const std::string& name, const T t = T())
    {
        return EdgeProperty<T>(eprops_.get_or_add<T>(name, t));
    }

    //! remove the halfedge property \p p
    template <class T>
    void remove_halfedge_property(HalfedgeProperty<T>& p)
    {
        hprops_.remove(p);
    }
    template <class T>
    void remove_halfedge_property(const std::string& name)
    {
        auto p = get_halfedge_property<T>(name);
        remove_halfedge_property(p);
    }

    //! does the mesh have a halfedge property with name \p name?
    bool has_halfedge_property(const std::string& name) const
    {
        return hprops_.exists(name);
    }

    //! remove the edge property \p p
    template <class T>
    void remove_edge_property(EdgeProperty<T>& p)
    {
        eprops_.remove(p);
    }
    template <class T>
    void remove_edge_property(const std::string& name)
    {
        auto p = get_edge_property<T>(name);
        remove_edge_property(p);
    }

    //! does the mesh have an edge property with name \p name?
    bool has_edge_property(const std::string& name) const
    {
        return eprops_.exists(name);
    }

    //! get the type_info \p T of halfedge property named \p name. returns an
    //! typeid(void) if the property does not exist or if the type does not
    //! match.
    const std::type_info& get_halfedge_property_type(const std::string& name)
    {
        return hprops_.get_type(name);
    }

    //! get the type_info \p T of vertex property named \p name. returns an
    //! typeid(void) if the property does not exist or if the type does not
    //! match.
    const std::type_info& get_vertex_property_type(const std::string& name)
    {
        return vprops_.get_type(name);
    }

    //! get the type_info \p T of edge property named \p name. returns an
    //! typeid(void) if the property does not exist or if the type does not
    //! match.
    const std::type_info& get_edge_property_type(const std::string& name)
    {
        return eprops_.get_type(name);
    }

    //! returns the names of all vertex properties
    std::vector<std::string> vertex_properties() const
    {
        return vprops_.properties();
    }

    //! returns the names of all halfedge properties
    std::vector<std::string> halfedge_properties() const
    {
        return hprops_.properties();
    }

    //! returns the names of all edge properties
    std::vector<std::string> edge_properties() const
    {
        return eprops_.properties();
    }

    //! add a face property of type \p T with name \p name and default value \c
    //! t.  fails if a property named \p name exists already, since the name has
    //! to be unique.  in this case it returns an invalid property
    template <class T>
    FaceProperty<T> add_face_property(const std::string& name, const T t = T())
    {
        return FaceProperty<T>(fprops_.add<T>(name, t));
    }

    //! get the face property named \p name of type \p T. returns an invalid
    //! VertexProperty if the property does not exist or if the type does not
    //! match.
    template <class T>
    FaceProperty<T> get_face_property(const std::string& name) const
    {
        return FaceProperty<T>(fprops_.get<T>(name));
    }

    //! if a face property of type \p T with name \p name exists, it is
    //! returned.  otherwise this property is added (with default value \p t)
    template <class T>
    FaceProperty<T> face_property(const std::string& name, const T t = T())
    {
        return FaceProperty<T>(fprops_.get_or_add<T>(name, t));
    }

    //! remove the face property \p p
    template <class T>
    void remove_face_property(FaceProperty<T>& p)
    {
        fprops_.remove(p);
    }
    template <class T>
    void remove_face_property(const std::string& name)
    {
        auto p = get_face_property<T>(name);
        remove_face_property(p);
    }

    //! does the mesh have a face property with name \p name?
    bool has_face_property(const std::string& name) const
    {
        return fprops_.exists(name);
    }

    //! get the type_info \p T of face property named \p name . returns an
    //! typeid(void) if the property does not exist or if the type does not
    //! match.
    const std::type_info& get_face_property_type(const std::string& name)
    {
        return fprops_.get_type(name);
    }

    //! returns the names of all face properties
    std::vector<std::string> face_properties() const
    {
        return fprops_.properties();
    }

    //! prints the names of all properties
    void property_stats() const;

    void copy_properties(const SurfaceMesh& src)
    {
        oprops_.copy(src.oprops_);
        vprops_.copy(src.vprops_);
        hprops_.copy(src.hprops_);
        eprops_.copy(src.eprops_);
        fprops_.copy(src.fprops_);
    }

    void copy_fprops(const SurfaceMesh& src, Face src_f, Face target_f)
    {
        constexpr size_t OFFSET = 2; // connectivity, deleted
        fprops_.copy_props(src.fprops_, src_f.idx(), target_f.idx(), OFFSET);
    }
    void copy_vprops(const SurfaceMesh& src, Vertex src_v, Vertex target_v)
    {
        constexpr size_t OFFSET = 3; // point, connectivity, deleted
        vprops_.copy_props(src.vprops_, src_v.idx(), target_v.idx(), OFFSET);
    }
    void copy_eprops(const SurfaceMesh& src, Edge src_e, Edge target_e)
    {
        constexpr size_t OFFSET = 1; // deleted
        eprops_.copy_props(src.eprops_, src_e.idx(), target_e.idx(), OFFSET);
    }
    void copy_hprops(const SurfaceMesh& src, Halfedge src_h, Halfedge target_h)
    {
        constexpr size_t OFFSET = 1; // connectivity
        hprops_.copy_props(src.hprops_, src_h.idx(), target_h.idx(), OFFSET);
    }

    //!@}
    //! \name Iterators and circulators
    //!@{

    //! returns start iterator for vertices
    VertexIterator vertices_begin() const
    {
        return VertexIterator(Vertex(0), this);
    }

    //! returns end iterator for vertices
    VertexIterator vertices_end() const
    {
        return VertexIterator(Vertex(vertices_size()), this);
    }

    //! returns vertex container for C++11 range-based for-loops
    VertexContainer vertices() const
    {
        return VertexContainer(vertices_begin(), vertices_end());
    }

    //! returns start iterator for halfedges
    HalfedgeIterator halfedges_begin() const
    {
        return HalfedgeIterator(Halfedge(0), this);
    }

    //! returns end iterator for halfedges
    HalfedgeIterator halfedges_end() const
    {
        return HalfedgeIterator(Halfedge(halfedges_size()), this);
    }

    //! returns halfedge container for C++11 range-based for-loops
    HalfedgeContainer halfedges() const
    {
        return HalfedgeContainer(halfedges_begin(), halfedges_end());
    }

    //! returns start iterator for edges
    EdgeIterator edges_begin() const { return EdgeIterator(Edge(0), this); }

    //! returns end iterator for edges
    EdgeIterator edges_end() const
    {
        return EdgeIterator(Edge(edges_size()), this);
    }

    //! returns edge container for C++11 range-based for-loops
    EdgeContainer edges() const
    {
        return EdgeContainer(edges_begin(), edges_end());
    }

    //! returns circulator for vertices around vertex \p v
    VertexAroundVertexCirculator vertices(Vertex v) const
    {
        return VertexAroundVertexCirculator(this, v);
    }

    //! returns circulator for outgoing halfedges around vertex \p v
    HalfedgeAroundVertexCirculator halfedges(Vertex v) const
    {
        return HalfedgeAroundVertexCirculator(this, v);
    }

    //! returns start iterator for faces
    FaceIterator faces_begin() const { return FaceIterator(Face(0), this); }

    //! returns end iterator for faces
    FaceIterator faces_end() const
    {
        return FaceIterator(Face(faces_size()), this);
    }

    //! returns face container for C++11 range-based for-loops
    FaceContainer faces() const
    {
        return FaceContainer(faces_begin(), faces_end());
    }

    //! returns circulator for faces around vertex \p v
    FaceAroundVertexCirculator faces(Vertex v) const
    {
        return FaceAroundVertexCirculator(this, v);
    }

    //! returns circulator for vertices of face \p f
    VertexAroundFaceCirculator vertices(Face f) const
    {
        return VertexAroundFaceCirculator(this, f);
    }

    //! returns circulator for halfedges of face \p f
    HalfedgeAroundFaceCirculator halfedges(Face f) const
    {
        return HalfedgeAroundFaceCirculator(this, f);
    }

    //!@}
    //! \name Higher-level Topological Operations
    //!@{

    //! Subdivide the edge \p e = (v0,v1) by splitting it into the two edge
    //! (v0,p) and (p,v1). Note that this function does not introduce any
    //! other edge or faces. It simply splits the edge. Returns halfedge that
    //! points to \p p.
    //! \sa insert_vertex(Edge, Vertex)
    //! \sa insert_vertex(Halfedge, Vertex)
    Halfedge insert_vertex(Edge e, const Point& p)
    {
        return insert_vertex(halfedge(e, 0), add_vertex(p));
    }

    //! Subdivide the edge \p e = (v0,v1) by splitting it into the two edge
    //! (v0,v) and (v,v1). Note that this function does not introduce any
    //! other edge or faces. It simply splits the edge. Returns halfedge
    //! that points to \p p. \sa insert_vertex(Edge, Point) \sa
    //! insert_vertex(Halfedge, Vertex)
    Halfedge insert_vertex(Edge e, Vertex v)
    {
        return insert_vertex(halfedge(e, 0), v);
    }

    //! Subdivide the halfedge \p h = (v0,v1) by splitting it into the two halfedges
    //! (v0,v) and (v,v1). Note that this function does not introduce any
    //! other edge or faces. It simply splits the edge. Returns the halfedge
    //! that points from v1 to \p v.
    //! \sa insert_vertex(Edge, Point)
    //! \sa insert_vertex(Edge, Vertex)
    Halfedge insert_vertex(Halfedge h0, Vertex v);

    //! find the halfedge from start to end
    Halfedge find_halfedge(Vertex start, Vertex end) const;

    //! find the edge (a,b)
    Edge find_edge(Vertex a, Vertex b) const;

    //! returns whether the mesh a triangle mesh. this function simply tests
    //! each face, and therefore is not very efficient.
    //! \sa trianglate(), triangulate(Face)
    bool is_triangle_mesh() const;

    //! returns whether the mesh a quad mesh. this function simply tests
    //! each face, and therefore is not very efficient.
    bool is_quad_mesh() const;

    //! returns whether collapsing the halfedge \p v0v1 is topologically legal.
    //! \attention This function is only valid for triangle meshes.
    bool is_collapse_ok(Halfedge v0v1);

    //! Collapse the halfedge \p h by moving its start vertex into its target
    //! vertex. For non-boundary halfedges this function removes one vertex, three
    //! edges, and two faces. For boundary halfedges it removes one vertex, two
    //! edges and one face.
    //! \attention This function is only valid for triangle meshes.
    //! \attention Halfedge collapses might lead to invalid faces. Call
    //! is_collapse_ok(Halfedge) to be sure the collapse is legal.
    //! \attention The removed items are only marked as deleted. You have
    //! to call garbage_collection() to finally remove them.
    void collapse(Halfedge h);

    //! returns whether removing the edge \p e is topologically legal.
    bool is_removal_ok(Edge e);

    //! Remove edge and merge its two incident faces into one.
    //! This operation requires that the edge has two incident faces
    //! and that these two are not equal.
    bool remove_edge(Edge e);

    //! Split the face \p f by first adding point \p p to the mesh and then
    //! inserting edges between \p p and the vertices of \p f. For a triangle
    //! this is a standard one-to-three split.
    //! \sa split(Face, Vertex)
    Vertex split(Face f, const Point& p)
    {
        Vertex v = add_vertex(p);
        split(f, v);
        return v;
    }

    //! Split the face \p f by inserting edges between \p v and the vertices
    //! of \p f. For a triangle this is a standard one-to-three split.
    //! \sa split(Face, const Point&)
    void split(Face f, Vertex v);

    //! Split the edge \p e by first adding point \p p to the mesh and then
    //! connecting it to the two vertices of the adjacent triangles that are
    //! opposite to edge \p e. Returns the halfedge pointing to \p p that is
    //! created by splitting the existing edge \p e.
    //!
    //! \attention This function is only valid for triangle meshes.
    //! \sa split(Edge, Vertex)
    Halfedge split(Edge e, const Point& p) { return split(e, add_vertex(p)); }

    //! Split the edge \p e by connecting vertex \p v it to the two
    //! vertices of the adjacent triangles that are opposite to edge \c
    //! e. Returns the halfedge pointing to \p v that is created by splitting
    //! the existing edge \p e.
    //!
    //! \attention This function is only valid for triangle meshes.
    //! \sa split(Edge, Point)
    Halfedge split(Edge e, Vertex v);

    //! insert edge between the to-vertices v0 of \p h0 and v1 of \p h1.
    //! returns the new halfedge from v0 to v1.
    //! \attention \p h0 and \p h1 have to belong to the same face
    Halfedge insert_edge(Halfedge h0, Halfedge h1);

    //! Check whether flipping edge \p e is topologically
    //! \attention This function is only valid for triangle meshes.
    //! \sa flip(Edge)
    bool is_flip_ok(Edge e) const;

    //! Flip the edge \p e . Removes the edge \p e and add an edge between the
    //! two vertices opposite to edge \p e of the two incident triangles.
    //! \attention This function is only valid for triangle meshes.
    //! \attention Flipping an edge may result in a non-manifold mesh, hence check
    //! for yourself whether this operation is allowed or not!
    //! \sa is_flip_ok()
    void flip(Edge e);

    //! returns the valence (number of incident edges or neighboring
    //! vertices) of vertex \p v.
    size_t valence(Vertex v) const;

    //! returns the valence of face \p f (its number of vertices)
    size_t valence(Face f) const;

    //! returns the center of face \p f (average position of vertices)
    Point center(Face f) const;

    //! attempts to fix non-manifold vertices by inserting a new vertex per connected patch
    void duplicate_non_manifold_vertices();

    //! removes any broken faces
    void remove_degenerate_faces();

    //! deletes the vertex \p v from the mesh
    void delete_vertex(Vertex v);

    //! deletes the edge \p e from the mesh
    void delete_edge(Edge e);

    //! deletes the face \p f from the mesh
    void delete_face(Face f);

    //! \brief deletes all flagged faces and restores consistency
    //! \details this is _a lot_ faster for deleting large, preferably connected regions of the
    //! mesh than individually calling delete_face for each face. This is because delete_face
    //! needs to return a consistent halfedge structure after every single remove, even if the
    //! restored parts will be removed during a later call.
    //! This method can remove all removable parts first, and only then restore consistency
    //! where it is actually necessary.
    //! Another advantage is that most parts of this can be parallelized with OpenMP, which would
    //! not work for delete_face.
    void delete_many_faces(const FaceProperty<bool>& faces);

    //! Split the mesh into subset meshes. All vertices and edges must be uniquely assigned to
    //! one subset (or PMP_INVALID_INDEX) and may not be shared.
    void split_mesh(std::vector<SurfaceMesh>& output,
                    FaceProperty<IndexType>& face_dist,
                    VertexProperty<IndexType>& vertex_dist,
                    HalfedgeProperty<IndexType>& halfedge_dist);

    //! Split the mesh into subset meshes. Vertices and edges can be shared between subsets.
    void split_mesh(std::vector<SurfaceMesh>& output,
                    FaceProperty<IndexType>& face_dist);

    void join_mesh(const std::vector<SurfaceMesh*>& input);

    //! are there any deleted entities?
    inline bool has_garbage() const { return has_garbage_; }

    //!@}
    //! \name Geometry-related Functions
    //!@{

    //! position of a vertex (read only)
    const Point& position(Vertex v) const { return vpoint_[v]; }

    //! position of a vertex
    Point& position(Vertex v) { return vpoint_[v]; }

    //! \return vector of point positions
    std::vector<Point>& positions() { return vpoint_.vector(); }

    //! compute the bounding box of the object
    BoundingBox bounds() const;

    //! compute the length of edge \p e.
    Scalar edge_length(Edge e) const
    {
        return (vpoint_[vertex(e, 0)] - vpoint_[vertex(e, 1)]).norm();
    }

    //!@}

private:
    //! \name Connectivity Types
    //!@{

    //! This type stores the vertex connectivity
    struct VertexConnectivity
    {
        //! an outgoing halfedge per vertex (it will be a boundary halfedge
        //! for boundary vertices)
        Halfedge halfedge_;
    };

    //! This type stores the halfedge connectivity
    struct HalfedgeConnectivity
    {
        Face face_;              //!< face incident to halfedge
        Vertex vertex_;          //!< vertex the halfedge points to
        Halfedge next_halfedge_; //!< next halfedge
        Halfedge prev_halfedge_; //!< previous halfedge
    };

    //! This type stores the face connectivity
    //! \sa VertexConnectivity, HalfedgeConnectivity
    struct FaceConnectivity
    {
        Halfedge halfedge_; //!< a halfedge that is part of the face
    };

    //!@}

public:
    //! \name Allocate new elements
    //!@{

    //! \brief Allocate a new vertex, resize vertex properties accordingly.
    //! \throw AllocationException in case of failure to allocate a new vertex.
    Vertex new_vertex()
    {
        return new_vertices(1);
    }
    Vertex new_vertices(size_t n)
    {
        if (PMP_MAX_INDEX - 1 - n <= vertices_size())
        {
            auto what = "SurfaceMesh: cannot allocate vertex, max. index reached";
            throw AllocationException(what);
        }
        vprops_.push_back(n);
        return Vertex(vertices_size() - 1);
    }

    //! \brief Allocate a new edge, resize edge and halfedge properties accordingly.
    //! \throw AllocationException in case of failure to allocate a new edge.
    Halfedge new_edge()
    {
        return new_edges(1);
    }
    Halfedge new_edges(size_t n)
    {
        if (PMP_MAX_INDEX - 1 - 2 * n <= halfedges_size())
        {
            auto what = "SurfaceMesh: cannot allocate edge, max. index reached";
            throw AllocationException(what);
        }

        eprops_.push_back(n);
        hprops_.push_back(2 * n);

        return halfedge(Edge(edges_size() - 1), 0);
    }

    //! \brief Allocate a new edge, resize edge and halfedge properties accordingly.
    //! \throw AllocationException in case of failure to allocate a new edge.
    //! \param start starting Vertex of the new edge
    //! \param end end Vertex of the new edge
    Halfedge new_edge(Vertex start, Vertex end)
    {
        assert(start != end);

        Halfedge h0(new_edge());
        Halfedge h1(opposite_halfedge(h0));

        set_vertex(h0, end);
        set_vertex(h1, start);

        return h0;
    }

    //! \brief Allocate a new face, resize face properties accordingly.
    //! \throw AllocationException in case of failure to allocate a new face.
    Face new_face()
    {
        return new_faces(1);
    }
    Face new_faces(size_t n)
    {
        if (PMP_MAX_INDEX - 1 - n <= faces_size())
        {
            auto what = "SurfaceMesh: cannot allocate face, max. index reached";
            throw AllocationException(what);
        }
        fprops_.push_back(n);
        return Face(faces_size() - 1);
    }

    //!@}

private:
    //! \name Helper functions
    //!@{

    //! make sure that the outgoing halfedge of vertex \p v is a boundary
    //! halfedge if \p v is a boundary vertex.
    void adjust_outgoing_halfedge(Vertex v);

    //! Helper for halfedge collapse
    void remove_edge_helper(Halfedge h);

    //! Helper for halfedge collapse
    void remove_loop_helper(Halfedge h);

    Halfedge find_next_candidate(Halfedge h, std::function<bool(Halfedge)> pred) const;
    Halfedge find_prev_candidate(Halfedge h, std::function<bool(Halfedge)> pred) const;

    //!@}
    //! \name Private members
    //!@{

    friend SurfaceMeshIO;

    // property containers for each entity type and object
    PropertyContainer oprops_;
    PropertyContainer vprops_;
    PropertyContainer hprops_;
    PropertyContainer eprops_;
    PropertyContainer fprops_;

    // point coordinates
    VertexProperty<Point> vpoint_;

    // connectivity information
    VertexProperty<VertexConnectivity> vconn_;
    HalfedgeProperty<HalfedgeConnectivity> hconn_;
    FaceProperty<FaceConnectivity> fconn_;

    // markers for deleted entities
    VertexProperty<bool> vdeleted_;
    EdgeProperty<bool> edeleted_;
    FaceProperty<bool> fdeleted_;

    // numbers of deleted entities
    IndexType deleted_vertices_;
    IndexType deleted_edges_;
    IndexType deleted_faces_;

    // indicate garbage present
    bool has_garbage_;

    // helper data for add_face()
    typedef std::pair<Halfedge, Halfedge> NextCacheEntry;
    typedef std::vector<NextCacheEntry> NextCache;
    std::vector<Vertex> add_face_vertices_;
    std::vector<Halfedge> add_face_halfedges_;
    std::vector<bool> add_face_is_new_;
    std::vector<bool> add_face_needs_adjust_;
    NextCache add_face_next_cache_;

    //!@}
};

//!@}

} // namespace pmp
