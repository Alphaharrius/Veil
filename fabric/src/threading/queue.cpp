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

#include "src/threading/queue.hpp"

using namespace veil::threading;

Queuee::Queuee() : status(STAT_IDLE), reentrance_count(0), target(nullptr), exit_queue(false), queuee_notified(false) {}

void Queuee::queue(Queue &queue) {
    // Check if the current queuee queues to on the same queue it have been queued, if so the current acquire can be
    // considered as a reentrance behavior, increment the reentrance count and exit the method. It is enforced that the
    // root client will assign the reentrance acquire operation to the queuee that have acquired the same target.
    if (this->status != STAT_IDLE && this->target == &queue) {
        this->reentrance_count++;
        return;
    }

    // Mark the target queuee.
    this->target = &queue;

    // Check if the queue has queuee, enters the spin process if there is.
    if (queue.last_queuee.load() != nullptr)
        // Spinning for MAX_SPIN_COUNT amount of time to see if the queue is available before using the full process of
        // queue acquisition. This will save some CPU resources in a contested situation.
        // TODO: The value of MAX_SPIN_COUNT must be profiled and updated to a more suitable value.
        for (uint32 spin_count = 0; spin_count < MAX_SPIN_COUNT; spin_count++) {
            // Using atomic compare exchange to acquire the queue if it returns to empty state within the spin period.
            Queuee *null_queuee = nullptr;
            if (queue.last_queuee.compare_exchange_strong(null_queuee, this)) {
                // We can skip all subsequent procedure in acquiring the queue, this prevents all forms of thread
                // blocking which would be resource intensive.
                goto Acquire;
            }
        }

    {
        // Perform atomic exchange for the last queuee address with the address of this queuee, this ensures only one
        // competing monitor will queue behind the top queuee.
        Queuee *last_queuee = queue.last_queuee.exchange(this);
        // If the last queuee monitor is nullptr, it implies that the queue has yet to be acquired and this queuee is
        // the first owner, and is allowed to proceed without blocking on the condition variable.
        if (last_queuee != nullptr) {
            // The current queue is active and waited in a queue.
            this->status = STAT_QUEUE;
            std::unique_lock<std::mutex> m_lock(last_queuee->blocking_m);
            while (!last_queuee->exit_queue) {
                // Wait until the last queuee signals the exit of the queue, and check for the valid wakeup condition to
                // prevent spurious wakeup.
                last_queuee->blocking_cv.wait(m_lock);
            }
            // Signal the last queuee that this queuee have been queuee_notified and resume in queue acquisition. The
            // exit procedure of the last queuee will check if this flag is set before exiting its looping call to exit
            // the current queuee.
            last_queuee->queuee_notified = true;
        }
    }

    Acquire:
    // At this stage this queuee have fully acquired the queue object.
    this->status = STAT_ACQUIRE;
}

bool Queuee::exit(Queue &queue) {
    // Return the method directly if the queue to exit from does not match with the target queue as exiting before
    // owning a target will result in a blocking as this procedure tries to notify a non-existing queuee behind.
    if (&queue != this->target) {
        // Trivial action.
        return false;
    }

    // The check above ensured the queue to exit from is the target queue, this check ensured the queue have been
    // queued for more than once, thus we can decrement the reentrance count and safely exit this operation.
    // NOTE: We will execute the complex procedure beneath only when this count is 0, which is the actual exit from
    // the queue.
    if (this->reentrance_count > 0) {
        this->reentrance_count--;
        // Exiting from a reentrance state.
        return true;
    }

    Queuee *this_queuee = this;
    // Using atomic compare & exchange operation to reset the last queued monitor of the lockable target, if the last
    // queuee is this queuee, if the exchange is successful, the queue is reset and available for another fresh
    // acquisition; else, this implicitly shows that there are another queuee queued behind this queuee.
    if (!queue.last_queuee.compare_exchange_strong(this_queuee, nullptr)) {
        // Set this queuee to exit state, when the queued queuee behind validated this state, it will be awakened from
        // its blocking state and acquire the queue.
        this->exit_queue = true;
        // Notify the queuee behind until it signals this queuee.
        while (!this->queuee_notified) {
            // Prevent compiler optimization at level o3.
            asm volatile ("");
            // Since there will only be one queuee queued behind, notify_one is sufficient.
            this->blocking_cv.notify_one();
        }
    }

    // Reset the queuee attributes to prepare for another fresh start.
    this->status = STAT_IDLE;
    this->target = nullptr;
    this->exit_queue = false;
    this->queuee_notified = false;

    // Exiting successfully.
    return true;
}

QueueClient::QueueClient() : nested_level(0) {}

QueueClient::~QueueClient() {
    // Release all memory allocated by the cache.
    this->TArena<Queuee>::free();
}

void QueueClient::wait(Queue &target) {
    Queuee *reentrance = nullptr;
    Queuee *available = nullptr;

    memory::TArenaIterator<Queuee> iterator(*this);
    Queuee *current = iterator.next();
    // The purpose of this loop is to look for queuee that have been queued on the target to be queued (reentrance), or
    // to look for a reusable queuee from another completed target operation.
    while (current != nullptr &&
           // Added for optimization. nested_level != 0 indicates this monitor have waited on some target before the
           // current wait operation, and should continue searching for the reentrance queuee if possible; else, when an
           // available queuee is found then the search is successful.
           (available == nullptr || this->nested_level != 0) && reentrance == nullptr) {
        // Reusable queuee will be marked as idle.
        if (current->status == Queuee::STAT_IDLE) available = current;
            // The queuee to reentry
        else if (current->target == &target) reentrance = current;
        current = iterator.next();
    }

    if (reentrance != nullptr) available = reentrance;

    // Add a new queuee if no reusable queuee is found.
    if (available == nullptr) {
        available = this->allocate();
        // Instantiate the new queuee.
        new(available) Queuee();
    }
    available->queue(target);
    this->nested_level++;
}

void QueueClient::exit(Queue &target) {
    // Return if the client hasn't wait on any target.
    if (this->nested_level == 0) return;

    memory::TArenaIterator<Queuee> iterator(*this);
    Queuee *current = iterator.next();
    do {
        // Search for the child queuee which have acquired the target queue, the first occurrence will also be the only
        // occurrence as QueueClient::wait ensured all reentrance behavior to be focused into a single queuee. The only
        // occurrence will also be in STAT_ACQUIRE as this operation will not be reached it is still in STAT_QUEUE, in
        // other words the thread of this client is still in blocking state.
        if (current->exit(target)) {
            // Decrement the nested level upon all successful or reentrance release.
            this->nested_level--;
            // No need to go on further.
            break;
        }
        current = iterator.next();
    } while (current != nullptr);
}
