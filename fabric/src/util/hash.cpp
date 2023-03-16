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

#include "src/util/hash.hpp"

using namespace veil::util;

inline uint64 binary_rotate(uint64 &target, uint32 offset) {
    return (target << offset) | (target >> (sizeof(uint64) * 8 - offset));
}

uint64 veil::util::standard_u64_hash_function(uint8 *input_data, uint32 input_length) {
    static const uint32 HASH_SEED_LENGTH = 7;

    uint64 hash_seeds[HASH_SEED_LENGTH] = {
            0xEB2DF331CD23AC43,
            0xCD23AC43BB4681C3,
            0xBB4681C3B54FCBAF,
            0xB54FCBAFDF08ED0B,
            0xDF08ED0BD913138F,
            0xD913138FE4FECE2D,
            0xE4FECE2DEB2DF331
    };

    uint64 hash_output = hash_seeds[input_length % HASH_SEED_LENGTH];

    uint8 *current_byte = input_data;
    uint32 accumulated_sum = 0;
    for (uint32 digest_index = 0; digest_index < input_length; digest_index++) {
        hash_output *= *current_byte;
        hash_output = binary_rotate(hash_output, 37);

        uint32 selected_seed_index = *current_byte % HASH_SEED_LENGTH;
        hash_output += hash_seeds[selected_seed_index];
        hash_output = binary_rotate(hash_output, 17);

        accumulated_sum += *current_byte;
        selected_seed_index = accumulated_sum % HASH_SEED_LENGTH;
        hash_output *= hash_seeds[selected_seed_index];
    }

    hash_output = binary_rotate(hash_output, 41);
    hash_output *= hash_seeds[accumulated_sum % HASH_SEED_LENGTH];
    hash_output = binary_rotate(hash_output, 23);
    hash_output += hash_seeds[accumulated_sum % HASH_SEED_LENGTH];
    hash_output = binary_rotate(hash_output, 17);
    hash_output *= hash_seeds[accumulated_sum % HASH_SEED_LENGTH];

    return hash_output;
}

uint64 veil::util::standard_u64_hash_function(uint64 input_data) {
    return util::standard_u64_hash_function((uint8 *) &input_data, sizeof(input_data));
}

uint64 veil::util::standard_u64_hash_function(const void *pointer) {
    return util::standard_u64_hash_function((uint64) pointer);
}
