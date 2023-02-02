#include "memory/memory.h"

namespace veil::memory {

    Pointer *Allocator::allocate(AllocateRequest &request) {
        static Pointer *(Algorithm::*function)(
                Allocator &allocator,
                AllocateRequest &request) = &Algorithm::allocator_allocate;
        return (this->algorithm->*function)(allocator, request);
    }

    void Management::allocator_acquire(Allocator &allocator, PointerAcquireRequest &request) {
        static void (Algorithm::*function)(
                Allocator &allocator,
                PointerAcquireRequest &request) = &Algorithm::allocator_acquire;
        (this->algorithm->*function)(allocator, request);
    }

    void Management::allocator_release(Allocator &allocator, PointerReleaseRequest &request) {
        static void (Algorithm::*function)(
                Allocator &allocator,
                PointerReleaseRequest &request) = &Algorithm::allocator_release;
        (this->algorithm->*function)(allocator, request);
    }

}
