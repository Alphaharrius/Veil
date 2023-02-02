#include "memory/memory.h"

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

    Pointer::Pointer(uint8 *address, uint32 size) : address(address), size(size) {}

    Allocator::Allocator(Management &management) : management(&management) {}

    ManagementInitRequest::ManagementInitRequest(uint64 heap_memory_size, Algorithm *algorithm, void *algorithm_params)
            : heap_memory_size(heap_memory_size), algorithm(algorithm), algorithm_params(algorithm_params) {
    }

    AllocateRequest::AllocateRequest(uint32 size) : size(size) {}

    PointerActionRequest::PointerActionRequest(Pointer *pointer) : pointer(pointer) {}

    PointerAcquireRequest::PointerAcquireRequest(
            Pointer *pointer, bool exclusive) : pointer(pointer), exclusive(exclusive), address(nullptr) {}

}
