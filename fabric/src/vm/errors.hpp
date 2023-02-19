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

#ifndef VEIL_FABRIC_SRC_VM_ERRORS_HPP
#define VEIL_FABRIC_SRC_VM_ERRORS_HPP

#include "src/typedefs.hpp"

// The error handling approach of the Veil runtime is to use error codes confined to their resultant namespace level,
// each level will map the error code from the previous level to their own error code. This header is used side by side
// with the request & executor structure defined in 'diagnostics.h'.

namespace veil {

    /// Defines no error presence in the current operation.
    static const uint32 ERR_NONE = 0;

}

namespace veil::os {

    static const uint32 ERR_NOMEM = ERR_NONE + 1;

}

namespace veil::memory {

    static const uint32 ERR_HEAP_OVERFLOW = os::ERR_NOMEM + 1;
    static const uint32 ERR_HOST_NOMEM = ERR_HEAP_OVERFLOW + 1;

    static const uint32 ERR_INV_HEAP_SIZE = ERR_HOST_NOMEM + 1;
    static const uint32 ERR_NO_ALGO = ERR_INV_HEAP_SIZE + 1;
    static const uint32 ERR_ALGO_INIT = ERR_NO_ALGO + 1;

}

namespace veil::threading {

    static const uint32 ERR_NO_RES = memory::ERR_NO_ALGO + 1;
    static const uint32 ERR_DEADLOCK = ERR_NO_RES + 1;
    static const uint32 ERR_INV_JOIN = ERR_DEADLOCK + 1;

}

#endif //VEIL_FABRIC_SRC_VM_ERRORS_HPP
