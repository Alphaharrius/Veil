#ifndef VEIL_MEMORY_H
#define VEIL_MEMORY_H

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

#include <string>

#include "typedefs.h"
#include "diagnostics.h"
#include "resource.h"

/// The namespace of the memory management system of the Veil virtual machine, this system is designed to provide only
/// the template of the memory access interface visible to the rest of the VM implementation. Any implementation of the
/// memory management algorithm that fulfills the requirement of this template defined in the documentation of
/// \c memory::Algorithm can be used without affecting the vanilla
/// behavior of the runtime.
namespace veil::memory {

    // Forward declaration.
    class Management;

    class Allocator;

    class Pointer;

    class Algorithm;

    /// The request as a parameter for allocating a \c Pointer from an \c Allocator.
    struct AllocateRequest : public util::Request {
        /// The byte size of the pointer to be allocated.
        uint32 size;

        /// \param size The byte size of the pointer to be allocated.
        explicit AllocateRequest(uint32 size);
    };

    /// The request as a parameter for acquiring a \c Pointer from an \c Allocator.
    struct PointerAcquireRequest : public util::Request {
        /// The pointer to be acquired.
        Pointer *pointer;
        /// The current address of the pointer, a returned parameter of the request.
        uint8 *address;
        /// Whether the acquisition is exclusive or not, please be noted that this value is in suggestion basis, the
        /// underlying algorithm might alter the value according to different situations.
        bool exclusive;

        /// \param pointer   The pointer to be acquired.
        /// \param exclusive Whether the acquisition is exclusive or not, defaults to \c false.
        explicit PointerAcquireRequest(Pointer *pointer, bool exclusive = false);
    };

    /// The request as a parameter for performing actions to a \c Pointer from an \c Allocator.
    struct PointerActionRequest : public util::Request {
        /// The pointer to be acted on.
        Pointer *pointer;

        /// \param pointer The pointer to be acted on.
        explicit PointerActionRequest(Pointer *pointer);
    };

    /// The request as a parameter to initialize the memory management and provide params for the chosen \c Algorithm.
    struct ManagementInitRequest : public util::Request {
        /// The maximum utilizable memory managed by the memory management, this includes the heap memory and the stack
        /// memory for each of the VM threads.
        uint64 max_heap_size;
        /// The memory management algorithm to be used in the current \c Management object.
        Algorithm *algorithm;
        /// The pointer of the parameters (if any) for the chosen \c Algorithm.
        void *algorithm_params;

        /// \param max_heap_size    The maximum utilizable memory managed by the memory management, this includes the
        ///                         heap memory and the stack memory for each of the VM threads.
        /// \param algorithm_params The pointer of the parameters (if any) for the chosen \c Algorithm.
        explicit ManagementInitRequest(uint64 max_heap_size, Algorithm *algorithm, void *algorithm_params = nullptr);
    };

    /// The interface for implementing a memory management algorithm to be used by the memory management of the virtual
    /// machine. An concrete instance of an algorithm should contain all its architecture, structures and data
    /// implicitly, only provides the methods for accessing memory management feature specified in this interface.
    /// TODO: Add algorithm implementation requirements.
    class Algorithm : public util::RequestConsumer {
    public:
        /// \brief Initialize the memory management algorithm.
        /// \attention Algorithm specific parameters can be passed with the attribute \c
        /// ManagementInitRequest::algorithm_params, which a custom structure can be defined and passed as a \c void
        /// pointer.
        /// \param request The request of the initialization.
        virtual void initialize(ManagementInitRequest &request) = 0;

        /// \brief Terminate all the implicit algorithm-specific sub-routines and delete all implicit algorithm-specific
        /// data structures within the memory management.
        /// \param request The request of the termination operation.
        virtual void terminate(util::Request &request) = 0;

        /// \brief Create an \c Allocator with its parent \c Management.
        /// \attention Since the structure of the \c Allocator is not specified, thus does not contain any necessary
        /// attributes to perform any "local" action. The suggested style of implementing this function is to create a
        /// subclass of \c Allocator, then define the algorithm specific structures in the subclass, then cast into a
        /// pointer of \c Allocator as the return object type.
        /// \param management The parent \c Management for the creation.
        /// \param request    The request of the action.
        /// \return           An \c Allocator of the provided \a management.
        virtual Allocator *create_allocator(Management &management, util::Request &request) = 0;

        /// \brief Allocate a memory sector and store the relevant information within a \c Pointer.
        /// \attention Since the structure of the \c Pointer is not specified, thus does not contain any necessary
        /// attributes to perform algorithm specific operations. The suggested style of implementing this function is to
        /// create a subclass of \c Pointer, then define the algorithm specific structures in the subclass, then cast
        /// into a pointer of \c Pointer as the return object type.
        /// \param allocator The \c Allocator which the new \c Pointer is requested from.
        /// \param request   The request of the action.
        /// \return          An \c Pointer of the memory sector requested by \a request.
        /// \sa \c Pointer
        virtual Pointer *allocator_pointer_allocate(Allocator &allocator, AllocateRequest &request) = 0;

        /// \brief Reserve an unused \c Pointer for future use.
        /// \attention This method allows the algorithm to implements a infrastructure for efficient \c Pointer reuse
        /// without full garbage collection. For example stacking unused pointers with allocated memory sector to be
        /// reused when new acquisition request is made.
        /// \param allocator The \c Allocator which this operation is performed from.
        /// \param request   The request of the action.
        virtual void allocator_pointer_reserve(Allocator &allocator, PointerActionRequest &request) = 0;

        /// \brief Acquire the access right to a pointer with exclusive access depends on
        /// \c PointerAcquireRequest::exclusive.
        /// \attention Since the attribute \c PointerAcquireRequest::exclusive is suggestive, thus the underlying
        /// algorithm can decide whether to exercise the request if the value is set \c false; but a request with value
        /// set to \c true must acquire the pointer exclusively. If a garbage collector exists in the algorithm, then
        /// this function should also activate the concurrent lock or anything equivalent to prevent concurrent
        /// modification of the information stored within the table of \c Pointer.
        /// \param allocator The \c Allocator of the local management.
        /// \param request   The request of the operation.
        virtual void allocator_pointer_acquire(Allocator &allocator, PointerAcquireRequest &request) = 0;

        /// \brief Release the access right to a pointer.
        /// \attention If a garbage collector exists in the algorithm, then this function should also deactivate the
        /// concurrent lock or anything equivalent to allow the collector to function within the memory management.
        /// \param allocator The \c Allocator of the local management.
        /// \param request   The request of the operation.
        virtual void allocator_pointer_release(Allocator &allocator, PointerActionRequest &request) = 0;
    };

    /// A \c Pointer represents a static placeholder that stores the address and byte size of the its associated memory
    /// sector.
    struct Pointer {
        /// The byte size of the memory sector, the maximum allowed size of memory sector is 4GiB.
        const uint32 size;

        /// \param size The byte size of the memory sector.
        explicit Pointer(uint32 size);
    };

    // TODO: Add documentations.
    class Allocator : public util::RequestConsumer {
    public:
        // TODO: Add documentations.
        explicit Allocator(Management &management);

        // TODO: Add documentations.
        Pointer *allocate(AllocateRequest &request);

        // TODO: Add documentations.
        void reserve(PointerActionRequest &request);

        // TODO: Add documentations.
        void acquire(PointerAcquireRequest &request);

        // TODO: Add documentations.
        void release(PointerActionRequest &request);

        /// The allocator must be provided by the memory management directly, the way to instantiate an object of this
        /// class is to call the constructor on a memory section with the size of this class.
        void *operator new(size_t size) = delete;

        /// The allocator must be recycled or deleted by it's parent \c Management.
        void operator delete(void *) = delete;

    private:
        // TODO: Add documentations.
        Management *management;
    };

    struct MemorySector : Pointer, util::Resource<MemorySector> {
        const uint8 *address;

        explicit MemorySector(const uint8 *address, uint32 size);

        /// \c MemorySector must be provided by the \c Management directly, the way to instantiate an object of this
        /// class is to call the constructor on a memory section with the size of this class.
        void *operator new(size_t size) = delete;

        /// \c MemorySector must be recycled or deleted by it's parent \c Management.
        void operator delete(void *) = delete;
    };

    // TODO: Add documentations.
    class Management {
    public:
        // TODO: Add documentations.
        const uint64 MAX_HEAP_SIZE;

        // TODO: Add documentations.
        static Management *create(ManagementInitRequest &request);

        // TODO: Add documentations.
        Allocator *create_allocator(util::Request &request);

    private:
        // TODO: Add documentations.
        uint64 allocated_heap_size;

        // TODO: Add documentations.
        Management(Algorithm *algorithm, uint64 max_heap_size, void *structure);

        // TODO: Add documentations.
        Algorithm *algorithm;

        /// The structure instantiated by the chosen \c Algorithm to store runtime resources for the memory management
        /// system to function.
        /// \attention The structure should only be constructed & destructed by the \c Algorithm, which is invoked by
        ///  the static \c Management::create method.
        void *structure;

        // TODO: Add documentations.
        MemorySector *allocate_heap_sector(AllocateRequest &request);

        // The class Allocator needs to access delegate functions encapsulating the operations from the algorithm.
        friend class Allocator;
        // The class Algorithm needs to access the attribute Management::structure for runtime memory management.
        friend class Algorithm;
    };

}

#endif //VEIL_MEMORY_H
