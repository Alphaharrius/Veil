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

#include "src/threading/management.hpp"
#include "src/core/runtime.hpp"

using namespace veil::threading;

VMThread::VMThread(std::string &name, Management &management) : vm::Constituent<Runtime>(*management.get_root()),
                                                                vm::Constituent<Management>(management),
                                                                vm::HasName(name),
                                                                interrupted(false) {}

void VMThread::start(VMService &service, uint32 &error) {
    service.bind(*this);
    embedded.start(service, error);
}

void VMThread::join(uint32 &error) {
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

VMService::VMService(std::string &name) : HasName(name) {}
