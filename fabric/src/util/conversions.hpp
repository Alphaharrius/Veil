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

#ifndef VEIL_FABRIC_SRC_UTIL_CONVERSIONS_HPP
#define VEIL_FABRIC_SRC_UTIL_CONVERSIONS_HPP

#include "src/typedefs.hpp"

namespace veil::util {

    int8 to_signed(uint8 val);

    uint8 to_unsigned(int8 val);

    int16 to_signed(uint16 val);

    uint16 to_unsigned(int16 val);

    int32 to_signed(uint32 val);

    uint32 to_unsigned(int32 val);

    int64 to_signed(uint64 val);

    uint64 to_unsigned(int64 val);

};

#endif //VEIL_FABRIC_SRC_UTIL_CONVERSIONS_HPP
