// Copyright 2011-2020 the Polygon Mesh Processing Library developers.
// Copyright 2001-2005 by Computer Graphics Group, RWTH Aachen
// Distributed under a MIT-style license, see PMP_LICENSE.txt for details.

#pragma once

#include <cassert>

#include <string>
#include <vector>
#include <algorithm>
#include <typeinfo>
#include <iostream>

namespace pmp {

class BasePropertyArray
{
public:
    //! Default constructor
    BasePropertyArray(const std::string& name) : name_(name) {}

    //! Destructor.
    virtual ~BasePropertyArray() {}

    //! Reserve memory for n elements.
    virtual void reserve(size_t n) = 0;

    //! Resize storage to hold n elements.
    virtual void resize(size_t n) = 0;

    //! Free unused memory.
    virtual void free_memory() = 0;

    //! Extend the number of elements by n.
    virtual void push_back(size_t n = 1) = 0;

    //! Let two elements swap their storage place.
    virtual void swap(size_t i0, size_t i1) = 0;

    //! Copy an element from another array.
    virtual void copy_prop(BasePropertyArray* src, size_t src_i, size_t dst_i) = 0;

    //! Return a deep copy of self.
    virtual BasePropertyArray* clone() const = 0;

    //! Return an empty copy of self.
    virtual BasePropertyArray* empty_copy() const = 0;

    //! Return the type_info of the property
    virtual const std::type_info& type() = 0;

    //! Return the name of the property
    const std::string& name() const { return name_; }

protected:
    std::string name_;
};

template <class T>
class PropertyArray : public BasePropertyArray
{
public:
    typedef T ValueType;
    typedef std::vector<ValueType> VectorType;
    typedef typename VectorType::reference reference;
    typedef typename VectorType::const_reference const_reference;

    PropertyArray(const std::string& name, T t = T())
        : BasePropertyArray(name), value_(t)
    {
    }

public: // interface of BasePropertyArray
    void reserve(size_t n) override { data_.reserve(n); }

    void resize(size_t n) override { data_.resize(n, value_); }

    void push_back(size_t n = 1) override { data_.insert(data_.end(), n, value_); }

    void free_memory() override { data_.shrink_to_fit(); }

    void swap(size_t i0, size_t i1) override
    {
        std::swap(data_[i0], data_[i1]);
    }

    void copy_prop(BasePropertyArray* src, size_t src_i, size_t dst_i) override
    {
        PropertyArray<T>* src_prop = dynamic_cast<PropertyArray<T>*>(src);
        assert(src_prop);
        data_[dst_i] = src_prop->data_[src_i];
    }

    BasePropertyArray* clone() const override
    {
        PropertyArray<T>* p = new PropertyArray<T>(name_, value_);
        p->data_ = data_;
        return p;
    }

    BasePropertyArray* empty_copy() const override
    {
        return new PropertyArray<T>(name_, value_);
    }

    const std::type_info& type() override { return typeid(T); }

public:
    //! Get pointer to array (does not work for T==bool)
    T* data() { return data_.data(); }

    //! Get pointer to array (does not work for T==bool)
    const T* data() const { return data_.data(); }

    //! Get reference to the underlying vector
    std::vector<T>& vector() { return data_; }

    //! Get reference to the underlying vector
    const std::vector<T>& vector() const { return data_; }

    //! Access the i'th element. No range check is performed!
    reference operator[](size_t idx)
    {
        assert(idx < data_.size());
        return data_[idx];
    }

    //! Const access to the i'th element. No range check is performed!
    const_reference operator[](size_t idx) const
    {
        assert(idx < data_.size());
        return data_[idx];
    }

private:
    VectorType data_;
    ValueType value_;
};

// specialization for bool properties
// std::vector<bool> is a specialization that uses one bit per element, which does not allow data() access
template <>
inline bool* PropertyArray<bool>::data()
{
    throw std::runtime_error("PropertyArray<bool>::data() not supported");
}
template <>
inline const bool* PropertyArray<bool>::data() const
{
    throw std::runtime_error("PropertyArray<bool>::data() not supported");
}
template <>
inline void PropertyArray<bool>::swap(size_t i0, size_t i1)
{
    data_.swap(data_[i0], data_[i1]);
}

template <class T>
class Property
{
public:
    typedef typename PropertyArray<T>::reference reference;
    typedef typename PropertyArray<T>::const_reference const_reference;

    friend class PropertyContainer;
    friend class SurfaceMesh;

public:
    Property(PropertyArray<T>* p = nullptr) : parray_(p) {}

    void reset() { parray_ = nullptr; }

    operator bool() const { return parray_ != nullptr; }

    reference operator[](size_t i)
    {
        assert(parray_ != nullptr);
        return (*parray_)[i];
    }

    const_reference operator[](size_t i) const
    {
        assert(parray_ != nullptr);
        return (*parray_)[i];
    }

    T* data()
    {
        assert(parray_ != nullptr);
        return parray_->data();
    }

    const T* data() const
    {
        assert(parray_ != nullptr);
        return parray_->data();
    }

    std::vector<T>& vector()
    {
        assert(parray_ != nullptr);
        return parray_->vector();
    }

    const std::vector<T>& vector() const
    {
        assert(parray_ != nullptr);
        return parray_->vector();
    }

    const std::string& name() const
    {
        assert(parray_ != nullptr);
        return parray_->name();
    }

    PropertyArray<T>& array()
    {
        assert(parray_ != nullptr);
        return *parray_;
    }

    const PropertyArray<T>& array() const
    {
        assert(parray_ != nullptr);
        return *parray_;
    }

private:
    PropertyArray<T>* parray_;
};

template <class T>
class ConstProperty
{
public:
    typedef typename PropertyArray<T>::const_reference const_reference;

    friend class PropertyContainer;
    friend class SurfaceMesh;

public:
    ConstProperty(const PropertyArray<T>* p = nullptr) : parray_(p) {}
    ConstProperty(const Property<T>& p) : ConstProperty(&p.array()) {}

    void reset() { parray_ = nullptr; }

    operator bool() const { return parray_ != nullptr; }

    const_reference operator[](size_t i) const
    {
        assert(parray_ != nullptr);
        return (*parray_)[i];
    }

    const T* data() const
    {
        assert(parray_ != nullptr);
        return parray_->data();
    }

    const std::vector<T>& vector()
    {
        assert(parray_ != nullptr);
        return parray_->vector();
    }

    const std::string& name() const
    {
        assert(parray_ != nullptr);
        return parray_->name();
    }

    const PropertyArray<T>& array() const
    {
        assert(parray_ != nullptr);
        return *parray_;
    }

private:
    const PropertyArray<T>* parray_;
};

class PropertyContainer
{
public:
    // default constructor
    PropertyContainer() : size_(0) {}

    // destructor (deletes all property arrays)
    virtual ~PropertyContainer() { clear(); }

    // copy constructor: performs deep copy of property arrays
    PropertyContainer(const PropertyContainer& rhs) { operator=(rhs); }

    // move constructor
    PropertyContainer(PropertyContainer&& rhs) = default;

    // assignment: performs deep copy of property arrays
    PropertyContainer& operator=(const PropertyContainer& rhs)
    {
        if (this != &rhs)
        {
            clear();
            parrays_.resize(rhs.n_properties());
            size_ = rhs.size();
            for (size_t i = 0; i < parrays_.size(); ++i)
                parrays_[i] = rhs.parrays_[i]->clone();
        }
        return *this;
    }

    // move assignment
    PropertyContainer& operator=(PropertyContainer&& rhs) = default;

    // returns the current size of the property arrays
    size_t size() const { return size_; }

    // returns the number of property arrays
    size_t n_properties() const { return parrays_.size(); }

    // returns a vector of all property names
    std::vector<std::string> properties() const
    {
        std::vector<std::string> names;
        for (size_t i = 0; i < parrays_.size(); ++i)
            names.push_back(parrays_[i]->name());
        return names;
    }

    // add a property with name \p name and default value \p t
    template <class T>
    Property<T> add(const std::string& name, const T t = T())
    {
        // if a property with this name already exists, return an invalid property
        for (size_t i = 0; i < parrays_.size(); ++i)
        {
            if (parrays_[i]->name() == name)
            {
                std::cerr << "[PropertyContainer] A property with name \""
                          << name
                          << "\" already exists. Returning invalid property.\n";
                return Property<T>();
            }
        }

        // otherwise add the property
        PropertyArray<T>* p = new PropertyArray<T>(name, t);
        p->resize(size_);
        parrays_.push_back(p);
        return Property<T>(p);
    }

    // do we have a property with a given name?
    bool exists(const std::string& name) const
    {
        for (size_t i = 0; i < parrays_.size(); ++i)
            if (parrays_[i]->name() == name)
                return true;
        return false;
    }

    // get a property by its name. returns invalid property if it does not exist.
    template <class T>
    Property<T> get(const std::string& name)
    {
        for (size_t i = 0; i < parrays_.size(); ++i)
            if (parrays_[i]->name() == name)
                return Property<T>(
                    dynamic_cast<PropertyArray<T>*>(parrays_[i]));
        return Property<T>();
    }

    // get a property by its name. returns invalid property if it does not exist.
    template <class T>
    ConstProperty<T> get(const std::string& name) const
    {
        for (size_t i = 0; i < parrays_.size(); ++i)
            if (parrays_[i]->name() == name)
                return ConstProperty<T>(dynamic_cast<const PropertyArray<T>*>(parrays_[i]));
        return ConstProperty<T>();
    }

    // returns a property if it exists, otherwise it creates it first.
    template <class T>
    Property<T> get_or_add(const std::string& name, const T t = T())
    {
        Property<T> p = get<T>(name);
        if (!p)
            p = add<T>(name, t);
        return p;
    }

    // get the type of property by its name. returns typeid(void) if it does not exist.
    const std::type_info& get_type(const std::string& name)
    {
        for (size_t i = 0; i < parrays_.size(); ++i)
            if (parrays_[i]->name() == name)
                return parrays_[i]->type();
        return typeid(void);
    }

    // delete a property
    template <class T>
    void remove(Property<T>& h)
    {
        std::vector<BasePropertyArray*>::iterator it = parrays_.begin(),
                                                  end = parrays_.end();
        for (; it != end; ++it)
        {
            if (*it == h.parray_)
            {
                delete *it;
                parrays_.erase(it);
                h.reset();
                break;
            }
        }
    }

    // delete all properties
    void clear()
    {
        for (size_t i = 0; i < parrays_.size(); ++i)
            delete parrays_[i];
        parrays_.clear();
        size_ = 0;
    }

    // reserve memory for n entries in all arrays
    void reserve(size_t n)
    {
        for (size_t i = 0; i < parrays_.size(); ++i)
            parrays_[i]->reserve(n);
    }

    // resize all arrays to size n
    void resize(size_t n)
    {
        for (size_t i = 0; i < parrays_.size(); ++i)
            parrays_[i]->resize(n);
        size_ = n;
    }

    // free unused space in all arrays
    void free_memory()
    {
        for (size_t i = 0; i < parrays_.size(); ++i)
            parrays_[i]->free_memory();
    }

    // add a new element to each vector
    void push_back(size_t n = 1)
    {
        for (size_t i = 0; i < parrays_.size(); ++i)
            parrays_[i]->push_back(n);
        size_ += n;
    }

    // swap elements i0 and i1 in all arrays
    void swap(size_t i0, size_t i1)
    {
        for (size_t i = 0; i < parrays_.size(); ++i)
            parrays_[i]->swap(i0, i1);
    }

    void copy(const PropertyContainer& src)
    {
        std::unordered_map<std::string, size_t> map;
        for (size_t i = 0; i < parrays_.size(); ++i)
            map[parrays_[i]->name()] = i;

        for (auto& src_prop : src.parrays_)
        {
            if (map.find(src_prop->name()) == map.end())
            {
                BasePropertyArray* p = src_prop->empty_copy();
                p->resize(size_);
                parrays_.push_back(p);
            }
        }
    }

    /// maps from src->index to this->index
    using IndexMap = std::vector<std::pair<IndexType, IndexType>>;
    IndexMap gen_map(const PropertyContainer& src, size_t offset) const
    {
        std::unordered_map<std::string, size_t> map;
        for (size_t i = 0; i < parrays_.size(); ++i)
            map[parrays_[i]->name()] = i;

        IndexMap ret;
        for (size_t src_i = offset; src_i < src.parrays_.size(); ++src_i)
        {
            auto it = map.find(src.parrays_[src_i]->name());
            if (it != map.end())
                ret.emplace_back(src_i, it->second);
        }
        return ret;
    }

    void copy_props(const PropertyContainer& src, size_t src_i, size_t target_i, const IndexMap& map)
    {
        for (auto& [ a, b ] : map)
        {
            parrays_[b]->copy_prop(src.parrays_[a], src_i, target_i);
        }
    }

private:
    std::vector<BasePropertyArray*> parrays_;
    size_t size_;
};

} // namespace pmp
