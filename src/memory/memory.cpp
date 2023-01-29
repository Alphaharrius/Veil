#include "memory/memory.h"

namespace veil::memory {

    Pointer *Allocator::allocate(AllocateRequest &request) {
        return this->management->allocator_allocate(*this, request);
    }

    void Allocator::acquire(PointerAcquireRequest &request) {
        this->management->allocator_acquire(*this, request);
    }

    void Allocator::release(PointerReleaseRequest &request) {
        this->management->allocator_release(*this, request);
    }

    Allocator *Management::create_allocator(diagnostics::Request &request) {
        static Allocator *(Algorithm::*function)(
                Management &management,
                diagnostics::Request &request) = &Algorithm::create_allocator;
        return (this->algorithm->*function)(*this, request);
    }

    Pointer *Management::allocator_allocate(Allocator &allocator, AllocateRequest &request) {
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
