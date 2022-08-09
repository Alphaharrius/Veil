#ifndef VEIL_SYNCHRONIZATION_H
#define VEIL_SYNCHRONIZATION_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "util/type-alias.h"
#include "util/runtime-resources.h"

namespace veil::concurrent {
    class Synchronizable;

    class Synchronizer;

    class ReentranceMonitor;

    enum CommunicationSignal {
        NONE, SIGNAL_RELEASE, REPLY_RELEASED
    };

    struct Synchronizable {
    private:
        std::atomic<ReentranceMonitor *> last_queued = std::atomic<ReentranceMonitor *>(nullptr);

        friend ReentranceMonitor;
    };

    /// This class implements the reentrance monitor, which is allowed to
    /// reacquire synchronizable objects for multiple times. This class can
    /// only be bounded to one target at a time, bounding to multiple targets
    /// may lead to unexpected behavior. An object of this class can be chained
    /// in a doubly-linked list and be reused in other context.
    class ReentranceMonitor :
            public util::ChainableResource<ReentranceMonitor>,
            public util::ReusableResource<ReentranceMonitor> {
    public:
        /// Create a monitor object.
        explicit ReentranceMonitor();

        /// Acquire a synchronizable target, if the target have already be acquired by this monitor,
        /// the method will return with an increment to the reentrance count; else the monitor will
        /// attempt to acquire the target. The behavior of the locking will be a ordered queue based on
        /// the order of invocation using this method.
        /// \param target The synchronizable target.
        void acquire(Synchronizable *target);

        /// Release a synchronizable target, if the target is not acquired previously by this monitor,
        /// this method will be returned immediately; else the monitor will release the target and notify
        /// the next queued monitor if any. If the reentrance count of this monitor with this target is
        /// greater than 1, this method will return with a decrement to the reentrance count.
        /// \param target The synchronizable target.
        /// \return true if the target was acquired by this monitor including the reentrance case; false if else.
        bool release(Synchronizable *target);

    protected:
        /// For internal use only.
        void reset() override;

        /// For internal use only.
        void reuse(ReentranceMonitor reuse_info) override;

    private:
        /// Indicate the idle status of the monitor, this value will be set when the monitor
        /// attempts to acquire the target (set at the beginning of the process); unset when
        /// the monitor completes the release of the target (when the reentrance count is 0
        /// after then release). true by default.
        bool idle;
        /// Count the number of reentrance for this monitor in acquiring the target, this
        /// value will be incremented if the target to be acquired equals the target acquired.
        /// 0 by default.
        uint64 reentrance_count;
        /// The mutex used in tandem with the condition variable to block & notify a thread.
        std::mutex blocking_mutex;
        /// The condition variable used in tandem with the mutex to block & notify a thread.
        std::condition_variable blocking_cond_var;
        /// To prevent spurious wakeup, which is a commonly known issue of the condition variable, this signal serves
        /// as a condition for the queued monitor to be awaken properly. The queued monitor will be awaken only if this
        /// is set to the release signal.
        CommunicationSignal broadcast_signal;
        /// This signal is used to determine whether the queued monitor is awaken.
        CommunicationSignal receiving_signal;
        /// The address of the acquired target, nullptr by default.
        Synchronizable *owned_synchronizable;

        friend class Synchronizer;
    };

    class Synchronizer {
    public:
        Synchronizer();

        ~Synchronizer();

        void acquire(Synchronizable *target);

        void release(Synchronizable *target);

    private:
        ReentranceMonitor *monitor;
        uint64 nested_level;
        uint64 monitor_count;
    };

}

#endif //VEIL_SYNCHRONIZATION_H
