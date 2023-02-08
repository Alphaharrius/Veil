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

    Management *Management::new_instance(ManagementInitRequest &request) {
        if (!request.algorithm) {
            util::RequestConsumer::set_error(request, memory::ERR_NO_ALGO);
            return nullptr;
        }

        uint32 host_page_size = natives::get_page_size();
        // Ensure that the max heap size is a multiple of the host page size.
        uint64 max_heap_size = request.max_heap_size % host_page_size ?
                               request.max_heap_size + host_page_size : request.max_heap_size;
        // Ensure the adjusted max heap size is supported by the algorithm.
        if (max_heap_size > request.algorithm->max_supported_heap_size()) {
            util::RequestConsumer::set_error(request, memory::ERR_INV_HEAP_SIZE);
            return nullptr;
        }

        auto *management = new Management(request.algorithm, request.max_heap_size);

        AlgorithmInitRequest algo_request(management, request.max_heap_size, request.algorithm_params);
        request.algorithm->initialize(algo_request);
        if (!algo_request.is_ok()) {
            // The management object can be deleted directly as the only injected allocated memory is stored within
            // Management::structure, which is not allocated in case of failure.
            delete management;
            util::RequestConsumer::set_error(request, algo_request.get_error());
            return nullptr;
        }
        return management;
    }

    Management::Management(Algorithm *algorithm, uint64 max_heap_size) : MAX_HEAP_SIZE(max_heap_size),
                                                                         algorithm(algorithm),
                                                                         mapped_heap_size(0),
                                                                         structure(nullptr) {}

    void Management::heap_map(HeapMapRequest &request) {
        // Increment atomically by the request size.
        uint64 current_mapped_size = this->mapped_heap_size.fetch_add(request.size);
        // The total mapped size from the host should not be greater than the limit.
        if (current_mapped_size > this->MAX_HEAP_SIZE) {
            util::RequestConsumer::set_error(request, memory::ERR_HEAP_OVERFLOW);
            return;
        }
        natives::Mmap m(nullptr, request.size, true, true);
        if (!m.access()) {
            switch (m.get_error()) {
                case natives::ERR_NOMEM:
                    util::RequestConsumer::set_error(request, memory::ERR_HOST_NOMEM);
            }
            return;
        }
        request.address = m.get_result();
    }

    std::string Management::get_error_info(uint32 status) {
        // TODO: Implement error info retrieval process.
        return "";
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

    AlgorithmInitRequest::AlgorithmInitRequest(Management *management, uint64 max_heap_size, void *algorithm_params) :
            management(management),
            max_heap_size(max_heap_size),
            algorithm_params(algorithm_params) {}

}
