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

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

#include <windows.h>

#if defined(_MSC_VER)
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#endif

#elif defined(__linux__) || defined(__linux) || defined(linux)

#include <execinfo.h>

#endif

#include <iostream>

#include "src/vm/diagnostics.hpp"
#include "src/veil.hpp"
#include "src/threading/os.hpp"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "performance-unnecessary-value-param"

void veil::force_exit_on_error(std::string reason, LineInfo line_info) {
    static os::Mutex log_error_m;
    {
        os::CriticalSection _(log_error_m);
        std::cerr << "===\n"
                  << "A critical error is detected by the runtime environment!\n"
                  << "Reason: " << reason << "\n"
                  << "At: " << line_info.function_name << "() "
                  << line_info.filename << ":" << line_info.line_number << "\n"
                  << "Runtime: " << veil::VM_NAME << "\n"
                  << "Version: " << veil::VM_VERSION.to_string() << "\n";
        veil::print_callstack_trace();
    }
    ::exit(1);
}

void veil::implementation_fault(std::string reason, veil::LineInfo line_info) {
    veil::force_exit_on_error("Implementation fault :: " + reason, line_info);
}

void veil::assertion_error(std::string reason, veil::LineInfo line_info) {
    veil::force_exit_on_error("Assertion error :: " + reason, line_info);
}

// @formatter:off
void veil::print_callstack_trace() {
    static const int32 MAX_TRACE_COUNT = 32;
    static os::Mutex print_callstack_trace_m;
    os::CriticalSection _(print_callstack_trace_m);

    void *stack[MAX_TRACE_COUNT];

    std::cerr << "===\n"
              << "Callstack trace of thread(" << os::Thread::current_thread_id() << "):\n";

#   if (defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)) and defined(_MSC_VER)
    SymInitialize(GetCurrentProcess(), nullptr, true);

    CONTEXT context;
    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&context);

    WORD frames = CaptureStackBackTrace(0, 100, stack, nullptr);

    for (int i = 0; i < frames; ++i) {
        auto address = (DWORD64) (stack[i]);
        DWORD64 displacement = 0;
        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        auto symbol = (PSYMBOL_INFO) buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;
        if (SymFromAddr(GetCurrentProcess(), address, &displacement, symbol))
            std::cerr << "\t[" << i << "] " << symbol->Name << "()\n";
    }

    SymCleanup(GetCurrentProcess());
#   else
    std::cerr << "\tThe current platform does not support stack tracing...\n";
#   endif
    std::cerr << "===\n";
}
// @formatter:on

#pragma clang diagnostic pop
