#include "memory/memory.h"
#include "errors.h"

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

#include "resource-impl.h"
#include "natives.h"

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

    Management::Management(Algorithm *algorithm, uint64 max_heap_size, void *structure) : algorithm(algorithm),
                                                                                          MAX_HEAP_SIZE(max_heap_size),
                                                                                          structure(structure),
                                                                                          mapped_heap_size(0) {}

    void Management::heap_map(HeapMapRequest &request) {
        // Increment atomically by the request size.
        uint64 current_mapped_size = this->mapped_heap_size.fetch_add(request.size);
        // The total mapped size from the host should not be greater than the limit.
        if (current_mapped_size > this->MAX_HEAP_SIZE) {
            request.error = memory::ERR_HEAP_OVERFLOW;
            return;
        }
        natives::Mmap m(nullptr, request.size, true, true);
        if (!m.access()) {
            switch (m.get_error()) {
                case natives::ERR_NOMEM: request.error = memory::ERR_HOST_NOMEM;
            }
            return;
        }
        request.address = m.get_result();
    }

    Pointer::Pointer(uint32 size) : size(size) {}

    Allocator::Allocator(Management &management) : management(&management) {}

    const Management *Allocator::get_management() {
        return this->management;
    }

    ManagementInitRequest::ManagementInitRequest(uint64 max_heap_size, Algorithm *algorithm, void *algorithm_params)
            : max_heap_size(max_heap_size), algorithm(algorithm), algorithm_params(algorithm_params) {
    }

    AllocateRequest::AllocateRequest(uint64 size) : size(size) {}

    PointerActionRequest::PointerActionRequest(Pointer *pointer) : pointer(pointer) {}

    PointerAcquireRequest::PointerAcquireRequest(
            Pointer *pointer, bool exclusive) : pointer(pointer), exclusive(exclusive), address(nullptr) {}

    uint8 *PointerAcquireRequest::get_address() {
        return this->address;
    }

    HeapMapRequest::HeapMapRequest(uint64 size) : AllocateRequest(size) {}

    uint8 *HeapMapRequest::get_address() {
        return this->address;
    }

}
