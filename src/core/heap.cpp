#include "core/heap.h"
#include "util/natives.h"

uint8 *veil::HeapSection::allocate(uint32 request_size) {
    if (request_size == 0 || this->allocation_offset + request_size >= this->allocation_ceiling + 1) {
        return nullptr;
    }
    uint8 *allocated_address = this->allocation_offset;
    memset((void *) allocated_address, 0, request_size);
    this->allocation_offset += request_size;
    return allocated_address;
}

void veil::HeapSection::reuse(veil::HeapSection reset_info) {
    this->allocation_address = reset_info.allocation_address;
    this->allocation_offset = reset_info.allocation_offset;
    this->allocation_ceiling = reset_info.allocation_ceiling;
    this->allocation_size = reset_info.allocation_size;
}

void veil::HeapSection::reset() {
    this->allocation_offset = this->allocation_address;
}

veil::Heap::Heap(HeapCreationInfo &creation_info) {
    if (creation_info.memory_size == 0 || creation_info.memory_size % natives::get_page_size() != 0) {
        creation_info.error = HeapCreationInfo::Error::INVALID_MEMORY_SIZE;
        return;
    }
    if (creation_info.section_size == 0 ||
        creation_info.memory_size % creation_info.section_size != 0) {
        creation_info.error = HeapCreationInfo::Error::INVALID_INTERNAL_SECTION_SIZE;
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
    this->section_size = creation_info.section_size;
}

veil::Heap::~Heap() {
    if (this->memory_address != nullptr) {
        natives::OperationStatus operation_status;
        natives::virtual_free(this->memory_address, this->memory_size, operation_status);
    }
}

veil::HeapSection *veil::Heap::allocate_section() {
    if (this->memory_offset + this->section_size < this->memory_ceiling + 1) {
        auto *internal_pool = new HeapSection(this->memory_offset,
                                              this->section_size);
        this->memory_offset += this->section_size;
        return internal_pool;
    }
    return nullptr;
}

veil::concurrent::Synchronizable &veil::Heap::get_allocation_lock() {
    return this->allocation_lock;
}

veil::MemoryReference::MemoryReference(uint8 *address) {
    this->colored_reference = 0ULL;
    this->update_address(address);
}

uint64 veil::MemoryReference::get_reference() const {
    return this->colored_reference;
}

uint8 *veil::MemoryReference::get_address() const {
    return (uint8 *) (this->colored_reference & MemoryReference::ADDRESS_BIT_MASK);
}

void veil::MemoryReference::update_address(const uint8 *address) {
    uint64 masked_reference = ((uint64) address) & MemoryReference::ADDRESS_BIT_MASK;
    this->colored_reference &= ~MemoryReference::ADDRESS_BIT_MASK;
    this->colored_reference |= masked_reference;
}

void veil::MemoryReference::set_color(uint8 position, bool value) {
    uint64 mask = (1ULL << (MemoryReference::ADDRESS_BIT_COUNT + position));
    if (value) {
        this->colored_reference |= mask;
    } else {
        this->colored_reference &= ~mask;
    }
}

bool veil::MemoryReference::get_color(uint8 position) const {
    uint64 mask = (1ULL << (MemoryReference::ADDRESS_BIT_COUNT + position));
    return this->colored_reference & mask;
}

uint16 *veil::MemoryReference::color_bits() {
    return ((uint16 *) &this->colored_reference) + 3;
}

veil::MemoryReference *veil::ReferenceTable::get_reference() {
    return (veil::MemoryReference *) this->allocate(sizeof(veil::MemoryReference));
}

veil::MemoryReference *veil::ReferenceTable::reuse_reference(uint32 memory_size) {
    veil::MemoryReference *reusable = nullptr;
    for (auto *current = (veil::MemoryReference *) this->allocation_address;
         current < (veil::MemoryReference *) this->allocation_offset; current++) {
        // todo >> this condition should includes a check of whether the referenced memory size matches.
        // todo >> the requested memory size should be at least 80% of the referenced memory size.
        if (current->get_color(veil::MemoryReference::FREED_BIT_POSITION)) {
            reusable = current;
            break;
        }
    }
    return reusable;
}
