#ifndef VEIL_HEAP_H
#define VEIL_HEAP_H

#include "util/runtime-resources.h"
#include "util/type-alias.h"
#include "util/synchronization.h"
#include "util/natives.h"

namespace veil {

    struct Allocation {
        [[nodiscard]] bool is_allocated() const;

        uint8 *address = nullptr;
        uint32 size = 0;
    };

    class AllocationPool:
            public veil::util::ChainableResource<AllocationPool>,
            public veil::util::ReusableResource<AllocationPool>,
            private veil::concurrent::Synchronizable {
    public:
        AllocationPool(uint8 *allocation_address, uint32 allocation_size);
        void allocate(Allocation *allocation);
        void reset() override;
        void reuse(AllocationPool reset_info) override;
    private:
        uint8 *allocation_address;
        uint8 *allocation_offset;
        uint8 *allocation_ceiling;
        uint32 allocation_size;
    };

    struct HeapCreationInfo {
        enum Error {
            NONE,
            INVALID_MEMORY_SIZE,
            INVALID_INTERNAL_ALLOCATION_POOL_SIZE,
            ALLOCATION_ERROR
        };

        HeapCreationInfo(uint64 memory_size, uint32 internal_allocation_pool_size);

        uint64 memory_size;
        uint32 internal_allocation_pool_size;
        HeapCreationInfo::Error error;
        natives::OperationStatus allocation_status {};
    };

    class Heap {
    public:
        explicit Heap(HeapCreationInfo &creation_info);
        ~Heap();
        AllocationPool *allocate_internal_allocation_pool();
        AllocationPool *allocate_external_allocation_pool(uint64 request_size,
                                                          natives::OperationStatus &allocation_status);
    private:
        uint8 *memory_address;
        uint8 *memory_offset;
        uint8 *memory_ceiling;
        uint64 memory_size;
        uint32 internal_allocation_pool_size;
    };

}

#endif //VEIL_HEAP_H
