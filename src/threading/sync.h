#ifndef VEIL_SYNC_H
#define VEIL_SYNC_H

#include <atomic>
#include <condition_variable>

#include "typedefs.h"

namespace veil::threading {

    class MutexAccessor;

    class MutexHolder {
    private:
        std::atomic<MutexAccessor *> top_queued =  std::atomic<MutexAccessor *>(nullptr);

        friend class MutexAccessor;
    };

    class MutexAccessor {
    private:
        bool idle;
        uint32 reentrance_count;
        std::mutex blocking_m;
        std::condition_variable blocking_cv;
        MutexAccessor *owned;
    };

}

#endif //VEIL_SYNC_H
