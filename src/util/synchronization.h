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

    enum CommunicationSignal {NONE, SIGNAL_RELEASE, REPLY_RELEASED};

    struct Synchronizable {
        std::atomic<ReentranceMonitor *> last_queued = std::atomic<ReentranceMonitor *>(nullptr);
    };

    class ReentranceMonitor:
            public util::ChainableResource<ReentranceMonitor>,
            public util::ReusableResource<ReentranceMonitor> {
    public:
        explicit ReentranceMonitor();
        void acquire(Synchronizable *target);
        bool release(Synchronizable *target);
        void reset() override;
        void reuse(ReentranceMonitor reuse_info) override;
    private:
        bool idle;
        uint64 reentrance_count;
        std::mutex blocking_mutex;
        CommunicationSignal broadcast_signal;
        CommunicationSignal receiving_signal;
        Synchronizable *owned_synchronizable;
        std::condition_variable blocking_cond_var;

        friend Synchronizer;
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
