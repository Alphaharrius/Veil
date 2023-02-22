#include "src/threading/os.hpp"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

#include <windows.h>

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)

#endif

using namespace veil::os;

atomic_u64::atomic_u64(uint64 initial) : embedded(initial) {}

uint64 atomic_u64::load() const {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return InterlockedOr64((volatile LONG64 *) &this->embedded, 0ULL);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#   endif
}

void atomic_u64::store(uint64 value) const {
    uint64 _ = this->exchange(value);
}

uint64 atomic_u64::exchange(uint64 value) const {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return InterlockedExchange64((volatile LONGLONG *) &this->embedded, (LONGLONG) value);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#   endif
}

uint64 atomic_u64::compare_exchange(uint64 compare, uint64 value) const {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return InterlockedCompareExchange64((volatile LONG64 *) &this->embedded, static_cast<int64>(value),
                                        static_cast<int64>(compare));
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#   endif
}

uint64 atomic_u64::fetch_add(uint64 value) const {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return InterlockedAddAcquire64((volatile LONG64 *) &this->embedded, static_cast<int64>(value));
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#   endif
}

uint64 atomic_u64::fetch_sub(uint64 value) const {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return InterlockedExchangeSubtract((volatile unsigned long long *) &this->embedded, static_cast<int64>(value));
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#   endif
}

uint64 atomic_u64::fetch_or(uint64 value) const {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return InterlockedOr64((volatile LONGLONG *) &this->embedded, static_cast<int64>(value));
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#   endif
}

uint64 atomic_u64::fetch_xor(uint64 value) const {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return InterlockedXor64((volatile LONG64 *) &this->embedded, static_cast<int64>(value));
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#   endif
}
