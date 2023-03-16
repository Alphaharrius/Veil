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

#ifndef VEIL_FABRIC_SRC_UTIL_HASH_HPP
#define VEIL_FABRIC_SRC_UTIL_HASH_HPP

#include "src/typedefs.hpp"

namespace veil::util {

    uint64 standard_u64_hash_function(uint8 *input_data, uint32 input_length);

    uint64 standard_u64_hash_function(uint64 input_data);

    uint64 standard_u64_hash_function(const void *pointer);

}

#endif //VEIL_FABRIC_SRC_UTIL_HASH_HPP
