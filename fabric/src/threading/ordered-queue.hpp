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

#ifndef VEIL_FABRIC_SRC_THREADING_ORDERED_QUEUE_HPP
#define VEIL_FABRIC_SRC_THREADING_ORDERED_QUEUE_HPP

#include "src/typedefs.hpp"
#include "src/memory/global.hpp"
#include "src/threading/os.hpp"

namespace veil::threading {

    /// The implicit object used by <code>OrderedQueueClient</code> to achieve reentrance & nested ability, made
    /// possible by using one child <code>OrderedQueuee</code> per one wait operation. In a "race condition", many
    /// <code>OrderedQueuee</code> objects will form a linked list like structure, the exclusive access to the target
    /// <code>OrderedQueue</code> will be passed on in sequence until the last <code>OrderedQueuee</code> is reached.
    class OrderedQueuee;

    /// For an object which will be interacted by <code>OrderedQueueClient</code> to achieve thread safety should extend
    /// this class. This synchronization primitive is designed from start to have minimal memory footprint on the
    /// protected object, and offset the heavy bulk to the thread owned <code>OrderedQueueClient</code>, thus is
    /// suitable to use with a large set of objects.
    /// \sa OrderedQueueClient
    class OrderedQueue {
    private:
        /// Atomic pointer caches the last <code>OrderedQueuee</code> waited in the queue, this attribute will be
        /// exchanged atomically to ensure only one <code>OrderedQueuee</code> can be queued after another <code>
        /// OrderedQueuee</code>.
        os::atomic_pointer_t<OrderedQueuee> last_queuee =  os::atomic_pointer_t<OrderedQueuee>(nullptr);

        friend class OrderedQueuee;
    };

    class OrderedQueuee : memory::ArenaObject {
    public:
        /// The idle value for <code>OrderedQueuee::status</code> flag.
        static const uint8 STAT_IDLE = 0;
        /// The value for <code>OrderedQueuee::status</code> flag refers to the queueing state before the target
        /// acquisition.
        static const uint8 STAT_QUEUE = 1;
        /// The value for <code>OrderedQueuee::status</code> flag refers to the state which the thread associated to the
        /// parent <code>OrderedQueueClient</code> have exclusive access to the target.
        static const uint8 STAT_ACQUIRE = 2;

        OrderedQueuee();

        /// Attempt to queue in the <code>queue</code> by using spin locking, this can be used as a light weight attempt
        /// to achieve the effect of <code>OrderedQueue::queue(OrderedQueue&)</code>.
        /// \param  queue The queue to be queued in.
        /// \return <code>true</code> if the attempt is successful; <code>false</code> if else due to others have been
        ///         queued there.
        bool try_queue(OrderedQueue &queue);

        /// Wait in the <code>queue</code> if it is occupied, <code>OrderedQueuee::MAX_SPIN_COUNT</code> of spin locking
        /// will be used before the calling thread enters blocking state. Once the thread is blocked, this method will
        /// not return until the previous <code>OrderedQueuee</code> to invoke <code>OrderedQueuee::exit(OrderedQueue&)
        /// </code> and pass the exclusive access right to the current thread.
        /// \param queue The queue to be queued in.
        void queue(OrderedQueue &queue);

        /// Leaving the state of exclusive access to the <code>queue</code> and notify the subsequent <code>
        /// OrderedQueuee</code> to leave thread blocking state. If there is no <code>OrderedQueuee</code> behind the
        /// current one, the <code>queue</code> will be reset to available.
        /// \param queue The currently occupied queue.
        bool exit(OrderedQueue &queue);

    private:
        /// The status of this queuee, can be either <code>OrderedQueuee::STAT_IDLE</code> , <code>
        /// OrderedQueuee::STAT_QUEUE</code> and <code>OrderedQueuee::STAT_ACQUIRE</code>.
        /// <br><br>
        /// The time position where these flags will be set: <ul>
        ///     <li> <code>OrderedQueuee::STAT_IDLE</code> : The default value, also set when the <code>OrderedQueuee
        ///          </code> completed the access cycle by invoking <code>OrderedQueuee::exit(OrderedQueue&)</code>
        ///     <li> <code>OrderedQueuee::STAT_QUEUE</code> : Right after a failed sequence of spin locking, if the
        ///          target is being occupied.
        ///     <li> <code>OrderedQueuee::STAT_ACQUIRE</code> : At the end of <code>OrderedQueuee::queue(OrderedQueue&)
        ///          </code> when the target is accessed exclusively.
        /// </ul>
        uint8 status;
        /// The number of reentrance locking assigned to this instance.
        uint32 reentrance_count;
        /// The target queue this instance have been assigned to wait on.
        OrderedQueue *target;

        /// The condition variable used to block & notify the thread associated with the subsequent instance.
        os::ConditionVariable blocking_cv;

        /// A flag as a signal for the subsequent instance to confirm the passing of exclusive access, since spurious
        /// wakeup by condition variable can happen, according to the source
        /// ( https://en.wikipedia.org/wiki/Spurious_wakeup ).
        volatile bool exit_queue;
        /// A flag as a signal for the prior instance to confirm the wakeup of the current instance, thus completes the
        /// transfer of exclusive access.
        volatile bool queuee_notified;

        friend class OrderedQueueClient;
    };

    /// This synchronization primitive implements a wait-queue to eliminates race condition, and the locking procedure
    /// will be invoked only the lock (the queue object) cannot be acquired through the use of a spin lock.
    /// <br> Reentrance queueing and nested locking is supported in this design, the <code>OrderedQueueClient</code>
    /// which have waited more than once on the same queue will be considered as waited once in the first wait position.
    class OrderedQueueClient : private memory::TArena<OrderedQueuee> {
    public:
        OrderedQueueClient();

        /// This method is called when disposing the client to free the memory allocated by the <code>TArena</code>.
        ~OrderedQueueClient();

        /// Wait on the <code>target</code> for exclusive access, this method will not return until the exclusive access
        /// right is acquired.
        /// \param target The target to be waited on.
        /// \sa OrderedQueuee::wait
        void wait(OrderedQueue &target);

        /// Leaving the state of exclusive access to the <code>target</code>.
        /// \param target The currently occupied queue.
        void exit(OrderedQueue &target);

    private:
        /// The number of <code>OrderedQueue</code> object which this instance have exclusive access right.
        uint32 nested_level;
    };

}

#endif //VEIL_FABRIC_SRC_THREADING_ORDERED_QUEUE_HPP
