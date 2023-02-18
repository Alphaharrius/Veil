/// This file is part of the Veil distribution (https://github.com/Alphaharrius/Veil).
/// Copyright (c) 2023 Alphaharrius.
///
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, version 3.
///
/// This program is distributed in the hope that it will be useful, but
/// WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
/// General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef VEIL_FABRIC_SRC_MEMORY_MEMORY_HPP
#define VEIL_FABRIC_SRC_MEMORY_MEMORY_HPP

#include "src/typedefs.hpp"

namespace veil::memory {

    // Forward declaration
    class Region;

    /// All VM objects which will be allocated in the process heap should extend this class, this class provides a
    /// generalized backend of how the memory is allocated, and log error and force terminate the process on a failed
    /// allocation.
    class HeapObject {
    public:
        void *operator new(uint64 size);

        void operator delete(void *address);

        void *operator new[](uint64 size) = delete;

        void operator delete[](void *address) = delete;
    };

    /// All VM objects that only allocate on the program stack or embedded directly to its parent object should extend
    /// this class, this class forbids descendants to be allocated to the process heap.
    class ValueObject {
    public:
        void *operator new(uint64 size) = delete;

        void operator delete(void *address) = delete;

        void *operator new[](uint64 size) = delete;

        void operator delete[](void *address) = delete;
    };

    /// All VM objects that allocates to a arena-allocator \c TArena should extend this class.
    class ArenaObject {
    };

    class Arena : public ValueObject {
    public:
        class Iterator;

        static const uint32 DEFAULT_POOL_SIZE = 4096;

        explicit Arena(uint32 pool_size = DEFAULT_POOL_SIZE);

        void *allocate(uint32 size);

        void *inflate(uint32 init_offset);

        void free();

    private:
        uint32 pool_size;
        Region *base;

        friend class Arena::Iterator;
    };

    class Region : public HeapObject {
    public:

        explicit Region(uint32 pool_size);

        ~Region();

        void *allocate(uint32 size);

    private:
        uint64 pool_size;
        uint8 *pool;
        uint8 *bump;

        Region *next;

        friend class Arena;

        friend class Arena::Iterator;
    };

    class Arena::Iterator : public ValueObject {
    public:
        explicit Iterator(Arena &arena);

        void *next(uint64 step);

    private:
        Region *target;
        uint64 offset;
    };

    template<typename T>
    class TArenaIterator;

    template<typename T>
    class TArena {
    public:
        static const uint32 DEFAULT_POOL_LEN = 64;

        explicit TArena(uint32 pool_len = DEFAULT_POOL_LEN);

        T *allocate();

        T *inflate();

        void free();

    private:
        Arena embedded;

        friend class TArenaIterator<T>;
    };

    template<typename T>
    TArena<T>::TArena(uint32 pool_len) : embedded(sizeof(T) * pool_len) {}

    template<typename T>
    T *TArena<T>::allocate() { return static_cast<T *>(embedded.allocate(sizeof(T))); }

    template<typename T>
    T *TArena<T>::inflate() { return embedded.inflate(sizeof(T)); }

    template<typename T>
    void TArena<T>::free() { this->embedded.free(); }

    template<typename T>
    class TArenaIterator : public Arena::Iterator {
    public:
        explicit TArenaIterator(TArena<T> &arena);

        T *next();

    private:
        TArena<T> *arena;
    };

    template<typename T>
    TArenaIterator<T>::TArenaIterator(TArena<T> &arena) : Arena::Iterator(arena.embedded) {}

    template<typename T>
    T *TArenaIterator<T>::next() {
        return static_cast<T *>(this->Arena::Iterator::next(sizeof(T)));
    }

}

#endif //VEIL_FABRIC_SRC_MEMORY_MEMORY_HPP
