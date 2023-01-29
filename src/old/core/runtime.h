#ifndef VEIL_RUNTIME_H
#define VEIL_RUNTIME_H

#include "heap.h"

namespace veil {

    class Runtime {
    public:
        static Runtime &get_instance();

        Runtime(Runtime const&) = delete;
        void operator=(Runtime const&) = delete;
    private:
        explicit Runtime();
        ~Runtime();

        veil::Heap heap;
    };

}

#endif //VEIL_RUNTIME_H
