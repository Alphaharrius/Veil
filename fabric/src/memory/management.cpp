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

#include "src/memory/management.hpp"
#include "src/errors.hpp"
#include "src/memory/os.hpp"
#include "src/memory/config.hpp"

using namespace veil::memory;

Allocator::Allocator(Management &management) : vm::Constituent<Management>(management) {}

Allocator *Management::create_allocator(vm::Request &request) {
    return this->algorithm->create_allocator(*this, request);
}

Management *Management::new_instance(Runtime &runtime, MemoryInitRequest &request) {
    if (!request.algorithm) {
        vm::RequestConsumer::set_error(request, memory::ERR_NO_ALGO);
        return nullptr;
    }

    uint32 host_page_size = os::get_page_size();
    // Ensure that the max heap size is a multiple of the host page size.
    uint64 max_heap_size = config::max_heap_size % host_page_size ?
                           config::max_heap_size + host_page_size : config::max_heap_size;
    // Ensure the adjusted max heap size is supported by the algorithm.
    if (max_heap_size > request.algorithm->max_supported_heap_size()) {
        vm::RequestConsumer::set_error(request, memory::ERR_INV_HEAP_SIZE);
        return nullptr;
    }

    auto *management = new Management(runtime, request.algorithm, max_heap_size);

    AlgorithmInitRequest algo_request(management, request.algorithm_params);
    request.algorithm->initialize(algo_request);
    if (!algo_request.is_ok()) {
        // The management object can be deleted directly as the only injected allocated memory is stored within
        // Management::structure, which is not allocated in case of failure.
        delete management;
        vm::RequestConsumer::set_error(request, algo_request.get_error());
        return nullptr;
    }
    return management;
}

Management::Management(Runtime &runtime, Algorithm *algorithm, uint64 max_heap_size) :
        vm::Constituent<Runtime>(runtime),
        MAX_HEAP_SIZE(max_heap_size),
        algorithm(algorithm),
        mapped_heap_size(0),
        structure(nullptr) {}

void Management::heap_map(HeapMapRequest &request) {
    // Increment atomically by the request size.
    uint64 current_mapped_size = this->mapped_heap_size.fetch_add(request.size);
    // The total mapped size from the host should not be greater than the limit.
    if (current_mapped_size > this->MAX_HEAP_SIZE) {
        vm::RequestConsumer::set_error(request, memory::ERR_HEAP_OVERFLOW);
        return;
    }
    uint32 error;
    request.address = static_cast<uint8 *>(os::mmap(nullptr, request.size, true, true, error));
    switch (error) {
    case os::ERR_NOMEM: vm::RequestConsumer::set_error(request, memory::ERR_HOST_NOMEM);
    case veil::ERR_NONE:
    default: break;
    }
}

std::string Management::get_error_info(uint32 status) {
    // TODO: Implement error info retrieval process.
    return "";
}

Pointer::Pointer(uint32 size) : size(size) {}

MemoryInitRequest::MemoryInitRequest(
        Algorithm *algorithm,
        void *algorithm_params) :
        algorithm(algorithm), algorithm_params(algorithm_params) {}

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

AlgorithmInitRequest::AlgorithmInitRequest(Management *management, void *algorithm_params) :
        management(management),
        algorithm_params(algorithm_params) {}
