#ifndef VEIL_MEMORY_H
#define VEIL_MEMORY_H

#include <string>

#include "typedefs.h"
#include "diagnostics.h"

namespace veil::memory {

    class Management;

    class Allocator;

    class Pointer;

    /// The request as a parameter for allocating a \c Pointer from an \c Allocator.
    struct AllocateRequest : public util::Request {
        /// The byte size of the pointer to be allocated.
        uint32 size;

        /// \param size The byte size of the pointer to be allocated.
        explicit AllocateRequest(uint32 size) : size(size) {}
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
        explicit PointerAcquireRequest(Pointer *pointer, bool exclusive = false) : pointer(pointer),
                                                                                   exclusive(exclusive),
                                                                                   address(nullptr) {}
    };

    /// The request as a parameter for performing actions to a \c Pointer from an \c Allocator.
    struct PointerActionRequest : public diagnostics::Request {
        /// The pointer to be acted on.
        Pointer *pointer;

        /// \param pointer The pointer to be acted on.
        explicit PointerActionRequest(Pointer *pointer) : pointer(pointer) {}
    };

    /// The request as a parameter to initialize the memory management and provide params for the chosen \c Algorithm.
    struct ManagementInitRequest : public util::Request {
        /// The maximum utilizable memory managed by the memory management, this includes the heap memory and the stack
        /// memory for each of the VM threads.
        uint64 heap_memory_size;
        /// The pointer of the parameters (if any) for the chosen \c Algorithm.
        void *algorithm_params;

        /// \param heap_memory_size The maximum utilizable memory managed by the memory management, this includes the
        ///                         heap memory and the stack memory for each of the VM threads.
        /// \param algorithm_params The pointer of the parameters (if any) for the chosen \c Algorithm.
        explicit ManagementInitRequest(
                uint64 heap_memory_size, void *algorithm_params = nullptr)
                : heap_memory_size(heap_memory_size), algorithm_params(algorithm_params) {}
    };

    /// The interface for implementing a memory management algorithm to be used by the memory management of the virtual
    /// machine. An concrete instance of an algorithm should contain all its architecture, structures and data
    /// implicitly, only provides the methods for accessing memory management feature specified in this interface.
    class Algorithm : public util::RequestConsumer {
    public:
        /// TODO: Add documentation.
        virtual void initialize(ManagementInitRequest &request) = 0;

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
    class Pointer {
    public:
        /// The byte size of the memory sector, the maximum allowed size of memory sector is 4GiB.
        const uint32 size;

        /// \param address The address (virtual address assigned by the host operating system) of the memory sector.
        /// \param size The byte size of the memory sector.
        Pointer(uint8 *address, uint32 size) : address(address), size(size) {}

    protected:
        /// The address (virtual address assigned by the host operating system) of the memory sector, which can be
        /// changed dynamically, thus this value is obtained via method \c Allocator::acquire.
        uint8 *address;
    };

    // TODO: Add documentations.
    class Allocator : public util::RequestConsumer {
    public:
        // TODO: Add documentations.
        explicit Allocator(Management &management) : management(&management) {}

        // TODO: Add documentations.
        Pointer *allocate(AllocateRequest &request);

        // TODO: Add documentations.
        void acquire(PointerAcquireRequest &request);

        // TODO: Add documentations.
        void reserve(PointerActionRequest &request);

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

    /// The request as a parameter to initialize the \c Management object.
    struct InitializeManagementRequest : public util::Request {
        /// The memory management algorithm to be used in the current \c Management object.
        Algorithm *algorithm;

        /// \param algorithm The memory management algorithm to be used in the current \c Management object.
        explicit InitializeManagementRequest(Algorithm &algorithm) : algorithm(&algorithm) {}
    };

    // TODO: Add documentations.
    class Management {
    public:
        // TODO: Design and implement specification of the constructor.

        // TODO: Add documentations.
        Allocator *create_allocator(util::Request &request);

    private:
        // TODO: Add documentations.
        Algorithm *algorithm;

        // The class Allocator needs to access the delegate functions encapsulating the operations from the algorithm.
        friend class Allocator;
    };

}

#endif //VEIL_MEMORY_H
