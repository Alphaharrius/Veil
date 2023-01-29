#ifndef VEIL_MEMORY_H
#define VEIL_MEMORY_H

#include <string>

#include "typedefs.h"
#include "diagnostics.h"

namespace veil::memory {

    class Management;

    class Allocator;

    class Pointer;

    /// The request object as a parameter for allocating a \c Pointer from an \c Allocator.
    struct AllocateRequest: public diagnostics::Request {
        /// The byte size of the pointer to be allocated.
        uint32 size;

        /// \param size The byte size of the pointer to be allocated.
        explicit AllocateRequest(uint32 size): size(size) {}
    };

    /// The request object as a parameter for acquiring a \c Pointer from an \c Allocator.
    struct PointerAcquireRequest: public diagnostics::Request {
        /// The pointer to be acquired.
        Pointer *pointer;
        /// The current address of the pointer, a returned parameter of the request.
        uint8 *address;

        /// \param pointer The pointer to be acquired.
        explicit PointerAcquireRequest(Pointer *pointer): pointer(pointer), address(nullptr) {}
    };

    /// The request object as a parameter for releasing a \c Pointer from an \c Allocator.
    struct PointerReleaseRequest: public diagnostics::Request {
        /// The pointer to be released.
        Pointer *pointer;

        /// \param pointer The pointer to be released.
        explicit PointerReleaseRequest(Pointer *pointer): pointer(pointer) {}
    };

    /// The interface for implementing a memory management algorithm to be used by the memory management of the virtual
    /// machine. An concrete instance of an algorithm should contain all its architecture, structures and data
    /// implicitly, only provides the methods for accessing memory management feature specified in this interface.
    class Algorithm: public diagnostics::RequestConsumer {
    public:
        /// TODO: Design and implement the initialization operation.

        /// Since the structure of the \c Allocator is not specified, thus does not contain any necessary attributes to
        /// perform any "local" action. The suggested style of implementing this function is to create a subclass of
        /// \c Allocator, then define the algorithm specific structures in the subclass, then cast into a pointer of
        /// \c Allocator as the return object type.
        /// \param management The parent \c Management for the creation.
        /// \param request    The request of the action.
        /// \return           An \c Allocator of the provided \a management.
        virtual Allocator *create_allocator(Management &management, diagnostics::Request &request) = 0;

        virtual Pointer *allocator_allocate(Allocator &allocator, AllocateRequest &request) = 0;

        virtual void allocator_acquire(Allocator &allocator, PointerAcquireRequest &request) = 0;

        virtual void allocator_release(Allocator &allocator, PointerReleaseRequest &request) = 0;
    };

    class Pointer {
    public:
        uint32 size;

    protected:
        uint8 *address;
    };

    class Allocator: public diagnostics::RequestConsumer {
    public:
        explicit Allocator(Management &management): management(&management) {}

        Pointer *allocate(AllocateRequest &request);

        void acquire(PointerAcquireRequest &request);

        void release(PointerReleaseRequest &request);

        /// The allocator must be provided by the memory management directly, the way to instantiate an object of this
        /// class is to call the constructor on a memory section with the size of this class.
        void *operator new(size_t size) = delete;

        /// The allocator must be recycled or deleted by it's distributor which is the memory management.
        void operator delete(void *) = delete;

    private:
        Management *management;
    };

    struct InitializeManagementRequest: public diagnostics::Request {
        Algorithm *algorithm;

        explicit InitializeManagementRequest(Algorithm &algorithm) : algorithm(&algorithm) {}
    };

    class Management {
    public:
        // TODO: Design and implement specification of the constructor.

        Allocator *create_allocator(diagnostics::Request &request);

    private:
        Algorithm *algorithm;

        Pointer *allocator_allocate(Allocator &allocator, AllocateRequest &request);

        void allocator_acquire(Allocator &allocator, PointerAcquireRequest &request);

        void allocator_release(Allocator &allocator, PointerReleaseRequest &request);

        friend class Allocator;
    };

}

#endif //VEIL_MEMORY_H
