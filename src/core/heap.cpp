#include "core/heap.h"
#include "util/natives.h"
#include "util/diagnostics.h"

uint8 *veil::HeapSection::allocate(uint32 request_size) {
    // NOTE: The operations are not atomic thus is not thread-safe, but an allocation
    // with the heap section is expected to be used by its parent thread only.
    if (request_size == 0 ||
        // Return nullptr if the allocation condition is not fulfilled,
        // especially when this section does not have any explicit free memories.
        this->allocation_offset + request_size - 1 >= this->allocation_ceiling) {
        return nullptr;
    }
    // The current allocation offset is the available address for this request size.
    uint8 *allocated_address = this->allocation_offset;
    // As the heap is allocated from the system, some bits might be set by previous usage,
    // reset all bits of this allocation before using is the most efficient way.
    memset((void *) allocated_address, 0, request_size);
    // Increment the allocation offset by the request size.
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
    // Retrieve the system page size.
    uint32 page_size = natives::get_page_size();
    // Round the section size to an integer multiple of the system page size.
    if (creation_info.section_size % page_size != 0)
        creation_info.section_size = creation_info.section_size - (creation_info.section_size % page_size) + page_size;
    // Round the total memory size to an integer multiple of the rounded section size, thus the total memory size is
    // also commensurate with the system page size.
    if (creation_info.memory_size % creation_info.memory_size != 0)
        creation_info.memory_size =
                creation_info.memory_size - (creation_info.memory_size % creation_info.section_size) +
                creation_info.section_size;
    // Make system call to reserve a memory section of the requested size.
    void *allocated_address = veil::natives::virtual_allocate(creation_info.memory_size,
                                                              creation_info.allocation_status);
    if (!creation_info.allocation_status.success) {
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
        // The heap is assumed to be singular throughout the virtual machine, the status of this
        // operation is trivial because the entire virtual machine process is being terminated.
        natives::OperationStatus operation_status;
        // The mapped memory can only be freed by another system call.
        natives::virtual_free(this->memory_address, this->memory_size, operation_status);
    }
}

veil::HeapSection *veil::Heap::allocate_heap_section() {
    // todo: using an integer to store the current allocated index, as all sections are having same size.
    if (this->memory_offset + this->section_size - 1 < this->memory_ceiling) {
        auto *internal_pool = new HeapSection(this->memory_offset,
                                              this->section_size);
        this->memory_offset += this->section_size;
        return internal_pool;
    }
    return nullptr;
}

veil::ReferenceTable *veil::Heap::allocate_reference_table() {
    HeapSection *heap_section = this->allocate_heap_section();
    // The reference table is just a mask class of a heap section.
    // todo: there must be a better approach..
    return reinterpret_cast<veil::ReferenceTable *>(heap_section);
}

veil::concurrent::Synchronizable &veil::Heap::get_allocation_lock() {
    return this->allocation_lock;
}

uint64 veil::MemoryReference::get_reference() const {
    return this->colored_pointer;
}

uint8 *veil::MemoryReference::get_address() const {
    return (uint8 *) (this->colored_pointer & MemoryReference::ADDRESS_BIT_MASK);
}

void veil::MemoryReference::update_address(const uint8 *address) {
    uint64 masked_reference = ((uint64) address) & MemoryReference::ADDRESS_BIT_MASK;
    this->colored_pointer &= ~MemoryReference::ADDRESS_BIT_MASK;
    this->colored_pointer |= masked_reference;
}

void veil::MemoryReference::set_color(uint8 position, bool value) {
    uint64 bit_mask = (1ULL << (MemoryReference::ADDRESS_BIT_COUNT + position));
    if (value) {
        this->colored_pointer |= bit_mask;
    } else {
        this->colored_pointer &= ~bit_mask;
    }
}

bool veil::MemoryReference::get_color(uint8 position) const {
    uint64 mask = (1ULL << (MemoryReference::ADDRESS_BIT_COUNT + position));
    return this->colored_pointer & mask;
}

uint16 *veil::MemoryReference::color_bits() {
    return ((uint16 *) &this->colored_pointer) + 3;
}

uint32 veil::MemoryReference::get_memory_size() const {
    return this->size;
}

uint32 veil::MemoryReference::get_reference() {
    return this->reference_count.load();
}

void veil::MemoryReference::reference() {
    this->reference_count++;
}

void veil::MemoryReference::dereference() {
    assert(this->reference_count.load() == 0);
    this->reference_count--;
}

veil::MemoryReference *veil::ReferenceTable::get_reference() {
    auto *memory_reference = (veil::MemoryReference *) this->allocate(sizeof(veil::MemoryReference));
    // Call the constructor on the existing allocation for this reference object.
    new(memory_reference) veil::MemoryReference();
    return memory_reference;
}

veil::MemoryReference *veil::ReferenceTable::reuse_reference(uint32 request_size) {
    veil::MemoryReference *reusable = nullptr;
    bool memory_reusable = false;
    for (auto *current = (veil::MemoryReference *) this->allocation_address;
         current < (veil::MemoryReference *) this->allocation_offset && !memory_reusable; current++) {
        if (current->get_color(veil::MemoryReference::FREED_BIT_POSITION)) {
            reusable = current;
            uint32 memory_size = current->get_memory_size();
            memory_reusable = request_size == memory_size ||
                              (request_size < memory_size && (float) request_size / (float) memory_size > 0.8f);
        }
    }
    if (reusable != nullptr && !memory_reusable) {
        *reusable->color_bits() = 0;
        reusable->update_address(nullptr);
    }
    return reusable;
}
