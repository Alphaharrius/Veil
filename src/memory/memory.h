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
#include <atomic>

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
        const uint64 size;

        /// \param size The byte size of the pointer to be allocated.
        explicit AllocateRequest(uint64 size);
    };

    /// The request as a parameter for acquiring a \c Pointer from an \c Allocator.
    class PointerAcquireRequest : public util::Request {
    public:
        /// The pointer to be acquired.
        const Pointer *pointer;
        /// Whether the acquisition is exclusive or not, please be noted that this value is in suggestion basis, the
        /// underlying algorithm might alter the value according to different situations.
        const bool exclusive;

        /// \param pointer   The pointer to be acquired.
        /// \param exclusive Whether the acquisition is exclusive or not, defaults to \c false.
        explicit PointerAcquireRequest(Pointer *pointer, bool exclusive = false);

        /// \return The current address of the pointer.
        uint8 *get_address();

    private:
        /// The current address of the pointer, a returned parameter of the request.
        uint8 *address;
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
    /// <br>
    /// <b>The implementation requirements of the Veil runtime memory management system:</b>
    /// <ul>
    ///     <li> An \c Algorithm object must not include implicit attributes, all algorithm-specific runtime data
    ///          structures must be stored within \c Management::structure. </li>
    ///     <li> The algorithm must define termination operation for the removal of all its associated sub-routines and
    ///          data structures within \c Algorithm::terminate which will be invoked by the termination routine of the
    ///          memory management. </li>
    ///     <li> The thread synchronization feature of the Veil runtime is provided by the memory management algorithm,
    ///          this might sounds weird but keep in mind that this feature is inplace to enforce thread-safety of Veil
    ///          data objects ( \c Pointer ) in the first place. </li>
    ///     <li> When a \c Pointer is acquired exclusively via \c Algorithm::allocator_pointer_acquire method, the
    ///          algorithm must initiate a lock operation of the target \c Pointer to enforce thread-safety; otherwise
    ///          if the acquisition is not exclusive, the algorithm held the final decision on whether the acquisition
    ///          will be exclusive. </li>
    ///     <li> The maximum memory size associated with a \c Pointer must not exceed 4GiB, allocation larger than this
    ///          should be handled beyond the management algorithm. </li>
    /// </ul>
    class Algorithm : public util::RequestConsumer {
    public:
        /// \brief Provides the name of the algorithm.
        /// \attention This name will be logged by the runtime logger if necessary.
        /// \return The name of the algorithm.
        virtual std::string name() = 0;

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

        /// \brief The maximum supported heap size of this memory management algorithm implementation.
        /// \attention The parent \c Management will check this value on initialization, if this value is smaller than
        /// the user requested size, the VM runtime will be terminated.
        /// \return The maximum supported heap size.
        virtual uint64 max_supported_heap_size() = 0;

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
        /// \param request   The request of the action, the maximum allowed size associated with a pointer is 4GiB.
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
    /// \attention Since the structure of the \c Pointer is not specified, thus does not contain any necessary
    /// attributes to perform algorithm specific operations. The suggested style of implementing this function is to
    /// create a subclass of \c Pointer, then define the algorithm specific structures in the subclass, then cast
    /// into a pointer of \c Pointer as the return object type.
    struct Pointer {
        /// The byte size of the memory sector, the maximum allowed size of memory sector is 4GiB.
        const uint32 size;

        /// \param size The byte size of the memory sector.
        explicit Pointer(uint32 size);
    };

    /// This class acts as an interface for each thread of the runtime to interact with the memory management, each of
    /// the thread or it's constituents will hold one (not necessarily separate) instance of this class.
    /// \attention Since the structure of the \c Allocator is not specified, thus does not contain any necessary
    /// attributes to perform any "local" action. The suggested style of implementing this function is to create a
    /// subclass of \c Allocator, then define the algorithm specific structures in the subclass, then cast into a
    /// pointer of \c Allocator as the return object type.
    class Allocator : public util::RequestConsumer {
    public:
        /// \param management The parent \c Management of this instance.
        explicit Allocator(Management &management);

        /// \brief Allocate a memory sector and store the relevant information within a \c Pointer.
        /// \param request   The request of the action, the maximum allowed size associated with a pointer is 4GiB.
        /// \return          An \c Pointer of the memory sector requested by \a request.
        /// \sa \c Pointer
        Pointer *allocate(AllocateRequest &request);

        /// \brief Reserve an unused \c Pointer for future use.
        /// \param request   The request of the action.
        void reserve(PointerActionRequest &request);

        /// \brief Acquire the access right to a pointer with exclusive access depends on
        /// \c PointerAcquireRequest::exclusive.
        /// \param request   The request of the operation.
        void acquire(PointerAcquireRequest &request);

        /// \brief Release the access right to a pointer.
        /// \param request   The request of the operation.
        void release(PointerActionRequest &request);

        /// \return The parent \c Management of this instance.
        const Management *get_management();

        /// The allocator must be provided by the memory management directly, the way to instantiate an object of this
        /// class is to call the constructor on a memory section with the size of this class.
        void *operator new(size_t size) = delete;

        /// The allocator must be recycled or deleted by it's parent \c Management.
        void operator delete(void *) = delete;

    private:
        /// The parent \c Management of this instance.
        const Management *management;
    };

    /// The request as a parameter to map a heap memory section.
    class HeapMapRequest : public AllocateRequest {
    public:
        /// \return The mapped address.
        uint8 *get_address();

        /// \param size The size of the memory section to be mapped, in 64bit addressing mode.
        explicit HeapMapRequest(uint64 size);

    private:
        /// The mapped address as the returned value.
        uint8 *address = nullptr;

        // Allow modification of the address from the heap_map function call.
        friend class Management;
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

        // TODO: Add documentations.
        std::string get_error_info(uint32 status) override;

    private:
        // TODO: Add documentations.
        std::atomic_uint64_t mapped_heap_size;

        // TODO: Add documentations.
        Management(Algorithm *algorithm, uint64 max_heap_size, void *structure);

        // TODO: Add documentations.
        Algorithm *algorithm;

        /// The structure instantiated by the chosen \c Algorithm to store runtime resources for the memory management
        /// system to function.
        /// \attention The structure should only be constructed & destructed by the \c Algorithm, which is invoked by
        ///  the static \c Management::create method.
        void *structure;

        /// Map a heap memory section in 64bit swap memory space, the mapped section is permitted to perform
        /// READ & WRITE operations.
        /// \param request The request of the mapping.
        void heap_map(HeapMapRequest &request);

        // The class Allocator needs to access delegate functions encapsulating the operations from the algorithm.
        friend class Allocator;

        // The class Algorithm needs to access the attribute Management::structure for runtime memory management.
        friend class Algorithm;
    };

}

#endif //VEIL_MEMORY_H
