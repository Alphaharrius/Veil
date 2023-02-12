#include "queue.h"

using namespace veil::threading;

Queuee::Queuee() : idle(true), reentrance_count(0), owned(nullptr), exit_queue(false), queuee_notified(false) {}

void Queuee::queue(Queue &queue) {
    // Check if the current queuee queues to on the same queue it have been queued, if so the current acquire can be
    // considered as a reentrance behavior, increment the reentrance count and exit the method. It is enforced that the
    // root client will assign the reentrance acquire operation to the queuee that have acquired the same target.
    if (!this->idle && this->owned != nullptr && this->owned == &queue) {
        this->reentrance_count++;
        return;
    }

    // Spinning for MAX_SPIN_COUNT amount of time to see if the queue is available before using the full process of
    // queue acquisition. This will save some CPU resources in a contested situation.
    // TODO: The value of MAX_SPIN_COUNT must be profiled and updated to a more suitable value.
    for (uint32 spin_count = 0; queue.last_queuee.load() != nullptr && spin_count < MAX_SPIN_COUNT; spin_count++) {
        // Prevent compiler optimization at level o3.
        asm volatile ("");
    }

    // The current queue is active and waited in a queue.
    this->idle = false;

    // Perform atomic exchange for the last queuee address with the address of this queuee, this ensures only one
    // competing monitor will queue behind the top queuee.
    Queuee *last_queuee = queue.last_queuee.exchange(this);
    // If the last queuee monitor is nullptr, it implies that the queue has yet to be owned and this queuee is the first
    // owner, and is allowed to proceed without blocking on the condition variable.
    if (last_queuee != nullptr) {
        std::unique_lock<std::mutex> m_lock(last_queuee->blocking_m);
        while (!last_queuee->exit_queue) {
            // Wait until the last queuee signals the exit of the queue, and check for the valid wakeup condition to
            // prevent spurious wakeup.
            last_queuee->blocking_cv.wait(m_lock);
        }
        // Signal the last queuee that this queuee have been queuee_notified and resume in queue acquisition. The exit
        // procedure of the last queuee will check if this flag is set before exiting its looping call to exit the
        // current queuee.
        last_queuee->queuee_notified = true;
    }

    // At this stage this queuee have fully acquired the queue object.
    this->owned = &queue;
}

bool Queuee::exit(Queue &queue) {
    // Return the method directly if the queue to exit from does not match with the owned queue as exiting before
    // owning a target will result in a blocking as this procedure tries to notify a non-existing queuee behind.
    if (&queue != this->owned) {
        // Trivial action.
        return false;
    }

    // The check above ensured the queue to exit from is the owned queue, this check ensured the queue have been
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
    this->idle = true;
    this->owned = nullptr;
    this->exit_queue = false;
    this->queuee_notified = false;

    // Exiting successfully.
    return true;
}
