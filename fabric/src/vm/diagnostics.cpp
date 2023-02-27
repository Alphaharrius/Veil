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

#include <iostream>

#include "src/vm/diagnostics.hpp"
#include "src/veil.hpp"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "performance-unnecessary-value-param"

void veil::force_exit_on_error(std::string reason, LineInfo line_info) {
    std::cerr << "A critical error is detected by the runtime environment!" << std::endl
              << "Reason: " << reason << std::endl
              << "At: " << line_info.function_name << "() "
              << line_info.filename << ":" << line_info.line_number << std::endl
              << "Runtime: " << veil::VM_NAME << " version(" << veil::VM_VERSION << ")" << std::endl;
    ::exit(1);
}

void veil::implementation_fault(std::string reason, veil::LineInfo line_info) {
    veil::force_exit_on_error("Implementation fault :: " + reason, line_info);
}

void veil::assertion_error(std::string reason, veil::LineInfo line_info) {
    veil::force_exit_on_error("Assertion error :: " + reason, line_info);
}

#pragma clang diagnostic pop
