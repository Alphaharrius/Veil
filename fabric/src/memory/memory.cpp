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

#include <cassert>

#include "src/memory/memory.hpp"
#include "src/memory/os.hpp"

using namespace veil::memory;

void *HeapObject::operator new(uint64 size) {
    return os::malloc(size);
}

void HeapObject::operator delete(void *address) {
    os::free(address);
}

Region::Region(uint32 pool_size) : pool_size(pool_size), next(nullptr) {
    this->pool = static_cast<uint8 *>(os::malloc(pool_size));
    // The bump address will be at the start of the pool address.
    this->bump = this->pool;
}

void *Region::allocate(uint32 size) {
    if (this->bump + size >= this->pool + this->pool_size) {
        return nullptr;
    }

    void *alloc = this->bump;
    this->bump += size;
    return alloc;
}

Region::~Region() { veil::os::free(this->pool); }

Arena::Arena(uint32 pool_size) : pool_size(pool_size), base(new Region(pool_size)) {}

void *Arena::allocate(uint32 size) {
    void *address = base->allocate(size);
    if (!address) {
        address = this->inflate(size);
    }
    // The address from inflation will not be nullptr.
    return address;
}

void *Arena::inflate(uint32 init_offset) {
    auto *inflated = new Region(this->pool_size);
    inflated->next = this->base;
    this->base = inflated;

    return inflated->allocate(init_offset);
}

void Arena::free() {
    Region *target = this->base;
    while (target) {
        Region *next = target->next;
        delete target;
        target = next;
    }
}

Arena::Iterator::Iterator(Arena &arena) : target(arena.base), offset(0) {}

void *Arena::Iterator::next(uint64 step) {
    while (this->target) {
        uint8 *offed = this->target->pool + this->offset;
        if (offed + step <= target->bump) {
            this->offset += step;
            return offed;
        }
        else if (this->target->next) {
            this->target = target->next;
            this->offset = 0;
        } else break;
    }
    return nullptr;
}
