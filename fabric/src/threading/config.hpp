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

    /// The maximum time in milliseconds that a requesting thread of a pause action (which will be the thread scheduler)
    /// will wait before throwing an <b>abort</b> signal to terminate the VM. We assumed that all pause actions will be
    /// called after waking the target thread from its sleep, and <code>os::Thread::static_sleep(uint32)</code> will not
    /// be used in long periods that exceeds this value, any thread that fail to respond within this period will be
    /// considered to be in a deadlock state, thus the abort is justified.
    static uint32 pause_request_wait_milliseconds = 60000; // 1 minute by default.

}

#endif //VEIL_FABRIC_SRC_THREADING_CONFIG_HPP
