#include "src/threading/os.hpp"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

#include <windows.h>

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)

#endif

using namespace veil::os;

atomic_u32::atomic_u32(uint32 initial) : embedded(initial) {}

uint32 atomic_u32::load() const {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return InterlockedOr((volatile LONG *) &this->embedded, 0);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#   if defined(__GNUC__) || defined(__GNUG__)
    // Using the GNU implementation available with the GCC compiler:
    // https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins

    // Using __ATOMIC_SEQ_CST makes atomic operation a optimization barrier, and ensures consistency across all threads.
    return __atomic_load_n((volatile uint32 *) &this->embedded, __ATOMIC_SEQ_CST);
#   else
#   error "Atomic operations not supported in the current build environment."
#   endif
#   endif
}

void atomic_u32::store(uint32 value) const {
    // Storing is the same as exchanging with the returned value ignored.
    uint32 _ = this->exchange(value);
}

uint32 atomic_u32::exchange(uint32 value) const {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return InterlockedExchange((volatile LONG *) &this->embedded, (LONG) value);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    #   if defined(__GNUC__) || defined(__GNUG__)
    // Using the GNU implementation available with the GCC compiler:
    // https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins

    // Using __ATOMIC_SEQ_CST makes atomic operation a optimization barrier, and ensures consistency across all threads.
    return __atomic_exchange_n((volatile uint32 *) &this->embedded, value, __ATOMIC_SEQ_CST);
#   else
#   error "Atomic operations not supported in the current build environment."
#   endif
#   endif
}

uint32 atomic_u32::compare_exchange(uint32 compare, uint32 value) const {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return InterlockedCompareExchange((volatile LONG *) &this->embedded, static_cast<int32>(value),
                                      static_cast<int32>(compare));
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    #   if defined(__GNUC__) || defined(__GNUG__)
    // Using the GNU implementation available with the GCC compiler:
    // https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins

    // This value will be exchanged with the value of the atomic value if unmatched; remains the same if matched since
    // both values are the same.
    uint32 expected = compare;
    // Using __ATOMIC_SEQ_CST makes atomic operation a optimization barrier, and ensures consistency across all threads.
    __atomic_compare_exchange_n(
            (volatile uint32 *) &this->embedded, &expected, value, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return expected;
#   else
#   error "Atomic operations not supported in the current build environment."
#   endif
#   endif
}

uint32 atomic_u32::fetch_add(uint32 value) const {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return InterlockedAdd((volatile LONG *) &this->embedded, static_cast<int32>(value));
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    #   if defined(__GNUC__) || defined(__GNUG__)
    // Using the GNU implementation available with the GCC compiler:
    // https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins

    // Using __ATOMIC_SEQ_CST makes atomic operation a optimization barrier, and ensures consistency across all threads.
    return __atomic_add_fetch((volatile uint32 *) &this->embedded, value, __ATOMIC_SEQ_CST);
#   else
#   error "Atomic operations not supported in the current build environment."
#   endif
#   endif
}

uint32 atomic_u32::fetch_sub(uint32 value) const {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return InterlockedExchangeSubtract((volatile uint32 *) &this->embedded, static_cast<uint32>(value));
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    #   if defined(__GNUC__) || defined(__GNUG__)
    // Using the GNU implementation available with the GCC compiler:
    // https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins

    // Using __ATOMIC_SEQ_CST makes atomic operation a optimization barrier, and ensures consistency across all threads.
    return __atomic_add_fetch((volatile uint32 *) &this->embedded, value, __ATOMIC_SEQ_CST);
#   else
#   error "Atomic operations not supported in the current build environment."
#   endif
#   endif
}

uint32 atomic_u32::fetch_or(uint32 value) const {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return InterlockedOr((volatile LONG *) &this->embedded, static_cast<int32>(value));
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    #   if defined(__GNUC__) || defined(__GNUG__)
    // Using the GNU implementation available with the GCC compiler:
    // https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins

    // Using __ATOMIC_SEQ_CST makes atomic operation a optimization barrier, and ensures consistency across all threads.
    return __atomic_add_fetch((volatile uint32 *) &this->embedded, value, __ATOMIC_SEQ_CST);
#   else
#   error "Atomic operations not supported in the current build environment."
#   endif
#   endif
}

uint32 atomic_u32::fetch_xor(uint32 value) const {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return InterlockedXor((volatile LONG *) &this->embedded, static_cast<int32>(value));
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    #   if defined(__GNUC__) || defined(__GNUG__)
    // Using the GNU implementation available with the GCC compiler:
    // https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins

    // Using __ATOMIC_SEQ_CST makes atomic operation a optimization barrier, and ensures consistency across all threads.
    return __atomic_add_fetch((volatile uint32 *) &this->embedded, value, __ATOMIC_SEQ_CST);
#   else
#   error "Atomic operations not supported in the current build environment."
#   endif
#   endif
}

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
    return InterlockedAdd64((volatile LONG64 *) &this->embedded, static_cast<int64>(value));
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
