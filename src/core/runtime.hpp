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

#ifndef VEIL_SRC_CORE_RUNTIME_HPP
#define VEIL_SRC_CORE_RUNTIME_HPP

#include "typedefs.hpp"
#include "vm/structures.hpp"
#include "core/runtime.forward.hpp"
#include "memory/memory.hpp"
#include "memory/management.hpp"
#include "threading/management.hpp"

namespace veil {

    class Runtime :
            public memory::ValueObject,
            public vm::Composite<memory::Management>,
            public vm::Composite<threading::Management> {
    private:
        Runtime(memory::Management &memory_management, threading::Management &threading_management);
    };

}

#endif //VEIL_SRC_CORE_RUNTIME_HPP
