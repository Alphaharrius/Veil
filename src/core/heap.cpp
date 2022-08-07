#include "core/heap.h"
#include "util/diagnostics.h"
#include "util/natives.h"

bool veil::Allocation::is_allocated() const {
    return this->address != nullptr && this->size != 0;
}

veil::AllocationPool::AllocationPool(uint8 *allocation_address, uint32 allocation_size) {
    not_null(allocation_address);
    assert(allocation_size > 0);

    this->allocation_address = allocation_address;
    this->allocation_offset = allocation_address;
    this->allocation_size = allocation_size;
    this->allocation_ceiling = allocation_address + allocation_size;
}

void veil::AllocationPool::allocate(veil::Allocation *allocation) {
    not_null(allocation);

    if (allocation->size == 0 || this->allocation_offset + allocation->size < this->allocation_ceiling) {
        return;
    }
    allocation->address = this->allocation_offset;
    this->allocation_offset += allocation->size;
}

void veil::AllocationPool::reuse(veil::AllocationPool reset_info) {
    this->allocation_address = reset_info.allocation_address;
    this->allocation_offset = reset_info.allocation_offset;
    this->allocation_ceiling = reset_info.allocation_ceiling;
    this->allocation_size = reset_info.allocation_size;
}

void veil::AllocationPool::reset() {
    this->allocation_offset = this->allocation_address;
}

veil::HeapCreationInfo::HeapCreationInfo(uint64 memory_size, uint32 internal_allocation_pool_size) {
    this->memory_size = memory_size;
    this->internal_allocation_pool_size = internal_allocation_pool_size;
    this->error = HeapCreationInfo::Error::NONE;
}

veil::Heap::Heap(HeapCreationInfo &creation_info) {
    if (creation_info.memory_size == 0 || creation_info.memory_size % natives::get_page_size() != 0) {
        creation_info.error = HeapCreationInfo::Error::INVALID_MEMORY_SIZE;
        return;
    }
    if (creation_info.internal_allocation_pool_size == 0 ||
        creation_info.memory_size % creation_info.internal_allocation_pool_size != 0) {
        creation_info.error = HeapCreationInfo::Error::INVALID_INTERNAL_ALLOCATION_POOL_SIZE;
        return;
    }
    void *allocated_address = veil::natives::virtual_allocate(creation_info.memory_size,
                                                              creation_info.allocation_status);
    if (!creation_info.allocation_status.success) {
        creation_info.error = HeapCreationInfo::Error::ALLOCATION_ERROR;
        return;
    }
    this->memory_address = static_cast<uint8 *>(allocated_address);
    this->memory_offset = this->memory_address;
    this->memory_ceiling = this->memory_address + creation_info.memory_size;
    this->memory_size = creation_info.memory_size;
    this->internal_allocation_pool_size = creation_info.internal_allocation_pool_size;
}

veil::Heap::~Heap() {

}

veil::AllocationPool *veil::Heap::allocate_internal_allocation_pool() {
    return nullptr;
}

veil::AllocationPool *veil::Heap::allocate_external_allocation_pool(uint64 request_size,
                                                                    natives::OperationStatus &allocation_status) {
    return nullptr;
}
