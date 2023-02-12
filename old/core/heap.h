#ifndef VEIL_HEAP_H
#define VEIL_HEAP_H

#include "../util/runtime-resources.h"
#include "../util/type-alias.h"
#include "../util/synchronization.h"
#include "../util/natives.h"

namespace veil {

    class HeapSection :
            public util::ChainableResource<HeapSection>,
            public util::ReusableResource<HeapSection &>,
            private concurrent::Synchronizable {
    public:
        HeapSection(uint8 *allocation_address, uint32 allocation_size) :
                allocation_address(allocation_address),
                allocation_offset(allocation_address),
                allocation_size(allocation_size),
                allocation_ceiling(allocation_address + allocation_size) {}

        HeapSection() : HeapSection(nullptr, 0) {}

        uint8 *allocate(uint32 request_size);

        void reset() override;

        void reuse(HeapSection &reset_info) override;

    protected:
        uint8 *allocation_address;
        uint8 *allocation_offset;
        uint8 *allocation_ceiling;
        uint32 allocation_size;
    };

    struct HeapCreationInfo {
        HeapCreationInfo(uint64 memory_size, uint32 section_size) :
                memory_size(memory_size),
                section_size(section_size) {}

        uint64 memory_size;
        uint32 section_size;
        natives::OperationStatus allocation_status{};
    };

    class ReferenceTable;

    class Heap {
    public:
        explicit Heap(HeapCreationInfo &creation_info);

        ~Heap();

        void allocate_heap_section(HeapSection &heap_section);

        concurrent::Synchronizable &get_allocation_lock();

    private:
        concurrent::Synchronizable allocation_lock;

        uint8 *memory_address;
        uint8 *memory_offset;
        uint8 *memory_ceiling;
        uint64 memory_size;
        uint32 section_size;
    };

    class MemoryReference : private concurrent::Synchronizable {
    public:
        static const uint8 ADDRESS_BIT_COUNT = 48;
        static const uint64 ADDRESS_BIT_MASK = ~((~0ULL) << ADDRESS_BIT_COUNT);

        static const uint8 FREED_BIT_POSITION = 15;

        MemoryReference() : colored_pointer(0ULL), size(0), reference_count(1) {}

        [[nodiscard]] uint64 get_reference() const;

        [[nodiscard]] uint8 *get_address() const;

        void update_address(const uint8 *address);

        void set_color(uint8 position, bool value);

        [[nodiscard]] bool get_color(uint8 position) const;

        uint16 *color_bits();

        [[nodiscard]] uint32 get_memory_size() const;

        uint32 get_reference();

        void reference();

        void dereference();

    private:
        uint64 colored_pointer;
        uint32 size;
        std::atomic<uint32> reference_count;
    };

    class ReferenceTable : protected HeapSection {
    public:
        MemoryReference *get_reference();

        MemoryReference *reuse_reference(uint32 request_size);
    };

}

#endif //VEIL_HEAP_H
