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

#ifndef VEIL_FABRIC_SRC_THREADING_QUEUE_HPP
#define VEIL_FABRIC_SRC_THREADING_QUEUE_HPP

#include <atomic>

#include "src/typedefs.hpp"
#include "src/memory/global.hpp"
#include "src/threading/os.hpp"

namespace veil::threading {

    // Forward declaration
    class Queuee;

    /// For an object which will be interacted by \c QueueClient to achieve thread safety should extend this class.
    /// This synchronization primitive is designed from start to have minimal memory footprint on the protected object,
    /// and offset the heavy bulk to the thread owned \c QueueClient, thus is suitable to manage a large amount of
    /// object.
    /// \sa \c QueueClient
    class Queue {
    private:
        /// Atomic pointer caches the last \c Queuee waited in the queue, this attribute will be exchanged atomically to
        /// ensure only one \c Queuee can be queued after another \c Queuee.
        std::atomic<Queuee *> last_queuee =  std::atomic<Queuee *>(nullptr);

        friend class Queuee;
    };

    /// The implicit object used by \c QueueClient to achieve reentrance & nested ability, made possible by using one
    /// child \c Queuee per one wait operation. In a "race condition", many \c Queuee objects will form a linked list
    /// like structure, the exclusive access to the target \c Queue will be passed on in sequence until the last
    /// \c Queuee is reached.
    class Queuee : memory::ArenaObject {
    public:
        /// The idle value for \c Queuee::status flag.
        static const uint8 STAT_IDLE = 0;
        /// The value for \c Queuee::status flag refers to the queueing state before the target acquisition.
        static const uint8 STAT_QUEUE = 1;
        /// The value for \c Queuee::status flag refers to the state which the thread associated to the parent
        /// \c QueueClient have exclusive access to the target.
        static const uint8 STAT_ACQUIRE = 2;

        Queuee();

        /// Wait in the \p queue if it is occupied, \c Queuee::MAX_SPIN_COUNT of spin locking will be used before the
        /// calling thread enters blocking state. Once the thread is blocked, this method will not return until the
        /// prior \c Queuee to invoke \c Queuee::exit and pass the exclusive access right to the current thread.
        /// \param queue The queue to be waited.
        void queue(Queue &queue);

        /// Leaving the state of exclusive access to the \p queue and notify the subsequent \c Queuee to leave thread
        /// blocking state. If there is no \c Queuee behind the current one, the \p queue will be reset to available.
        /// \param queue The currently occupied queue.
        bool exit(Queue &queue);

    private:
        /// The status of this queuee, can be either \c Queuee::STAT_IDLE , \c Queuee::STAT_QUEUE and
        /// \c Queuee::STAT_ACQUIRE.
        /// <br><br>
        /// The time position where these flags will be set: <ul>
        ///     <li> \c STAT_IDLE : The default value, also set when the \c Queuee completed the access cycle by
        ///          invoking \c Queuee::exit </li>
        ///     <li> \c STAT_QUEUE : Right after a failed sequence of spin locking, if the target is being occupied.
        ///     <li> \c STAT_ACQUIRE : At the end of \c Queuee::queue when the target is accessed exclusively.
        /// </ul>
        uint8 status;
        /// The number of reentrance locking assigned to this instance.
        uint32 reentrance_count;
        /// The target queue this instance have been assigned to wait on.
        Queue *target;

        /// The condition variable used to block & notify the thread associated with the subsequent instance.
        os::ConditionVariable blocking_cv;

        /// A flag as a signal for the subsequent instance to confirm the passing of exclusive access, since spurious
        /// wakeup by condition variable can happen, according to the source
        /// ( https://en.wikipedia.org/wiki/Spurious_wakeup ).
        volatile bool exit_queue;
        /// A flag as a signal for the prior instance to confirm the wakeup of the current instance, thus completes the
        /// transfer of exclusive access.
        volatile bool queuee_notified;

        friend class QueueClient;
    };

    /// This synchronization primitive implements a wait-queue to eliminates race condition, and the locking procedure
    /// will be invoked only the lock (the queue object) cannot be acquired through the use of a spin lock.
    /// <br> Reentrance queueing and nested locking is supported in this design, the \c QueueClient which have waited
    /// more than once on the same queue will be considered as waited once in the first wait position.
    class QueueClient : private memory::TArena<Queuee> {
    public:
        QueueClient();

        /// This method is called when disposing the client to free the memory allocated by the \c TArena.
        ~QueueClient();

        /// Wait on the \p target for exclusive access, this method will not return until the exclusive access right is
        /// acquired.
        /// \param target The target to be waited on.
        /// \sa \c Queuee::wait
        void wait(Queue &target);

        /// Leaving the state of exclusive access to the \p target.
        /// \param target The currently occupied queue.
        void exit(Queue &target);

    private:
        /// The number of \c Queue object which this instance have exclusive access right.
        uint32 nested_level;
    };

}

#endif //VEIL_FABRIC_SRC_THREADING_QUEUE_HPP
