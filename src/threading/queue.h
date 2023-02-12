#ifndef VEIL_QUEUE_H
#define VEIL_QUEUE_H

#include <atomic>
#include <condition_variable>

#include "typedefs.h"
#include "resource.h"

namespace veil::threading {

    class Queuee;

    class Queue {
    private:
        std::atomic<Queuee *> last_queuee =  std::atomic<Queuee *>(nullptr);

        friend class Queuee;
    };

    class Queuee {
    public:
        Queuee();

        void queue(Queue &queue);

        bool exit(Queue &queue);

    private:
        bool idle;
        uint32 reentrance_count;
        Queue *owned;

        std::mutex blocking_m;
        std::condition_variable blocking_cv;

        bool exit_queue;
        bool queuee_notified;
    };

    class QueueClient : private util::Storage<Queuee *> {
    public:
        void wait(Queue &mutex);

        void exit(Queue &mutex);

    private:
        uint32 nested_level;
    };

}

#endif //VEIL_QUEUE_H
