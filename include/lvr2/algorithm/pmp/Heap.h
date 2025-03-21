// Copyright 2011-2019 the Polygon Mesh Processing Library developers.
// Distributed under a MIT-style license, see PMP_LICENSE.txt for details.

#pragma once

#include <vector>

namespace pmp {

//! \brief A class implementing a heap.
//! \ingroup algorithms
template <class HeapEntry, class HeapInterface>
class Heap : private std::vector<HeapEntry>
{
public:
    typedef Heap<HeapEntry, HeapInterface> This;

    //! Constructor
    Heap() : HeapVector() {}

    //! Construct with a given \p HeapInterface.
    Heap(const HeapInterface& i) : HeapVector(), interface_(i) {}

    //! Destructor.
    ~Heap(){};

    //! clear the heap
    void clear() { HeapVector::clear(); }

    //! is heap empty?
    bool empty() { return HeapVector::empty(); }

    //! returns the size of heap
    unsigned int size() { return (unsigned int)HeapVector::size(); }

    //! reserve space for N entries
    void reserve(unsigned int n) { HeapVector::reserve(n); }

    //! reset heap position to -1 (not in heap)
    void reset_heap_position(HeapEntry h)
    {
        interface_.set_heap_position(h, -1);
    }

    //! is an entry in the heap?
    bool is_stored(HeapEntry h)
    {
        return interface_.get_heap_position(h) != -1;
    }

    //! insert the entry h
    void insert(HeapEntry h)
    {
        This::push_back(h);
        upheap(size() - 1);
    }

    //! get the first entry
    HeapEntry front()
    {
        assert(!empty());
        return entry(0);
    }

    //! delete the first entry
    void pop_front()
    {
        assert(!empty());
        interface_.set_heap_position(entry(0), -1);
        if (size() > 1)
        {
            entry(0, entry(size() - 1));
            HeapVector::resize(size() - 1);
            downheap(0);
        }
        else
            HeapVector::resize(size() - 1);
    }

    //! remove an entry
    void remove(HeapEntry h)
    {
        int pos = interface_.get_heap_position(h);
        interface_.set_heap_position(h, -1);

        assert(pos != -1);
        assert((unsigned int)pos < size());

        // last item ?
        if ((unsigned int)pos == size() - 1)
            HeapVector::resize(size() - 1);

        else
        {
            entry(pos, entry(size() - 1)); // move last elem to pos
            HeapVector::resize(size() - 1);
            downheap(pos);
            upheap(pos);
        }
    }

    //! update an entry: change the key and update the position to
    //! reestablish the heap property.
    void update(HeapEntry h)
    {
        int pos = interface_.get_heap_position(h);
        assert(pos != -1);
        assert((unsigned int)pos < size());
        downheap(pos);
        upheap(pos);
    }

    //! \brief Check heap condition.
    //! \return \c true if heap condition is satisfied, \c false if not.
    bool check()
    {
        bool ok(true);
        unsigned int i, j;
        for (i = 0; i < size(); ++i)
        {
            if (((j = left(i)) < size()) &&
                interface_.greater(entry(i), entry(j)))
            {
                ok = false;
            }
            if (((j = right(i)) < size()) &&
                interface_.greater(entry(i), entry(j)))
            {
                ok = false;
            }
        }
        return ok;
    }

private:
    // typedef
    typedef std::vector<HeapEntry> HeapVector;

    //! Upheap. Establish heap property.
    void upheap(unsigned int idx)
    {
        HeapEntry h = entry(idx);
        unsigned int parentIdx;

        while ((idx > 0) && interface_.less(h, entry(parentIdx = parent(idx))))
        {
            entry(idx, entry(parentIdx));
            idx = parentIdx;
        }

        entry(idx, h);
    }

    //! Downheap. Establish heap property.
    void downheap(unsigned int idx)
    {
        HeapEntry h = entry(idx);
        unsigned int childIdx;
        unsigned int s = size();

        while (idx < s)
        {
            childIdx = left(idx);
            if (childIdx >= s)
                break;

            if ((childIdx + 1 < s) &&
                (interface_.less(entry(childIdx + 1), entry(childIdx))))
                ++childIdx;

            if (interface_.less(h, entry(childIdx)))
                break;

            entry(idx, entry(childIdx));
            idx = childIdx;
        }

        entry(idx, h);
    }

    //! Get the entry at index idx
    inline HeapEntry entry(unsigned int idx)
    {
        assert(idx < size());
        return (This::operator[](idx));
    }

    //! Set entry H to index idx and update H's heap position.
    inline void entry(unsigned int idx, HeapEntry h)
    {
        assert(idx < size());
        This::operator[](idx) = h;
        interface_.set_heap_position(h, idx);
    }

    //! Get parent's index
    inline unsigned int parent(unsigned int i) { return (i - 1) >> 1; }

    //! Get left child's index
    inline unsigned int left(unsigned int i) { return (i << 1) + 1; }

    //! Get right child's index
    inline unsigned int right(unsigned int i) { return (i << 1) + 2; }

    //! Instance of HeapInterface
    HeapInterface interface_;
};

} // namespace pmp
