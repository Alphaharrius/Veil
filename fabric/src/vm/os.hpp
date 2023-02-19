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

#ifndef VEIL_FABRIC_SRC_VM_OS_HPP
#define VEIL_FABRIC_SRC_VM_OS_HPP

#include <cstdlib>

#include "src/typedefs.hpp"

namespace veil::os {

    void force_exit_on_error(
            const char *reason, const char *filename, const char *function_name, uint32 line_number);

#   define VeilForceExitOnError(reason) veil::os::force_exit_on_error(reason, __FILE__, __func__, __LINE__)

}

#endif //VEIL_FABRIC_SRC_VM_OS_HPP
