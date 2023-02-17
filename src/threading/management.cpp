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

#include "threading/management.hpp"
#include "core/runtime.hpp"

using namespace veil::threading;

VMThread::VMThread(std::string &name, Runtime &runtime) : vm::Constituent<Runtime>(runtime),
                                                          vm::HasName(name),
                                                          interrupted(false) {}

void VMThread::start() {
    // Register a VM thread when it starts, so the threading management can manage its life cycle.
    vm::Constituent<Runtime>::root->vm::Composite<threading::Management>::get_composition()->register_thread(*this);
    embedded.start(*this);
}

void VMThread::join() {
    embedded.join();
}

void VMThread::interrupt() {
    interrupted.store(true);
}

bool VMThread::is_interrupted() {
    return interrupted.load();
}

void Management::register_thread(VMThread &thread) {
    // TODO: Requires thread safety.
    VMThread **address = this->allocate();
    *address = &thread;
}
