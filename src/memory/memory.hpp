#ifndef VEIL_MEMORY_HPP
#define VEIL_MEMORY_HPP

#include "typedefs.hpp"
#include "os.hpp"

namespace veil::memory {

    class Region;

    class HeapObject {
    public:
        void *operator new(unsigned long long size);
        void operator delete(void *address);

        void *operator new[](unsigned long long size) = delete;
        void operator delete[](void *address) = delete;
    };

     class ValueObject {
     public:
         void *operator new(unsigned long long size) = delete;
         void operator delete(void *address) = delete;

         void *operator new[](unsigned long long size) = delete;
         void operator delete[](void *address) = delete;
     };

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

    template <typename T>
    class TArenaIterator;

    template<typename T>
    class TArena : public ValueObject {
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

    template <typename T>
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

#endif //VEIL_MEMORY_HPP
