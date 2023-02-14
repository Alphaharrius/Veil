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

#ifndef VEIL_NATIVES_HPP
#define VEIL_NATIVES_HPP

#include "typedefs.hpp"
#include "vm/structures.hpp"

namespace veil::natives {

    uint32 get_page_size();

    template<typename T>
    class NativeAccess {
    public:
        virtual bool access() = 0;

        T get_result();

        uint32 get_error();

    protected:
        T result;
        uint32 error = 0;
    };

    template<typename T>
    T NativeAccess<T>::get_result() {
        return this->result;
    }

    template<typename T>
    uint32 NativeAccess<T>::get_error() {
        return this->error;
    }

    class Mmap : public NativeAccess<uint8 *> {
    public:
        Mmap(void *address, uint64 size, bool readwrite, bool reserve);

        bool access() override;

    private:
        void *address;
        uint64 size;
        bool readwrite, reserve;
    };

}

#endif //VEIL_NATIVES_HPP
