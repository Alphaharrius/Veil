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

#include "src/util/conversions.hpp"
#include "src/vm/diagnostics.hpp"

int8 veil::util::to_signed(uint8 val) {
    static const uint8 MAX_INT8 = (1U << 7) - 1;
    VeilAssert(val <= MAX_INT8, "Integer conversion overflow.");
    return (int8) val;
}

uint8 veil::util::to_unsigned(int8 val) {
    VeilAssert(val >= 0, "Negative value cannot be converted.");
    return (uint8) val;
}

int16 veil::util::to_signed(uint16 val) {
    static const uint16 MAX_INT16 = (1U << 15) - 1;
    VeilAssert(val <= MAX_INT16, "Integer conversion overflow.");
    return (int16) val;
}

uint16 veil::util::to_unsigned(int16 val) {
    VeilAssert(val >= 0, "Negative value cannot be converted.");
    return (uint16) val;
}

int32 veil::util::to_signed(uint32 val) {
    static const uint32 MAX_INT32 = (1UL << 31) - 1;
    VeilAssert(val <= MAX_INT32, "Integer conversion overflow.");
    return (int32) val;
}

uint32 veil::util::to_unsigned(int32 val) {
    VeilAssert(val >= 0, "Negative value cannot be converted.");
    return (uint32) val;
}

int64 veil::util::to_signed(uint64 val) {
    static const uint64 MAX_INT64 = (1ULL << 63) - 1;
    VeilAssert(val <= MAX_INT64, "Integer conversion overflow.");
    return (int64) val;
}

uint64 veil::util::to_unsigned(int64 val) {
    VeilAssert(val >= 0, "Negative value cannot be converted.");
    return (uint64) val;
}


