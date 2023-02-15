#ifndef VEIL_ARENA_HPP
#define VEIL_ARENA_HPP

#include "typedefs.hpp"
#include "os.hpp"

namespace veil::memory {

    class Arena {
    public:
        static const uint32 POOL_SIZE = 4096;

        Arena();

    private:
        uint8 *pool;
        Arena *next;
    };

}

#endif //VEIL_ARENA_HPP
