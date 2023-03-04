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

#ifndef VEIL_FABRIC_SRC_THREADING_ATOMIC_HPP
#define VEIL_FABRIC_SRC_THREADING_ATOMIC_HPP

#include "src/typedefs.hpp"

namespace veil::os {

    struct atomic_u32_t {
    public:
        explicit atomic_u32_t(uint32 initial);

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

    struct atomic_u64_t {
    public:
        explicit atomic_u64_t(uint64 initial);

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

    struct atomic_bool_t {
    public:
        explicit atomic_bool_t(bool initial);

        [[nodiscard]] bool load() const;

        void store(bool value) const;

        [[nodiscard]] bool exchange(bool value) const;

    private:
        atomic_u32_t embedded;
    };

    template<typename T>
    struct atomic_pointer_t {
    public:
        explicit atomic_pointer_t(T *initial);

        [[nodiscard]] T *load() const;

        void store(T *value) const;

        [[nodiscard]] T *exchange(T *value) const;

        [[nodiscard]] T *compare_exchange(T *compare, T *value) const;

    private:
        atomic_u64_t embedded;
    };

    template<typename T>
    atomic_pointer_t<T>::atomic_pointer_t(T *initial) : embedded(reinterpret_cast<uint64>(initial)) {}

    template<typename T>
    T *atomic_pointer_t<T>::load() const {
        return reinterpret_cast<T *>(embedded.load());
    }

    template<typename T>
    void atomic_pointer_t<T>::store(T *value) const {
        embedded.store(reinterpret_cast<uint64>(value));
    }

    template<typename T>
    T *atomic_pointer_t<T>::exchange(T *value) const {
        return reinterpret_cast<T *>(embedded.exchange(reinterpret_cast<uint64>(value)));
    }

    template<typename T>
    T *atomic_pointer_t<T>::compare_exchange(T *compare, T *value) const {
        return reinterpret_cast<T *>(
                embedded.compare_exchange(reinterpret_cast<uint64>(compare), reinterpret_cast<uint64>(value)));
    }

}

#endif //VEIL_FABRIC_SRC_THREADING_ATOMIC_HPP
