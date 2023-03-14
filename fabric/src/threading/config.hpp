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

#ifndef VEIL_FABRIC_SRC_THREADING_CONFIG_HPP
#define VEIL_FABRIC_SRC_THREADING_CONFIG_HPP

#include "src/typedefs.hpp"

/// Global configuration for threading modules, all attributes defined in this namespace should be fixed after the
/// instantiation of the VM.
namespace veil::threading::config {

    /// A global configuration used by all thread synchronization primitives to control the maximum spin count before
    /// performing a wait operation on the target object.
    static uint32 mutex_spin_count = 32;

    static uint32 pause_request_wait_milliseconds = 60000;

}

#endif //VEIL_FABRIC_SRC_THREADING_CONFIG_HPP
