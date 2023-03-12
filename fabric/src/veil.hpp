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

#ifndef VEIL_HPP
#define VEIL_HPP

#include <string>

#include "src/typedefs.hpp"

namespace veil {

    struct VMVersion {
        const int32 major;
        const int32 minor;
        const int32 patch;
        const int32 build_no;

        [[nodiscard]] std::string to_string() const;
    };

    std::string VMVersion::to_string() const {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch) + "-b" +
               std::to_string(build_no);
    }

    const std::string VM_NAME = "Veil Fabric (64-bit) Standard Runtime";
    const VMVersion VM_VERSION = {0, 0, 0, 0};

}

#endif //VEIL_HPP
