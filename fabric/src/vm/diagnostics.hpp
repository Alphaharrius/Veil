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

#ifndef VEIL_FABRIC_SRC_VM_DIAGNOSTICS_HPP
#define VEIL_FABRIC_SRC_VM_DIAGNOSTICS_HPP

#include <cstdlib>
#include <cassert>
#include <string>

#include "src/typedefs.hpp"
#include "src/vm/diagnostics.hpp"

namespace veil {

    struct LineInfo {
    public:
        std::string filename;
        std::string function_name;
        int line_number;

        LineInfo(const char *filename, const char *function_name, int line_number) : filename(filename),
                                                                                     function_name(function_name),
                                                                                     line_number(line_number) {}
    };

    void force_exit_on_error(std::string reason, LineInfo line_info);

    void implementation_fault(std::string reason, LineInfo line_info);

    void assertion_error(std::string reason, LineInfo line_info);

}

#define VeilGetLineInfo veil::LineInfo(__FILE__, __func__, __LINE__)

#if defined(VEIL_ENABLE_DEBUG)
#define VeilAssert(expression, message) { if (!(expression)) veil::assertion_error(message, VeilGetLineInfo); }
#else
#define VeilAssert(expression, message) {}
#endif

#endif //VEIL_FABRIC_SRC_VM_DIAGNOSTICS_HPP
