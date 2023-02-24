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

#ifndef VEIL_FABRIC_SRC_THREADING_OS_HPP
#define VEIL_FABRIC_SRC_THREADING_OS_HPP

#include "src/memory/global.hpp"
#include "src/vm/structures.hpp"

namespace veil::os {

    class Thread : public memory::ValueObject {
    public:
        enum Status {
            Idle, Started, Joined
        };

        static void static_sleep(uint32 milliseconds);

        Thread();

        ~Thread();

        void start(vm::Executable &callable, uint32 &error);

        void join(uint32 &error);

        Status get_status();

    private:
        void *os_thread;
        Status status;
    };

    class Mutex : public memory::ValueObject {
    public:
        Mutex();

        ~Mutex();

        void lock();

        void unlock();

    private:
        void *os_mutex;

        friend class ConditionVariable;
    };

    class ConditionVariable : public memory::ValueObject {
    public:
        ConditionVariable();

        ~ConditionVariable();

        void wait();

        bool wait_for(int32 milliseconds);

        void notify();

        void notify_all();

    private:
        Mutex associate;
        void *os_cv;
    };

    struct atomic_u32 {
    public:
        explicit atomic_u32(uint32 initial);

        [[nodiscard]] uint32 load() const;

        void store(uint32 value) const;

        [[nodiscard]] uint32 exchange(uint32 value) const;

        [[nodiscard]] uint32 compare_exchange(uint32 compare, uint32 value) const;

        [[nodiscard]] uint32 fetch_add(uint32 value) const;

        [[nodiscard]] uint32 fetch_sub(uint32 value) const;

        [[nodiscard]] uint32 fetch_or(uint32 value) const;

        [[nodiscard]] uint32 fetch_xor(uint32 value) const;

    private:
        uint32 embedded;
    };

    struct atomic_u64 {
    public:
        explicit atomic_u64(uint64 initial);

        [[nodiscard]] uint64 load() const;

        void store(uint64 value) const;

        [[nodiscard]] uint64 exchange(uint64 value) const;

        [[nodiscard]] uint64 compare_exchange(uint64 compare, uint64 value) const;

        [[nodiscard]] uint64 fetch_add(uint64 value) const;

        [[nodiscard]] uint64 fetch_sub(uint64 value) const;

        [[nodiscard]] uint64 fetch_or(uint64 value) const;

        [[nodiscard]] uint64 fetch_xor(uint64 value) const;

    private:
        volatile uint64 embedded;
    };

    struct atomic_bool {
    public:
        explicit atomic_bool(bool initial);

        [[nodiscard]] bool load() const;

        void store(bool value) const;

    private:
        atomic_u32 embedded;
    };

    template<typename T>
    struct atomic_ptr {
    public:
        explicit atomic_ptr(T *initial);

        [[nodiscard]] T *load() const;

        void store(T *value) const;

        [[nodiscard]] T *exchange(T *value) const;

        [[nodiscard]] T *compare_exchange(T *compare, T *value) const;

    private:
        atomic_u64 embedded;
    };

    template<typename T>
    atomic_ptr<T>::atomic_ptr(T *initial) : embedded(reinterpret_cast<uint64>(initial)) {}

    template<typename T>
    T *atomic_ptr<T>::load() const {
        return reinterpret_cast<T *>(embedded.load());
    }

    template<typename T>
    void atomic_ptr<T>::store(T *value) const {
        embedded.store(reinterpret_cast<uint64>(value));
    }

    template<typename T>
    T *atomic_ptr<T>::exchange(T *value) const {
        return reinterpret_cast<T *>(embedded.exchange(reinterpret_cast<uint64>(value)));
    }

    template<typename T>
    T *atomic_ptr<T>::compare_exchange(T *compare, T *value) const {
        return reinterpret_cast<T *>(
                embedded.compare_exchange(reinterpret_cast<uint64>(compare), reinterpret_cast<uint64>(value)));
    }

}

#endif //VEIL_FABRIC_SRC_THREADING_OS_HPP
