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

#ifndef VEIL_OS_HPP
#define VEIL_OS_HPP

#include <cstdlib>

#include "typedefs.hpp"
#include "vm/structures.hpp"

namespace veil::os {

    void *malloc(uint64 size);

    void free(void *address);

    uint32 get_page_size();

    void *mmap(void *address, uint64 size, bool readwrite, bool reserve, uint32 &error);

    void force_exit_on_error(
            const std::string& reason, std::string& filename, std::string& function_name, uint32 line_number);

}

#endif //VEIL_OS_HPP
