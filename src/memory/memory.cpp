#include "memory/memory.h"

namespace veil::memory {

    Pointer *Allocator::allocate(AllocateRequest &request) {
        static Pointer *(Algorithm::*function)(
                Allocator &allocator,
                AllocateRequest &request) = &Algorithm::allocator_pointer_allocate;
        return (this->management->algorithm->*function)(*this, request);
    }

    void Allocator::acquire(PointerAcquireRequest &request) {
        static void (Algorithm::*function)(
                Allocator &allocator,
                PointerAcquireRequest &request) = &Algorithm::allocator_pointer_acquire;
        (this->management->algorithm->*function)(*this, request);
    }

    void Allocator::reserve(PointerActionRequest &request) {
        static void (Algorithm::*function)(
                Allocator &allocator,
                PointerActionRequest &request) = &Algorithm::allocator_pointer_reserve;
        (this->management->algorithm->*function)(*this, request);
    }

    void Allocator::release(PointerActionRequest &request) {
        static void (Algorithm::*function)(
                Allocator &allocator,
                PointerActionRequest &request) = &Algorithm::allocator_pointer_release;
        (this->management->algorithm->*function)(*this, request);
    }

    Allocator *Management::create_allocator(util::Request &request) {
        static Allocator *(Algorithm::*function)(
                Management &management,
                util::Request &request) = &Algorithm::create_allocator;
        return (this->algorithm->*function)(*this, request);
    }

}
