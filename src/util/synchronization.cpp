#include "util/synchronization.h"
#include "util/diagnostics.h"

veil::concurrent::ReentranceMonitor::ReentranceMonitor() {
    this->idle = true;
    this->owned_synchronizable = nullptr;
    this->broadcast_signal = CommunicationSignal::NONE;
    this->receiving_signal = CommunicationSignal::NONE;
    this->reentrance_count = 0;
}

void veil::concurrent::ReentranceMonitor::acquire(veil::concurrent::Synchronizable *target) {
    not_null(target);

    // Check if the current monitor corresponds to an in placed lock on the same target as
    // the current acquire request, if so the current acquire can be considered as a reentrance
    // behavior, increment the reentrance count and exit the method.
    // We also assume that the parent synchronizer will assign the reentrance acquire operation
    // to the monitor that have already acquired the same target.
    if (!this->idle && this->owned_synchronizable != nullptr && this->owned_synchronizable == target) {
        this->reentrance_count++;
        return;
    }
    // Set the current monitor's idle state to false.
    this->idle = false;
    // Perform atomic exchange for the last queued monitor address with the address of the current
    // executor this ensures only one competing monitor will acquire the last queued monitor.
    ReentranceMonitor *last_queued = target->last_queued.exchange(this);
    // If the last queued monitor is `nullptr`, it means this synchronizable has yet to be owned,
    // this monitor is the first owner, and it is allowed to proceed without blocking on the
    // condition variable.
    if (last_queued != nullptr) {
        std::unique_lock<std::mutex> mutex_lock(last_queued->blocking_mutex);
        // Wait until the last queued monitor signals the release of the lockable, the while loop
        // checks for the valid wakeup condition to prevent spurious wakeup.
        while (last_queued->broadcast_signal != CommunicationSignal::SIGNAL_RELEASE) {
            last_queued->blocking_cond_var.wait(mutex_lock);
        }
        // Signal the last queued monitor that the current monitor is awake.
        last_queued->receiving_signal = CommunicationSignal::REPLY_RELEASED;
    }
    this->owned_synchronizable = target;
}

bool veil::concurrent::ReentranceMonitor::release(veil::concurrent::Synchronizable *target) {
    not_null(target);

    // Return the method directly if the target to be released does not match with
    // the owned target as releasing before owning a target will result in
    // a blocking by the communication void with the non-exist queued monitor.
    if (target != this->owned_synchronizable) {
        // Signals a trivial release.
        return false;
    }
    // The previous check ensured the target to be released matches with the owned target,
    // if the current monitor is in reentrance state, decrement the reentrance count and
    // exit the method.
    if (this->reentrance_count > 0) {
        this->reentrance_count--;
        // Signals a successful release.
        return true;
    }
    ReentranceMonitor *current_monitor = this;
    // Using atomic compare & exchange operation to reset the last queued monitor
    // of the lockable target, if the last queued monitor is the current monitor,
    // the exchange is successful and the lockable can be relocked by another monitor
    // without blocking; else, this implicitly proved that there are another monitor
    // queued behind the current monitor.
    if (!target->last_queued.compare_exchange_strong(current_monitor, nullptr)) {
        // Set the broadcast signal to `SIGNAL_RELEASE`, the blocked monitor
        // will awake if it verified the signal condition.
        this->broadcast_signal = CommunicationSignal::SIGNAL_RELEASE;
        // Notify the blocked monitor until it replies with the receiving signal `REPLY_RELEASED`.
        while (this->receiving_signal != CommunicationSignal::REPLY_RELEASED) {
            // Prevents compiler optimization.
            asm("");
            // There will be at max one monitor blocked, thus `notify_one` is sufficient.
            this->blocking_cond_var.notify_one();
        }
    }
    // Reset & reset the current monitor.
    this->reset();
    // Signals a successful release.
    return true;
}

void veil::concurrent::ReentranceMonitor::reset() {
    this->idle = true;
    this->owned_synchronizable = nullptr;
    this->broadcast_signal = CommunicationSignal::NONE;
    this->receiving_signal = CommunicationSignal::NONE;
    this->reentrance_count = 0;
}

void veil::concurrent::ReentranceMonitor::reuse(veil::concurrent::ReentranceMonitor reuse_info) {
    this->idle = reuse_info.idle;
    this->owned_synchronizable = reuse_info.owned_synchronizable;
    this->broadcast_signal = reuse_info.broadcast_signal;
    this->receiving_signal = reuse_info.broadcast_signal;
    this->reentrance_count = reuse_info.reentrance_count;
}

veil::concurrent::Synchronizer::Synchronizer() {
    this->nested_level = 0;
    // A synchronizer will have at least one monitor initially.
    this->monitor = new ReentranceMonitor();
    this->monitor_count = 1;
}

veil::concurrent::Synchronizer::~Synchronizer() {
    // Delete all monitors except for the anchor of the chain.
    this->monitor->iterate_clockwise(
            [](ReentranceMonitor *anchor) { return anchor->get_next(); },
            [](ReentranceMonitor *anchor, ReentranceMonitor *current) { delete current; },
            [](ReentranceMonitor *anchor, ReentranceMonitor *current) { return current == anchor; });
    // Delete the anchor monitor of the chain.
    delete this->monitor;
}

void veil::concurrent::Synchronizer::acquire(veil::concurrent::Synchronizable *target) {
    not_null(target);

    // The functionality of the reentrance monitor is preserved if only one child monitor
    // of the synchronizer is acquiring the target with reentrance manner, we would like to
    // (primary) search through the chain of monitor for one that have acquired the target,
    // or (secondary) search for an idle monitor for the new acquire procedure.

    // The monitor for the primary objective.
    ReentranceMonitor *reentrance = nullptr;
    // The monitor for the secondary objective.
    ReentranceMonitor *available = nullptr;
    ReentranceMonitor *anchor = this->monitor;
    ReentranceMonitor *current = anchor;
    do {
        // The condition for the primary objective.
        if (!current->idle && current->owned_synchronizable == target)
            reentrance = current;
        // The condition for the secondary objective.
        if (current->idle)
            available = current;
        // This loop is enhanced in favor of the primary objective as the monitors corresponding to the
        // latest acquire operations are stacked at the clockwise direction of the monitor chain,
        // so if the current operation is reentrance like, the objective is more likely to be satisfied.
        current = current->get_next();
    } while (
            // Added for optimization, if this synchronizer has yet to acquire any target, there will
            // never be a monitor matches the condition for the primary objective, thus we can exit with
            // a monitor that satisfies the secondary objective.
            (available == nullptr || this->nested_level != 0) &&
            // Exit when the primary objective is fulfilled.
            reentrance == nullptr && anchor != current);
    if (reentrance != nullptr)
        available = reentrance;
    // Allocate a new monitor and place as the anchor if there are no
    // available monitors for both the primary and secondary objective.
    if (available == nullptr) {
        available = new ReentranceMonitor();
        available->insert_before(this->monitor);
        this->monitor = available;
        this->monitor_count++;
    }
    available->acquire(target);
    // Increment the nested level of this synchronizer.
    this->nested_level++;
}

void veil::concurrent::Synchronizer::release(veil::concurrent::Synchronizable *target) {
    not_null(target);

    if (this->nested_level == 0)
        return;
    ReentranceMonitor *anchor = this->monitor;
    ReentranceMonitor *current = anchor;
    do {
        if (current->release(target)) {
            this->nested_level--;
            break;
        }
        current = current->get_next();
    } while (anchor != current);
}
