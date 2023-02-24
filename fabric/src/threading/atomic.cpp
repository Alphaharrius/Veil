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
#   if defined(__GNUC__) || defined(__GNUG__)
    // Using the GNU implementation available with the GCC compiler:
    // https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins

    // Using __ATOMIC_SEQ_CST makes atomic operation a optimization barrier, and ensures consistency across all threads.
    return __atomic_load_n((volatile uint64 *) &this->embedded, __ATOMIC_SEQ_CST);
#   else
#   error "Atomic operations not supported in the current build environment."
#   endif
#   endif
}

void atomic_u64::store(uint64 value) const {
    // Storing is the same as exchanging with the returned value ignored.
    uint64 _ = this->exchange(value);
}

uint64 atomic_u64::exchange(uint64 value) const {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return InterlockedExchange64((volatile LONGLONG *) &this->embedded, (LONGLONG) value);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#   if defined(__GNUC__) || defined(__GNUG__)
    // Using the GNU implementation available with the GCC compiler:
    // https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins

    // Using __ATOMIC_SEQ_CST makes atomic operation a optimization barrier, and ensures consistency across all threads.
    return __atomic_exchange_n((volatile uint64 *) &this->embedded, value, __ATOMIC_SEQ_CST);
#   else
#   error "Atomic operations not supported in the current build environment."
#   endif
#   endif
}

uint64 atomic_u64::compare_exchange(uint64 compare, uint64 value) const {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return InterlockedCompareExchange64((volatile LONG64 *) &this->embedded, static_cast<int64>(value),
                                        static_cast<int64>(compare));
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#   if defined(__GNUC__) || defined(__GNUG__)
    // Using the GNU implementation available with the GCC compiler:
    // https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins

    // This value will be exchanged with the value of the atomic value if unmatched; remains the same if matched since
    // both values are the same.
    uint64 expected = compare;
    // Using __ATOMIC_SEQ_CST makes atomic operation a optimization barrier, and ensures consistency across all threads.
    __atomic_compare_exchange_n(
            (volatile uint64 *) &this->embedded, &expected, value, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return expected;
#   else
#   error "Atomic operations not supported in the current build environment."
#   endif
#   endif
}

uint64 atomic_u64::fetch_add(uint64 value) const {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return InterlockedAdd64((volatile LONG64 *) &this->embedded, static_cast<int64>(value));
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#   if defined(__GNUC__) || defined(__GNUG__)
    // Using the GNU implementation available with the GCC compiler:
    // https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins

    // Using __ATOMIC_SEQ_CST makes atomic operation a optimization barrier, and ensures consistency across all threads.
    return __atomic_add_fetch((volatile uint64 *) &this->embedded, value, __ATOMIC_SEQ_CST);
#   else
#   error "Atomic operations not supported in the current build environment."
#   endif
#   endif
}

uint64 atomic_u64::fetch_sub(uint64 value) const {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return InterlockedExchangeSubtract((volatile unsigned long long *) &this->embedded, static_cast<int64>(value));
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#   if defined(__GNUC__) || defined(__GNUG__)
    // Using the GNU implementation available with the GCC compiler:
    // https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins

    // Using __ATOMIC_SEQ_CST makes atomic operation a optimization barrier, and ensures consistency across all threads.
    return __atomic_sub_fetch((volatile uint64 *) &this->embedded, value, __ATOMIC_SEQ_CST);
#   else
#   error "Atomic operations not supported in the current build environment."
#   endif
#   endif
}

uint64 atomic_u64::fetch_or(uint64 value) const {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return InterlockedOr64((volatile LONGLONG *) &this->embedded, static_cast<int64>(value));
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#   if defined(__GNUC__) || defined(__GNUG__)
    // Using the GNU implementation available with the GCC compiler:
    // https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins

    // Using __ATOMIC_SEQ_CST makes atomic operation a optimization barrier, and ensures consistency across all threads.
    return __atomic_or_fetch((volatile uint64 *) &this->embedded, value, __ATOMIC_SEQ_CST);
#   else
#   error "Atomic operations not supported in the current build environment."
#   endif
#   endif
}

uint64 atomic_u64::fetch_xor(uint64 value) const {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return InterlockedXor64((volatile LONG64 *) &this->embedded, static_cast<int64>(value));
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#   if defined(__GNUC__) || defined(__GNUG__)
    // Using the GNU implementation available with the GCC compiler:
    // https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins

    // Using __ATOMIC_SEQ_CST makes atomic operation a optimization barrier, and ensures consistency across all threads.
    return __atomic_xor_fetch((volatile uint64 *) &this->embedded, value, __ATOMIC_SEQ_CST);
#   else
#   error "Atomic operations not supported in the current build environment."
#   endif
#   endif
}

atomic_bool::atomic_bool(bool initial) : embedded(initial ? 1 : 0) {}

bool atomic_bool::load() const {
    return embedded.load() != 0;
}

void atomic_bool::store(bool value) const {
    return embedded.store(value ? 1 : 0);
}
