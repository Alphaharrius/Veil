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
        static const uint32 MAX_SPIN_COUNT = 128;

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
